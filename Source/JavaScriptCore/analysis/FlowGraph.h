/*
 *  FlowGraph.h
 *  JavaScriptCore
 *
 *  Created by Seth Just on 7/15/11.
 *  Copyright 2011 Alan Cleary and Seth Just. All rights reserved.
 *
 */

#include "config.h"

#ifndef FlowGraph_h
#define FlowGraph_h

typedef struct {
  unsigned int from;
  unsigned int to;
} edge_t;

namespace JSC {

// We store the CFG as an Adjacency list -- a linked list of edges

class AListNode {
  AListNode* m_next;
  edge_t m_edge;
  public:
    AListNode();
    AListNode( unsigned int from, unsigned int to );
    AListNode* next() { return m_next; }
    edge_t edge() { return m_edge; }
    void set_next(AListNode* nnext);
};

class FlowGraph {
  AListNode* head;
  AListNode* tail;
  public:
    FlowGraph() {}
    void add_edge( unsigned int from, unsigned int to);
};

}

#endif // FlowGraph_h
