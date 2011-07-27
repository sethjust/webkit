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

#define ADEBUG false

namespace JSC {

StaticAnalyzer::StaticAnalyzer(){
}

void StaticAnalyzer::genContextTable(CodeBlock* codeBlock) {
  int count = codeBlock->instructionCount();

  // Generate the CFG
  FlowGraph graph = FlowGraph(codeBlock);

  // Create arrays to hold information from DFS. Note we need to initialize every entry to 0
  int semi[count];
  for (int i=0; i<count; i++) { semi[i] = 0; }
  int vertex[count];
  for (int i=0; i<count; i++) { vertex[i] = 0; }

  // Perform DFS
  int lastIdx = graph.buildDFS(vertex, semi);

  // Dump CFG, including DFS information
  if (ADEBUG)
    graph.dump();
  
  // Dump vertex numbering from DFS (vertex[] maps number->node)
  if (ADEBUG) {
    printf("vertex has\n");
    for (int i=1; i<=lastIdx; i++) {
      printf("%d\t%d\n", i, vertex[i]);
    }
  }

  // Semi currently maps node->number (before we run calculations on it); copy that, as we'll need it later
  int number[count];
  for (int i=0; i<count; i++) { number[i] = semi[i]; }

  // Calculate semidominators
  for (int i=lastIdx; i>1; i--) {
    int node = vertex[i];
    int minv = i;

    AListNode* current = graph.Head();
    while (current) {
      edge_t* edge = current->edge();
      if (edge->from == node){
        if (semi[edge->to] < minv) {
          minv = semi[edge->to];
        }
      }

      current = current->next();
    }
    semi[node] = minv;
  }

  // Dump semidominators
  if (ADEBUG) {
    printf("semi has\n");
    for (int i=0; i<count; i++) {
      if (semi[i]) printf("%d\t%d\n", i, semi[i]);
    }
  }
  
  // Create array to hold idom information
  idom = new int[count];
  for (int i=0; i<count; i++) { idom[i] = 0; }

  // Calculate idoms
  for (int i=2; i<=lastIdx; i++) {
    int node = vertex[i];

    int minsu = lastIdx;
    int u = 0;
    int cnode = node;
    while (1) { // Loop until we hit a child of semi[node] in our DFS traversal
      if (number[cnode] == semi[node]) break;
      if (minsu > semi[cnode]) {
        minsu = semi[cnode];
        u = cnode;
      }
      
      // Loop over list of edges to find the parent of cnode in DFS
      AListNode* current = graph.Head();
      while (current) {
        edge_t* edge = current->edge();
        if ((edge->from == cnode) & (edge->inDFS)) {
          cnode = edge->to; // Step to parent in DFS
          break;
        }
        current = current->next();
      }
    }
    if (semi[node] == minsu) idom[node] = minsu;  // first case of Corrollary 1
    else idom[node] = idom[u];
  }

  // Convert idoms from DFS numbers to nodes
  for (int i=0; i<count; i++) {
    idom[i] = vertex[idom[i]];
  }

  // Dump immediate dominators
  if (ADEBUG) {
    printf("idom has\n");
    for (int i=0; i<count; i++) {
      if (idom[i]) printf("%d\t%d\n", i, idom[i]);
    }
    printf("\n");
  }

}

}
