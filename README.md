# Red Giant

Red Giant is a simple and light-weighted indexing and search system designed
for small scale personalized recommendation systems.

**Latest release: 0.1**

NOTE: This project is currently only for research and study. Do not use it in your productions!

[![Build Status](https://travis-ci.org/tinro/RedGiant.svg?branch=master)](https://travis-ci.org/tinro/RedGiant)

## Features

Supported features in version 0.1
* Reverse indexing to documents that are sets of weighted features.
* Defining named feature spaces that are groups of features.
* Retrieving documents with weighted features, combined by AND, OR and WAND operators.
* Defining multiple retrieval models which build different retrieval expressions.
* Recieving realtime updates and apply changes periodically.

To keep it simple, there are following assumptions and limitations.
* All documents and searches are sets of weighted features.
* Document metadata is not stored in this service. You may want to retrieve these metadata from other typical key-value services.
* The retrieval models are only linear combinations of vector products.
* No L2 ranking. If in need, you may have to build another external service to do this.
* No clustering feature, the size of index should be capable for a single server instance (usually no more than 100M documents).

## Build 

Prerequisite:

* gcc >= 4.8 or clang >= 3.4
* autotools (autoconf >= 2.68, automake >= 1.11)
* libevent >= 2.0
* liblog4cxx >= 0.10
* libcppunit >= 1.13 (for unit tests)

Run `./make.sh` under your shell to build (and run unit tests).

## Run

In `src/main` folder, run `./redgiant conf/config-debug.json`.
The only parameter is the path to configuration file.

# Documentation

## Overview

## Documents

You can put documents through RESTful json interface, such as the following

    $ curl -XPUT -d '{
      "features": {
        "publisher": "id_test",
        "category": {
          "1": 0.1,
          "2": 0.2,
          "3": 0.3
        },
        "entity": {
          "aa": 0.1,
          "bb": 0.2,
          "cc": 0.3
        },
        "popularity": 0.6
      }
    }' "http://127.0.0.1:19980/document?uuid=4e73cdd7-de87-4e2e-bc70-7336469092bf"

## Requests

Then, you can query the index using an HTTP POST request

    $ curl -XPOST -d '{
      "features": {
        "category_declared": {
          "3": 3
        },
        "popularity": {
          "0": 10.0
        }
      }
    }' "http://127.0.0.1:19980/query?id=0002&count=10&debug=true&model=mixed"

## Features

## Ranking models

## Misc

