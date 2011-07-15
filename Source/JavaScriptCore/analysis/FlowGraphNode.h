/*
 *  FlowGraphNode.h
 *  JavaScriptCore
 *
 *  Created by Seth Just on 7/15/11.
 *  Copyright 2011 Alan Cleary and Seth Just. All rights reserved.
 *
 */

#include "config.h"

#ifndef FlowGraphNode_h
#define FlowGraphNode_h

namespace JSC {

class FlowGraphNode {
  FlowGraphNode* children[]; // pointer array to children in graph
  
public:
  FlowGraphNode();
  FlowGraphNode(int n); // constructor
};

}

#endif // FlowGraphNode_h
