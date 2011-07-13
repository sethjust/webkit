/*
 *  URLEntry.h
 *  JavaScriptCore
 *
 *  Created by Alan Cleary on 7/7/11.
 *  Copyright 2011 Alan Cleary and Seth Just. All rights reserved.
 *
 */

#ifndef URLEntry_h
#define URLEntry_h

namespace JSC {

	class URLEntry {
	
	private:
		const char* url; // taken from scripts url UString
		long value; //generated based on map index
		
	public:
		URLEntry(const char*, long value); // takes url string and generated binary
		const char* getURL() const; // returns key of self
		long getValue(); // returns value of self
	};
	
}

#endif // URLEntry_h
