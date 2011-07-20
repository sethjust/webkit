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
#include "ContextTable.h"

namespace JSC {

class CodeBlock;

class StaticAnalyzer {
  FlowGraph* createFlowGraph(CodeBlock*);
  void DFS(FlowGraph*);
public:
  StaticAnalyzer();
  ContextTable* genContextTable(CodeBlock*);
};

}

#endif // StaticAnalyzer_h
