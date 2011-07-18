/*
 *  FlowGraph.cpp
 *  JavaScriptCore
 *
 *  Created by Seth Just on 7/15/11.
 *  Copyright 2011 Alan Cleary and Seth Just. All rights reserved.
 *
 */

#include "FlowGraph.h"

namespace JSC {

  AListNode::AListNode() {
    m_next = NULL;
    m_edge.from = 0;
    m_edge.to = 0;
  }
  AListNode::AListNode( unsigned int from, unsigned int to ){
    m_next = NULL;
    m_edge.from = from;
    m_edge.to = to;
  }
  void AListNode::set_next(AListNode* nnext) {
    m_next = nnext;
  }
  void AListNode::dump() {
    printf("edge from %d to %d\n", m_edge.from, m_edge.to);
    if (m_next != NULL) {
      m_next->dump();
    }
  }

  FlowGraph::FlowGraph() {
    head = new AListNode();
    tail = head;
  }
  void FlowGraph::add_edge( unsigned int from, unsigned int to){
    AListNode* nnode = new AListNode(from, to);
    tail->set_next(nnode);
    tail = nnode;
  }
  void FlowGraph::dump() {
    printf("\nCFG has:\n");
    head -> dump();
    printf("\n");
  }

}
