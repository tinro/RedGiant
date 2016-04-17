#ifndef REDGIANT_CORE_READER_READER_TREE_INL_H_
#define REDGIANT_CORE_READER_READER_TREE_INL_H_

#include "core/reader/reader_tree.h"

namespace redgiant {

template <typename DocId, typename InputWeight, typename OutputWeight, typename ScoreCombiner>
DocId IntersectReader<DocId, InputWeight, OutputWeight, ScoreCombiner>::next(DocId current)
{
  // find the next possible position
  reader_cursors_[0] = readers_[0]->next(current);
  DocId cursor = reader_cursors_[0];
  size_t i = 1;
  while (cursor && i < readers_.size()) {
    DocId reader_cursors_[i] = readers_[i]->next(cursor-1);
    if (reader_cursors_[i] != cursor) {
      i = 0; // restart iterating from the first reader;
      cursor = reader_cursors_[i];
      continue;
    }
    ++i;
  }
  return cursor;
}

template <typename DocId, typename InputWeight, typename OutputWeight, typename ScoreCombiner>
OutputWeight IntersectReader<DocId, InputWeight, OutputWeight, ScoreCombiner>::read()
{
  return combiner_(readers_, [] (const std::unique_ptr<Reader>& reader) { return reader->read(); });
}

template <typename DocId, typename InputWeight, typename OutputWeight, typename ScoreCombiner>
OutputWeight IntersectReader<DocId, InputWeight, OutputWeight, ScoreCombiner>::upper_bound()
{
  return combiner_(readers_, [] (const std::unique_ptr<Reader>& reader) { return reader->upper_bound(); });
}

template <typename DocId, typename InputWeight, typename OutputWeight, typename ScoreCombiner>
DocId UnionReader<DocId, InputWeight, OutputWeight, ScoreCombiner>::next(DocId current)
{
  // iterate all possible readers
  DocId cursor = current;
  for (auto& reader : readers_) {
    DocId last = reader->next(current);
    if (last && last < cursor) {
      cursor = last;
    }
  }
  if (cursor == current) {
    return DocId();
  }
  return cursor;
}

template <typename DocId, typename InputWeight, typename OutputWeight, typename ScoreCombiner>
OutputWeight UnionReader<DocId, InputWeight, OutputWeight, ScoreCombiner>::read()
{
  return combiner_(readers_, [] (const std::unique_ptr<Reader>& reader) { return reader->read(); });
}

template <typename DocId, typename InputWeight, typename OutputWeight, typename ScoreCombiner>
OutputWeight UnionReader<DocId, InputWeight, OutputWeight, ScoreCombiner>::upper_bound()
{
  return combiner_(readers_, [] (const std::unique_ptr<Reader>& reader) { return reader->upper_bound(); });
}

template <typename DocId, typename InputWeight, typename OutputWeight, typename ScoreCombiner>
DocId SubtractReader<DocId, InputWeight, OutputWeight, ScoreCombiner>::next(DocId current)
{
  // find the next possible position
  reader_cursors_[0] = readers_[0]->next(current);
  DocId cursor = reader_cursors_[0];
  size_t i = 1;
  while (cursor && i < readers_.size()) {
    reader_cursors_[i] = readers_[i]->next(current-1);
    if (reader_cursors_[i] == cursor) {
      reader_cursors_[0] = readers_[0]->next(current);
      cursor = reader_cursors_[0];
      i = 1;
      continue;
    }
    ++i;
  }
  return cursor;
}

template <typename DocId, typename InputWeight, typename OutputWeight, typename ScoreCombiner>
OutputWeight SubtractReader<DocId, InputWeight, OutputWeight, ScoreCombiner>::read()
{
  return combiner_(readers_, [] (const std::unique_ptr<Reader>& reader) { return reader->read(); });
}

template <typename DocId, typename InputWeight, typename OutputWeight, typename ScoreCombiner>
OutputWeight SubtractReader<DocId, InputWeight, OutputWeight, ScoreCombiner>::upper_bound()
{
  return combiner_(readers_, [] (const std::unique_ptr<Reader>& reader) { return reader->upper_bound(); });
}
}

#endif /* REDGIANT_CORE_READER_READER_TREE_INL_H_ */
