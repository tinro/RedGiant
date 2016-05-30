# Red Giant

Red Giant is a simple and light-weighted indexing and search system designed
for small scale personalized recommendation systems.

## Release

**Latest release: 0.1**

[![Build Status](https://travis-ci.org/tinro/RedGiant.svg?branch=master)](https://travis-ci.org/tinro/RedGiant)

NOTE: This project is currently only for research and study. Do not use it in your productions!

## Overview

### Features

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

### Build 

Prerequisite:

* gcc >= 4.8 or clang >= 3.4
* autotools (autoconf >= 2.68, automake >= 1.11)
* libevent >= 2.0
* liblog4cxx >= 0.10
* libcppunit >= 1.13 (for unit tests)

Run `./make.sh` under your shell to build (and run unit tests).

### Run

In `src/main` folder, run `./redgiant conf/config-debug.json`.
The only parameter is the path to configuration file.

## Documentation

Red Giant is designed to provide relevance based (personalized) recommendations. It is one of the important methods of recommendation, especially for rich content that are updated frequently (for example, news). It is especially useful for small scale or early stage recommendation products.

In some more complex and sophisticated recommendation platforms, there may be multiple sources of recommendation that implementing different algorithms. And it is still important to have the relevance based method to provide the ability of retrieving contents without dependencies to click logs.

### Problem modeling

There are two parties in personalized recommendation systems: users and documents. Each document consists of a set of weighted features. By mining user logs (as well as directly collecting from users), we assign each user a set of weighted features. The prolem is now abstracted as: given a set of document D = {d1, d2, ... dn} and the weighted features of each document di = {(f_ij, w_ij) | j = 1,2,...x_i}, then given a user with his profile u = {(f_k, w_k) | k=1,2, ...,y}, get the highested ranked documents that the user may be insterested in.

### Features and feature spaces

For both documents and queries, features are grouped into feature spaces. A weighted feature set in given space is called a feature vector. Two feature vectors in the same feature space could be dot-producted to calculate their similarity. This is the basic idea of dot-product based recommendation.

Each feature has a unique key in a given space. Keys could be shared across different spaces. Each feature is asigned with an Id, which is a 64-bit unsigned int number. The highest 8 bits of feature Id represent Id of the feature space, and the following 56 bits are calculated from the key. An Id is considered globally unique, though it is still possible to have hash collisions. We will simply ignore the collisions. An Id with lower 56 bits set to 2^56-1 is considered as invalid.

The key of features could be either integer (unsigned) or string. If the key is an integer, it should be in the range [0, 2^56-2]. If the key is a string, the lower 56-bits of its hash result is used to create feature Id. The Id of feature spaces should be in the range [0, 2^8-1]

All feature spaces used in the system should be configured in the configuration file before service startup. In the `feature_spaces` section, here is an example feature space configuration.

    {"id": 5,   "name": "category_declared",  "type": "integer"},

The `id` field is the id of feature space, which is not required to utilize in order. The `name` field is the name of feature space, it will be referred in document and query json. The `type` field defines whether the feature keys should be `integer` or `string`, as described above.

### Ranking models

### Documents

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

### Queries

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

### Misc

