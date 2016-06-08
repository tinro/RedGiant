#ifndef SRC_MAIN_CORE_IMPL_BASE_INDEX_IMPL_H_
#define SRC_MAIN_CORE_IMPL_BASE_INDEX_IMPL_H_

#include <memory>
#include <mutex>
#include <utility>
#include <unordered_map>
#include <vector>

#include "core/impl/freezable_posting_list.h"
#include "core/index/posting_list.h"
#include "core/query/posting_list_query.h"
#include "core/reader/posting_list_reader.h"
#include "third_party/lock/shared_mutex.h"

namespace redgiant {
/*
 * - This class implements the reverse index based on hash map, where keys are
 *   TermIds and values are posting lists. All changes to the index are
 *   deferred and wrote to pending change list. Changes take effect only after
 *   apply() calls.
 * - The posting lists stored in this index are FreezablePostingList(s), the
 *   posting lists should be frozen before it become valid for reading. Frozen
 *   means the wrapped posting list will not allow subsequent changes (but it
 *   is still valid to switch to another posting list).
 * - Once a posting list is read from index, it should be safe to read from the
 *   reader at any time later.
 */
template <typename DocTraits>
class BaseIndexImpl {
public:
  friend class BaseIndexImplTest;
  typedef typename DocTraits::DocId DocId;
  typedef typename DocTraits::TermId TermId;
  typedef typename DocTraits::TermWeight TermWeight;
  typedef typename DocTraits::ExpireTime ExpireTime;
  typedef typename DocTraits::DocIdHash DocIdHash;
  typedef typename DocTraits::TermIdHash TermIdHash;
  typedef PostingListReader<DocId, const TermWeight&> RawReader;

  template <typename Score>
  using Query = PostingListQuery<DocId, Score, const TermWeight&>;
  template <typename Score>
  using QueryPair = std::pair<TermId, std::unique_ptr<Query<Score>>>;
  template <typename Score>
  using Reader = PostingListReader<DocId, Score>;
  template <typename Score>
  using ReaderPair = std::pair<TermId, std::unique_ptr<Reader<Score>>>;
  template <typename Score>
  using Results = std::vector<std::pair<DocId, Score>>;

  BaseIndexImpl(size_t initial_buckets);

  // create from snapshot
  // may throw exception: std::ios_base::failure
  // Loader&& accepts both lvalues and rvalues (for temporarily constructed loader)
  template <typename Loader>
  BaseIndexImpl(size_t initial_buckets, Loader&& loader);

  // g++ has bug with =default destructor for extern declared templates.
  ~BaseIndexImpl() { }

  size_t get_term_count() const;

  size_t get_bucket_count() const;

  float get_load_factor() const;

  std::unique_ptr<RawReader> peek(TermId term_id) const;

  template <typename Score>
  std::unique_ptr<Reader<Score>> query(TermId term_id, const Query<Score>& query) const;

  template <typename Score>
  std::vector<ReaderPair<Score>> batch_query(const std::vector<QueryPair<Score>>& queries) const;

protected:
  typedef PostingList<DocId, TermWeight> PList;
  typedef FreezablePostingList<DocId, TermWeight> FreezablePList;
  typedef PostingListFactory<DocId, TermWeight> PListFactory;

  std::shared_ptr<PList> query_internal(TermId term_id);

  std::shared_ptr<FreezablePList> change_internal(TermId term_id, bool create);

  int create_update_internal(DocId doc_id, TermId term_id, const TermWeight& weights);

  int remove_internal(DocId doc_id, TermId term_id);

  int remove_internal(DocId doc_id, std::vector<TermId> terms);

  int apply_internal();

  // dump to snapshot
  // may throw exception: std::ios_base::failure
  // Dumper&& accepts both lvalues and rvalues (for temporarily constructed loader)
  template <typename Dumper>
  size_t dump_internal(Dumper&& dumper);

protected:
  mutable shared_mutex query_mutex_;
  mutable std::mutex change_mutex_;
  // protected by query_mutex_
  std::unordered_map<TermId, std::shared_ptr<PList>> index_;
  // protected by change_mutex_
  std::unordered_map<TermId, std::shared_ptr<FreezablePList>> changed_index_;
  std::unique_ptr<PListFactory> factory_;
};

} /* namespace redgiant */

#endif /* SRC_MAIN_CORE_IMPL_BASE_INDEX_IMPL_H_ */
