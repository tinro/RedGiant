#include "index/document_index.h"

#include "core/impl/row_index_impl-inl.h"
#include "core/snapshot/snapshot.h"

namespace redgiant {

template class BaseIndexImpl<DocumentTraits>;
template class RowIndexImpl<DocumentTraits>;

DocumentIndex::DocumentIndex(size_t initial_buckets, size_t max_size, const std::string& file_name)
: Base(initial_buckets, max_size, SnapshotLoader(file_name)) {
}

size_t DocumentIndex::dump(const std::string& file_name) {
  return Base::dump(SnapshotDumper(file_name));
}

} /* namespace redgiant */
