/*
 *  StaticAnalyzer.h
 *  JavaScriptCore
 *
 *  Created by Seth Just on 7/15/11.
 *  Copyright 2011 Utah State University. All rights reserved.
 *
 */

#ifndef StaticAnalyzer_h
#define StaticAnalyzer_h

#include "config.h"
#include "FlowGraph.h"

namespace JSC {

class CodeBlock;

class StaticAnalyzer {
  FlowGraph* createFlowGraph(CodeBlock*);
  int* idom;
public:
  StaticAnalyzer();
  void genContextTable(CodeBlock*);
  int IDom(int node) { return idom[node]; }
  int count;
};

}

#endif // StaticAnalyzer_h
