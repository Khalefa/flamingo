/*
  $Id: example.cc 5240 2010-05-19 16:51:45Z abehm $

  Copyright (C) 2010 by The Regents of the University of California
	
  Redistribution of this file is permitted under
  the terms of the BSD license.
    
  Date: 02/04/2008
  Author: Alexander Behm <abehm (at) ics.uci.edu>
*/

#include "common/query.h"
#include "common/simmetric.h"

#include "ftindexermem.h"
#include "ftindexerdiscardlists.h"
#include "ftindexercombinelists.h"
#include "listmerger/divideskipmerger.h"
#include "listmerger/scancountmerger.h"

#include "ftsearchermem.h"
#include "ftindexerondisk.h"
#include "ftsearcherondisk.h"
#include "listmerger/ondiskmergersimple.h"
#include "listmerger/ondiskmergeradapt.h"

#include "util/misc.h"

// create a dummy dictionary
extern void readString(vector<string>& data, const string& filenameData, unsigned count, unsigned maxLineLen);
std::vector<string> dictionary;
void initDictionary();

void memBasicUsage1();
void memBasicUsage2();
void memBasicUsage3();
void memDiscardLists();
void memCombineLists();
void memAdvancedUsage();

void ondiskBasicUsage1();
void ondiskBasicUsage2();

int main() {
  
  initDictionary();

  memBasicUsage1();
  memBasicUsage2();
  memBasicUsage3();
  memDiscardLists();
  memCombineLists();
  memAdvancedUsage();
  
  ondiskBasicUsage1();
  ondiskBasicUsage2();
  
  return 0;
}

void initDictionary() {
  // params: target vector, filename, number strings to read, max line length
  readString(dictionary, "data/female_names.txt", 4000, 20);
}


// builds in-memory index with length filter for partitioning
// fills index from existing collection
void memBasicUsage1() {  
  cout << "----- MEM BASIC USAGE 1 ----" << endl;

  // create gramgenerator and similarity metric
  GramGenFixedLen gramGen(2); // using fixed-length grams
  SimMetricEd simMetric(gramGen); // using the edit distance
  //SimMetricJacc simMetric(gramGen); // using jaccard similarity
  //SimMetricCos simMetric(gramGen); // using cosine similarity
  //SimMetricDice simMetric(gramGen); // using dice similarity
  
  // create simple indexer with default template arguments
  // default: in-memory index using Array<unsigned> as an inverted list container
  // first create a string container and fill it with strings to index
  StringContainerVector strContainer(true); // true indicates statistics gathering, e.g. for auto part filtering
  strContainer.fillContainer(dictionary.begin(), dictionary.end()); // fill the container from a collection
  FtIndexerMem<> indexer(&strContainer, &gramGen, 20, 10); // maxStrLen=20, fanout=10
  indexer.addPartFilter(new LengthFilter(20)); // add length filtering with a maximum string length of 20
  indexer.buildIndex();
  
  // create merger
  DivideSkipMerger<> merger;
  // create searcher passing merger and indexer with default template arguments
  // default: same as indexer, i.e. assumed simple indexer with Array<unsigned> as inverted lists and DivideSkipMerger as merger type
  FtSearcherMem<> searcher(&merger, &indexer);
  
  vector<unsigned> resultStringIDs;
  Query query("kathrin", simMetric, 1.0f); // query string, similarity metric, similarity threshold
  searcher.search(query, resultStringIDs);  
  cout << "SIMILAR STRINGS: " << endl;
  for(unsigned i = 0; i < resultStringIDs.size(); i++) {
    string tmp;
    strContainer.retrieveString(tmp, resultStringIDs.at(i));
    cout << tmp << endl;
  }
  
  cout << "SAVING INDEX" << endl;
  indexer.saveIndex("exampleindex.ix");

  cout << "LOADING INDEX" << endl;
  FtIndexerMem<> indexerLoaded(&strContainer);
  indexerLoaded.loadIndex("exampleindex.ix");
  
  resultStringIDs.clear();
  searcher.setFtIndexer(&indexerLoaded);
  searcher.search(query, resultStringIDs);  
  cout << "SIMILAR STRINGS: " << endl;
  for(unsigned i = 0; i < resultStringIDs.size(); i++) {
    string tmp;
    strContainer.retrieveString(tmp, resultStringIDs.at(i));
    cout << tmp << endl;
  }

  cout << "----------------------" << endl << endl;
}

// builds in-memory index with charsum filter for partitioning
// fills index from data file
void memBasicUsage2() {  
  cout << "----- MEM BASIC USAGE 2 ----" << endl;

  // create gramgenerator and similarity metric
  GramGenFixedLen gramGen(2); // using fixed-length grams
  SimMetricEd simMetric(gramGen); // using the edit distance
  
  // create simple indexer with default template arguments
  // default: in-memory index using Array<unsigned> as an inverted list container
  // first create a string container and fill it with strings to index
  StringContainerVector strContainer;
  strContainer.fillContainer("data/female_names.txt", 4000); // fill the container from a datafile and use the first 4000 lines
  FtIndexerMem<> indexer(&strContainer, &gramGen, 20, 10); // maxStrLen=20, fanout=10
  indexer.addPartFilter(new CharsumFilter(20)); // add charsum filtering with a maximum string length of 20
  indexer.buildIndex();
  
  // create merger
  DivideSkipMerger<> merger;
  // create searcher passing merger and indexer with default template arguments
  // default: same as indexer, i.e. assumed simple indexer with Array<unsigned> as inverted lists and DivideSkipMerger as merger type
  FtSearcherMem<> searcher(&merger, &indexer);
  
  vector<unsigned> resultStringIDs;
  Query query("kathrin", simMetric, 1.0f);
  searcher.search(query, resultStringIDs);  
  cout << "SIMILAR STRINGS: " << endl;
  for(unsigned i = 0; i < resultStringIDs.size(); i++) {
    string tmp;
    strContainer.retrieveString(tmp, resultStringIDs.at(i));
    cout << tmp << endl;
  }
  
  cout << "SAVING INDEX" << endl;
  indexer.saveIndex("exampleindex.ix");

  cout << "LOADING INDEX" << endl;
  FtIndexerMem<> indexerLoaded(&strContainer);
  indexerLoaded.loadIndex("exampleindex.ix");
  
  resultStringIDs.clear();
  searcher.setFtIndexer(&indexerLoaded);
  searcher.search(query, resultStringIDs);
  cout << "SIMILAR STRINGS: " << endl;
  for(unsigned i = 0; i < resultStringIDs.size(); i++) {
    string tmp;
    strContainer.retrieveString(tmp, resultStringIDs.at(i));
    cout << tmp << endl;
  }

  cout << "----------------------" << endl << endl;
}

// builds in-memory index with length filter for partitioning
// fills index from data file
// uses non-default list-merging algorithm, ScanCount
void memBasicUsage3() {  
  cout << "----- MEM BASIC USAGE 3 ----" << endl;

  // create gramgenerator and similarity metric
  GramGenFixedLen gramGen(2); // using fixed-length grams
  SimMetricJacc simMetric(gramGen); // using the jaccard distance (using set semantics)
  
  // create simple indexer with default template arguments
  // default: in-memory index using Array<unsigned> as an inverted list container
  // first create a string container and fill it with strings to index
  StringContainerVector strContainer;
  strContainer.fillContainer("data/female_names.txt", 4000); // fill the container from a datafile and use the first 4000 lines
  FtIndexerMem<> indexer(&strContainer, &gramGen, 20, 10); // maxStrLen=20, fanout=10
  indexer.addPartFilter(new LengthFilter(20)); // add length filtering with a maximum string length of 50
  indexer.buildIndex();
  
  // create merger
  ScanCountMerger<> merger(4000);
  // create searcher, specifying a non-default merger
  FtSearcherMem<FtIndexerMem<>, ScanCountMerger<> > searcher(&merger, &indexer);
  
  vector<unsigned> resultStringIDs;
  Query query("kathrin", simMetric, 0.7f);
  searcher.search(query, resultStringIDs);  
  cout << "SIMILAR STRINGS: " << endl;
  for(unsigned i = 0; i < resultStringIDs.size(); i++) {
    string tmp;
    strContainer.retrieveString(tmp, resultStringIDs.at(i));
    cout << tmp << endl;
  }
  
  cout << "SAVING INDEX" << endl;
  indexer.saveIndex("exampleindex.ix");

  cout << "LOADING INDEX" << endl;
  FtIndexerMem<> indexerLoaded(&strContainer);
  indexerLoaded.loadIndex("exampleindex.ix");
  
  resultStringIDs.clear();
  searcher.setFtIndexer(&indexerLoaded);
  searcher.search(query, resultStringIDs);  
  cout << "SIMILAR STRINGS: " << endl;
  for(unsigned i = 0; i < resultStringIDs.size(); i++) {
    string tmp;
    strContainer.retrieveString(tmp, resultStringIDs.at(i));
    cout << tmp << endl;
  }

  cout << "----------------------" << endl << endl;
}

// builds in-memory index with length filter for partitioning
// compresses index by discarding some inverted lists (based on impact tto given workload)
// fills index from existing collection
// other compressed indexers are:
// FtIndexerDiscardListsLLF - discard long lists first
// FtIndexerDiscardListsSLF - discard short lists first
// FtIndexerDiscardListsRandom - randomly discard lists first
// FtIndexerDiscardListsPanicCost - minimize number of panics
// FtIndexerDiscardListsTimeCost - minimize total running time
// FtIndexerCombineListsBasic - combine lists based on correlation
// FtIndexerCombineListsCost - combine lists based on total running time
void memDiscardLists() {
  cout << "----- MEM DISCARDLISTS ----" << endl;

  // create gramgenerator and similarity metric
  GramGenFixedLen gramGen(2);
  SimMetricEd simMetric(gramGen);
  
  // create index compressed with holes, using TimeCost as hole selection algorithm
  // default template parameters: in-memory index using Array<unsigned> as an inverted list
  StringContainerVector strContainer;
  strContainer.fillContainer(dictionary.begin(), dictionary.end());
  // params: string container, gram generator, compression ratio, training workload, training metric, training threshold, ratio cost, 
  //         data sampling fraction, queries sampling fraction, max sting length, fanout
  FtIndexerDiscardListsTimeCost<> 
    indexer(&strContainer, &gramGen, 0.5f, &dictionary, &simMetric, 1.0f, false, 0.01f, 0.25f, 20, 10);
  indexer.addPartFilter(new LengthFilter(20));
  indexer.buildIndex();
  
  // create merger
  DivideSkipMerger<> merger;
  // create searcher passing merger and indexer
  // must specify indexer type as template argument
  FtSearcherMem<FtIndexerDiscardListsTimeCost<> > searcher(&merger, &indexer);
  
  vector<unsigned> resultStringIDs;
  Query query("kathrin", simMetric, 1.0f);
  searcher.search(query, resultStringIDs);  
  cout << "SIMILAR STRINGS: " << endl;
  for(unsigned i = 0; i < resultStringIDs.size(); i++) {
    string tmp;
    strContainer.retrieveString(tmp, resultStringIDs.at(i));
    cout << tmp << endl;
  }
  
  cout << "SAVING INDEX" << endl;
  indexer.saveIndex("exampleindex.ix");

  cout << "LOADING INDEX" << endl;
  FtIndexerDiscardListsTimeCost<> indexerLoaded(&strContainer);
  indexerLoaded.loadIndex("exampleindex.ix");
  
  resultStringIDs.clear();
  searcher.setFtIndexer(&indexerLoaded);
  searcher.search(query, resultStringIDs);  
  cout << "SIMILAR STRINGS: " << endl;
  for(unsigned i = 0; i < resultStringIDs.size(); i++) {
    string tmp;
    strContainer.retrieveString(tmp, resultStringIDs.at(i));
    cout << tmp << endl;
  }
  
  cout << "----------------------------" << endl << endl;;
}

// builds in-memory index with length filter for partitioning
// compresses index by combining some inverted lists (based on impact tto given workload)
// fills index from existing collection
// other compressed indexers are:
// FtIndexerDiscardListsLLF - discard long lists first
// FtIndexerDiscardListsSLF - discard short lists first
// FtIndexerDiscardListsRandom - randomly discard lists first
// FtIndexerDiscardListsPanicCost - minimize number of panics
// FtIndexerDiscardListsTimeCost - minimize total running time
// FtIndexerCombineListsBasic - combine lists based on correlation
// FtIndexerCombineListsCost - combine lists based on total running time
void memCombineLists() {
  cout << "----- MEM COMPRESSION UNION ----" << endl;

  // create gramgenerator and similarity metric
  GramGenFixedLen gramGen(2);
  SimMetricEd simMetric(gramGen);
  
  // create index compressed with holes, using TimeCost as hole selection algorithm
  // default template parameters: in-memory index using Array<unsigned> as an inverted list
  StringContainerVector strContainer;
  strContainer.fillContainer(dictionary.begin(), dictionary.end());
  // params: string container, gram generator, compression ratio, training workload, training metric, training threshold,  
  //         data sampling fraction, queries sampling fraction, max sting length, fanout
  FtIndexerCombineListsCost<> 
    indexer(&strContainer, &gramGen, 0.5f, &dictionary, &simMetric, 1.0f, 0.01f, 1.0f, 20, 10);
  indexer.addPartFilter(new LengthFilter(20));
  indexer.buildIndex();
  
  // create merger
  DivideSkipMerger<> merger;
  // create searcher passing merger and indexer
  // must specify indexer type as template argument
  FtSearcherMem< FtIndexerCombineListsCost<> > searcher(&merger, &indexer);
  
  vector<unsigned> resultStringIDs;
  Query query("kathrin", simMetric, 1.0f);
  searcher.search(query, resultStringIDs);  
  cout << "SIMILAR STRINGS: " << endl;
  for(unsigned i = 0; i < resultStringIDs.size(); i++) {
    string tmp;
    strContainer.retrieveString(tmp, resultStringIDs.at(i));
    cout << tmp << endl;
  }
  
  cout << "SAVING INDEX" << endl;
  indexer.saveIndex("exampleindex.ix");

  cout << "LOADING INDEX" << endl;
  FtIndexerCombineListsCost<> indexerLoaded(&strContainer);
  indexerLoaded.loadIndex("exampleindex.ix");
  
  resultStringIDs.clear();
  searcher.setFtIndexer(&indexerLoaded);
  searcher.search(query, resultStringIDs);  
  cout << "SIMILAR STRINGS: " << endl;
  for(unsigned i = 0; i < resultStringIDs.size(); i++) {
    string tmp;
    strContainer.retrieveString(tmp, resultStringIDs.at(i));
    cout << tmp << endl;
  }
  
  cout << "----------------------------" << endl << endl;;
}

// builds in-memory index with length filter for partitioning
// compresses index by discarding some inverted lists (based on impact tto given workload)
// fills index from existing collection
// uses non-default list-merging algorithm, ScanCount
// explicitly specifies template parameters, and constructor arguments
void memAdvancedUsage() {
  cout << "----- MEM ADVANCED USAGE ----" << endl;

  // create gramgenerator and similarity metric
  GramGenFixedLen gramGen(2);
  SimMetricEd simMetric(gramGen);
  
  // create index compressed with holes, using TimeCost as hole selection algorithm 
  // and specifiying the non-default string container and Array<unsigned> as inverted list
  StringContainerRM strContainer;
  strContainer.createContainer("tmpcollection.txt");
  strContainer.openContainer("tmpcollection.txt");  
  strContainer.fillContainer(dictionary.begin(), dictionary.end());
  // params: string container, gram generator, compression ratio, training workload, training metric, training threshold, ratio cost, 
  //         data sampling fraction, queries sampling fraction, max sting length, fanout
  FtIndexerDiscardListsTimeCost<StringContainerRM, Array<unsigned> > 
    indexer(&strContainer, &gramGen, 0.5f, &dictionary, &simMetric, 2.0f, false, 0.01f, 0.25f, 20, 10);
  indexer.addPartFilter(new LengthFilter(20));
  indexer.buildIndex();
  
  // create merger specifying inverted list type
  ScanCountMerger<Array<unsigned> > merger(dictionary.size());
  // create searcher passing merger and indexer
  // specify all template arguments, need to specify merger if not default (DivideSkipMerger)
  FtSearcherMem<FtIndexerDiscardListsTimeCost<StringContainerRM, Array<unsigned> >, ScanCountMerger<Array<unsigned> > > searcher(&merger, &indexer);
  
  vector<unsigned> resultStringIDs;
  Query query("kathrin", simMetric, 1.0f);
  searcher.search(query, resultStringIDs);  
  cout << "SIMILAR STRINGS: " << endl;
  for(unsigned i = 0; i < resultStringIDs.size(); i++) {
    string tmp;
    strContainer.retrieveString(tmp, resultStringIDs.at(i));
    cout << tmp << endl;
  }
  
  cout << "SAVING INDEX" << endl;
  indexer.saveIndex("exampleindex.ix");

  cout << "LOADING INDEX" << endl;
  FtIndexerDiscardListsTimeCost<StringContainerRM, Array<unsigned> > indexerLoaded(&strContainer);
  indexerLoaded.loadIndex("exampleindex.ix");
  
  resultStringIDs.clear();
  searcher.setFtIndexer(&indexerLoaded);
  searcher.search(query, resultStringIDs);  
  cout << "SIMILAR STRINGS: " << endl;
  for(unsigned i = 0; i < resultStringIDs.size(); i++) {
    string tmp;
    strContainer.retrieveString(tmp, resultStringIDs.at(i));
    cout << tmp << endl;
  }
  
  cout << "-------------------------" << endl << endl;
}

// builds disk-based index on disk-based string collection
// uses length filter for partitioning, and default list-merging algorithm
// fills index from existing collection
void ondiskBasicUsage1() {
  cout << "----- ONDISK BASIC USAGE 1 ----" << endl;

  // create gramgenerator and similarity metric
  GramGenFixedLen gramGen(2);
  SimMetricEd simMetric(gramGen);
  
  // using disk-based string container
  StringContainerRM strContainer(PHO_AUTO);
  strContainer.createAndOpen("collection.rm");
  strContainer.fillContainer(dictionary.begin(), dictionary.end()); // fill the container from dictionary
  // params: container, gramgen, disableStreamBuffer, index filename, bytes per run, max str len, fanout
  FtIndexerOnDisk<> indexer(&strContainer, &gramGen, false, "invlists.ix", 50000, 20, 10);
  indexer.addPartFilter(new LengthFilter(20)); // param: max str len
  indexer.buildIndex();
  
  // create merger specifying inverted list type
  OnDiskMergerSimple<> merger;
  // create searcher passing merger and indexer
  FtSearcherOnDisk<> searcher(&merger, &indexer);
  
  vector<unsigned> resultStringIDs;
  Query query("kathrin", simMetric, 1.0f);
  searcher.search(query, resultStringIDs);  
  cout << "SIMILAR STRINGS: " << endl;
  for(unsigned i = 0; i < resultStringIDs.size(); i++) {
    string tmp;
    strContainer.retrieveString(tmp, resultStringIDs.at(i));
    cout << tmp << endl;
  }
  
  cout << "SAVING INDEX" << endl;
  indexer.saveIndex("exampleindex.ix");

  cout << "LOADING INDEX" << endl;
  FtIndexerOnDisk<> indexerLoaded(&strContainer);
  indexerLoaded.loadIndex("exampleindex.ix");
  
  resultStringIDs.clear();
  searcher.setFtIndexer(&indexerLoaded);
  searcher.search(query, resultStringIDs);  
  cout << "SIMILAR STRINGS: " << endl;
  for(unsigned i = 0; i < resultStringIDs.size(); i++) {
    string tmp;
    strContainer.retrieveString(tmp, resultStringIDs.at(i));
    cout << tmp << endl;
  }
  
  cout << "-------------------------" << endl << endl;
}


// builds disk-based index on disk-based string collection
// uses length filter for partitioning
// uses adaptive list-merging algorithm
// fills index from existing collection
void ondiskBasicUsage2() {
  cout << "----- ONDISK BASIC USAGE 2 ----" << endl;

  // create gramgenerator and similarity metric
  GramGenFixedLen gramGen(2);
  SimMetricEd simMetric(gramGen);
  
  // using disk-based string container
  StringContainerRM strContainer(PHO_AUTO);
  strContainer.createAndOpen("collection.rm");
  strContainer.fillContainer("data/female_names.txt", 4000); // fill the container from a datafile and use the first 4000 lines
  // params: container, gramgen, disableStreamBuffer, index filename, bytes per run, max str len, fanout
  FtIndexerOnDisk<> indexer(&strContainer, &gramGen, false, "invlists.ix", 50000, 20, 10);
  indexer.autoAddPartFilter(); // automatically choose a partitioning filter based on container stats
  indexer.buildIndex();
  
  // create merger specifying inverted list type
  OnDiskMergerAdapt<> merger;
  // create searcher passing merger and indexer
  FtSearcherOnDisk<FtIndexerOnDisk<>, OnDiskMergerAdapt<> > searcher(&merger, &indexer);
  
  vector<unsigned> resultStringIDs;
  Query query("kathrin", simMetric, 1.0f);
  searcher.search(query, resultStringIDs);  
  cout << "SIMILAR STRINGS: " << endl;
  for(unsigned i = 0; i < resultStringIDs.size(); i++) {
    string tmp;
    strContainer.retrieveString(tmp, resultStringIDs.at(i));
    cout << tmp << endl;
  }
  
  cout << "SAVING INDEX" << endl;
  indexer.saveIndex("exampleindex.ix");

  cout << "LOADING INDEX" << endl;
  FtIndexerOnDisk<> indexerLoaded(&strContainer);
  indexerLoaded.loadIndex("exampleindex.ix");
  
  resultStringIDs.clear();
  searcher.setFtIndexer(&indexerLoaded);
  searcher.search(query, resultStringIDs);  
  cout << "SIMILAR STRINGS: " << endl;
  for(unsigned i = 0; i < resultStringIDs.size(); i++) {
    string tmp;
    strContainer.retrieveString(tmp, resultStringIDs.at(i));
    cout << tmp << endl;
  }
  
  cout << "-------------------------" << endl << endl;
}
