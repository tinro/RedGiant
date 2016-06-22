#include "index/document_index_manager.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <utility>
#include <vector>

#include "core/reader/wand_reader.h"
#include "core/reader/wand_reader-inl.h"
#include "data/document.h"
#include "data/document_id.h"
#include "data/query_request.h"
#include "index/document_query.h"
#include "utils/logger.h"
#include "utils/stop_watch.h"

namespace redgiant {

DECLARE_LOGGER(logger, __FILE__);

// TODO: remove this
// for debug
static std::string readers_to_string (const std::vector<DocumentIndexManager::ReaderPair>& readers) {
  std::ostringstream os;
  size_t i = 0;
  for (auto& r: readers) {
    if (i > 0) {
      os << ", ";
    }
    os  << "(0x" << std::hex << std::setw(16) << std::setfill('0') <<  r.first
        << "," << std::dec << r.second->size() << ")";
    ++i;
  }
  return os.str();
}

const std::string DocumentIndexManager::kIndexFileNamePrefix = "doc_";

DocumentIndexManager::DocumentIndexManager(size_t doc_initial_buckets, size_t doc_max_size)
: index_(doc_initial_buckets, doc_max_size) {
}

DocumentIndexManager::DocumentIndexManager(size_t doc_initial_buckets, size_t doc_max_size,
    const std::string& snapshot_prefix)
: index_(doc_initial_buckets, doc_max_size, snapshot_prefix + kIndexFileNamePrefix + "0") {
}

int DocumentIndexManager::remove(DocId doc_id) {
  return index_.remove(doc_id);
}

int DocumentIndexManager::batch_remove(const std::vector<DocId> doc_ids) {
  return index_.batch_remove(doc_ids);
}

int DocumentIndexManager::update(std::shared_ptr<Document> doc, time_t expire_time) {
  StopWatch watch;
  int ret = 0;
  DocTerms weights;
  // loop in all feature vectors
  for (const auto& feature_vector: doc->get_feature_vectors()) {
    // loop in features in vectors
    for (const auto& feature_pair: feature_vector.get_features()) {
      weights.emplace_back(feature_pair.first->get_id(), feature_pair.second);
    }
  }
  return index_.update(doc->get_id(), weights, expire_time);
}

int DocumentIndexManager::batch_update(const std::vector<std::shared_ptr<Document>>& docs, time_t expire_time) {
  StopWatch watch;
  std::vector<RowTuple> update_docs;
  for (const auto& doc: docs) {
    DocTerms weights;
    // loop in all feature vectors
    for (const auto& feature_vector: doc->get_feature_vectors()) {
      // loop in features in vectors
      for (const auto& feature_pair: feature_vector.get_features()) {
        weights.emplace_back(feature_pair.first->get_id(), feature_pair.second);
      }
    }
    update_docs.emplace_back(doc->get_id(), std::move(weights), expire_time);
  }
  return index_.batch_update(update_docs);
}

auto DocumentIndexManager::peek_term(TermId term_id) const
-> std::unique_ptr<RawReader> {
  return index_.peek(term_id);
}

//auto DocumentIndexManager::peek_doc(DocId doc_id) const;
//-> std::shared_ptr<Document> {
//  return index_.peek(term_id);
//}

auto DocumentIndexManager::query(const QueryRequest& request, const DocumentQuery& query) const
-> std::unique_ptr<Reader> {
  StopWatch watch;

  size_t query_size = query.get_doc_queries().size();
  if (query_size == 0) {
    // do not let it be empty
    return nullptr;
  }

  std::vector<ReaderPair> readers = index_.batch_query(query.get_doc_queries());

  if (request.is_debug()) {
    LOG_INFO(logger, "[query:%s] document query %zu terms, found %zu terms.",
        request.get_request_id().c_str(), query.get_doc_queries().size(), readers.size());
    LOG_TRACE(logger, "[query:%s] document query found terms:%s",
        request.get_request_id().c_str(), readers_to_string(readers).c_str());
  }

  if (readers.size() == 0) {
    return nullptr;
  }
  if (readers.size() == 1) {
    return std::move(readers[0].second);
  }

  // TODO: simplify this
  std::vector<std::unique_ptr<Reader>> simple_readers;
  simple_readers.reserve(readers.size());
  for (auto& reader: readers) {
    simple_readers.push_back(std::move(reader.second));
  }
  return std::make_unique<WandReader<DocId, Score>>(std::move(simple_readers));
}

int DocumentIndexManager::dump(const std::string& snapshot_prefix) {
  LOG_INFO(logger, "start dumping document index to snapshot %s", snapshot_prefix.c_str());
  try {
    std::string file_name = snapshot_prefix + kIndexFileNamePrefix + "0";
    size_t ret = index_.dump(file_name);
    LOG_INFO(logger, "dump completed. dump size:%zu, file name:%s", ret, file_name.c_str());
  } catch (std::ios_base::failure& e) {
    LOG_ERROR(logger, "document index dump failed. reason:%s", e.what());
    return -1;
  }
  return 0;
}

int DocumentIndexManager::do_maintain(time_t time) {
  StopWatch watch;
  int32_t expire_time = time;
  LOG_INFO(logger, "start maintaining document index. expire_time=%d", expire_time);

  int ret = 0;
  int expired = 0;
  std::pair<int, int> doc_apply_ret = index_.apply(expire_time);
  if (doc_apply_ret.first >= 0) {
    LOG_DEBUG(logger, "maintained %d items in doc feature index, %d expired.",
        doc_apply_ret.first, doc_apply_ret.second);
    ret += doc_apply_ret.first;
    expired += doc_apply_ret.second;
  }

  LOG_INFO(logger, "document index maintain finished. total maintained=%d, expired=%d.", ret, expired);
  return ret;
}

} /* namespace redgiant */
