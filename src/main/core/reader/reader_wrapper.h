#ifndef REDGIANT_CORE_READER_READER_WRAPPER_H_
#define REDGIANT_CORE_READER_READER_WRAPPER_H_

#include <memory>
#include <utility>
#include <vector>
#include "core/reader/algorithms.h"
#include "core/reader/posting_list_reader.h"

namespace redgiant {
template <typename DocId, typename InputWeight, typename OutputWeight>
class ReaderWrapper: public PostingListReader<DocId, OutputWeight> {
public:
  typedef PostingListReader<DocId, InputWeight> Reader;

  ReaderWrapper(std::unique_ptr<Reader> reader)
  : reader_(reader) {
  }

  virtual ~ReaderWrapper() = default;

  virtual DocId next(DocId current) {
    return reader_->next(current);
  }

  virtual OutputWeight read() {
    return reader_->read();
  }

  virtual OutputWeight upper_bound() {
    return reader_->upper_bound();
  }

private:
  std::unique_ptr<Reader> reader_;
};
} /* namespace redgiant */

#endif /* REDGIANT_CORE_READER_READER_WRAPPER_H_ */
