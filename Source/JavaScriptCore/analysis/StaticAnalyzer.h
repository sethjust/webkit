/*
 *  StaticAnalyzer.h
 *  JavaScriptCore
 *
 *  Created by Seth Just on 7/15/11.
 *  Copyright 2011 Alan Cleary and Seth Just. All rights reserved.
 *
 */

#include "config.h"
#include "FlowGraphNode.h"
#include "CodeBlock.h"
#include "Instruction.h"

#ifndef StaticAnalyzer_h
#define StaticAnalyzer_h

namespace JSC {

class StaticAnalyzer {
  
public:
  StaticAnalyzer();
  void genContextTable(CodeBlock*);
};

}

#endif // StaticAnalyzer_h
