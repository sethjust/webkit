/*
 *  URLEntry.cpp
 *  JavaScriptCore
 *
 *  Created by Alan Cleary on 7/7/11.
 *  Copyright 2011 Alan Cleary and Seth Just. All rights reserved.
 *
 */

#include "URLEntry.h"

namespace JSC {

	URLEntry::URLEntry(const char* src, long value) {
//		printf("URLEntry recived %s\n", src);
		url = strdup(src);
//		printf("URLEntry stored %s as %s\n", src, url);
		this->value = value;
	}
	
	const char* URLEntry::getURL() const {
		//printf("URLEntry is returning url %s\n", url);
		return url;
	}
	
	long URLEntry::getValue() {
		return value;
	}
	
}
