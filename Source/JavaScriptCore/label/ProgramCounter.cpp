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
		//Location is set to -1 to ensure we never pop the sentinel node, as we will never execute the opcode at location -1
		node = new PCNode(NULL, JSLabel(), -1, NULL);
		len = 0;
	}
	
	void ProgramCounter::Push(JSLabel l, int i, Register* r) {
		node = new PCNode(node, Head().Join(l), i, r);
		len++;
	}
	
	void ProgramCounter::Pop() {
		PCNode *temp = node;
		node = temp->Next();
		len--;
	}
	
	JSLabel ProgramCounter::Head() {
		return node->Val();
	}
	
	int ProgramCounter::Loc() {
		return node->Loc();
	}
	
	int ProgramCounter::Len(){
		return len;
	}
	
	void ProgramCounter::Join(JSLabel label) {
		node = new PCNode(node->Next(), node->Val().Join(label), node->Loc(), node->reg);
	}
	
	Register* ProgramCounter::Reg() {
		return node->reg;
	}
}
