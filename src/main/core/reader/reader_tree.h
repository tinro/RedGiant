#ifndef SRC_MAIN_REDGIANT_CORE_READER_READER_TREE_H_
#define SRC_MAIN_REDGIANT_CORE_READER_READER_TREE_H_

#include <memory>
#include <utility>
#include <vector>
#include "core/reader/algorithms.h"
#include "core/reader/posting_list_reader.h"

namespace redgiant {
template <typename DocId, typename InputWeight, typename OutputWeight = InputWeight>
class ReaderTree: public PostingListReader<DocId, OutputWeight> {
public:
  typedef PostingListReader<DocId, InputWeight> Reader;

  ReaderTree(std::vector<std::unique_ptr<Reader>>&& readers)
  : readers_(std::move(readers)), reader_cursors_(readers_.size(), 0) {
  }

  virtual ~ReaderTree() = default;

  virtual DocId next(DocId current) = 0;
  virtual OutputWeight read() = 0;
  virtual OutputWeight upper_bound() = 0;

protected:
  std::vector<std::unique_ptr<Reader>> readers_;
  std::vector<DocId> reader_cursors_;
};

template <typename DocId, typename InputWeight, typename OutputWeight = InputWeight, typename ScoreCombiner = AddScore<OutputWeight>>
class IntersectReader: public ReaderTree<DocId, InputWeight, OutputWeight> {
public:
  typedef ReaderTree<DocId, InputWeight, OutputWeight> Base;
  typedef typename Base::Reader Reader;

  IntersectReader(std::vector<std::unique_ptr<Reader>>&& readers)
  : Base(std::move(readers)), combiner_() {
  }

  IntersectReader(std::vector<std::unique_ptr<Reader>>&& readers, const ScoreCombiner& combiner)
  : Base(std::move(readers)), combiner_(combiner) {
  }

  virtual ~IntersectReader() = default;

  virtual DocId next(DocId current);
  virtual OutputWeight read();
  virtual OutputWeight upper_bound();

private:
  using Base::readers_;
  using Base::reader_cursors_;
  ScoreCombiner combiner_;
};

template <typename DocId, typename InputWeight, typename OutputWeight = InputWeight, typename ScoreCombiner = AddScore<OutputWeight>>
class UnionReader: public ReaderTree<DocId, InputWeight, OutputWeight> {
public:
  typedef ReaderTree<DocId, InputWeight, OutputWeight> Base;
  typedef typename Base::Reader Reader;

  UnionReader(std::vector<std::unique_ptr<Reader>>&& readers)
  : Base(std::move(readers)), combiner_() {
  }

  UnionReader(std::vector<std::unique_ptr<Reader>>&& readers, const ScoreCombiner& combiner)
  : Base(std::move(readers)), combiner_(combiner) {
  }

  virtual ~UnionReader() = default;

  virtual DocId next(DocId current);
  virtual OutputWeight read();
  virtual OutputWeight upper_bound();

private:
  using Base::readers_;
  using Base::reader_cursors_;
  ScoreCombiner combiner_;
};

template <typename DocId, typename InputWeight, typename OutputWeight = InputWeight, typename ScoreCombiner = FirstScore<OutputWeight>>
class SubtractReader: public ReaderTree<DocId, InputWeight, OutputWeight> {
public:
  typedef ReaderTree<DocId, InputWeight, OutputWeight> Base;
  typedef typename Base::Reader Reader;

  SubtractReader(std::vector<std::unique_ptr<Reader>>&& readers)
  : Base(std::move(readers)), combiner_() {
  }

  SubtractReader(std::vector<std::unique_ptr<Reader>>&& readers, const ScoreCombiner& combiner)
  : Base(std::move(readers)), combiner_(combiner) {
  }

  virtual ~SubtractReader() = default;

  virtual DocId next(DocId current);
  virtual OutputWeight read();
  virtual OutputWeight upper_bound();

private:
  using Base::readers_;
  using Base::reader_cursors_;
  ScoreCombiner combiner_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_REDGIANT_CORE_READER_READER_TREE_H_ */
