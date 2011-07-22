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
  bool* branch;
public:
  StaticAnalyzer();
  void genContextTable(CodeBlock*);
  std::pair<int, bool> Context(int);
};

}

#endif // StaticAnalyzer_h
