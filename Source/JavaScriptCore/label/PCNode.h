/*
 *  PCNode.h
 *  JavaScriptCore
 *
 *  Created by Alan Cleary on 6/28/11.
 *  Copyright 2011 Alan Cleary and Seth Just. All rights reserved.
 *
 */

#ifndef PCNode_h
#define PCNode_h

#include "JSLabel.h"

namespace JSC {

	class PCNode {
		PCNode *node; // pointer to previous node
		JSLabel value; // node value
		int loc;
		
	public:
		PCNode(PCNode*, JSLabel, long); // constructor
		PCNode* Next(); // next node (for pop)
		JSLabel Val(); // return value
		long Loc(); 
	};
}

#endif // PCNode_h
