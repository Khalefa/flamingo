/*
  $Id$

  Copyright (C) 2010 by The Regents of the University of California
	
  Redistribution of this file is permitted under
  the terms of the BSD license.
    
  Date: 09/23/2008
  Author: Alexander Behm <abehm (at) ics.uci.edu>
*/

#ifndef _ondiskmergerabs_h_
#define _ondiskmergerabs_h_

#include "filtertree/gramlist.h"
#include "filtertree/gramlistondisk.h"
#include "filtertree/statsutil.h"
#include "filtertree/stringcontainer.h"
#include "common/query.h"
#include "common/filtertypes.h"

template <class OnDiskMergerConcrete, class InvList>
class OnDiskMergerAbs {

protected:
  GramGen* gramGen;
  
public:   
  bool singleFilterOpt;
  StringContainerRM* strContainer;   // TODO: JUST FOR EXPERIMENTS, REMOVE LATER
  unordered_map<unsigned, string>* gramcode2gram;
  
  unsigned numberStrings;
  unsigned numberSeeks;

  QueryCharsumStats* queryCharsumStats;
  StrCharsumStats* charsumStats;
  CharsumFilter* charsumFilter;

  OnDiskMergerAbs(bool singleFilterOpt = true) { 
    this->singleFilterOpt = singleFilterOpt;
    numberStrings = 1;
    gramGen = NULL;
    queryCharsumStats = NULL;
    charsumStats = NULL;
    charsumFilter = NULL;
    numberSeeks = 0;
  }
  
  bool estimationParamsNeeded() {
    return static_cast<OnDiskMergerConcrete*>(this)->estimationParamsNeeded_Impl();
  }
  
  void setEstimationParams(float readInvListTimeSlope, float readInvListTimeIntercept, float readStringAvgTime) { 
    static_cast<OnDiskMergerConcrete*>(this)->setEstimationParams_Impl(readInvListTimeSlope, readInvListTimeIntercept, readStringAvgTime);
  }

  void setGramGen(GramGen* gramGen) { 
    this->gramGen = gramGen;
  }
  
  void merge(const Query& query,
	     vector<vector<QueryGramList<InvList>* >* >& qgls,
	     unsigned threshold,
	     fstream& invListsFile,
	     unsigned numberFilters,
	     vector<unsigned>& results) {
    static_cast<OnDiskMergerConcrete*>(this)->merge_Impl(query, qgls, threshold, invListsFile, numberFilters, results);
  }
  
  string getName() {
    return "OnDiskMergerAbs";
  }
};

#endif
