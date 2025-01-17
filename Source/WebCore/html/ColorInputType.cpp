/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
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
#include "ColorInputType.h"

#include "Color.h"
#include "ElementWithPseudoId.h"
#include "HTMLInputElement.h"
#include "ShadowRoot.h"
#include <wtf/PassOwnPtr.h>
#include <wtf/text/WTFString.h>

#if ENABLE(INPUT_COLOR)

namespace WebCore {

static bool isValidColorString(const String& value)
{
    if (value.isEmpty())
        return false;
    if (value[0] != '#')
        return false;

    // We don't accept #rgb and #aarrggbb formats.
    if (value.length() != 7)
        return false;
    Color color(value);
    return color.isValid() && !color.hasAlpha();
}

PassOwnPtr<InputType> ColorInputType::create(HTMLInputElement* element)
{
    return adoptPtr(new ColorInputType(element));
}

bool ColorInputType::isColorControl() const
{
    return true;
}

const AtomicString& ColorInputType::formControlType() const
{
    return InputTypeNames::color();
}

bool ColorInputType::supportsRequired() const
{
    return false;
}

String ColorInputType::fallbackValue()
{
    return String("#000000");
}

String ColorInputType::sanitizeValue(const String& proposedValue)
{
    if (proposedValue.isNull())
        return proposedValue;

    if (!isValidColorString(proposedValue))
        return fallbackValue();

    return proposedValue.lower();
}

void ColorInputType::createShadowSubtree()
{
    Document* document = element()->document();
    RefPtr<HTMLElement> wrapperElement = ElementWithPseudoId::create(document, "-webkit-color-swatch-wrapper");
    ExceptionCode ec = 0;
    wrapperElement->appendChild(ElementWithPseudoId::create(document, "-webkit-color-swatch"), ec);
    element()->ensureShadowRoot()->appendChild(wrapperElement.release(), ec);
    
    updateColorSwatch();
}

void ColorInputType::valueChanged()
{
    updateColorSwatch();
}

void ColorInputType::updateColorSwatch()
{
    HTMLElement* colorSwatch = shadowColorSwatch();
    if (!colorSwatch)
        return;

    ExceptionCode ec;
    colorSwatch->style()->setProperty("background-color", element()->value(), ec);
}

HTMLElement* ColorInputType::shadowColorSwatch() const
{
    Node* shadow = element()->shadowRoot();
    return shadow ? toHTMLElement(shadow->firstChild()->firstChild()) : 0;
}

} // namespace WebCore

#endif // ENABLE(INPUT_COLOR)
