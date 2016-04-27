#ifndef SRC_MAIN_REDGIANT_CORE_SNAPSHOT_SNAPSHOT_READER_H_
#define SRC_MAIN_REDGIANT_CORE_SNAPSHOT_SNAPSHOT_READER_H_

#include "core/reader/posting_list_reader.h"
#include "core/snapshot/snapshot.h"

namespace redgiant {
template <typename DocId, typename Weight>
class SnapshotReader: public PostingListReader<DocId, Weight> {
public:
  SnapshotReader(SnapshotLoader& loader)
  : loader_(loader) {
  }

  virtual ~SnapshotReader() = default;

  virtual DocId next(DocId current) {
    DocId doc_id;
    Weight weight;
    loader_.load(doc_id);
    while (!!doc_id && doc_id <= current) {
      loader_.load(weight);
      loader_.load(doc_id);
    }
    return doc_id;
  }

  virtual Weight read() {
    Weight weight;
    loader_.load(weight);
    return weight;
  }

  virtual Weight upper_bound() {
    return Weight();
  }

private:
  SnapshotLoader& loader_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_REDGIANT_CORE_SNAPSHOT_SNAPSHOT_READER_H_ */
