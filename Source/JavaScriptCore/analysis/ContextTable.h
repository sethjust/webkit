/*
 *  ContextTable.h
 *  JavaScriptCore
 *
 *  Created by Seth Just on 7/19/11.
 *  Copyright 2011 Utah State University. All rights reserved.
 *
 */

#ifndef ContextTable_h
#define ContextTable_h

namespace JSC {
	/*

	
	
	class context_t {
	private:
		int loc;
		bool ipd;
	public:
		context_t();
		context_t(int, bool);
		inline int first() { return loc; };
		inline int second() { return ipd; };
	};
	
	class ContextTable {
		
		context_t table[];
		
	private:
	public:
		ContextTable(int);
		~ContextTable();
		context_t& operator[](int);
		
	};
	

	class ContextTable {
	private:
		context_t*;
	public:
		
	};

	class ContextTable {
	private:
		context_t* table;
	private:
		int operator[](int node) { return table[node]; };
		ContextTable & operator=(const ContextTable &rhs);
		void add(int, context_t);
	} contextTable;

  //typedef context_t *ContextTable;
//  class ContextTable {
//    context_t m_table[];
//    public:
//    ContextTable(int n);
//  };
*/
}

#endif // ContextTable_h
