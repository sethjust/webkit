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

#include "JSValue.h"

namespace JSC {
	
	class ProgramCounter {
		ProgramCounter *node; // pointer to other conuters
		JSValue value; // valuer of node
		
	public:
		ProgramCounter(ProgramCounter, JSValue); // constructor
		void Push(JSValue);
		void Pop(JSValue);
	};
}

#endif // ProgramCounter_h
