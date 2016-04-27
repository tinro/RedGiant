#ifndef SRC_MAIN_REDGIANT_CORE_INDEX_POSTING_LIST_H_
#define SRC_MAIN_REDGIANT_CORE_INDEX_POSTING_LIST_H_

#include <memory>
#include <utility>
#include "core/reader/posting_list_reader.h"

namespace redgiant {
template <typename DocId, typename Weight>
class PostingList {
public:
  typedef PostingList<DocId, Weight> PList;
  typedef PostingListReader<DocId, const Weight&> Reader;

  PostingList() = default;
  virtual ~PostingList() = default;

  /*
   * -  Return true if the posting list is empty.
   */
  virtual bool empty() const = 0;

  /*
   * -  Insert or update the given doc_id with its corresponding weight. We may also need to update the stored
   *    upper_bound in the object.
   * -  Refuse to update if doc_id is invalid, and return 0. Otherwise if support update, return 1.
   * -  The update may be delay applied by calling apply().
   */
  virtual int update(DocId doc_id, const Weight& weight) = 0;

  /*
   * -  Remove the given doc_id and its corresponding weight. We may also need to update the stored upper_bound in the
   *    object. However normally we could not update it.
   * -  Return the updated number, 1 if removed or 0 otherwise.
   * -  The update may be delay applied by calling apply().
   */
  virtual int remove(DocId doc_id) = 0;

  /*
   * -  Apply the pending changes if it delays update and remove calls.
   * -  Do nothing if the update and remove calls are effected immediately
   */
  virtual void apply() = 0;

  /*
   * -  Create a PostingListReader class iterates over the contents of this object.
   * -  The PostingListReader hold pointers to contents of this object with a shared life time with the input
   *    shared_ptr. So the input shared_ptr shall eventually owns this object.
   */
  virtual std::unique_ptr<Reader> create_reader(std::shared_ptr<PList> shared_list) const = 0;
};

template <typename DocId, typename Weight>
class PostingListFactory {
public:
  typedef PostingList<DocId, Weight> PList;
  typedef PostingListFactory<DocId, Weight> Factory;
  typedef PostingListReader<DocId, Weight> ReaderByVal;
  typedef PostingListReader<DocId, const Weight&> ReaderByRef;

  PostingListFactory() = default;
  virtual ~PostingListFactory() = default;

  /*
   * -  Create a posting list and return by shared_ptr.
   */
  virtual std::shared_ptr<PList> create_posting_list() = 0;

  /*
   * -  Create a posting list from the given by-value reader and return by shared_ptr.
   */
  virtual std::shared_ptr<PList> create_posting_list(std::unique_ptr<ReaderByVal> reader) = 0;

  /*
   * -  Create a posting list from the given by-ref reader and return by shared_ptr.
   */
  virtual std::shared_ptr<PList> create_posting_list(std::unique_ptr<ReaderByRef> reader) = 0;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_REDGIANT_CORE_INDEX_POSTING_LIST_H_ */
