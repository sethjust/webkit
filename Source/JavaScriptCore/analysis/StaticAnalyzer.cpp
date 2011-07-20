/*
 *  StaticAnalyzer.cpp
 *  JavaScriptCore
 *
 *  Created by Seth Just on 7/15/11.
 *  Copyright 2011 Utah State University. All rights reserved.
 *
 */

#include "StaticAnalyzer.h"
#include "CodeBlock.h"
#include "Instruction.h"

#define ADEBUG 1

namespace JSC {

StaticAnalyzer::StaticAnalyzer(){ }

void StaticAnalyzer::DFS(FlowGraph*) {

}

ContextTable* StaticAnalyzer::genContextTable(CodeBlock* codeBlock) {
  FlowGraph graph = FlowGraph(codeBlock);
  
  if (ADEBUG)
    graph.dump();

  ContextTable* table = new ContextTable;

  return table;

}

}
