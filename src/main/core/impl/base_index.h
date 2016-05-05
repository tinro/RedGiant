#ifndef SRC_MAIN_CORE_IMPL_BASE_INDEX_H_
#define SRC_MAIN_CORE_IMPL_BASE_INDEX_H_

#include <map>
#include <memory>
#include <mutex>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include "core/index/posting_list.h"
#include "core/query/posting_list_query.h"
#include "core/reader/posting_list_reader.h"
#include "third_party/lock/shared_mutex.h"

namespace redgiant {
template <typename DocTraits>
class BaseIndex {
public:
  typedef typename DocTraits::DocId DocId;
  typedef typename DocTraits::TermId TermId;
  typedef typename DocTraits::TermWeight TermWeight;
  typedef typename DocTraits::ExpireTime ExpireTime;
  typedef typename DocTraits::DocIdHash DocIdHash;
  typedef typename DocTraits::TermIdHash TermIdHash;
  typedef PostingListReader<DocId, const TermWeight&> RawReader;

  template <typename Score>
  using Query = PostingListQuery<DocId, Score, const TermWeight&>;
  template <typename Score>
  using QueryPair = std::pair<TermId, std::unique_ptr<Query<Score>>>;
  template <typename Score>
  using Reader = PostingListReader<DocId, Score>;
  template <typename Score>
  using ReaderPair = std::pair<TermId, std::unique_ptr<Reader<Score>>>;
  template <typename Score>
  using Results = std::vector<std::pair<DocId, Score>>;

  BaseIndex(size_t initial_buckets);

  // create from snapshot
  // may throw exception: std::ios_base::failure
  // Loader&& accepts both lvalues and rvalues (for temporarily constructed loader)
  template <typename Loader>
  BaseIndex(size_t initial_buckets, Loader&& loader);

  // g++ has bug with =default destructor for extern declared templates.
  ~BaseIndex() { }

  size_t get_term_count() const;

  size_t get_bucket_count() const;

  float get_load_factor() const;

  std::unique_ptr<RawReader> peek(TermId term_id) const;

  template <typename Score>
  std::unique_ptr<Reader<Score>> query(TermId term_id, const Query<Score>& query) const;

  template <typename Score>
  int batch_query(const std::vector<QueryPair<Score>>& queries, std::vector<ReaderPair<Score>>* readers) const;

protected:
  typedef PostingList<DocId, TermWeight> PList;
  typedef PostingListFactory<DocId, TermWeight> PListFactory;

  template <typename Changeset>
  int create_update_internal(DocId doc_id, TermId term_id, const TermWeight& weights, Changeset& changeset);

  template <typename Changeset>
  int remove_internal(DocId doc_id, TermId term_id, Changeset& changeset);

  template <typename Changeset>
  int remove_internal(DocId doc_id, std::vector<TermId> terms, Changeset& changeset);

  template <typename Changeset>
  int apply_internal(Changeset& changeset);

  // dump to snapshot
  // may throw exception: std::ios_base::failure
  // Dumper&& accepts both lvalues and rvalues (for temporarily constructed loader)
  template <typename Dumper>
  size_t dump_internal(Dumper&& dumper);

protected:
  std::unordered_map<TermId, std::shared_ptr<PList>> index_;
  std::unique_ptr<PListFactory> safe_factory_;
  mutable shared_mutex query_mutex_;
};

} /* namespace redgiant */

#endif /* SRC_MAIN_CORE_IMPL_BASE_INDEX_H_ */
