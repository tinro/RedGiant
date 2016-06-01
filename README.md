# Red Giant

Red Giant is a simple and light-weighted indexing and search system designed
for small scale personalized recommendation systems.

## Release

**Latest release: 0.2.0**

[![Build Status](https://travis-ci.org/tinro/RedGiant.svg?branch=master)](https://travis-ci.org/tinro/RedGiant)

NOTE: Currently this project is only for research and study, it is not recommended to use it in productions. Use at you own risk!

## Overview

### Features

Supported features in version 0.2.0
* Reverse indexing to documents consist of sets of weighted features.
* Defining named feature spaces that are groups of features.
* Retrieving documents with weighted features, combined by AND, OR and WAND operators.
* Defining multiple retrieval models which build different retrieval expressions.
* Recieving realtime updates and apply changes periodically.
* Persist index to files and / or restore index from files.

To keep it simple, there are following assumptions or limitations.
* Documents and searches are defined by preprocessed weighted features. There is no built-in text processor.
* Document metadata is not stored. You may need to retrieve metadata from external key-value stores.
* The ranking models are linear combinations of vector dot-products.
* No L2 ranking. You may have to build an external service to do this if in need.
* No cluster service. The size of index should be capable for a single server instance (usually no more than 100M documents). If you have more documents you may need to handle sharding and replica from the caller side.

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

The `id` field is the id of feature space, which is not required to utilize in order. The `name` field is the name of feature space, it will be referred in document and query JSON. The `type` field defines whether the feature keys should be `integer` or `string`, as described above.

### Ranking models

A ranking model describes how to map input feature spaces to feature spaces of documents, as well as how to combine the relevace scores calculated from multiple feature spaces. Currently there are two types of models implemented, and we can define multiple instances of each type of models with different configurations.

#### Direct model
This is the most straight forward model, that each input feature space is dot product with the same feature space of documents, and combined with the unitary weight of 1.0. There is no extra configuration, so we only need to define one instance of this type of model. 

      { "name": "default", "type": "direct" }

This is also the default model of the default settings.

#### Feature mapping model

Each input feature space is explicitly mapped to a feature space (may be itself or different) with a weight. The feature spaces that are not explicitly mapped are ignored. We can define multiple instances of feature mapping models for different purpose.

Here is an example of feature mapping model configurations.

      { "name": "category_only", "type": "mapping", "mappings": [
        { "from": "category_inferred", "to": "category", "weight": 1.0 },
        { "from": "category_declared", "to": "category", "weight": 1.0 }
      ]}

In practice, we may define user interested document categories in user profiles. They may come from different methods, for example, mined from logs or declared by users. They are stored in different feature spaces (`category_inferred` and `category_declared`) to support online combination with adjustable weights. In this example, both feature spaces are mapped to `category` feature space of documents, and the scores are combined with weights both in 1.0. All other input feature spaces are ignored.

### Documents

There is a RESTful JSON interface to read, write, update and delete documents.

    http://<SERVER ADDRESS>/document

#### Write/Update document(s)

Use `PUT` method to write documents to index, or update existing documents. The `PUT` method is reentrant, but calling it multiple times may have performance impact. Currently we can only write one document in a time.

Here is an example request:

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

The request is processed asynchronously. For HTTP code 200 is returned for valid requests, and 400 is returned for mal-formed requests.

Here are supported request parameters

| Name    | Type    | Requirement | Description |
|---------|---------|-------------|-------------|
| uuid    | string  | optional    | The uuid of the document, in GUID format. If omitted, an "uuid" field in JSON body is required. |
| ttl     | integer | optional    | Time to live of the document, in seconds calculated from the time of request. If omitted, the default value is used. |

Here are the fields in the JSON body

| Name    | Type    | Requirement | Description |
|---------|---------|-------------|-------------|
| uuid    | string  | optional    | The uuid of the document. Required if uuid is not specified in query parameters . |
| features | JSON Object | required | The weighted features of the document. |

The `features` field contains key-value pairs as JSON objects. They keys are name of feature spaces and the values define weighted features in that feature space. There are basically three formats of feature spaces:

* Complete format: the value of feature space is a JSON object which is they key-value pairs of weighted features, keys are keys of features and values are weights. For example `"category": { "1": 0.1, "2": 0.2, "3": 0.3 }`.
* Single feature: the value of feature space is string that is the only feature key in the feature space. Its weight is considered as 1.0. For example `"publisher": "id_test"` is a shortcut to `"publisher": { "id_test": 1.0 }`.
* Single value: the value of feature space is a number that is the weight of the only feature in the feature space. The key of the feature is considered as `"0"` and usually represented as an integer. For example: `"popularity": 0.6` is a shortcut to `"popularity": { "0": "0.6" }`.

#### Read document(s)

Not implemented.

#### Delete document(s)

Not implemented.

### Queries

The query interface is used to get ranked documents with given profile. It is an HTTP interface on the following address.

    http://<SERVER ADDRESS>/query

Then, you can query the index using an HTTP POST request

    $ curl -XPOST -d '{
      "features": {
        "category_declared": {
          "3": 3.0
        },
        "entity_inferred": {
          "bb": 5.0
        },
        "popularity": {
          "0": 10.0
        }
      }
    }' "http://127.0.0.1:19980/query?id=0002&count=10&debug=true&model=mixed"

For valid requests, either there are results found or not, HTTP code 200 is returned. Code 400 is returned for mal-formed requests, and 500 is returned on internal errors.

Here are supported request parameters

| Name    | Type    | Requirement | Description |
|---------|---------|-------------|-------------|
| id      | string  | required    | The id of the query request, in string format defined by users, used in server logs. It is the caller's resposibility to make it valid and unique. |
| count   | integer | required    | Maximum number of documents to retrieve from the index. |
| model   | integer | optional    | Name of the ranking model. If omitted, the default model configured is used. |
| debug   | boolean | optional    | Whether to print debug logs on the server, default to false. It is the caller's resposiblity to ensure it is not abused. |

Here are the fields in the JSON body

| Name    | Type    | Requirement | Description |
|---------|---------|-------------|-------------|
| features | JSON Object | required | The weighted features of the query request. |

The `features` field is also key-value pairs of feature spaces similar as documents. However only the complete format is supported. So that the values of feature spaces should be JSON objects containing key-value pairs of weighted featurs.

### Dump and Restore

The index could be persisted to file(s), and restored from file(s). There is a `snapshot_prefix` configuration in the `index` section. There may be one or multiple files generated, and the paths to the files are started with this prefix string. The prefix could be either absolute or relative path, ends in either directory seperator ('/') or file name prefix. The directories should exist before persistence happens.

There are mainly two ways to persist index.

* Configure `dump_on_exit` and `restore_on_startup`, then the index will automatically dump to snapshot files on exit, and restored from snapshot on startup. If there are configuration changes during service outage, please make sure that the `id` of feature spaces are not changed.
* Call `/snapshot` endpoint, then the service will update the snapshot files. During snapshot creation, changes to index are temporarily disabled (async update is still available), while the service is still able to deal with search queries.

