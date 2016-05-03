Red Giant
=========

Red Giant is a simple and light-weighted indexing and search system designed
for small scale personalized recommendation systems.

Latest version: 0.0.1 (not completed yet)

It is simple since:
* All documents are consindered as a set of weighted features.
* All inputs are sets of weighted features.
* The size of index should be capable for a single server instance (usually no
  more than 100M documents).

It supports the following features:
* WAND search in documents.
* Defining group of features.
* Defining retrieval models.

Build
--------
Prerequisite:
* gcc >= 4.8 or clang >= 3.4
* libevent >= 2.0
* liblog4cxx >= 0.10
* libcppunit >= 1.12 (for unit tests)

Make:
run `./make.sh` under your shell.
