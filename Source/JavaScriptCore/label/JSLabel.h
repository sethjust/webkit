/*
 *  JSLabel.h
 *  JavaScriptCore
 *
 *  Created by Alan Cleary on 6/23/11.
 *  Copyright 2011 Alan Cleary and Seth Just. All rights reserved.
 *
 */

#ifndef JSLabel_h
#define JSLabel_h

namespace JSC {
	
	class JSLabel {
		long *label; // label's value
	public:
		JSLabel(); // constructor
		JSLabel(long); // overload
		long Val() const; // return label value
		JSLabel Join(JSLabel l); // join labels
	};
	
}

#endif // JSLabel_h
