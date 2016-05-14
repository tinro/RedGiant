Red Giant
=========

Red Giant is a simple and light-weighted indexing and search system designed
for small scale personalized recommendation systems.

Latest version: 0.0.1 (not completed yet)

[![Build Status](https://travis-ci.org/tinro/RedGiant.svg?branch=master)](https://travis-ci.org/tinro/RedGiant)

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
* autotools (autoconf >= 2.68, automake >= 1.11)
* libevent >= 2.0
* liblog4cxx >= 0.10
* libcppunit >= 1.13 (for unit tests)

Make:
run `./make.sh` under your shell.
