/*
 *  ContextTable.h
 *  JavaScriptCore
 *
 *  Created by Seth Just on 7/19/11.
 *  Copyright 2011 Utah State University. All rights reserved.
 *
 */

namespace JSC {
  typedef std::pair<int, bool> context_t;

  typedef context_t ContextTable[];
//  class ContextTable {
//    context_t m_table[];
//    public:
//    ContextTable(int n);
//  };

}
