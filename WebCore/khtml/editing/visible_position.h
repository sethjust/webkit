/*
 * Copyright (C) 2004 Apple Computer, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#ifndef KHTML_EDITING_VISIBLE_POSITION_H
#define KHTML_EDITING_VISIBLE_POSITION_H

#include "xml/dom_position.h"
#include "text_affinity.h"

namespace DOM {
    class Range;
    class RangeImpl;
}

namespace khtml {

class VisiblePosition
{
public:
    typedef DOM::NodeImpl NodeImpl;
    typedef DOM::Position Position;

    VisiblePosition() { }
    VisiblePosition(NodeImpl *, long offset, EAffinity affinity=DOWNSTREAM);
    explicit VisiblePosition(const Position &, EAffinity affinity=DOWNSTREAM);

    void clear() { m_deepPosition.clear(); }

    bool isNull() const { return m_deepPosition.isNull(); }
    bool isNotNull() const { return m_deepPosition.isNotNull(); }

    Position position() const { return rangeCompliantEquivalent(m_deepPosition); }
    Position deepEquivalent() const { return m_deepPosition; }
    
    Position upstreamDeepEquivalent() const;

    friend bool operator==(const VisiblePosition &a, const VisiblePosition &b);

    VisiblePosition next() const;
    VisiblePosition previous() const;

    bool isLastInBlock() const;

    void debugPosition(const char *msg = "") const;

#ifndef NDEBUG
    void formatForDebugger(char *buffer, unsigned length) const;
#endif
    
private:
    void initUpstream(const Position &);
    void initDownstream(const Position &);

    static Position deepEquivalent(const Position &);
    static Position rangeCompliantEquivalent(const Position &);

    static long maxOffset(const NodeImpl *);
    static bool isAtomicNode(const NodeImpl *);
    
    static Position previousVisiblePosition(const Position &);
    static Position nextVisiblePosition(const Position &);

    static Position previousPosition(const Position &);
    static Position nextPosition(const Position &);

    static bool atStart(const Position &);
    static bool atEnd(const Position &);

    static bool isCandidate(const Position &);
    
    Position m_deepPosition;
};

inline bool operator==(const VisiblePosition &a, const VisiblePosition &b)
{
    return a.m_deepPosition == b.m_deepPosition;
}

inline bool operator!=(const VisiblePosition &a, const VisiblePosition &b)
{
    return !(a == b);
}

DOM::Range makeRange(const VisiblePosition &start, const VisiblePosition &end);
bool setStart(DOM::Range &, const VisiblePosition &start);
bool setStart(DOM::RangeImpl *, const VisiblePosition &start);
bool setEnd(DOM::Range &, const VisiblePosition &start);
bool setEnd(DOM::RangeImpl *, const VisiblePosition &start);
VisiblePosition startVisiblePosition(const DOM::Range &);
VisiblePosition startVisiblePosition(const DOM::RangeImpl *);
VisiblePosition endVisiblePosition(const DOM::Range &);
VisiblePosition endVisiblePosition(const DOM::RangeImpl *);

} // namespace khtml

#endif // KHTML_EDITING_VISIBLE_POSITION_H
