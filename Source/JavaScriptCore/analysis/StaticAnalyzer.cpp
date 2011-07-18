/*
 *  StaticAnalyzer.cpp
 *  JavaScriptCore
 *
 *  Created by Seth Just on 7/15/11.
 *  Copyright 2011 Alan Cleary and Seth Just. All rights reserved.
 *
 */

#include "StaticAnalyzer.h"
#include "CodeBlock.h"
#include "Instruction.h"

#define ADEBUG 0

namespace JSC {

StaticAnalyzer::StaticAnalyzer(){ }

void StaticAnalyzer::genContextTable(CodeBlock* codeBlock) {
  if (ADEBUG)
    printf("genContextTable called on instructions starting at %lx\n", (long) codeBlock->instructions().begin());

  Instruction* begin = codeBlock->instructions().begin();
  Instruction* vPC = begin;

  unsigned int count = codeBlock->instructionCount();

  while (vPC < codeBlock->instructions().end()) {
    Opcode opcode = vPC->u.opcode;
    int length = opcodeLengths[vPC->u.opcode];

    if (ADEBUG)
      printf("Opcode at pos %d is %d with length %d\n", (int) (vPC-begin), opcode, length );

    vPC += length; // advance 1 opcode
  }
}

}
