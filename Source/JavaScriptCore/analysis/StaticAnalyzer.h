/*
 *  StaticAnalyzer.h
 *  JavaScriptCore
 *
 *  Created by Seth Just on 7/15/11.
 *  Copyright 2011 Alan Cleary and Seth Just. All rights reserved.
 *
 */

#ifndef StaticAnalyzer_h
#define StaticAnalyzer_h

#include "config.h"
#include "FlowGraph.h"

namespace JSC {

class CodeBlock;

class StaticAnalyzer {
  
public:
  StaticAnalyzer();
  void genContextTable(CodeBlock*);
};

}

#endif // StaticAnalyzer_h
