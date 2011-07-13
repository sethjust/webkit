/*
 *  URLMap.h
 *  JavaScriptCore
 *
 *  Created by Alan Cleary on 7/7/11.
 *  Copyright 2011 Alan Cleary and Seth Just. All rights reserved.
 *
 */

#ifndef URLMap_h
#define URLMap_h

#include "URLEntry.h"

namespace JSC {
	
	const int OUT_OF_BOUNDS = 100; // return instead of null
	const int MAP_SIZE = 64; // map size
	
	class URLMap {
	
	private:
		URLEntry **map; // the map!
		int idx; // current postition in map
		static URLMap* s_map; // member pointer to intself
		
		void append(const char*); // appends url onto map
		int search(const char*); // searches for url in the map and returns its index
		const char* filter(const char*); // filter urls
		inline bool capacity() { return (idx+1 < MAP_SIZE); }; // checks if the map is full
		
	public: // puts/gets will be fed with program->sourceURL().utf8().data() from the interpretor (subject to change for functions and evals)
		
		// ctor, copy-ctor, and assignment opperators
		URLMap(); // map constructor
		URLMap(URLMap const&);
		URLMap& operator = (URLMap const&);
		static URLMap& urlmap(); // validate member pointer
		
		long get(const char*); // takes script url as string and finds value
		void put(const char*); // puts script url string in map and generates a value
		long head(); // returns JSLabel at head of map
		~URLMap(); // deconstructor
	};
	
}

#endif //URLMap
