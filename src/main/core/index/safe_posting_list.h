#ifndef SRC_MAIN_REDGIANT_CORE_INDEX_SAFE_POSTING_LIST_H_
#define SRC_MAIN_REDGIANT_CORE_INDEX_SAFE_POSTING_LIST_H_

#include <memory>
#include <mutex>
#include <utility>
#include "core/index/posting_list.h"
#include "core/reader/posting_list_reader.h"
#include "core/reader/reader_utils.h"
#include "core/lock/spin_lock.h"

namespace redgiant {
/*
 * Thread safe wrapper for parallel reading, implemented based on a spin lock.
 * Any operations calling const operations (except empty()) are thread-safe.
 * Any parallel operations calling non-const operations (and empty()) need a lock to protect.
 */
template <typename DocId, typename Weight>
class SafePostingList: public PostingList<DocId, Weight> {
public:
  typedef PostingList<DocId, Weight> Base;
  typedef typename Base::PList PList;
  typedef typename Base::Reader Reader;
  typedef PostingListFactory<DocId, Weight> Factory;

  SafePostingList(std::shared_ptr<Factory> factory)
  : factory_(std::move(factory)), write_instance_(), read_instance_(factory_->create_posting_list()) {
  }

  SafePostingList(std::shared_ptr<Factory> factory, std::shared_ptr<PList> plist)
  : factory_(std::move(factory)), write_instance_(), read_instance_(std::move(plist))  {
  }

  virtual ~SafePostingList() = default;

  // need external lock protect
  virtual bool empty() const {
    std::unique_lock<SpinLock> lock_read(spin_);
    return (!write_instance_) && read_instance_->empty();
  }

  // need external lock protect
  virtual int update(DocId doc_id, const Weight& weight) {
    if (!write_instance_) {
      write_instance_ = fork_write_instance();
    }
    return write_instance_->update(doc_id, weight);
  }

  // need external lock protect
  virtual int remove(DocId doc_id) {
    if (!write_instance_) {
      write_instance_ = fork_write_instance();
    }
    return write_instance_->remove(doc_id);
  }

  // need external lock protect
  virtual void apply() {
    if (write_instance_) {
      std::unique_lock<SpinLock> lock_read(spin_);
      read_instance_ = std::move(write_instance_);
    }
  }

  // safe to access without external lock
  virtual std::unique_ptr<Reader> create_reader(std::shared_ptr<PList> shared_list) const {
    // we do not need to hold this object during read; just hold the read_instance_ is ok.
    (void) shared_list;
    std::unique_lock<SpinLock> lock_read(spin_);
    std::shared_ptr<PList> plist = read_instance_;
    lock_read.unlock();
    return create_reader_shared(std::move(plist));
  }

private:
  // safe to access without external lock
  std::shared_ptr<PList> fork_write_instance() {
    std::unique_lock<SpinLock> lock_read(spin_);
    std::shared_ptr<PList> plist = read_instance_;
    lock_read.unlock();
    return factory_->create_posting_list(create_reader_shared(std::move(plist)));
  }

  std::shared_ptr<Factory> factory_;
  std::shared_ptr<PList> write_instance_;
  std::shared_ptr<PList> read_instance_;
  mutable SpinLock spin_;
};

template <typename DocId, typename Weight>
class SafePostingListFactory: public PostingListFactory<DocId, Weight> {
public:
  typedef PostingListFactory<DocId, Weight> Base;
  typedef typename Base::PList PList;
  typedef typename Base::Factory Factory;
  typedef typename Base::ReaderByVal ReaderByVal;
  typedef typename Base::ReaderByRef ReaderByRef;

  SafePostingListFactory(std::shared_ptr<Factory> factory)
  : factory_(std::move(factory)) {
  }

  virtual ~SafePostingListFactory() = default;

  virtual std::shared_ptr<PList> create_posting_list() {
    return std::shared_ptr<PList>(new SafePostingList<DocId, Weight>(factory_));
  }

  virtual std::shared_ptr<PList> create_posting_list(std::unique_ptr<ReaderByVal> reader) {
    return std::shared_ptr<PList>(new SafePostingList<DocId, Weight>(
        factory_, factory_->create_posting_list(std::move(reader))));
  }

  virtual std::shared_ptr<PList> create_posting_list(std::unique_ptr<ReaderByRef> reader) {
    return std::shared_ptr<PList>(new SafePostingList<DocId, Weight>(
        factory_, factory_->create_posting_list(std::move(reader))));
  }

private:
  std::shared_ptr<Factory> factory_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_REDGIANT_CORE_INDEX_SAFE_POSTING_LIST_H_ */
