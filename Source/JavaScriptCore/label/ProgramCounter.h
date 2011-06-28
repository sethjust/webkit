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
	
	class ProgramCounter {
		PCNode *node; // current node
	public:
		ProgramCounter(); // constructor
		void Push(JSLabel); // push node to stack
		JSLabel Pop(); // remove and return head
		JSLabel Head(); // return head
	};
}

#endif // ProgramCounter_h
