/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "Stream.h"

#if ENABLE(MEDIA_STREAM)

#include "Event.h"
#include "ScriptExecutionContext.h"

namespace WebCore {

PassRefPtr<Stream> Stream::create(MediaStreamFrameController* frameController, const String& label)
{
    return adoptRef(new Stream(frameController, label));
}

Stream::Stream(MediaStreamFrameController* frameController, const String& label, bool isGeneratedStream)
    : StreamClient(frameController, label, isGeneratedStream)
    , m_readyState(LIVE)
{
}

Stream::~Stream()
{
}

Stream* Stream::toStream()
{
    return this;
}

void Stream::streamEnded()
{
    ASSERT(m_readyState != ENDED);
    m_readyState = ENDED;
    dispatchEvent(Event::create(eventNames().endedEvent, false, false));
}

ScriptExecutionContext* Stream::scriptExecutionContext() const
{
    return mediaStreamFrameController() ? mediaStreamFrameController()->scriptExecutionContext() : 0;
}

EventTargetData* Stream::eventTargetData()
{
    return &m_eventTargetData;
}

EventTargetData* Stream::ensureEventTargetData()
{
    return &m_eventTargetData;
}

} // namespace WebCore

#endif // ENABLE(MEDIA_STREAM)
