/*
 * Copyright (C) 2005, 2006 Apple Computer, Inc.  All rights reserved.
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

#include "config.h"
#include "ReplaceSelectionCommand.h"

#include "ApplyStyleCommand.h"
#include "BeforeTextInsertedEvent.h"
#include "CSSComputedStyleDeclaration.h"
#include "CSSPropertyNames.h"
#include "Document.h"
#include "DocumentFragment.h"
#include "EditingText.h"
#include "EventNames.h"
#include "Element.h"
#include "Frame.h"
#include "HTMLElement.h"
#include "HTMLInterchange.h"
#include "HTMLInputElement.h"
#include "HTMLNames.h"
#include "SelectionController.h"
#include "TextIterator.h"
#include "htmlediting.h"
#include "markup.h"
#include "visible_units.h"

namespace WebCore {

using namespace EventNames;
using namespace HTMLNames;

static bool isInterchangeNewlineNode(const Node *node)
{
    static String interchangeNewlineClassString(AppleInterchangeNewline);
    return node && node->hasTagName(brTag) && 
           static_cast<const Element *>(node)->getAttribute(classAttr) == interchangeNewlineClassString;
}

static bool isInterchangeConvertedSpaceSpan(const Node *node)
{
    static String convertedSpaceSpanClassString(AppleConvertedSpace);
    return node->isHTMLElement() && 
           static_cast<const HTMLElement *>(node)->getAttribute(classAttr) == convertedSpaceSpanClassString;
}

ReplacementFragment::ReplacementFragment(Document* document, DocumentFragment* fragment, bool matchStyle, const Selection& selection)
    : m_document(document),
      m_fragment(fragment),
      m_matchStyle(matchStyle), 
      m_hasInterchangeNewlineAtStart(false), 
      m_hasInterchangeNewlineAtEnd(false)
{
    if (!m_document)
        return;
    if (!m_fragment)
        return;
    if (!m_fragment->firstChild())
        return;
    
    Element* editableRoot = selection.rootEditableElement();
    ASSERT(editableRoot);
    if (!editableRoot)
        return;
    
    Node* shadowAncestorNode = editableRoot->shadowAncestorNode();
    
    if (!editableRoot->getHTMLEventListener(khtmlBeforeTextInsertedEvent) &&
        // FIXME: Remove these checks once textareas and textfields actually register an event handler.
        !(shadowAncestorNode && shadowAncestorNode->renderer() && shadowAncestorNode->renderer()->isTextField() && static_cast<HTMLInputElement*>(shadowAncestorNode)->isNonWidgetTextField()) &&
        !(shadowAncestorNode && shadowAncestorNode->renderer() && shadowAncestorNode->renderer()->isTextArea()) &&
        editableRoot->isContentRichlyEditable()) {
        removeInterchangeNodes(m_fragment->firstChild());
        return;
    }

    Node* styleNode = selection.base().node();
    RefPtr<Node> holder = insertFragmentForTestRendering(styleNode);
    
    RefPtr<Range> range = Selection::selectionFromContentsOfNode(holder.get()).toRange();
    String text = plainText(range.get());
    // Give the root a chance to change the text.
    RefPtr<BeforeTextInsertedEvent> evt = new BeforeTextInsertedEvent(text);
    ExceptionCode ec = 0;
    editableRoot->dispatchEvent(evt, ec, true);
    ASSERT(ec == 0);
    if (text != evt->text() || !editableRoot->isContentRichlyEditable()) {
        restoreTestRenderingNodesToFragment(holder.get());
        removeNode(holder);

        m_fragment = createFragmentFromText(selection.toRange().get(), evt->text());
        if (!m_fragment->firstChild())
            return;
        holder = insertFragmentForTestRendering(styleNode);
    }
    
    removeInterchangeNodes(holder->firstChild());
    
    removeUnrenderedNodes(holder.get());
    restoreTestRenderingNodesToFragment(holder.get());
    removeNode(holder);
}

bool ReplacementFragment::isEmpty() const
{
    return (!m_fragment || !m_fragment->firstChild()) && !m_hasInterchangeNewlineAtStart && !m_hasInterchangeNewlineAtEnd;
}

Node *ReplacementFragment::firstChild() const 
{ 
    return m_fragment->firstChild(); 
}

Node *ReplacementFragment::lastChild() const 
{ 
    return m_fragment->lastChild(); 
}

void ReplacementFragment::removeNodePreservingChildren(Node *node)
{
    if (!node)
        return;

    while (RefPtr<Node> n = node->firstChild()) {
        removeNode(n);
        insertNodeBefore(n.get(), node);
    }
    removeNode(node);
}

void ReplacementFragment::removeNode(PassRefPtr<Node> node)
{
    if (!node)
        return;
    
    Node *parent = node->parentNode();
    if (!parent)
        return;
    
    ExceptionCode ec = 0;
    parent->removeChild(node.get(), ec);
    ASSERT(ec == 0);
}

void ReplacementFragment::insertNodeBefore(Node *node, Node *refNode)
{
    if (!node || !refNode)
        return;
        
    Node *parent = refNode->parentNode();
    if (!parent)
        return;
        
    ExceptionCode ec = 0;
    parent->insertBefore(node, refNode, ec);
    ASSERT(ec == 0);
}

PassRefPtr<Node> ReplacementFragment::insertFragmentForTestRendering(Node* context)
{
    Node* body = m_document->body();
    if (!body)
        return 0;

    RefPtr<StyledElement> holder = static_pointer_cast<StyledElement>(createDefaultParagraphElement(m_document.get()));
    
    ExceptionCode ec = 0;

    // Copy the whitespace style from the context onto this element.
    Node* n = context;
    while (n && !n->isElementNode())
        n = n->parentNode();
    if (n) {
        RefPtr<CSSComputedStyleDeclaration> contextStyle = new CSSComputedStyleDeclaration(static_cast<Element*>(n));
        CSSStyleDeclaration* style = holder->style();
        style->setProperty(CSS_PROP_WHITE_SPACE, contextStyle->getPropertyValue(CSS_PROP_WHITE_SPACE), false, ec);
        ASSERT(ec == 0);
    }
    
    holder->appendChild(m_fragment, ec);
    ASSERT(ec == 0);
    
    body->appendChild(holder.get(), ec);
    ASSERT(ec == 0);
    
    m_document->updateLayoutIgnorePendingStylesheets();
    
    return holder.release();
}

void ReplacementFragment::restoreTestRenderingNodesToFragment(Node *holder)
{
    if (!holder)
        return;
    
    ExceptionCode ec = 0;
    while (RefPtr<Node> node = holder->firstChild()) {
        holder->removeChild(node.get(), ec);
        ASSERT(ec == 0);
        m_fragment->appendChild(node.get(), ec);
        ASSERT(ec == 0);
    }
}

void ReplacementFragment::removeUnrenderedNodes(Node* holder)
{
    Vector<Node*> unrendered;

    for (Node* node = holder->firstChild(); node; node = node->traverseNextNode(holder))
        if (!isNodeRendered(node) && !isTableStructureNode(node))
            unrendered.append(node);

    size_t n = unrendered.size();
    for (size_t i = 0; i < n; ++i)
        removeNode(unrendered[i]);
}

void ReplacementFragment::removeInterchangeNodes(Node* startNode)
{
    Node* node = startNode;
    Node* newlineAtStartNode = 0;
    Node* newlineAtEndNode = 0;
    while (node) {
        Node *next = node->traverseNextNode();
        if (isInterchangeNewlineNode(node)) {
            if (next || node == startNode) {
                m_hasInterchangeNewlineAtStart = true;
                newlineAtStartNode = node;
            }
            else {
                m_hasInterchangeNewlineAtEnd = true;
                newlineAtEndNode = node;
            }
        }
        else if (isInterchangeConvertedSpaceSpan(node)) {
            RefPtr<Node> n = 0;
            while ((n = node->firstChild())) {
                removeNode(n);
                insertNodeBefore(n.get(), node);
            }
            removeNode(node);
            if (n)
                next = n->traverseNextNode();
        }
        node = next;
    }

    if (newlineAtStartNode)
        removeNode(newlineAtStartNode);
    if (newlineAtEndNode)
        removeNode(newlineAtEndNode);
}

ReplaceSelectionCommand::ReplaceSelectionCommand(Document* document, PassRefPtr<DocumentFragment> fragment,
        bool selectReplacement, bool smartReplace, bool matchStyle, bool preventNesting,
        EditAction editAction) 
    : CompositeEditCommand(document),
      m_selectReplacement(selectReplacement), 
      m_smartReplace(smartReplace),
      m_matchStyle(matchStyle),
      m_documentFragment(fragment),
      m_preventNesting(preventNesting),
      m_editAction(editAction)
{
}

bool ReplaceSelectionCommand::shouldMergeStart(bool selectionStartWasStartOfParagraph, bool fragmentHasInterchangeNewlineAtStart)
{
    VisiblePosition startOfInsertedContent(Position(m_firstNodeInserted.get(), 0));
    VisiblePosition prev = startOfInsertedContent.previous(true);
    if (prev.isNull())
        return false;
        
    return !selectionStartWasStartOfParagraph && 
           !fragmentHasInterchangeNewlineAtStart &&
           isStartOfParagraph(startOfInsertedContent) && 
           !startOfInsertedContent.deepEquivalent().node()->hasTagName(brTag) &&
           shouldMerge(startOfInsertedContent, prev);
}

bool ReplaceSelectionCommand::shouldMergeEnd(bool selectionEndWasEndOfParagraph)
{
    VisiblePosition endOfInsertedContent(Position(m_lastNodeInserted.get(), maxDeepOffset(m_lastNodeInserted.get())));
    VisiblePosition next = endOfInsertedContent.next(true);
    if (next.isNull())
        return false;

    return !selectionEndWasEndOfParagraph &&
           isEndOfParagraph(endOfInsertedContent) && 
           !endOfInsertedContent.deepEquivalent().node()->hasTagName(brTag) &&
           shouldMerge(endOfInsertedContent, next);
}

static bool isMailPasteAsQuotationNode(Node* node)
{
    return node && node->hasTagName(blockquoteTag) && node->isElementNode() && static_cast<Element*>(node)->getAttribute(classAttr) == ApplePasteAsQuotation;
}

// Virtual method used so that ReplaceSelectionCommand can update the node's it tracks.
void ReplaceSelectionCommand::removeNodePreservingChildren(Node* node)
{
    if (m_firstNodeInserted == node)
        m_firstNodeInserted = node->traverseNextNode();
    if (m_lastNodeInserted == node)
        m_lastNodeInserted = node->lastChild() ? node->lastChild() : node->traverseNextSibling();
    CompositeEditCommand::removeNodePreservingChildren(node);
}

bool ReplaceSelectionCommand::shouldMerge(const VisiblePosition& from, const VisiblePosition& to)
{
    if (from.isNull() || to.isNull())
        return false;
        
    Node* fromNode = from.deepEquivalent().node();
    Node* toNode = to.deepEquivalent().node();
    Node* fromNodeBlock = enclosingBlock(fromNode);
    return !enclosingNodeOfType(fromNode, &isMailPasteAsQuotationNode) &&
           fromNodeBlock && (!fromNodeBlock->hasTagName(blockquoteTag) || isMailBlockquote(fromNodeBlock))  &&
           enclosingListChild(fromNode) == enclosingListChild(toNode) &&
           enclosingTableCell(fromNode) == enclosingTableCell(toNode) &&
           !(fromNode->renderer() && fromNode->renderer()->isTable()) &&
           !(toNode->renderer() && toNode->renderer()->isTable()) && 
           !fromNode->hasTagName(hrTag) && !toNode->hasTagName(hrTag);
}

void ReplaceSelectionCommand::removeRedundantStyles(Node* mailBlockquoteEnclosingSelectionStart)
{
    // There's usually a top level style span that holds the document's default style, push it down.
    Node* node = m_firstNodeInserted.get();
    if (isStyleSpan(node) && mailBlockquoteEnclosingSelectionStart) {
        // Calculate the document default style.
        RefPtr<CSSMutableStyleDeclaration> blockquoteStyle = Position(mailBlockquoteEnclosingSelectionStart, 0).computedStyle()->copyInheritableProperties();
        RefPtr<CSSMutableStyleDeclaration> spanStyle = static_cast<HTMLElement*>(node)->inlineStyleDecl();
        spanStyle->merge(blockquoteStyle.get());  
    }
    else if (isStyleSpan(node)) {
    
        RefPtr<CSSMutableStyleDeclaration> parentStyle
            = Position(node, 0).computedStyle()->copyInheritableProperties();

        RefPtr<Node> child = node->firstChild();
        while (child) {
            RefPtr<Node> next = child->nextSibling();
            if (isStyleSpan(child.get())) {
                HTMLElement* elem = static_cast<HTMLElement*>(child.get());
                CSSMutableStyleDeclaration* inlineStyleDecl = elem->inlineStyleDecl();
                // be defensive because we used to sometimes leave unstyled Apple style spans in the DOM,
                // and we could be processing an old email with that flaw
                if (!inlineStyleDecl)
                    setNodeAttribute(elem, styleAttr, parentStyle->cssText());
                else {
                    inlineStyleDecl->merge(parentStyle.get(), false);
                    setNodeAttribute(elem, styleAttr, inlineStyleDecl->cssText());
                }
            } else if (node->isElementNode()) {
                RefPtr<Node> clone = node->cloneNode(false);
                int index = child->nodeIndex();
                removeNode(child.get());
                insertNodeAt(clone.get(), node, index);
                appendNode(child.get(), clone.get());
            }
            child = next;
        }
        
        removeNodePreservingChildren(node);
    }
    
    // Compute and save the non-redundant styles for all HTML elements.
    // Don't do any mutation here, because that would cause the diffs to trigger layouts.
    Vector<RefPtr<CSSMutableStyleDeclaration> > styles;
    Vector<RefPtr<HTMLElement> > elements;
    for (node = m_firstNodeInserted.get(); node; node = node->traverseNextNode()) {
        if (node->isHTMLElement()) {
            elements.append(static_cast<HTMLElement*>(node));
            RefPtr<CSSMutableStyleDeclaration> style
                = Position(node, 0).computedStyle()->copyInheritableProperties();
            RefPtr<CSSMutableStyleDeclaration> parentStyle
                = Position(node->parentNode(), 0).computedStyle()->copyInheritableProperties();
            parentStyle->diff(style.get());
            styles.append(style.release());
        }
        if (node == m_lastNodeInserted)
            break;
    }
    
    size_t count = styles.size();
    for (size_t i = 0; i < count; ++i) {
        HTMLElement* element = elements[i].get();

        // Handle case where the element was already removed by earlier processing.
        // It's possible this no longer occurs, but it did happen in an earlier version
        // that processed elements in a less-determistic order, and I can't prove it
        // does not occur.
        if (!element->inDocument())
            continue;

        // Remove empty style spans.
        if (isStyleSpan(element) && !element->hasChildNodes()) {
            removeNodeAndPruneAncestors(element);
            continue;
        }

        // Remove redundant style tags and style spans.
        CSSMutableStyleDeclaration* style = styles[i].get();
        if (style->length() == 0
                && (isStyleSpan(element)
                    || element->hasTagName(bTag)
                    || element->hasTagName(fontTag)
                    || element->hasTagName(iTag)
                    || element->hasTagName(uTag))) {
            removeNodePreservingChildren(element);
            continue;
        }

        // Clear redundant styles from elements.
        CSSMutableStyleDeclaration* inlineStyleDecl = element->inlineStyleDecl();
        if (inlineStyleDecl) {
            inlineStyleDecl->removeInheritableProperties();
            inlineStyleDecl->merge(style, true);
            setNodeAttribute(element, styleAttr, inlineStyleDecl->cssText());
        }
    }
}

void ReplaceSelectionCommand::handlePasteAsQuotationNode()
{
    Node* node = m_firstNodeInserted.get();
    if (isMailPasteAsQuotationNode(node))
        static_cast<Element*>(node)->setAttribute(classAttr, "");
}

void ReplaceSelectionCommand::doApply()
{
    Selection selection = endingSelection();
    ASSERT(selection.isCaretOrRange());
    ASSERT(selection.start().node());
    if (selection.isNone() || !selection.start().node())
        return;
    
    if (!selection.isContentRichlyEditable())
        m_matchStyle = true;
    
    Element* currentRoot = selection.rootEditableElement();
    ReplacementFragment fragment(document(), m_documentFragment.get(), m_matchStyle, selection);
    
    if (fragment.isEmpty())
        return;
    
    if (m_matchStyle)
        m_insertionStyle = styleAtPosition(selection.start());
    
    VisiblePosition visibleStart = selection.visibleStart();
    VisiblePosition visibleEnd = selection.visibleEnd();
    
    bool selectionEndWasEndOfParagraph = isEndOfParagraph(visibleEnd);
    bool selectionStartWasStartOfParagraph = isStartOfParagraph(visibleStart);
    Node* mailBlockquoteEnclosingSelectionStart = nearestMailBlockquote(visibleStart.deepEquivalent().node());
    
    Node* startBlock = enclosingBlock(visibleStart.deepEquivalent().node());
    
    if (selectionStartWasStartOfParagraph && selectionEndWasEndOfParagraph ||
        startBlock == currentRoot ||
        startBlock && startBlock->renderer() && startBlock->renderer()->isListItem())
        m_preventNesting = false;
    
    Position insertionPos = selection.start();
    
    if (selection.isRange()) {
        // When the end of the selection being pasted into is at the end of a paragraph, and that selection
        // spans multiple blocks, not merging may leave an empty line.
        // When the start of the selection being pasted into is at the start of a block, not merging 
        // will leave hanging block(s).
        bool mergeBlocksAfterDelete = isEndOfParagraph(visibleEnd) || isStartOfBlock(visibleStart);
        deleteSelection(false, mergeBlocksAfterDelete, true);
        visibleStart = endingSelection().visibleStart();
        if (fragment.hasInterchangeNewlineAtStart()) {
            if (isEndOfParagraph(visibleStart) && !isStartOfParagraph(visibleStart)) {
                if (!isEndOfDocument(visibleStart))
                    setEndingSelection(visibleStart.next());
            } else
                insertParagraphSeparator();
        }
        insertionPos = endingSelection().start();
    } else {
        ASSERT(selection.isCaret());
        if (fragment.hasInterchangeNewlineAtStart()) {
            VisiblePosition next = visibleStart.next(true);
            if (isEndOfParagraph(visibleStart) && !isStartOfParagraph(visibleStart) && next.isNotNull())
                setEndingSelection(next);
            else 
                insertParagraphSeparator();
        }
        // We split the current paragraph in two to avoid nesting the blocks from the fragment inside the current block.
        // For example paste <div>foo</div><div>bar</div><div>baz</div> into <div>x^x</div>, where ^ is the caret.  
        // As long as the  div styles are the same, visually you'd expect: <div>xbar</div><div>bar</div><div>bazx</div>, 
        // not <div>xbar<div>bar</div><div>bazx</div></div>
        if (m_preventNesting && !isEndOfParagraph(visibleStart) && !isStartOfParagraph(visibleStart)) {
            insertParagraphSeparator();
            setEndingSelection(endingSelection().visibleStart().previous());
        }
        insertionPos = endingSelection().start();
    }
    
    // Inserting content could cause whitespace to collapse, e.g. inserting <div>foo</div> into hello^ world.
    prepareWhitespaceAtPositionForSplit(insertionPos);
    
    // NOTE: This would be an incorrect usage of downstream() if downstream() were changed to mean the last position after 
    // p that maps to the same visible position as p (since in the case where a br is at the end of a block and collapsed 
    // away, there are positions after the br which map to the same visible position as [br, 0]).  
    Node* endBR = insertionPos.downstream().node()->hasTagName(brTag) ? insertionPos.downstream().node() : 0;
    
    startBlock = enclosingBlock(insertionPos.node());
    
    // Adjust insertionPos to prevent nesting.
    if (m_preventNesting && startBlock) {
        ASSERT(startBlock != currentRoot);
        VisiblePosition visibleInsertionPos(insertionPos);
        if (isEndOfBlock(visibleInsertionPos) && !(isStartOfBlock(visibleInsertionPos) && fragment.hasInterchangeNewlineAtEnd()))
            insertionPos = positionAfterNode(startBlock);
        else if (isStartOfBlock(visibleInsertionPos))
            insertionPos = positionBeforeNode(startBlock);
    }

    // Paste into run of tabs splits the tab span.
    insertionPos = positionOutsideTabSpan(insertionPos);
    
    // Paste at start or end of link goes outside of link.
    insertionPos = positionAvoidingSpecialElementBoundary(insertionPos);

    Frame *frame = document()->frame();
    
    // FIXME: Improve typing style.
    // See this bug: <rdar://problem/3769899> Implementation of typing style needs improvement
    frame->clearTypingStyle();
    setTypingStyle(0);    
    
    // We're finished if there is nothing to add.
    if (!fragment.firstChild())
        return;
    
    // 1) Insert the content.
    // 2) Remove redundant styles and style tags, this inner <b> for example: <b>foo <b>bar</b> baz</b>.
    // 3) Merge the start of the added content with the content before the position being pasted into.
    // 4) Do one of the following: a) expand the last br if the fragment ends with one and it collapsed,
    // b) merge the last paragraph of the incoming fragment with the paragraph that contained the 
    // end of the selection that was pasted into, or c) handle an interchange newline at the end of the 
    // incoming fragment.
    // 5) Add spaces for smart replace.
    // 6) Select the replacement if requested, and match style if requested.
    
    VisiblePosition startOfInsertedContent, endOfInsertedContent;
    
    RefPtr<Node> refNode = fragment.firstChild();
    RefPtr<Node> node = refNode->nextSibling();
    
    fragment.removeNode(refNode);
    insertNodeAtAndUpdateNodesInserted(refNode.get(), insertionPos.node(), insertionPos.offset());
    
    while (node) {
        Node* next = node->nextSibling();
        fragment.removeNode(node);
        insertNodeAfterAndUpdateNodesInserted(node.get(), refNode.get());
        refNode = node;
        node = next;
    }
    
    removeRedundantStyles(mailBlockquoteEnclosingSelectionStart);
    
    endOfInsertedContent = VisiblePosition(Position(m_lastNodeInserted.get(), maxDeepOffset(m_lastNodeInserted.get())));
    startOfInsertedContent = VisiblePosition(Position(m_firstNodeInserted.get(), 0));
    
    // We inserted before the startBlock to prevent nesting, and the content before the startBlock wasn't in its own block and
    // didn't have a br after it, so the inserted content ended up in the same paragraph.
    if (startBlock && insertionPos.node() == startBlock->parentNode() && (unsigned)insertionPos.offset() < startBlock->nodeIndex() && !isStartOfParagraph(startOfInsertedContent))
        insertNodeAt(createBreakElement(document()).get(), startOfInsertedContent.deepEquivalent().node(), startOfInsertedContent.deepEquivalent().offset());
    
    Position lastPositionToSelect;
    
    bool interchangeNewlineAtEnd = fragment.hasInterchangeNewlineAtEnd();

    if (shouldRemoveEndBR(endBR)) {
        if (interchangeNewlineAtEnd) {
            interchangeNewlineAtEnd = false;
            m_lastNodeInserted = endBR;
            lastPositionToSelect = VisiblePosition(Position(m_lastNodeInserted.get(), 0)).deepEquivalent();
        } else
            removeNodeAndPruneAncestors(endBR);
    }
        
    if (shouldMergeStart(selectionStartWasStartOfParagraph, fragment.hasInterchangeNewlineAtStart())) {
        VisiblePosition destination = startOfInsertedContent.previous();
        VisiblePosition startOfParagraphToMove = startOfInsertedContent;
        
        // FIXME: Maintain positions for the start and end of inserted content instead of keeping nodes.  The nodes are
        // only ever used to create positions where inserted content starts/ends.
        moveParagraph(startOfParagraphToMove, endOfParagraph(startOfParagraphToMove), destination);
        m_firstNodeInserted = endingSelection().visibleStart().deepEquivalent().downstream().node();
        if (!m_lastNodeInserted->inDocument())
            m_lastNodeInserted = endingSelection().visibleEnd().deepEquivalent().upstream().node();
    }
            
    endOfInsertedContent = VisiblePosition(Position(m_lastNodeInserted.get(), maxDeepOffset(m_lastNodeInserted.get())));
    startOfInsertedContent = VisiblePosition(Position(m_firstNodeInserted.get(), 0));
    
    if (interchangeNewlineAtEnd) {
        VisiblePosition next = endOfInsertedContent.next(true);

        if (selectionEndWasEndOfParagraph || !isEndOfParagraph(endOfInsertedContent) || next.isNull()) {
            if (!isStartOfParagraph(endOfInsertedContent)) {
                setEndingSelection(endOfInsertedContent);
                insertParagraphSeparator();

                // Select up to the paragraph separator that was added.
                lastPositionToSelect = endingSelection().visibleStart().deepEquivalent();
                updateNodesInserted(lastPositionToSelect.node());
            }
        } else {
            // Select up to the beginning of the next paragraph.
            lastPositionToSelect = next.deepEquivalent().downstream();
        }

    } else if (m_lastNodeInserted->hasTagName(brTag)) {
        // We want to honor the last incoming line break, so, if it will collapse away because of quirks mode, 
        // add an extra one.
        // FIXME: This will expand a br inside a block: <div><br></div>
        // FIXME: Should we expand all incoming brs that collapse because of quirks mode?
        if (!document()->inStrictMode() && isEndOfBlock(endOfInsertedContent) && !isStartOfParagraph(endOfInsertedContent))
            insertNodeBeforeAndUpdateNodesInserted(createBreakElement(document()).get(), m_lastNodeInserted.get());
            
    } else if (shouldMergeEnd(selectionEndWasEndOfParagraph)) {
    
        // Merging two paragraphs will destroy the moved one's block styles.  Always move forward to preserve
        // the block style of the paragraph already in the document, unless the paragraph to move would include the
        // what was the start of the selection that was pasted into.
        bool mergeForward = !inSameParagraph(startOfInsertedContent, endOfInsertedContent) || isStartOfParagraph(startOfInsertedContent);
        
        VisiblePosition destination = mergeForward ? endOfInsertedContent.next() : endOfInsertedContent;
        VisiblePosition startOfParagraphToMove = mergeForward ? startOfParagraph(endOfInsertedContent) : endOfInsertedContent.next();

        moveParagraph(startOfParagraphToMove, endOfParagraph(startOfParagraphToMove), destination);
        // Merging forward will remove m_lastNodeInserted from the document.
        // FIXME: Maintain positions for the start and end of inserted content instead of keeping nodes.  The nodes are
        // only ever used to create positions where inserted content starts/ends.
        if (mergeForward) {
            m_lastNodeInserted = destination.previous().deepEquivalent().node();
            if (!m_firstNodeInserted->inDocument())
                m_firstNodeInserted = endingSelection().visibleStart().deepEquivalent().node();
        }
    }
    
    handlePasteAsQuotationNode();
    
    endOfInsertedContent = VisiblePosition(Position(m_lastNodeInserted.get(), maxDeepOffset(m_lastNodeInserted.get())));
    startOfInsertedContent = VisiblePosition(Position(m_firstNodeInserted.get(), 0));    
    
    // Add spaces for smart replace.
    if (m_smartReplace && currentRoot) {
        // Disable smart replace for password fields.
        Node* start = currentRoot->shadowAncestorNode();
        if (start->hasTagName(inputTag) && static_cast<HTMLInputElement*>(start)->inputType() == HTMLInputElement::PASSWORD)
            m_smartReplace = false;
    }
    if (m_smartReplace) {
        bool needsTrailingSpace = !isEndOfParagraph(endOfInsertedContent) &&
                                  !frame->isCharacterSmartReplaceExempt(endOfInsertedContent.characterAfter(), false);
        if (needsTrailingSpace) {
            RenderObject* renderer = m_lastNodeInserted->renderer();
            bool collapseWhiteSpace = !renderer || renderer->style()->collapseWhiteSpace();
            if (m_lastNodeInserted->isTextNode()) {
                Text* text = static_cast<Text*>(m_lastNodeInserted.get());
                insertTextIntoNode(text, text->length(), collapseWhiteSpace ? nonBreakingSpaceString() : " ");
            } else {
                RefPtr<Node> node = document()->createEditingTextNode(collapseWhiteSpace ? nonBreakingSpaceString() : " ");
                insertNodeAfterAndUpdateNodesInserted(node.get(), m_lastNodeInserted.get());
            }
        }
    
        bool needsLeadingSpace = !isStartOfParagraph(startOfInsertedContent) &&
                                 !frame->isCharacterSmartReplaceExempt(startOfInsertedContent.previous().characterAfter(), true);
        if (needsLeadingSpace) {
            RenderObject* renderer = m_lastNodeInserted->renderer();
            bool collapseWhiteSpace = !renderer || renderer->style()->collapseWhiteSpace();
            if (m_firstNodeInserted->isTextNode()) {
                Text* text = static_cast<Text*>(m_firstNodeInserted.get());
                insertTextIntoNode(text, 0, collapseWhiteSpace ? nonBreakingSpaceString() : " ");
            } else {
                RefPtr<Node> node = document()->createEditingTextNode(collapseWhiteSpace ? nonBreakingSpaceString() : " ");
                // Don't updateNodesInserted.  Doing so would set m_lastNodeInserted to be the node containing the 
                // leading space, but m_lastNodeInserted is supposed to mark the end of pasted content.
                insertNodeBefore(node.get(), m_firstNodeInserted.get());
                // FIXME: Use positions to track the start/end of inserted content.
                m_firstNodeInserted = node;
            }
        }
    }
    
    completeHTMLReplacement(lastPositionToSelect);
}

bool ReplaceSelectionCommand::shouldRemoveEndBR(Node* endBR)
{
    if (!endBR || !endBR->inDocument())
        return false;
        
    VisiblePosition visiblePos(Position(endBR, 0));
    
    return
        // The br is collapsed away and so is unnecessary.
        !document()->inStrictMode() && isEndOfBlock(visiblePos) && !isStartOfParagraph(visiblePos) ||
        // A br that was originally holding a line open should be displaced by inserted content or turned into a line break.
        // A br that was originally acting as a line break should still be acting as a line break, not as a placeholder.
        isStartOfParagraph(visiblePos) && isEndOfParagraph(visiblePos) && !m_lastNodeInserted->hasTagName(brTag);
}

void ReplaceSelectionCommand::completeHTMLReplacement(const Position &lastPositionToSelect)
{
    Position start;
    Position end;

    if (m_firstNodeInserted && m_firstNodeInserted->inDocument() && m_lastNodeInserted && m_lastNodeInserted->inDocument()) {
        
        Node* lastLeaf = m_lastNodeInserted->lastDescendant();
        Node* firstLeaf = m_firstNodeInserted->firstDescendant();
        
        start = Position(firstLeaf, 0);
        end = Position(lastLeaf, maxDeepOffset(lastLeaf));
        
        // FIXME (11475): Remove this and require that the creator of the fragment to use nbsps.
        rebalanceWhitespaceAt(start);
        rebalanceWhitespaceAt(end);

        if (m_matchStyle) {
            assert(m_insertionStyle);
            applyStyle(m_insertionStyle.get(), start, end);
        }    
        
        if (lastPositionToSelect.isNotNull())
            end = lastPositionToSelect;
    } else if (lastPositionToSelect.isNotNull())
        start = end = lastPositionToSelect;
    else
        return;
    
    if (m_selectReplacement)
        setEndingSelection(Selection(start, end, SEL_DEFAULT_AFFINITY));
    else
        setEndingSelection(Selection(end, SEL_DEFAULT_AFFINITY));
}

EditAction ReplaceSelectionCommand::editingAction() const
{
    return m_editAction;
}

void ReplaceSelectionCommand::insertNodeAfterAndUpdateNodesInserted(Node *insertChild, Node *refChild)
{
    insertNodeAfter(insertChild, refChild);
    updateNodesInserted(insertChild);
}

void ReplaceSelectionCommand::insertNodeAtAndUpdateNodesInserted(Node *insertChild, Node *refChild, int offset)
{
    insertNodeAt(insertChild, refChild, offset);
    updateNodesInserted(insertChild);
}

void ReplaceSelectionCommand::insertNodeBeforeAndUpdateNodesInserted(Node *insertChild, Node *refChild)
{
    insertNodeBefore(insertChild, refChild);
    updateNodesInserted(insertChild);
}

void ReplaceSelectionCommand::updateNodesInserted(Node *node)
{
    if (!node)
        return;

    if (!m_firstNodeInserted)
        m_firstNodeInserted = node;
    
    if (node == m_lastNodeInserted)
        return;
    
    m_lastNodeInserted = node->lastDescendant();
}

} // namespace WebCore
