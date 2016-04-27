#ifndef SRC_MAIN_REDGIANT_CORE_QUERY_POSTING_LIST_QUERY_H_
#define SRC_MAIN_REDGIANT_CORE_QUERY_POSTING_LIST_QUERY_H_

#include "core/reader/posting_list_reader.h"

namespace redgiant {
template<typename DocId, typename Score, typename InputWeight>
class PostingListQuery {
public:
  typedef PostingListReader<DocId, Score> Reader;
  typedef PostingListReader<DocId, InputWeight> InputReader;

  PostingListQuery() = default;
  virtual ~PostingListQuery() = default;

  virtual std::unique_ptr<Reader> query(std::unique_ptr<InputReader> reader) const = 0;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_REDGIANT_CORE_QUERY_POSTING_LIST_QUERY_H_ */
