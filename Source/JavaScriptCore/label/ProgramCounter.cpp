/*
 *  ProgramCounter.cpp
 *  JavaScriptCore
 *
 *  Created by Alan Cleary on 6/24/11.
 *  Copyright 2011 Alan Cleary and Seth Just. All rights reserved.
 *
 */

#include "ProgramCounter.h"

namespace JSC {
	
	ProgramCounter::ProgramCounter() {
		node = new PCNode(NULL, JSLabel());
	}
	
	void ProgramCounter::Push(JSLabel l) {
		node = new PCNode(node, l);
	}
	
	void ProgramCounter::Pop() {
		PCNode *temp = node;
		node = temp->Next();
	}
	
	JSLabel ProgramCounter::Head() {
		return node->Val();
	}
}
