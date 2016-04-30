#ifndef SRC_MAIN_CORE_INDEX_MAP_POSTING_LIST_H_
#define SRC_MAIN_CORE_INDEX_MAP_POSTING_LIST_H_

#include <map>
#include <memory>
#include <utility>
#include "core/index/posting_list.h"
#include "core/reader/algorithms.h"
#include "core/reader/posting_list_reader.h"
#include "core/reader/reader_utils.h"

namespace redgiant {
template <typename DocId, typename Weight, typename WeightMerger = MaxWeight<Weight>>
class MapPostingList: public PostingList<DocId, Weight> {
public:
  typedef PostingList<DocId, Weight> Base;
  typedef typename Base::PList PList;
  typedef typename Base::Reader Reader;
  typedef std::pair<DocId, Weight> PostingPair;
  typedef std::map<DocId, Weight> PostingMap;

  explicit MapPostingList(WeightMerger merger = WeightMerger())
  : upper_bound_(), merger_(std::move(merger)) {
    merger_(upper_bound_); // initialize
  }

  template <typename InputWeight>
  MapPostingList(PostingListReader<DocId, InputWeight>& reader, WeightMerger merger = WeightMerger())
  : upper_bound_(), merger_(std::move(merger)) {
    merger_(upper_bound_); // initialize
    for (DocId doc_id = reader.next(DocId()); !!doc_id; doc_id = reader.next(doc_id)) {
      Weight weight = reader.read();
      merger_(upper_bound_, weight);
      posting_.insert(posting_.end(), std::make_pair(doc_id, std::move(weight)));
    }
  }

  virtual ~MapPostingList() = default;

  virtual bool empty() const {
    return posting_.empty();
  }

  virtual int update(DocId doc_id, const Weight& weight) {
    if (!doc_id) {
      return 0;
    }
    posting_[doc_id] = weight;
    merger_(upper_bound_, weight);
    return 1;
  }

  virtual int remove(DocId doc_id) {
    auto iter = posting_.find(doc_id);
    if (iter != posting_.end()) {
      posting_.erase(iter);
      return 1;
    }
    return 0;
  }

  virtual void apply() {
  }

  virtual std::unique_ptr<Reader> create_reader(std::shared_ptr<PList> shared_list) const;

private:
  PostingMap posting_;
  Weight upper_bound_;
  WeightMerger merger_;
};

template <typename DocId, typename Weight>
class MapPostingListReader: public PostingListReader<DocId, const Weight&> {
public:
  typedef PostingList<DocId, Weight> PList;
  typedef std::pair<DocId, Weight> PostingPair;
  typedef std::map<DocId, Weight> PostingMap;

  MapPostingListReader(const PostingMap& posting, const Weight& upper_bound, std::shared_ptr<PList> ref)
  : ref_(std::move(ref)), posting_(&posting), upper_bound_(&upper_bound), iter_(posting_->begin()) {
  }

  virtual ~MapPostingListReader() = default;

  virtual DocId next(DocId current) {
    for (; iter_ != posting_->end(); ++iter_) {
      if (iter_->first > current) {
        return iter_->first;
      }
    }
    return DocId(); // invalid
  }

  virtual const Weight& read() {
    return iter_->second;
  }

  virtual const Weight& upper_bound() {
    return *upper_bound_;
  }

  virtual size_t size() const {
    return posting_->size();
  }

private:
  // Shared the lifetime with PostingList, make sure these values are always valid as long as reader valid.
  std::shared_ptr<PList> ref_;
  const PostingMap* posting_;
  const Weight* upper_bound_;
  typename PostingMap::const_iterator iter_;
};

template <typename DocId, typename Weight, typename WeightMerger>
auto MapPostingList<DocId, Weight, WeightMerger>::create_reader(std::shared_ptr<PList> shared_list) const
-> std::unique_ptr<Reader> {
  // the parameters of reader constructor are pointers to the internal vector and upper bound weight,
  // these pointers shares the life time with posting_list so that they are always valid as long as the reader valid.
  return std::unique_ptr<Reader>(new MapPostingListReader<DocId, Weight>(posting_, upper_bound_,
      std::move(shared_list)));
}

template <typename DocId, typename Weight, typename WeightMerger = MaxWeight<Weight>>
class MapPostingListFactory: public PostingListFactory<DocId, Weight> {
public:
  typedef PostingListFactory<DocId, Weight> Base;
  typedef typename Base::PList PList;
  typedef typename Base::ReaderByVal ReaderByVal;
  typedef typename Base::ReaderByRef ReaderByRef;

  MapPostingListFactory()
  : merger_() {
  }

  MapPostingListFactory(WeightMerger merger)
  : merger_(merger) {
  }

  virtual ~MapPostingListFactory() = default;

  virtual std::shared_ptr<PList> create_posting_list() {
    return std::shared_ptr<PList>(new MapPostingList<DocId, Weight, WeightMerger>(merger_));
  }

  virtual std::shared_ptr<PList> create_posting_list(std::unique_ptr<ReaderByVal> reader) {
    return std::shared_ptr<PList>(new MapPostingList<DocId, Weight, WeightMerger>(*reader, merger_));
  }

  virtual std::shared_ptr<PList> create_posting_list(std::unique_ptr<ReaderByRef> reader) {
    return std::shared_ptr<PList>(new MapPostingList<DocId, Weight, WeightMerger>(*reader, merger_));
  }

private:
  WeightMerger merger_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_CORE_INDEX_MAP_POSTING_LIST_H_ */
