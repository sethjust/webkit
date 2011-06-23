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
	
	/*
	 // Deconstruct JSLabel
	 JSLabel::~JSLabel() {
	 delete label;
	 }
	 */
	
	// Get labels value
	long JSLabel::Val() const {
		return *label;
	}
	
	// OR two labels
	JSLabel JSLabel::Join(JSLabel l) {
		return JSLabel (l.Val() | this->Val());
	}
	
}
