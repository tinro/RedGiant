#ifndef SRC_MAIN_CORE_IMPL_FREEZABLE_POSTING_LIST_H_
#define SRC_MAIN_CORE_IMPL_FREEZABLE_POSTING_LIST_H_

#include <memory>
#include <utility>

#include "core/index/posting_list.h"
#include "core/reader/posting_list_reader.h"
#include "core/reader/reader_utils.h"

namespace redgiant {
/*
 * - A wrapper of posting list implements thread safety for parallel reading.
 * - Once frozen, the wrapped posting list will not allow further changes, then
 *   it is safe to create read from it in multiple threads.
 * - To change a frozen posting list, you need to fork a new one and apply it
 *   after changes are made.
 * - Instances of this class should be protected by external shared_mutex.
 */
template <typename DocId, typename Weight>
class FreezablePostingList: public PostingList<DocId, Weight> {
public:
  typedef PostingList<DocId, Weight> Base;
  typedef typename Base::PList PList;
  typedef typename Base::Reader Reader;
  typedef PostingListFactory<DocId, Weight> Factory;
  typedef typename Factory::ReaderByVal ReaderByVal;
  typedef typename Factory::ReaderByRef ReaderByRef;

  // Create from the factory of internal posting list.
  FreezablePostingList(const Factory& factory, bool frozen = false)
  : instance_(factory.create_posting_list()), frozen_(frozen) {
  }

  // Create from the factory of internal posting list and an external reader.
  FreezablePostingList(const Factory& factory, std::unique_ptr<ReaderByVal> reader,
      bool frozen = false)
  : instance_(factory.create_posting_list(std::move(reader))), frozen_(frozen)  {
  }

  // Create from the factory of internal posting list and an external reader.
  FreezablePostingList(const Factory& factory, std::unique_ptr<ReaderByRef> reader,
      bool frozen = false)
  : instance_(factory.create_posting_list(std::move(reader))), frozen_(frozen)  {
  }

  // Create from a passed-in posting list.
  FreezablePostingList(std::shared_ptr<PList> plist, bool frozen = false)
  : instance_(std::move(plist)), frozen_(frozen)  {
  }

  virtual ~FreezablePostingList() = default;

  // need external read lock
  virtual bool empty() const {
    return instance_->empty();
  }

  // need external write lock
  virtual int update(DocId doc_id, const Weight& weight) {
    if (frozen_) {
      return 0;
    }
    return instance_->update(doc_id, weight);
  }

  // need external write lock
  virtual int remove(DocId doc_id) {
    if (frozen_) {
      return 0;
    }
    return instance_->remove(doc_id);
  }

  // need external read lock, and the input shared_list is ignored.
  // once the reader is created, it is safe to read from it any time later.
  virtual std::unique_ptr<Reader> create_reader(std::shared_ptr<PList> shared_list) const {
    // we should hold wrapped instance_ instead of this or shared_list,
    // since the wrapped instance may change.
    (void) shared_list;
    if (!frozen_) {
      // it is not safe to return reader to the internal instance if not frozen.
      return nullptr;
    }
    return create_reader_shared(instance_);
  }

  // need external write lock
  void freeze() {
    frozen_ = true;
  }

  // need external read lock
  std::shared_ptr<PList> get_instance() const {
    if (!frozen_) {
      // it is not safe to return reader to the internal instance if not frozen.
      return nullptr;
    }
    return instance_;
  }

  // safe to access without external lock
  std::shared_ptr<FreezablePostingList> fork(const Factory& factory) const {
    return std::make_shared(factory.create_posting_list(create_reader_shared(instance_)));
  }

private:
  std::shared_ptr<PList> instance_;
  bool frozen_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_CORE_IMPL_FREEZABLE_POSTING_LIST_H_ */
