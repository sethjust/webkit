/**
 * Copyright (C) 2006, 2007, 2010 Apple Inc. All rights reserved.
 *           (C) 2008 Torch Mobile Inc. All rights reserved. (http://www.torchmobile.com/) 
 * Copyright (C) 2010 Google Inc. All rights reserved.
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
#include "RenderTextControlSingleLine.h"

#include "CSSFontSelector.h"
#include "CSSStyleSelector.h"
#include "Chrome.h"
#include "Event.h"
#include "EventNames.h"
#include "Frame.h"
#include "FrameSelection.h"
#include "FrameView.h"
#include "HTMLInputElement.h"
#include "HTMLNames.h"
#include "HitTestResult.h"
#include "LocalizedStrings.h"
#include "MouseEvent.h"
#include "Page.h"
#include "PlatformKeyboardEvent.h"
#include "RenderLayer.h"
#include "RenderScrollbar.h"
#include "RenderTheme.h"
#include "Settings.h"
#include "SimpleFontData.h"
#include "TextControlInnerElements.h"

using namespace std;

namespace WebCore {

using namespace HTMLNames;

VisiblePosition RenderTextControlInnerBlock::positionForPoint(const IntPoint& point)
{
    IntPoint contentsPoint(point);

    // Multiline text controls have the scroll on shadowAncestorNode, so we need to take that
    // into account here.
    if (m_multiLine) {
        RenderTextControl* renderer = toRenderTextControl(node()->shadowAncestorNode()->renderer());
        if (renderer->hasOverflowClip())
            contentsPoint += renderer->layer()->scrolledContentOffset();
    }

    return RenderBlock::positionForPoint(contentsPoint);
}

// ----------------------------

RenderTextControlSingleLine::RenderTextControlSingleLine(Node* node, bool placeholderVisible)
    : RenderTextControl(node, placeholderVisible)
    , m_searchPopupIsVisible(false)
    , m_shouldDrawCapsLockIndicator(false)
    , m_searchEventTimer(this, &RenderTextControlSingleLine::searchEventTimerFired)
    , m_searchPopup(0)
{
    ASSERT(node->isHTMLElement());
    ASSERT(node->toInputElement());
}

RenderTextControlSingleLine::~RenderTextControlSingleLine()
{
    if (m_searchPopup) {
        m_searchPopup->popupMenu()->disconnectClient();
        m_searchPopup = 0;
    }
}

HTMLElement* RenderTextControlSingleLine::innerTextElement() const
{
    return inputElement()->innerTextElement();
}

inline HTMLElement* RenderTextControlSingleLine::innerBlockElement() const
{
    return inputElement()->innerBlockElement();
}

inline HTMLElement* RenderTextControlSingleLine::innerSpinButtonElement() const
{
    return inputElement()->innerSpinButtonElement();
}

inline HTMLElement* RenderTextControlSingleLine::resultsButtonElement() const
{
    return inputElement()->resultsButtonElement();
}

inline HTMLElement* RenderTextControlSingleLine::cancelButtonElement() const
{
    return inputElement()->cancelButtonElement();
}

#if ENABLE(INPUT_SPEECH)
inline HTMLElement* RenderTextControlSingleLine::speechButtonElement() const
{
    return inputElement()->speechButtonElement();
}
#endif

RenderStyle* RenderTextControlSingleLine::textBaseStyle() const
{
    HTMLElement* innerBlock = innerBlockElement();
    return innerBlock ? innerBlock->renderer()->style() : style();
}

void RenderTextControlSingleLine::addSearchResult()
{
    HTMLInputElement* input = inputElement();
    if (input->maxResults() <= 0)
        return;

    String value = input->value();
    if (value.isEmpty())
        return;

    Settings* settings = document()->settings();
    if (!settings || settings->privateBrowsingEnabled())
        return;

    int size = static_cast<int>(m_recentSearches.size());
    for (int i = size - 1; i >= 0; --i) {
        if (m_recentSearches[i] == value)
            m_recentSearches.remove(i);
    }

    m_recentSearches.insert(0, value);
    while (static_cast<int>(m_recentSearches.size()) > input->maxResults())
        m_recentSearches.removeLast();

    const AtomicString& name = autosaveName();
    if (!m_searchPopup)
        m_searchPopup = document()->page()->chrome()->createSearchPopupMenu(this);

    m_searchPopup->saveRecentSearches(name, m_recentSearches);
}

void RenderTextControlSingleLine::stopSearchEventTimer()
{
    m_searchEventTimer.stop();
}

void RenderTextControlSingleLine::showPopup()
{
    if (m_searchPopupIsVisible)
        return;

    if (!m_searchPopup)
        m_searchPopup = document()->page()->chrome()->createSearchPopupMenu(this);

    if (!m_searchPopup->enabled())
        return;

    m_searchPopupIsVisible = true;

    const AtomicString& name = autosaveName();
    m_searchPopup->loadRecentSearches(name, m_recentSearches);

    // Trim the recent searches list if the maximum size has changed since we last saved.
    HTMLInputElement* input = inputElement();
    if (static_cast<int>(m_recentSearches.size()) > input->maxResults()) {
        do {
            m_recentSearches.removeLast();
        } while (static_cast<int>(m_recentSearches.size()) > input->maxResults());

        m_searchPopup->saveRecentSearches(name, m_recentSearches);
    }

    m_searchPopup->popupMenu()->show(absoluteBoundingBoxRect(true), document()->view(), -1);
}

void RenderTextControlSingleLine::hidePopup()
{
    if (m_searchPopup)
        m_searchPopup->popupMenu()->hide();
}

void RenderTextControlSingleLine::subtreeHasChanged()
{
    RenderTextControl::subtreeHasChanged();

    ASSERT(node()->isElementNode());
    Element* element = static_cast<Element*>(node());
    bool wasChanged = element->wasChangedSinceLastFormControlChangeEvent();
    element->setChangedSinceLastFormControlChangeEvent(true);

    HTMLInputElement* input = inputElement();
    // We don't need to call sanitizeUserInputValue() function here because
    // HTMLInputElement::handleBeforeTextInsertedEvent() has already called
    // sanitizeUserInputValue().
    // sanitizeValue() is needed because IME input doesn't dispatch BeforeTextInsertedEvent.
    String value = text();
    if (input->isAcceptableValue(value))
        input->setValueFromRenderer(input->sanitizeValue(input->convertFromVisibleValue(value)));
    // Recalc for :invalid and hasUnacceptableValue() change.
    input->setNeedsStyleRecalc();

    if (cancelButtonElement())
        updateCancelButtonVisibility();

    // If the incremental attribute is set, then dispatch the search event
    if (input->searchEventsShouldBeDispatched())
        startSearchEventTimer();

    if (!wasChanged && node()->focused()) {
        if (Frame* frame = this->frame())
            frame->editor()->textFieldDidBeginEditing(static_cast<Element*>(node()));
    }

    if (node()->focused()) {
        if (Frame* frame = document()->frame())
            frame->editor()->textDidChangeInTextField(static_cast<Element*>(node()));
    }
}

void RenderTextControlSingleLine::paint(PaintInfo& paintInfo, const IntPoint& paintOffset)
{
    RenderTextControl::paint(paintInfo, paintOffset);

    if (paintInfo.phase == PaintPhaseBlockBackground && m_shouldDrawCapsLockIndicator) {
        IntRect contentsRect = contentBoxRect();

        // Center vertically like the text.
        contentsRect.setY((height() - contentsRect.height()) / 2);

        // Convert the rect into the coords used for painting the content
        contentsRect.moveBy(paintOffset + location());
        theme()->paintCapsLockIndicator(this, paintInfo, contentsRect);
    }
}

void RenderTextControlSingleLine::layout()
{
    int oldHeight = height();
    computeLogicalHeight();

    int oldWidth = width();
    computeLogicalWidth();

    bool relayoutChildren = oldHeight != height() || oldWidth != width();

    RenderBox* innerTextRenderer = innerTextElement()->renderBox();
    HTMLElement* innerBlock = innerBlockElement();
    RenderBox* innerBlockRenderer = innerBlock ? innerBlock->renderBox() : 0;

    // Set the text block height
    int desiredHeight = textBlockHeight();
    int currentHeight = innerTextRenderer->height();

    if (currentHeight > height()) {
        if (desiredHeight != currentHeight)
            relayoutChildren = true;
        innerTextRenderer->style()->setHeight(Length(desiredHeight, Fixed));
        if (innerBlock)
            innerBlockRenderer->style()->setHeight(Length(desiredHeight, Fixed));
    }

    // Set the text block width
    int desiredWidth = textBlockWidth();
    if (desiredWidth != innerTextRenderer->width())
        relayoutChildren = true;
    innerTextRenderer->style()->setWidth(Length(desiredWidth, Fixed));

    if (innerBlock) {
        int innerBlockWidth = width() - borderAndPaddingWidth();
        if (innerBlockWidth != innerBlockRenderer->width())
            relayoutChildren = true;
        innerBlockRenderer->style()->setWidth(Length(innerBlockWidth, Fixed));
    }

    RenderBlock::layoutBlock(relayoutChildren);

    // Center the child block vertically
    RenderBox* childBlock = innerBlockRenderer ? innerBlockRenderer : innerTextRenderer;
    currentHeight = childBlock->height();
    if (currentHeight < height())
        childBlock->setY((height() - currentHeight) / 2);

    // Ignores the paddings for the inner spin button.
    HTMLElement* innerSpinButton = innerSpinButtonElement();
    if (RenderBox* spinBox = innerSpinButton ? innerSpinButton->renderBox() : 0) {
        spinBox->setLocation(IntPoint(spinBox->x() + paddingRight(), borderTop()));
        spinBox->setHeight(height() - borderTop() - borderBottom());
    }

#if ENABLE(INPUT_SPEECH)
    if (RenderBox* button = speechButtonElement() ? speechButtonElement()->renderBox() : 0) {
        if (innerBlock) {
            // This is mostly the case where this is a search field. The speech button is a sibling
            // of the inner block and laid out at the far right.
            int x = width() - borderAndPaddingWidth() - button->width() - button->borderAndPaddingWidth();
            int y = (height() - button->height()) / 2;
            button->setLocation(IntPoint(x, y));
        } else {
            int x = width() - borderRight() - paddingRight() - button->width();
            RenderBox* spinBox = innerSpinButtonElement() ? innerSpinButtonElement()->renderBox() : 0;
            if (style()->isLeftToRightDirection())
                x -= spinBox ? spinBox->width() : 0;
            else
                innerTextRenderer->setX(paddingLeft() + borderLeft() + (spinBox ? spinBox->width() : 0));
            int y = (height() - button->height()) / 2;
            button->setLocation(IntPoint(x, y));
        }
    }
#endif
}

bool RenderTextControlSingleLine::nodeAtPoint(const HitTestRequest& request, HitTestResult& result, const IntPoint& pointInContainer, const IntPoint& accumulatedOffset, HitTestAction hitTestAction)
{
    // If we're within the text control, we want to act as if we've hit the inner text block element, in case the point
    // was on the control but not on the inner element (see Radar 4617841).

    // In a search field, we want to act as if we've hit the results block if we're to the left of the inner text block,
    // and act as if we've hit the close block if we're to the right of the inner text block.

    if (!RenderTextControl::nodeAtPoint(request, result, pointInContainer, accumulatedOffset, hitTestAction))
        return false;

    // If we hit a node inside the inner text element, say that we hit that element,
    // and if we hit our node (e.g. we're over the border or padding), also say that we hit the
    // inner text element so that it gains focus.
    if (result.innerNode()->isDescendantOf(innerTextElement()) || result.innerNode() == node())
        hitInnerTextElement(result, pointInContainer, accumulatedOffset);

    // If we found a spin button, we're done.
    HTMLElement* innerSpinButton = innerSpinButtonElement();
    if (innerSpinButton && result.innerNode() == innerSpinButton)
        return true;
#if ENABLE(INPUT_SPEECH)
    HTMLElement* speechButton = speechButtonElement();
    if (speechButton && result.innerNode() == speechButton)
        return true;
#endif
    // If we're not a search field, or we already found the speech, results or cancel buttons, we're done.
    HTMLElement* innerBlock = innerBlockElement();
    HTMLElement* resultsButton = resultsButtonElement();
    HTMLElement* cancelButton = cancelButtonElement();
    if (!innerBlock || result.innerNode() == resultsButton || result.innerNode() == cancelButton)
        return true;

    Node* innerNode = 0;
    RenderBox* innerBlockRenderer = innerBlock->renderBox();
    RenderBox* innerTextRenderer = innerTextElement()->renderBox();

    IntPoint localPoint = result.localPoint();
    localPoint.move(-innerBlockRenderer->location());

    int textLeft = accumulatedOffset.x() + x() + innerBlockRenderer->x() + innerTextRenderer->x();
    if (resultsButton && resultsButton->renderer() && pointInContainer.x() < textLeft)
        innerNode = resultsButton;

#if ENABLE(INPUT_SPEECH)
    if (!innerNode && speechButtonElement() && speechButtonElement()->renderer()) {
        int buttonLeft = accumulatedOffset.x() + x() + innerBlockRenderer->x() + innerBlockRenderer->width() - speechButtonElement()->renderBox()->width();
        if (pointInContainer.x() >= buttonLeft)
            innerNode = speechButtonElement();
    }
#endif

    if (!innerNode) {
        int textRight = textLeft + innerTextRenderer->width();
        if (cancelButton && cancelButton->renderer() && pointInContainer.x() > textRight)
            innerNode = cancelButton;
    }

    if (innerNode) {
        result.setInnerNode(innerNode);
        localPoint.move(-innerNode->renderBox()->location());
    }

    result.setLocalPoint(localPoint);
    return true;
}

void RenderTextControlSingleLine::forwardEvent(Event* event)
{
    RenderBox* innerTextRenderer = innerTextElement()->renderBox();

    if (event->type() == eventNames().blurEvent) {
        if (innerTextRenderer) {
            if (RenderLayer* innerLayer = innerTextRenderer->layer())
                innerLayer->scrollToOffset(!style()->isLeftToRightDirection() ? innerLayer->scrollWidth() : 0, 0);
        }

        capsLockStateMayHaveChanged();
    } else if (event->type() == eventNames().focusEvent)
        capsLockStateMayHaveChanged();

    if (!event->isMouseEvent()) {
        RenderTextControl::forwardEvent(event);
        return;
    }

#if ENABLE(INPUT_SPEECH)
    if (RenderBox* speechBox = speechButtonElement() ? speechButtonElement()->renderBox() : 0) {
        RenderBox* parent = innerTextRenderer ? innerTextRenderer : this;
        FloatPoint pointInTextControlCoords = parent->absoluteToLocal(static_cast<MouseEvent*>(event)->absoluteLocation(), false, true);
        if (speechBox->frameRect().contains(roundedIntPoint(pointInTextControlCoords))) {
            speechButtonElement()->defaultEventHandler(event);
            return;
        }
    }
#endif

    FloatPoint localPoint = innerTextRenderer->absoluteToLocal(static_cast<MouseEvent*>(event)->absoluteLocation(), false, true);
    int textRight = innerTextRenderer->borderBoxRect().maxX();

    HTMLElement* resultsButton = resultsButtonElement();
    HTMLElement* cancelButton = cancelButtonElement();
    if (resultsButton && localPoint.x() < innerTextRenderer->borderBoxRect().x())
        resultsButton->defaultEventHandler(event);
    else if (cancelButton && localPoint.x() > textRight)
        cancelButton->defaultEventHandler(event);
    else
        RenderTextControl::forwardEvent(event);
}

void RenderTextControlSingleLine::styleDidChange(StyleDifference diff, const RenderStyle* oldStyle)
{
    RenderTextControl::styleDidChange(diff, oldStyle);

    HTMLElement* innerBlock = innerBlockElement();
    if (RenderObject* innerBlockRenderer = innerBlock ? innerBlock->renderer() : 0) {
        // We may have set the width and the height in the old style in layout().
        // Reset them now to avoid getting a spurious layout hint.
        innerBlockRenderer->style()->setHeight(Length());
        innerBlockRenderer->style()->setWidth(Length());
        innerBlockRenderer->setStyle(createInnerBlockStyle(style()));
    }
    setHasOverflowClip(false);
}

void RenderTextControlSingleLine::capsLockStateMayHaveChanged()
{
    if (!node() || !document())
        return;

    // Only draw the caps lock indicator if these things are true:
    // 1) The field is a password field
    // 2) The frame is active
    // 3) The element is focused
    // 4) The caps lock is on
    bool shouldDrawCapsLockIndicator = false;

    if (Frame* frame = document()->frame())
        shouldDrawCapsLockIndicator = inputElement()->isPasswordField()
                                      && frame->selection()->isFocusedAndActive()
                                      && document()->focusedNode() == node()
                                      && PlatformKeyboardEvent::currentCapsLockState();

    if (shouldDrawCapsLockIndicator != m_shouldDrawCapsLockIndicator) {
        m_shouldDrawCapsLockIndicator = shouldDrawCapsLockIndicator;
        repaint();
    }
}

bool RenderTextControlSingleLine::hasControlClip() const
{
    bool clip = cancelButtonElement();
    return clip;
}

IntRect RenderTextControlSingleLine::controlClipRect(const IntPoint& additionalOffset) const
{
    // This should only get called for search & speech inputs.
    ASSERT(hasControlClip());

    IntRect clipRect = IntRect(innerBlockElement()->renderBox()->frameRect());
    clipRect.moveBy(additionalOffset);
    return clipRect;
}

int RenderTextControlSingleLine::textBlockWidth() const
{
    int width = RenderTextControl::textBlockWidth();

    HTMLElement* resultsButton = resultsButtonElement();
    if (RenderBox* resultsRenderer = resultsButton ? resultsButton->renderBox() : 0) {
        resultsRenderer->computeLogicalWidth();
        width -= resultsRenderer->width() + resultsRenderer->marginLeft() + resultsRenderer->marginRight();
    }

    HTMLElement* cancelButton = cancelButtonElement();
    if (RenderBox* cancelRenderer = cancelButton ? cancelButton->renderBox() : 0) {
        cancelRenderer->computeLogicalWidth();
        width -= cancelRenderer->width() + cancelRenderer->marginLeft() + cancelRenderer->marginRight();
    }

    HTMLElement* innerSpinButton = innerSpinButtonElement();
    if (RenderBox* spinRenderer = innerSpinButton ? innerSpinButton->renderBox() : 0) {
        spinRenderer->computeLogicalWidth();
        width -= spinRenderer->width() + spinRenderer->marginLeft() + spinRenderer->marginRight();
    }

#if ENABLE(INPUT_SPEECH)
    HTMLElement* speechButton = speechButtonElement();
    if (RenderBox* speechRenderer = speechButton ? speechButton->renderBox() : 0) {
        speechRenderer->computeLogicalWidth();
        width -= speechRenderer->width() + speechRenderer->marginLeft() + speechRenderer->marginRight();
    }
#endif

    return width;
}

float RenderTextControlSingleLine::getAvgCharWidth(AtomicString family)
{
    // Since Lucida Grande is the default font, we want this to match the width
    // of MS Shell Dlg, the default font for textareas in Firefox, Safari Win and
    // IE for some encodings (in IE, the default font is encoding specific).
    // 901 is the avgCharWidth value in the OS/2 table for MS Shell Dlg.
    if (family == AtomicString("Lucida Grande"))
        return scaleEmToUnits(901);

    return RenderTextControl::getAvgCharWidth(family);
}
    
int RenderTextControlSingleLine::preferredContentWidth(float charWidth) const
{
    int factor = inputElement()->size();
    if (factor <= 0)
        factor = 20;

    int result = static_cast<int>(ceilf(charWidth * factor));

    float maxCharWidth = 0.f;
    AtomicString family = style()->font().family().family();
    // Since Lucida Grande is the default font, we want this to match the width
    // of MS Shell Dlg, the default font for textareas in Firefox, Safari Win and
    // IE for some encodings (in IE, the default font is encoding specific).
    // 4027 is the (xMax - xMin) value in the "head" font table for MS Shell Dlg.
    if (family == AtomicString("Lucida Grande"))
        maxCharWidth = scaleEmToUnits(4027);
    else if (hasValidAvgCharWidth(family))
        maxCharWidth = roundf(style()->font().primaryFont()->maxCharWidth());

    // For text inputs, IE adds some extra width.
    if (maxCharWidth > 0.f)
        result += maxCharWidth - charWidth;

    HTMLElement* resultsButton = resultsButtonElement();
    if (RenderBox* resultsRenderer = resultsButton ? resultsButton->renderBox() : 0)
        result += resultsRenderer->borderLeft() + resultsRenderer->borderRight() +
                  resultsRenderer->paddingLeft() + resultsRenderer->paddingRight();

    HTMLElement* cancelButton = cancelButtonElement();
    if (RenderBox* cancelRenderer = cancelButton ? cancelButton->renderBox() : 0)
        result += cancelRenderer->borderLeft() + cancelRenderer->borderRight() +
                  cancelRenderer->paddingLeft() + cancelRenderer->paddingRight();

#if ENABLE(INPUT_SPEECH)
    HTMLElement* speechButton = speechButtonElement();
    if (RenderBox* speechRenderer = speechButton ? speechButton->renderBox() : 0) {
        result += speechRenderer->borderLeft() + speechRenderer->borderRight() +
                  speechRenderer->paddingLeft() + speechRenderer->paddingRight();
    }
#endif
    return result;
}

void RenderTextControlSingleLine::adjustControlHeightBasedOnLineHeight(int lineHeight)
{
    HTMLElement* resultsButton = resultsButtonElement();
    if (RenderBox* resultsRenderer = resultsButton ? resultsButton->renderBox() : 0) {
        resultsRenderer->computeLogicalHeight();
        setHeight(max(height(),
                  resultsRenderer->borderTop() + resultsRenderer->borderBottom() +
                  resultsRenderer->paddingTop() + resultsRenderer->paddingBottom() +
                  resultsRenderer->marginTop() + resultsRenderer->marginBottom()));
        lineHeight = max(lineHeight, resultsRenderer->height());
    }
    HTMLElement* cancelButton = cancelButtonElement();
    if (RenderBox* cancelRenderer = cancelButton ? cancelButton->renderBox() : 0) {
        cancelRenderer->computeLogicalHeight();
        setHeight(max(height(),
                  cancelRenderer->borderTop() + cancelRenderer->borderBottom() +
                  cancelRenderer->paddingTop() + cancelRenderer->paddingBottom() +
                  cancelRenderer->marginTop() + cancelRenderer->marginBottom()));
        lineHeight = max(lineHeight, cancelRenderer->height());
    }

    setHeight(height() + lineHeight);
}

void RenderTextControlSingleLine::updateFromElement()
{
    RenderTextControl::updateFromElement();

    if (cancelButtonElement())
        updateCancelButtonVisibility();

    if (!inputElement()->suggestedValue().isNull())
        setInnerTextValue(inputElement()->suggestedValue());
    else {
        if (node()->hasTagName(inputTag)) {
            // For HTMLInputElement, update the renderer value if the formControlValueMatchesRenderer()
            // flag is false. It protects an unacceptable renderer value from
            // being overwritten with the DOM value.
            if (!inputElement()->formControlValueMatchesRenderer())
                setInnerTextValue(inputElement()->visibleValue());
        }
    }

    if (m_searchPopupIsVisible)
        m_searchPopup->popupMenu()->updateFromElement();
}

void RenderTextControlSingleLine::cacheSelection(int start, int end)
{
    inputElement()->cacheSelection(start, end);
}

PassRefPtr<RenderStyle> RenderTextControlSingleLine::createInnerTextStyle(const RenderStyle* startStyle) const
{
    RefPtr<RenderStyle> textBlockStyle = RenderStyle::create();   
    textBlockStyle->inheritFrom(startStyle);
    adjustInnerTextStyle(startStyle, textBlockStyle.get());

    textBlockStyle->setWhiteSpace(PRE);
    textBlockStyle->setWordWrap(NormalWordWrap);
    textBlockStyle->setOverflowX(OHIDDEN);
    textBlockStyle->setOverflowY(OHIDDEN);

    // Do not allow line-height to be smaller than our default.
    if (textBlockStyle->fontMetrics().lineSpacing() > lineHeight(true, HorizontalLine, PositionOfInteriorLineBoxes))
        textBlockStyle->setLineHeight(Length(-100.0f, Percent));

    WebCore::EDisplay display = (innerBlockElement() || theme()->shouldHaveSpinButton(inputElement()) ? INLINE_BLOCK : BLOCK);
#if ENABLE(INPUT_SPEECH)
    if (inputElement()->isSpeechEnabled())
      display = INLINE_BLOCK;
#endif
    textBlockStyle->setDisplay(display);

    // We're adding one extra pixel of padding to match WinIE.
    textBlockStyle->setPaddingLeft(Length(1, Fixed));
    textBlockStyle->setPaddingRight(Length(1, Fixed));

    return textBlockStyle.release();
}

PassRefPtr<RenderStyle> RenderTextControlSingleLine::createInnerBlockStyle(const RenderStyle* startStyle) const
{
    RefPtr<RenderStyle> innerBlockStyle = RenderStyle::create();
    innerBlockStyle->inheritFrom(startStyle);

    innerBlockStyle->setDisplay(BLOCK);
    innerBlockStyle->setDirection(LTR);

    // We don't want the shadow dom to be editable, so we set this block to read-only in case the input itself is editable.
    innerBlockStyle->setUserModify(READ_ONLY);

    return innerBlockStyle.release();
}

void RenderTextControlSingleLine::updateCancelButtonVisibility() const
{
    RenderObject* cancelButtonRenderer = cancelButtonElement()->renderer();
    if (!cancelButtonRenderer)
        return;

    const RenderStyle* curStyle = cancelButtonRenderer->style();
    EVisibility buttonVisibility = visibilityForCancelButton();
    if (curStyle->visibility() == buttonVisibility)
        return;

    RefPtr<RenderStyle> cancelButtonStyle = RenderStyle::clone(curStyle);
    cancelButtonStyle->setVisibility(buttonVisibility);
    cancelButtonRenderer->setStyle(cancelButtonStyle);
}

EVisibility RenderTextControlSingleLine::visibilityForCancelButton() const
{
    return inputElement()->value().isEmpty() ? HIDDEN : VISIBLE;
}

const AtomicString& RenderTextControlSingleLine::autosaveName() const
{
    return static_cast<Element*>(node())->getAttribute(autosaveAttr);
}

void RenderTextControlSingleLine::startSearchEventTimer()
{
    unsigned length = text().length();

    // If there's no text, fire the event right away.
    if (!length) {
        stopSearchEventTimer();
        inputElement()->onSearch();
        return;
    }

    // After typing the first key, we wait 0.5 seconds.
    // After the second key, 0.4 seconds, then 0.3, then 0.2 from then on.
    m_searchEventTimer.startOneShot(max(0.2, 0.6 - 0.1 * length));
}

void RenderTextControlSingleLine::searchEventTimerFired(Timer<RenderTextControlSingleLine>*)
{
    inputElement()->onSearch();
}

// PopupMenuClient methods
void RenderTextControlSingleLine::valueChanged(unsigned listIndex, bool fireEvents)
{
    ASSERT(static_cast<int>(listIndex) < listSize());
    HTMLInputElement* input = inputElement();
    if (static_cast<int>(listIndex) == (listSize() - 1)) {
        if (fireEvents) {
            m_recentSearches.clear();
            const AtomicString& name = autosaveName();
            if (!name.isEmpty()) {
                if (!m_searchPopup)
                    m_searchPopup = document()->page()->chrome()->createSearchPopupMenu(this);
                m_searchPopup->saveRecentSearches(name, m_recentSearches);
            }
        }
    } else {
        input->setValue(itemText(listIndex));
        if (fireEvents)
            input->onSearch();
        input->select();
    }
}

String RenderTextControlSingleLine::itemText(unsigned listIndex) const
{
    int size = listSize();
    if (size == 1) {
        ASSERT(!listIndex);
        return searchMenuNoRecentSearchesText();
    }
    if (!listIndex)
        return searchMenuRecentSearchesText();
    if (itemIsSeparator(listIndex))
        return String();
    if (static_cast<int>(listIndex) == (size - 1))
        return searchMenuClearRecentSearchesText();
    return m_recentSearches[listIndex - 1];
}

String RenderTextControlSingleLine::itemLabel(unsigned) const
{
    return String();
}

String RenderTextControlSingleLine::itemIcon(unsigned) const
{
    return String();
}

bool RenderTextControlSingleLine::itemIsEnabled(unsigned listIndex) const
{
     if (!listIndex || itemIsSeparator(listIndex))
        return false;
    return true;
}

PopupMenuStyle RenderTextControlSingleLine::itemStyle(unsigned) const
{
    return menuStyle();
}

PopupMenuStyle RenderTextControlSingleLine::menuStyle() const
{
    return PopupMenuStyle(style()->visitedDependentColor(CSSPropertyColor), style()->visitedDependentColor(CSSPropertyBackgroundColor), style()->font(), style()->visibility() == VISIBLE, style()->display() == NONE, style()->textIndent(), style()->direction(), style()->unicodeBidi() == Override);
}

int RenderTextControlSingleLine::clientInsetLeft() const
{
    // Inset the menu by the radius of the cap on the left so that
    // it only runs along the straight part of the bezel.
    return height() / 2;
}

int RenderTextControlSingleLine::clientInsetRight() const
{
    // Inset the menu by the radius of the cap on the right so that
    // it only runs along the straight part of the bezel (unless it needs
    // to be wider).
    return height() / 2;
}

int RenderTextControlSingleLine::clientPaddingLeft() const
{
    int padding = paddingLeft();

    HTMLElement* resultsButton = resultsButtonElement();
    if (RenderBox* resultsRenderer = resultsButton ? resultsButton->renderBox() : 0)
        padding += resultsRenderer->width() + resultsRenderer->marginLeft() + resultsRenderer->paddingLeft() + resultsRenderer->marginRight() + resultsRenderer->paddingRight();

    return padding;
}

int RenderTextControlSingleLine::clientPaddingRight() const
{
    int padding = paddingRight();

    HTMLElement* cancelButton = cancelButtonElement();
    if (RenderBox* cancelRenderer = cancelButton ? cancelButton->renderBox() : 0)
        padding += cancelRenderer->width() + cancelRenderer->marginLeft() + cancelRenderer->paddingLeft() + cancelRenderer->marginRight() + cancelRenderer->paddingRight();

    return padding;
}

int RenderTextControlSingleLine::listSize() const
{
    // If there are no recent searches, then our menu will have 1 "No recent searches" item.
    if (!m_recentSearches.size())
        return 1;
    // Otherwise, leave room in the menu for a header, a separator, and the "Clear recent searches" item.
    return m_recentSearches.size() + 3;
}

int RenderTextControlSingleLine::selectedIndex() const
{
    return -1;
}

void RenderTextControlSingleLine::popupDidHide()
{
    m_searchPopupIsVisible = false;
}

bool RenderTextControlSingleLine::itemIsSeparator(unsigned listIndex) const
{
    // The separator will be the second to last item in our list.
    return static_cast<int>(listIndex) == (listSize() - 2);
}

bool RenderTextControlSingleLine::itemIsLabel(unsigned listIndex) const
{
    return listIndex == 0;
}

bool RenderTextControlSingleLine::itemIsSelected(unsigned) const
{
    return false;
}

void RenderTextControlSingleLine::setTextFromItem(unsigned listIndex)
{
    inputElement()->setValue(itemText(listIndex));
}

FontSelector* RenderTextControlSingleLine::fontSelector() const
{
    return document()->styleSelector()->fontSelector();
}

HostWindow* RenderTextControlSingleLine::hostWindow() const
{
    return document()->view()->hostWindow();
}

void RenderTextControlSingleLine::autoscroll()
{
    RenderLayer* layer = innerTextElement()->renderBox()->layer();
    if (layer)
        layer->autoscroll();
}

int RenderTextControlSingleLine::scrollWidth() const
{
    if (innerTextElement())
        return innerTextElement()->scrollWidth();
    return RenderBlock::scrollWidth();
}

int RenderTextControlSingleLine::scrollHeight() const
{
    if (innerTextElement())
        return innerTextElement()->scrollHeight();
    return RenderBlock::scrollHeight();
}

int RenderTextControlSingleLine::scrollLeft() const
{
    if (innerTextElement())
        return innerTextElement()->scrollLeft();
    return RenderBlock::scrollLeft();
}

int RenderTextControlSingleLine::scrollTop() const
{
    if (innerTextElement())
        return innerTextElement()->scrollTop();
    return RenderBlock::scrollTop();
}

void RenderTextControlSingleLine::setScrollLeft(int newLeft)
{
    if (innerTextElement())
        innerTextElement()->setScrollLeft(newLeft);
}

void RenderTextControlSingleLine::setScrollTop(int newTop)
{
    if (innerTextElement())
        innerTextElement()->setScrollTop(newTop);
}

bool RenderTextControlSingleLine::scroll(ScrollDirection direction, ScrollGranularity granularity, float multiplier, Node** stopNode)
{
    RenderLayer* layer = innerTextElement()->renderBox()->layer();
    if (layer && layer->scroll(direction, granularity, multiplier))
        return true;
    return RenderBlock::scroll(direction, granularity, multiplier, stopNode);
}

bool RenderTextControlSingleLine::logicalScroll(ScrollLogicalDirection direction, ScrollGranularity granularity, float multiplier, Node** stopNode)
{
    RenderLayer* layer = innerTextElement()->renderBox()->layer();
    if (layer && layer->scroll(logicalToPhysical(direction, style()->isHorizontalWritingMode(), style()->isFlippedBlocksWritingMode()), granularity, multiplier))
        return true;
    return RenderBlock::logicalScroll(direction, granularity, multiplier, stopNode);
}

PassRefPtr<Scrollbar> RenderTextControlSingleLine::createScrollbar(ScrollableArea* scrollableArea, ScrollbarOrientation orientation, ScrollbarControlSize controlSize)
{
    RefPtr<Scrollbar> widget;
    bool hasCustomScrollbarStyle = style()->hasPseudoStyle(SCROLLBAR);
    if (hasCustomScrollbarStyle)
        widget = RenderScrollbar::createCustomScrollbar(scrollableArea, orientation, this);
    else
        widget = Scrollbar::createNativeScrollbar(scrollableArea, orientation, controlSize);
    return widget.release();
}

HTMLInputElement* RenderTextControlSingleLine::inputElement() const
{
    return node()->toInputElement();
}

int RenderTextControlSingleLine::textBlockInsetLeft() const
{
    int inset = borderLeft() + clientPaddingLeft();
    if (HTMLElement* innerText = innerTextElement()) {
        if (RenderBox* innerTextRenderer = innerText->renderBox())
            inset += innerTextRenderer->paddingLeft();
    }
    return inset;
}
    
int RenderTextControlSingleLine::textBlockInsetRight() const
{
    int inset = borderRight() + clientPaddingRight();
    if (HTMLElement* innerText = innerTextElement()) {
        if (RenderBox* innerTextRenderer = innerText->renderBox())
            inset += innerTextRenderer->paddingRight();
    }
    return inset;
}

int RenderTextControlSingleLine::textBlockInsetTop() const
{
    RenderBox* innerRenderer = 0;

    if (HTMLElement* innerBlock = innerBlockElement())
        innerRenderer = innerBlock->renderBox();
    else if (HTMLElement* innerText = innerTextElement())
        innerRenderer = innerText->renderBox();
    
    return innerRenderer ? innerRenderer->y() : borderTop() + paddingTop();
}    

}
