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

#define ADEBUG 0

namespace JSC {

StaticAnalyzer::StaticAnalyzer(){ }

void StaticAnalyzer::DFS(FlowGraph*) {

}

ContextTable* StaticAnalyzer::genContextTable(CodeBlock* codeBlock) {
  FlowGraph graph = FlowGraph(codeBlock);

  int count = graph.Count();

  int semi[count];
  for (int i=0; i<count; i++) { semi[i] = 0; }
  int vertex[count];
  for (int i=0; i<count; i++) { vertex[i] = 0; }

  int lastIdx = graph.buildDFS(vertex, semi);
  
  if (ADEBUG)
    graph.dump();

  for (int i=lastIdx; i>1; i--) {
    int node = vertex[i];
    int minv = i;

    AListNode* current = graph.Head();
    while (current) {
      edge_t* edge = current->edge();
      if ((int) edge->from == node){
        if (semi[edge->to] < i) {
          minv = semi[edge->to];
        }
      }

      current = current->next();
    }
    semi[node] = minv;
  }

  printf("semi has\n");
  for (int i=0; i<count; i++) {
    if (semi[i]) printf("%d\t%d\n", i, semi[i]);
  }
  printf("vertex has\n");
  for (int i=1; i<=lastIdx; i++) {
    printf("%d\t%d\n", i, vertex[i]);
  }

  ContextTable* table = new ContextTable;

  return table;

}

}
