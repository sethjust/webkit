/*
 *  JSLabel.cpp
 *  JavaScriptCore
 *
 *  Created by Alan Cleary on 6/23/11.
 *  Copyright 2011 Alan Cleary and Seth Just. All rights reserved.
 *
 */

#include "config.h"
#include "JSLabel.h"

namespace JSC {
	
	// JSLabel constructor
	JSLabel::JSLabel() {
		label = new long;
		*label = 0;
	}
	
	// JSLabel constructor overload; Takes value
	JSLabel::JSLabel(long l) {
		label = new long;
		*label = l;
	}
	
	// Get labels value
	long JSLabel::Val() const {
		return *label;
	}
	
	// return new label with OR'd vals
	JSLabel JSLabel::Join(JSLabel l) {
		return *new JSLabel(l.Val() | this->Val());
	}
	
}
