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
		JSLabel value; // node value
		PCNode *node; // pointer to previous node
		
	public:
		PCNode(PCNode*, JSLabel); // constructor
		JSLabel Val(); // return value
		PCNode* Next(); // next node (for pop)
	};
}

#endif // PCNode_h
