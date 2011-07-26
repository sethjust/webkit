/*
 *  FlowGraph.cpp
 *  JavaScriptCore
 *
 *  Created by Seth Just on 7/15/11.
 *  Copyright 2011 Utah State University. All rights reserved.
 *
 */

#include "FlowGraph.h"
#include "CodeBlock.h"

namespace JSC {

  AListNode::AListNode() {
    m_next = NULL;
    m_edge.from = 0;
    m_edge.to = 0;
    m_edge.inDFS = 0;
  }
  AListNode::AListNode( int from, int to ){
    m_next = NULL;
    m_edge.from = from;
    m_edge.to = to;
    m_edge.inDFS = 0;
  }
  void AListNode::set_next(AListNode* nnext) {
    m_next = nnext;
  }
  void AListNode::dump() {
    if (m_edge.inDFS) printf("edge from %d to %d is in DFS\n", m_edge.from, m_edge.to);
    else printf("edge from %d to %d\n", m_edge.from, m_edge.to);
    if (m_next != NULL) {
      m_next->dump();
    }
  }

  FlowGraph::FlowGraph(CodeBlock* cb, bool* branch) {
    // Initialize instance variables
    codeBlock = cb;
    head = new AListNode();
    count = codeBlock->instructionCount();
    
    // Zero all branch flags
    for (int i=0; i<count; i++) { branch[i] = false; }

    // Initialize locals
    Instruction* begin = codeBlock->instructions().begin();
    Instruction* vPC = begin;

    // Loop over opcodes to build CFG
    while (vPC < codeBlock->instructions().end()) {
      Opcode opcode = vPC->u.opcode;
      int pos = (long) (vPC-begin);
      int length = opcodeLengths[vPC->u.opcode];

      switch (opcode) {
        // Unconditional w/ offset in vPC[1]
        case op_jmp: 
          add_edge(pos, pos + vPC[1].u.operand);
          break;

        // Conditional w/ single offset in vPC[2]
        case op_loop_if_true:
        case op_loop_if_false:
        case op_jtrue:
        case op_jfalse:
        case op_jeq_null:
        case op_jneq_null:
          add_edge(pos, pos + vPC[2].u.operand);
          add_edge(pos, pos+length);
          branch[pos] = true;
          break;

        // Conditional w/ single offset in vPC[3]
        case op_jneq_ptr:
        case op_loop_if_less:
        case op_loop_if_lesseq:
        case op_jnless:
        case op_jless:
        case op_jnlesseq:
        case op_jlesseq:
          add_edge(pos, pos + vPC[3].u.operand);
          add_edge(pos, pos+length);
          branch[pos] = true;
          break;

        // TODO: Switch tables need special treatment
        /*
        case op_switch_imm:
        case op_switch_char:
        case op_switch_string:
         */

        // End of method
        case op_end:
          //add_edge(pos, pos); // TODO: mark end node better?
          break;

        // Non-jumping/branching opcodes
        default:
          add_edge(pos, pos+length);
          break;
      }
      vPC += length; // advance 1 opcode
    }
  }

  void FlowGraph::add_edge( int from, int to){
    AListNode* nnode = new AListNode(from, to);
    nnode->set_next(head);
    head = nnode;
  }

  int FlowGraph::buildDFS(int vertex[], int semi[]) {
    
    int* curIdx = new int;
    *curIdx = 1;

    bool visited[count];
    for (int i=0; i<count; i++) {visited[i]=0;}
    
    DFS((int) (codeBlock->instructions().end() - codeBlock->instructions().begin() - OPCODE_LENGTH(op_end)),
        vertex,
        curIdx,
        semi,
        visited);

    return *curIdx - 1;

  }

  void FlowGraph::DFS(int node, int vertex[], int* curIdx, int semi[], bool visited[]) {
    vertex[*curIdx] = node;
    semi[node] = *curIdx;
    *curIdx += 1;
    visited[node] = 1;

    AListNode* current = head;
    while (current) {
      edge_t* edge = current->edge();
      if ((edge->to == node) & (!visited[edge->from])){
        edge->inDFS = 1;
        DFS(edge->from, vertex, curIdx, semi, visited);
      }

      current = current->next();
    }
  }

  void FlowGraph::dump() {
    printf("\nCFG has\n");
    head->dump();
  }

}

