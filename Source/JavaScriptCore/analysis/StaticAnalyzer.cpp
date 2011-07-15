/*
 *  StaticAnalyzer.cpp
 *  JavaScriptCore
 *
 *  Created by Seth Just on 7/15/11.
 *  Copyright 2011 Alan Cleary and Seth Just. All rights reserved.
 *
 */

#include "StaticAnalyzer.h"

namespace JSC {

StaticAnalyzer::StaticAnalyzer(){ }

void StaticAnalyzer::genContextTable(CodeBlock* codeBlock) {
  Vector<Instruction> instructions = codeBlock->instructions();
  printf("genContextTable called on instructions starting at %lx\n", (long) instructions.begin());
}

}
