/*
 *  ProgramCounter.h
 *  JavaScriptCore
 *
 *  Created by Alan Cleary on 6/24/11.
 *  Copyright 2011 Alan Cleary and Seth Just. All rights reserved.
 *
 */

#ifndef ProgramCounter_h
#define ProgramCounter_h

#include "config.h"
#include "JSLabel.h"
#include "PCNode.h"

namespace JSC {
	class Register;
	
	class ProgramCounter {
		PCNode *node; // current node
		int len;
	public:
		ProgramCounter(); // constructor
		void Push(JSLabel, int, Register*); // push node to stack
		void Pop(); // remove and return head
		JSLabel Head(); // return head
		int Loc();
		int Len();
		void Join(JSLabel);
		Register* Reg();
	};
}

#endif // ProgramCounter_h
