/*
 *  PCNode.cpp
 *  JavaScriptCore
 *
 *  Created by Alan Cleary on 6/28/11.
 *  Copyright 2011 Alan Cleary and Seth Just. All rights reserved.
 *
 */

#include "PCNode.h"
#include "config.h"

namespace JSC {
	
	PCNode::PCNode(PCNode *n, JSLabel l, int i, Register* r) {
		node = n;
		value = l;
		loc = i;
		reg = r;
	}
	
	JSLabel PCNode::Val() {
		return value;
	}
	
	int PCNode::Loc() {
		return loc;
	}
	
	PCNode* PCNode::Next() {
		return node;
	}
}
