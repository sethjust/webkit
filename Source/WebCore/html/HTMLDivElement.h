/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef HTMLDivElement_h
#define HTMLDivElement_h

#include "HTMLElement.h"

namespace WebCore {

#if ENABLE(SPELLCHECK_API)
class DOMStringList;
class SpellcheckRange;
class SpellcheckRangeList;
#endif

class HTMLDivElement : public HTMLElement {
public:
    static PassRefPtr<HTMLDivElement> create(Document*);
    static PassRefPtr<HTMLDivElement> create(const QualifiedName&, Document*);

#if ENABLE(SPELLCHECK_API)
    PassRefPtr<SpellcheckRangeList> spellcheckRanges();
    void addSpellcheckRange(unsigned long start, unsigned long length);
    void addSpellcheckRange(unsigned long start, unsigned long length, RefPtr<DOMStringList>, unsigned short options = 0);
    void removeSpellcheckRange(RefPtr<SpellcheckRange>);
#endif

protected:
    HTMLDivElement(const QualifiedName&, Document*);

private:
    virtual bool mapToEntry(const QualifiedName&, MappedAttributeEntry&) const;
    virtual void parseMappedAttribute(Attribute*);
};

} // namespace WebCore

#endif // HTMLDivElement_h
