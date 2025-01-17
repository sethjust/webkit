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

#ifndef Stream_h
#define Stream_h

#if ENABLE(MEDIA_STREAM)

#include "EventNames.h"
#include "EventTarget.h"
#include "MediaStreamFrameController.h"
#include "ScriptExecutionContext.h"
#include <wtf/Forward.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>

namespace WebCore {

class Stream : public RefCounted<Stream>,
               public EventTarget,
               public MediaStreamFrameController::StreamClient {
public:
    // Must match the constants in the .idl file.
    enum {
        LIVE = 1,
        ENDED = 2
    };

    static PassRefPtr<Stream> create(MediaStreamFrameController*, const String& label);
    virtual ~Stream();

    // FIXME: implement the record method when StreamRecorder is available.

    DEFINE_ATTRIBUTE_EVENT_LISTENER(ended);

    unsigned short readyState() const { return m_readyState; }
    const String& label() const { return clientId(); }

    // MediaStreamFrameController::StreamClient implementation.
    virtual void streamEnded();

    // EventTarget implementation.
    virtual Stream* toStream();
    virtual ScriptExecutionContext* scriptExecutionContext() const;

    using RefCounted<Stream>::ref;
    using RefCounted<Stream>::deref;

protected:
    Stream(MediaStreamFrameController*, const String& label, bool isGeneratedStream = false);

    // EventTarget implementation.
    virtual EventTargetData* eventTargetData();
    virtual EventTargetData* ensureEventTargetData();

    unsigned short m_readyState;

private:
    void onEnded();

    // EventTarget implementation.
    virtual void refEventTarget() { ref(); }
    virtual void derefEventTarget() { deref(); }

    EventTargetData m_eventTargetData;
};

} // namespace WebCore

#endif // ENABLE(MEDIA_STREAM)

#endif // Stream_h
