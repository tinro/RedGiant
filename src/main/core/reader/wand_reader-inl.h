#ifndef SRC_MAIN_CORE_READER_WAND_READER_INL_H_
#define SRC_MAIN_CORE_READER_WAND_READER_INL_H_

#include <algorithm>
#include <set>
#include "core/reader/wand_reader.h"

namespace redgiant {

template <typename DocId, typename Score>
WandReader<DocId, Score>::WandReader(std::vector<std::unique_ptr<Reader>>&& input_readers)
: readers_(std::move(input_readers)), reader_cursors_(readers_.size(), 0), upper_bounds_(readers_.size()),
  sorted_indexes_(readers_.size()), acc_upper_bounds_(readers_.size()), threshold_(0) {
  // zero initialized containers and threshold
  // keep the initial order by default
  size_t i = 0;
  std::generate(sorted_indexes_.begin(), sorted_indexes_.end(), [&i] { return i++; });
  // cache the upper bounds of input readers
  std::transform(readers_.begin(), readers_.end(),  upper_bounds_.begin(),
      [] (const std::unique_ptr<Reader>& reader) { return reader->upper_bound(); });
  // sum upper bound scores in the sequence of sorted terms
  std::partial_sum(upper_bounds_.begin(), upper_bounds_.end(), acc_upper_bounds_.begin());
}

template <typename DocId, typename Score>
DocId WandReader<DocId, Score>::next(DocId current) {
  size_t pivot = find_pivot(0);
  for (;;) {
    // check pivot valid
    if (pivot >= sorted_indexes_.size()) {
      // no more valid documents;
      return DocId();
    }
    size_t pivot_index = sorted_indexes_[pivot];
    DocId pivot_cursor = reader_cursors_[pivot_index];
    if (pivot_cursor > current) {
      if (reader_cursors_[sorted_indexes_[0]] == pivot_cursor) {
        // we found a document that may be greater than threshold
        // it will be qualified for later full-evaluation.
        return pivot_cursor;
      } else {
        pivot = step_next(pivot, --pivot_cursor);
      }
    } else {
      pivot = step_next(pivot, current);
    }
  }
}

template <typename DocId, typename Score>
Score WandReader<DocId, Score>::read() {
  Score score(0);
  DocId current = reader_cursors_[sorted_indexes_[0]];
  for (size_t index: sorted_indexes_) {
    // note readers_ is sorted by cursor
    if (reader_cursors_[index] != current) {
      break;
    }
    score += readers_[index]->read();
  }
  return score;
}

template <typename DocId, typename Score>
Score WandReader<DocId, Score>::upper_bound() {
  if (acc_upper_bounds_.size() > 0) {
    return acc_upper_bounds_.back();
  }
  return Score(0);
}

template <typename DocId, typename Score>
size_t WandReader<DocId, Score>::find_pivot(size_t from) {
  // find the first element, that is greater than threshold_
  auto i = std::upper_bound(acc_upper_bounds_.begin() + from, acc_upper_bounds_.end(), threshold_);
  // if not found, return value is acc_upper_bounds_.size() and also equal to readers_.size()
  return i - acc_upper_bounds_.begin();
}

template <typename DocId, typename Score>
size_t WandReader<DocId, Score>::pick_term(size_t pivot, DocId cursor) {
  (void)pivot;
  (void)cursor;
  // TODO: optimize
  // find a term, which is sorted before or equal to pivot
  // and its cursor is not greater than given cursor
  return 0;
}

template <typename DocId, typename Score>
size_t WandReader<DocId, Score>::step_next(size_t pivot, DocId cursor) {
  pivot++; // make sure pivot will not go negative
  while (pivot > 0 && acc_upper_bounds_[pivot-1] > threshold_) {
    // note: picked term may be the previous pivot itself
    // the current cursor of the picked term shall be not greater than input cursor
    size_t pick = pick_term(pivot, cursor);
    size_t pick_index = sorted_indexes_[pick];
    // read the picked term to greater than cursor
    DocId next_cursor = readers_[pick_index]->next(cursor);
    reader_cursors_[pick_index] = next_cursor;
    if (next_cursor) {
      // find the position to insert in
      move_term(pick);
    } else {
      // read to end of the posting list, remove the term0
      remove_term(pick);
    }
    pivot--;
  }
  // find the new pivot
  return find_pivot(pivot);
}

template <typename DocId, typename Score>
void WandReader<DocId, Score>::remove_term(size_t term) {
  Score score(0);
  if (term > 0) {
    score = acc_upper_bounds_[term-1];
  }
  // update the sorted list and summed scores
  for (size_t i = term+1; i < sorted_indexes_.size(); ++i) {
    // move term
    sorted_indexes_[i-1] = sorted_indexes_[i];
    // update scores
    score += upper_bounds_[sorted_indexes_[i]];
    acc_upper_bounds_[i-1] = score;
  }
  sorted_indexes_.pop_back();
  acc_upper_bounds_.pop_back();
}

template <typename DocId, typename Score>
void WandReader<DocId, Score>::move_term(size_t term) {
  Score score(0);
  if (term > 0) {
    score = acc_upper_bounds_[term-1];
  }
  size_t term_index = sorted_indexes_[term];
  DocId term_cursor = reader_cursors_[term_index];
  Score term_upper_bound = upper_bounds_[term_index];
  size_t i = term+1;
  // update the sorted list and summed scores
  for (; i < sorted_indexes_.size(); ++i) {
    // stop if found a greater cursor
    if (reader_cursors_[sorted_indexes_[i]] >= term_cursor) {
      break;
    }
    // move term
    sorted_indexes_[i-1] = sorted_indexes_[i];
    // update scores
    score += upper_bounds_[sorted_indexes_[i]];
    acc_upper_bounds_[i-1] = score;
  }
  // put term here
  sorted_indexes_[i-1] = term_index;
  // update scores
  score += term_upper_bound;
  acc_upper_bounds_[i-1] = score;
}

} /* namespace redgiant */

#endif /* SRC_MAIN_CORE_READER_WAND_READER_INL_H_ */
