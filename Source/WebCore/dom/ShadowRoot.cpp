/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "ShadowRoot.h"
#include "Element.h"

#include "Document.h"
#include "NodeRareData.h"
#include "ShadowContentSelector.h"

namespace WebCore {

ShadowRoot::ShadowRoot(Document* document)
    : TreeScope(document)
{
    ASSERT(document);
    
    // Assume document as parent scope.
    setParentTreeScope(document);
    // Shadow tree scopes have the scope pointer point to themselves.
    // This way, direct children will receive the correct scope pointer.
    ensureRareData()->setTreeScope(this);
}

ShadowRoot::~ShadowRoot()
{
}

String ShadowRoot::nodeName() const
{
    return "#shadow-root";
}

Node::NodeType ShadowRoot::nodeType() const
{
    return SHADOW_ROOT_NODE;
}

PassRefPtr<Node> ShadowRoot::cloneNode(bool)
{
    // ShadowRoot should not be arbitrarily cloned.
    return 0;
}

bool ShadowRoot::childTypeAllowed(NodeType type) const
{
    switch (type) {
    case ELEMENT_NODE:
    case PROCESSING_INSTRUCTION_NODE:
    case COMMENT_NODE:
    case TEXT_NODE:
    case CDATA_SECTION_NODE:
    case ENTITY_REFERENCE_NODE:
        return true;
    default:
        return false;
    }
}

void ShadowRoot::recalcStyle(StyleChange change)
{
    if (hasContentElement())
        reattach();
    else {
        for (Node* n = firstChild(); n; n = n->nextSibling())
            n->recalcStyle(change);
    }

    clearNeedsStyleRecalc();
    clearChildNeedsStyleRecalc();
}

ContainerNode* ShadowRoot::activeContentContainer()
{
    ShadowContentSelector* selector = ShadowContentSelector::currentInstance();
    if (!selector || selector->shadowRoot() != this)
        return 0;
    return selector->activeElement();
}

void ShadowRoot::hostChildrenChanged()
{
    if (!hasContentElement())
        return;
    // This results in forced detaching/attaching of the shadow render tree. See ShadowRoot::recalcStyle().
    setNeedsStyleRecalc();
}

bool ShadowRoot::hasContentElement() const
{
    for (Node* n = firstChild(); n; n = n->traverseNextNode(this)) {
        if (n->isContentElement())
            return true;
    }

    return false;
}

bool ShadowRoot::applyAuthorSheets() const
{
    return false;
}

void ShadowRoot::attach()
{
    ShadowContentSelector selector(this);
    TreeScope::attach();
}

}
