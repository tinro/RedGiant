#ifndef SRC_MAIN_CORE_READER_WAND_READER_H_
#define SRC_MAIN_CORE_READER_WAND_READER_H_

#include <memory>
#include <vector>
#include "core/reader/posting_list_reader.h"

namespace redgiant {
class WandReaderTest;

template <typename DocId, typename Score>
class WandReader : public PostingListReader<DocId, Score> {
public:
  friend class WandReaderTest;
  typedef PostingListReader<DocId, Score> Reader;

  WandReader(std::vector<std::unique_ptr<Reader>>&& input_readers);
  virtual ~WandReader() = default;

  virtual DocId next(DocId current);
  virtual Score read();
  virtual Score upper_bound();

  virtual void threshold(Score threshold) {
    threshold_ = threshold;
  }

private:
  size_t find_pivot(size_t from);
  size_t pick_term(size_t pivot, DocId cursor);
  size_t step_next(size_t pivot, DocId cursor);
  void remove_term(size_t term_index);
  void move_term(size_t term_index);

private:
  // saved readers
  std::vector<std::unique_ptr<Reader>> readers_;
  // cursors in the raw order of readers
  std::vector<DocId> reader_cursors_;
  // cached upper bounds of readers
  std::vector<Score> upper_bounds_;
  // indexes point to readers and sorted by cursor
  std::vector<size_t> sorted_indexes_;
  // accumulated upper bounds of readers in the sorted order
  std::vector<Score> acc_upper_bounds_;
  Score threshold_;
};

} /* namespace redgiant */

#endif /* SRC_MAIN_CORE_READER_WAND_READER_H_ */
