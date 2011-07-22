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

FlowGraph* StaticAnalyzer::createFlowGraph(CodeBlock* codeBlock) {
  Instruction* begin = codeBlock->instructions().begin();
  Instruction* vPC = begin;

  FlowGraph* graph = new FlowGraph();

  while (vPC < codeBlock->instructions().end()) {
    Opcode opcode = vPC->u.opcode;
    unsigned int pos = (int) (vPC-begin);
    unsigned int length = opcodeLengths[vPC->u.opcode];

    switch (opcode) {
      // Unconditional w/ offset in vPC[1]
      case op_jmp: 
        graph->add_edge(pos, pos + vPC[1].u.operand);
        break;

      // Conditional w/ single offset in vPC[2]
      case op_loop_if_true:
      case op_loop_if_false:
      case op_jtrue:
      case op_jfalse:
      case op_jeq_null:
      case op_jneq_null:
        graph->add_edge(pos, pos + vPC[2].u.operand);
        graph->add_edge(pos, pos+length);
        break;

      // Conditional w/ single offset in vPC[3]
      case op_jneq_ptr:
      case op_loop_if_less:
      case op_loop_if_lesseq:
      case op_jnless:
      case op_jless:
      case op_jnlesseq:
      case op_jlesseq:
        graph->add_edge(pos, pos + vPC[3].u.operand);
        graph->add_edge(pos, pos+length);
        break;

      // TODO: Switch tables need special treatment
      /*
      case op_switch_imm:
      case op_switch_char:
      case op_switch_string:
       */

      // End of method
      case op_end:
        graph->add_edge(pos, pos); // TODO: mark end node better?
        break;

      // Non-jumping/branching opcodes
      default:
        graph->add_edge(pos, pos+length);
        break;
    }

    vPC += length; // advance 1 opcode
  }
	
  return graph;

}

void StaticAnalyzer::DFS(FlowGraph*) {

}

ContextTable* StaticAnalyzer::genContextTable(CodeBlock* codeBlock) {
  FlowGraph* graph = createFlowGraph(codeBlock);
  
  if (ADEBUG)
    graph->dump();

  //ContextTable* table = new ContextTable;

  //return table;
  return contextTable;

}

}
