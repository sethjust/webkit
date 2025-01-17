/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2003, 2010 Apple Inc. All rights reserved.
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

#include "config.h"
#include "HTMLDivElement.h"

#include "Attribute.h"
#include "CSSPropertyNames.h"
#include "CSSValueKeywords.h"
#include "DOMStringList.h"
#include "DocumentMarkerController.h"
#include "HTMLNames.h"
#include "SpellcheckRange.h"
#include "SpellcheckRangeList.h"

namespace WebCore {

using namespace HTMLNames;

HTMLDivElement::HTMLDivElement(const QualifiedName& tagName, Document* document)
    : HTMLElement(tagName, document)
{
    ASSERT(hasTagName(divTag));
}

PassRefPtr<HTMLDivElement> HTMLDivElement::create(Document* document)
{
    return adoptRef(new HTMLDivElement(divTag, document));
}

PassRefPtr<HTMLDivElement> HTMLDivElement::create(const QualifiedName& tagName, Document* document)
{
    return adoptRef(new HTMLDivElement(tagName, document));
}

bool HTMLDivElement::mapToEntry(const QualifiedName& attrName, MappedAttributeEntry& result) const
{
    if (attrName == alignAttr) {
        result = eBlock;
        return false;
    }
    return HTMLElement::mapToEntry(attrName, result);
}

void HTMLDivElement::parseMappedAttribute(Attribute* attr)
{
    if (attr->name() == alignAttr) {
        String v = attr->value();
        if (equalIgnoringCase(attr->value(), "middle") || equalIgnoringCase(attr->value(), "center"))
           addCSSProperty(attr, CSSPropertyTextAlign, CSSValueWebkitCenter);
        else if (equalIgnoringCase(attr->value(), "left"))
            addCSSProperty(attr, CSSPropertyTextAlign, CSSValueWebkitLeft);
        else if (equalIgnoringCase(attr->value(), "right"))
            addCSSProperty(attr, CSSPropertyTextAlign, CSSValueWebkitRight);
        else
            addCSSProperty(attr, CSSPropertyTextAlign, v);
    } else
        HTMLElement::parseMappedAttribute(attr);
}

#if ENABLE(SPELLCHECK_API)
PassRefPtr<SpellcheckRangeList> HTMLDivElement::spellcheckRanges()
{
    return document()->markers()->userSpellingMarkersForNode(this);
}

void HTMLDivElement::addSpellcheckRange(unsigned long start, unsigned long length)
{
    addSpellcheckRange(start, length, DOMStringList::create(), 0);
}

void HTMLDivElement::addSpellcheckRange(unsigned long start, unsigned long length, RefPtr<DOMStringList> suggestions, unsigned short options)
{
    document()->markers()->addUserSpellingMarker(this, start, length, suggestions, options);
}

void HTMLDivElement::removeSpellcheckRange(RefPtr<SpellcheckRange> range)
{
    if (!range)
        return;
    document()->markers()->removeUserSpellingMarker(this, range->start(), range->length());
}
#endif

}
