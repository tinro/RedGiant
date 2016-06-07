#ifndef SRC_MAIN_INDEX_DOCUMENT_INDEX_H_
#define SRC_MAIN_INDEX_DOCUMENT_INDEX_H_

#include "core/impl/row_index_impl.h"
#include "core/impl/row_index_impl-inl.h"
#include "index/document_traits.h"

namespace redgiant {
extern template class BaseIndexImpl<DocumentTraits>;
extern template class RowIndexImpl<DocumentTraits>;

class DocumentIndex: public RowIndexImpl<DocumentTraits> {
public:
  typedef RowIndexImpl<DocumentTraits> Base;

  DocumentIndex(size_t initial_buckets, size_t max_size)
  : Base(initial_buckets, max_size) {
  }

  // restore from file
  // note: this may throws exception
  DocumentIndex(size_t initial_buckets, size_t max_size, const std::string& file_name);

  // have to leave an empty function here to workaround gcc bugs
  ~DocumentIndex() {
  }

  // dump to file
  // note: this may throws exception
  size_t dump(const std::string& file_name);

};
} /* namespace redgiant */

#endif /* SRC_MAIN_INDEX_DOCUMENT_INDEX_H_ */
