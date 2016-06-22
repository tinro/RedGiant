#ifndef SRC_MAIN_CORE_INDEX_STATIC_POSTING_LIST_H_
#define SRC_MAIN_CORE_INDEX_SEQUENTIAL_POSTING_LIST_H_

#include <memory>
#include <utility>
#include <vector>
#include "core/index/posting_list.h"
#include "core/reader/posting_list_reader.h"
#include "core/reader/reader_utils.h"

namespace redgiant {
template <typename DocId, typename Weight>
class SequentialPostingList: public PostingList<DocId, Weight> {
public:
  typedef PostingList<DocId, Weight> Base;
  typedef typename Base::PList PList;
  typedef typename Base::Reader Reader;
  typedef std::pair<DocId, Weight> PostingPair;
  typedef std::vector<PostingPair> PostingVec;

  SequentialPostingList()
  : upper_bound_() {
  }

  template <typename InputWeight, typename WeightMerger = MaxWeight<Weight>>
  SequentialPostingList(PostingListReader<DocId, InputWeight>& reader, WeightMerger merger = WeightMerger())
  : upper_bound_() {
    posting_ = read_all(reader, upper_bound_, merger);
  }

  virtual ~SequentialPostingList() = default;

  virtual bool empty() const {
    return posting_.empty();
  }

  virtual int update(DocId doc_id, const Weight& weight) {
    (void) doc_id;
    (void) weight;
    return 0;
  }

  virtual int remove(DocId doc_id) {
    (void) doc_id;
    return 0;
  }

  virtual std::unique_ptr<Reader> create_reader(std::shared_ptr<PList> shared_list) const;

private:
  PostingVec posting_;
  Weight upper_bound_;
};

template <typename DocId, typename Weight>
class SequentialPostingListReader: public PostingListReader<DocId, const Weight&> {
public:
  typedef PostingList<DocId, Weight> PList;
  typedef std::pair<DocId, Weight> PostingPair;
  typedef std::vector<PostingPair> PostingVec;

  SequentialPostingListReader(const PostingVec& posting, const Weight& upper_bound, std::shared_ptr<PList> ref)
  : ref_(std::move(ref)), posting_(&posting), upper_bound_(&upper_bound), iter_(posting_->begin()) {
  }

  virtual ~SequentialPostingListReader() = default;

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
  const PostingVec* posting_;
  const Weight* upper_bound_;
  typename PostingVec::const_iterator iter_;
};

template <typename DocId, typename Weight>
auto SequentialPostingList<DocId, Weight>::create_reader(std::shared_ptr<PList> shared_list) const
-> std::unique_ptr<Reader> {
  // the parameters of reader constructor are pointers to the internal vector and upper bound weight,
  // these pointers shares the life time with posting_list so that they are always valid as long as the reader valid.
  return std::make_unique<SequentialPostingListReader<DocId, Weight>>(posting_, upper_bound_,
      std::move(shared_list));
}

template <typename DocId, typename Weight>
class SequentialPostingListFactory: public PostingListFactory<DocId, Weight> {
public:
  typedef PostingListFactory<DocId, Weight> Base;
  typedef typename Base::PList PList;
  typedef typename Base::ReaderByVal ReaderByVal;
  typedef typename Base::ReaderByRef ReaderByRef;

  SequentialPostingListFactory() = default;

  virtual ~SequentialPostingListFactory() = default;

  virtual std::shared_ptr<PList> create_posting_list() const {
    return std::make_shared<SequentialPostingList<DocId, Weight>>();
  }

  virtual std::shared_ptr<PList> create_posting_list(std::unique_ptr<ReaderByVal> reader) const {
    return std::make_shared<SequentialPostingList<DocId, Weight>>(*reader);
  }

  virtual std::shared_ptr<PList> create_posting_list(std::unique_ptr<ReaderByRef> reader) const {
    return std::make_shared<SequentialPostingList<DocId, Weight>>(*reader);
  }
};
} /* namespace redgiant */

#endif /* SRC_MAIN_CORE_INDEX_STATIC_POSTING_LIST_H_ */
