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

#ifndef GeneratedStream_h
#define GeneratedStream_h

#if ENABLE(MEDIA_STREAM)

#include "Stream.h"
#include <wtf/Forward.h>

namespace WebCore {

class ExclusiveTrackList;
class MultipleTrackList;

class GeneratedStream : public Stream {
public:
    static PassRefPtr<GeneratedStream> create(MediaStreamFrameController*, const String& label, PassRefPtr<MultipleTrackList> audioTracks, PassRefPtr<ExclusiveTrackList> videoTracks);
    virtual ~GeneratedStream();

    void stop();

    PassRefPtr<MultipleTrackList> audioTracks() const;
    PassRefPtr<ExclusiveTrackList> videoTracks() const;

    // MediaStreamFrameController::StreamClient implementation.
    virtual void detachEmbedder();
    virtual void streamEnded();

    // EventTarget.
    virtual GeneratedStream* toGeneratedStream();

private:
    GeneratedStream(MediaStreamFrameController*, const String& label, PassRefPtr<MultipleTrackList> audioTracks, PassRefPtr<ExclusiveTrackList> videoTracks);
    class DispatchUpdateTask;
    friend class DispatchUpdateTask;

    void onStop();

    RefPtr<MultipleTrackList> m_audioTracks;
    RefPtr<ExclusiveTrackList> m_videoTracks;
};

} // namespace WebCore

#endif // ENABLE(MEDIA_STREAM)

#endif // GeneratedStream_h
