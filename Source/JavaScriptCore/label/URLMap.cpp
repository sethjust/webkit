/*
 *  URLMap.cpp
 *  JavaScriptCore
 *
 *  Created by Alan Cleary on 7/7/11.
 *  Copyright 2011 Alan Cleary and Seth Just. All rights reserved.
 *
 */

#include "URLMap.h"
#include "config.h"

namespace JSC {
	
	URLMap* URLMap::s_map = NULL; // initialize pointer
	
	URLMap::URLMap() {
		map = new URLEntry*[MAP_SIZE]; // create our "map"
		for (int i = 0; i < MAP_SIZE; i++) { // initialize map
			map[i] = NULL;
		};
		idx = 1; // initialize the current index
		// these are put manually for quick initialization and to avoid filter exceptions
		map[0] = new URLEntry("!", 1); // fallback given when map is full
		map[1] = new URLEntry("NULL", 2); // create generic value for NULL urls
	}
	
	URLMap& URLMap::urlmap() {
		if (s_map == NULL) { // is this the first call?
			s_map = new URLMap(); // create sole instance
		}
		return *s_map; // return pointer to instance
	}	
	
	// -----------Private----------- //
	
	void URLMap::append(const char* url) { // append url to map
		// NOTE: any call to append should check capacity() first
		idx++; // move current map lcoation
		map[idx] = new URLEntry(url, (long)2<<idx-1); // set map head to new URLEntry
	}
	
	int URLMap::search(const char* url) { // is the url already in the map?
		for (int i = idx; i > 0; i--) { // iterate from current index backwards
			if (strcmp(map[i]->getURL(), url) == 0) { // is it this one?
				return i; // return url index
			}
		}
		return OUT_OF_BOUNDS; // if the url isn't in the map
	}
	
	const char* URLMap::filter(const char* url) { // not implemented, for later use
		char str[5]; // create temporary string
		strncpy(str, url, 4); // set the string
		str[4] = '\0';
		if (strcmp(str, "http") == 0 || strcmp(str, "Inte") == 0) { // what's the urls origin?
			return url;
		}
		else { // this could potentially become recursive
			return "NULL";
		}
	}
	
	// -----------Public----------- //
	
	void URLMap::put(const char* url) { // put url on map
		url = strdup(filter(url));
		if (search(url) == OUT_OF_BOUNDS) { // if the url's not in the map add it
			if (capacity()) { // is the map full?
				append(url);
			}
		}
	}
	
	long URLMap::get(const char* url) { // get label for given url
		url = strdup(filter(url));
		int i = search(url);
		if (i == OUT_OF_BOUNDS) { // add to map and return value
			if (capacity()) { // is the map full?
				append(url);
				return head();
			}
			else { // return the fallback value
				return map[1]->getValue(); 
			}
		}
		else { // return value from list
			return map[i]->getValue();
		}
	}
	
	long URLMap::head() { // returns head of map
		return map[idx]->getValue();
	}
	
	char * URLMap::sHead() {
		char* temp;
		sprintf(temp, "%ld", URLMap::urlmap().head());
		return temp;
	}
	
	URLMap::~URLMap() { // deconstructor
		for (int i = idx; i >= 0; i--) { // delete non null entries
			delete map[i];
		}
		delete[] map; // delete the map :(
	}
	
}
