Red Giant core component
=========

A header-only, generic index composed by posting-lists and forward-indexes.

To be generic, this component defines different terminologies than other parts of this software.
- A "Doc" is something to be indexed and searched out, the id of Doc (aka. DocId) is the key of forward index.
- A "Term" is something can be used to find relative Docs, the id of Term (aka. TermId) become keys of posting lists.
- There is an N-N relation between Doc and Term. Each edge in the relation has a weight (TermWeight).

And there are some design guidelines in this index.
- The index is optimized for reading, so that it should minimize blockings during read.
- Writing performance may not be optimized as long as the performance metrics are acceptable.
- It is not required be strongly consistent. Writes take effects periodically in the background.
- Both Doc and the Doc-Term relation may have expire times. Expires are not required to be strongly acurate, but it should happen sometime after the expire time.

