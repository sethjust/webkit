/*
 *  FlowGraph.h
 *  JavaScriptCore
 *
 *  Created by Seth Just on 7/15/11.
 *  Copyright 2011 Utah State University. All rights reserved.
 *
 */

#ifndef FlowGraph_h
#define FlowGraph_h

#include "config.h"

typedef struct {
  int from;
  int to;
  bool inDFS;
} edge_t;

namespace JSC {
  
class CodeBlock; // To make includes all work properly -- we get errors if we straight-up #include CodeBlock.h

// We store the CFG as an Adjacency list -- a linked list of edges

class AListNode {
  AListNode* m_next;
  edge_t m_edge;
  public:
    AListNode();
    AListNode( int from, int to );
    AListNode* next() { return m_next; }
    edge_t* edge() { return &m_edge; }
    void set_next(AListNode* nnext);
    void dump();
};

class FlowGraph {
  AListNode* head;
  CodeBlock* codeBlock;
  void add_edge( int from, int to);
  void DFS(int node, int vertex[], int* curIdx, int semi[], bool visited[]);
  
  public:
    int count;
    AListNode* Head() { return head; }
    FlowGraph(CodeBlock* cb);
    CodeBlock* code_block() { return codeBlock; }
    int Count() { return count; }
    int buildDFS(int vertex[], int semi[]);
    void dump();
};

}

#endif // FlowGraph_h











