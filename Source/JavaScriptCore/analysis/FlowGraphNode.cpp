/*
 *  FlowGraphNode.cpp
 *  JavaScriptCore
 *
 *  Created by Seth Just on 7/15/11.
 *  Copyright 2011 Alan Cleary and Seth Just. All rights reserved.
 *
 */

#include "FlowGraphNode.h"

namespace JSC {

FlowGraphNode::FlowGraphNode(){
  *children = NULL;
}

FlowGraphNode::FlowGraphNode(int n){ // constructor
  *children = new FlowGraphNode[n];
}

}
