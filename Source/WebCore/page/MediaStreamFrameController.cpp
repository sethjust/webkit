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
#include "MediaStreamFrameController.h"

#if ENABLE(MEDIA_STREAM)

#include "DOMWindow.h"
#include "Document.h"
#include "ExclusiveTrackList.h"
#include "Frame.h"
#include "GeneratedStream.h"
#include "MediaStreamController.h"
#include "MultipleTrackList.h"
#include "NavigatorUserMediaErrorCallback.h"
#include "NavigatorUserMediaSuccessCallback.h"
#include "Page.h"
#include "SecurityOrigin.h"
#include <wtf/RefCounted.h>

namespace WebCore {

class MediaStreamFrameController::Request : public RefCounted<Request> {
    WTF_MAKE_NONCOPYABLE(Request);
public:
    virtual ~Request() { }

    ScriptExecutionContext* scriptExecutionContext() const { return m_scriptExecutionContext; }
    virtual bool isGenerateStreamRequest() const { return false; }
    virtual bool isRecordedDataRequest() const { return false; }

    virtual void abort() = 0;

protected:
    Request(ScriptExecutionContext* scriptExecutionContext)
        : m_scriptExecutionContext(scriptExecutionContext) { }

private:
    // This is guaranteed to have the lifetime of the Frame, and it's only used to make
    // the callback asynchronous. The original callback context is used in the call.
    ScriptExecutionContext* m_scriptExecutionContext;
};

class MediaStreamFrameController::GenerateStreamRequest : public Request {
    WTF_MAKE_NONCOPYABLE(GenerateStreamRequest);
public:
    static PassRefPtr<GenerateStreamRequest> create(ScriptExecutionContext* scriptExecutionContext,
                                                    PassRefPtr<NavigatorUserMediaSuccessCallback> successCallback,
                                                    PassRefPtr<NavigatorUserMediaErrorCallback> errorCallback)
    {
        return adoptRef(new GenerateStreamRequest(scriptExecutionContext, successCallback, errorCallback));
    }

    virtual ~GenerateStreamRequest() { }

    virtual bool isGenerateStreamRequest() const { return true; }

    virtual void abort()
    {
        if (m_errorCallback) {
            RefPtr<NavigatorUserMediaError> error = NavigatorUserMediaError::create(NavigatorUserMediaError::PERMISSION_DENIED);
            // The callback itself is made with the JS callback's context, not with the frame's context.
            m_errorCallback->scheduleCallback(scriptExecutionContext(), error);
        }
    }

    PassRefPtr<NavigatorUserMediaSuccessCallback> successCallback() const { return m_successCallback; }
    PassRefPtr<NavigatorUserMediaErrorCallback> errorCallback() const { return m_errorCallback; }

private:
    GenerateStreamRequest(ScriptExecutionContext* scriptExecutionContext,
                          PassRefPtr<NavigatorUserMediaSuccessCallback> successCallback,
                          PassRefPtr<NavigatorUserMediaErrorCallback> errorCallback)
        : Request(scriptExecutionContext)
        , m_successCallback(successCallback)
        , m_errorCallback(errorCallback) { }

    RefPtr<NavigatorUserMediaSuccessCallback> m_successCallback;
    RefPtr<NavigatorUserMediaErrorCallback> m_errorCallback;
};

void MediaStreamFrameController::RequestMap::abort(int requestId)
{
    get(requestId)->abort();
    remove(requestId);
}

void MediaStreamFrameController::RequestMap::abortAll()
{
    while (!isEmpty()) {
        begin()->second->abort();
        remove(begin());
    }
}

template <typename T>
void MediaStreamFrameController::ClientMapBase<T>::unregisterAll()
{
    while (!this->isEmpty()) {
        T key = this->begin()->first;
        // unregister should remove the element from the map.
        this->begin()->second->unregister();
        ASSERT_UNUSED(key, !this->contains(key));
    }
}

template <typename T>
void MediaStreamFrameController::ClientMapBase<T>::detachEmbedder()
{
    for (typename MapType::iterator it = this->begin(); it != this->end(); ++it)
        it->second->detachEmbedder();
}

MediaStreamFrameController::MediaStreamFrameController(Frame* frame)
    : m_frame(frame)
    , m_isInDetachedState(false)
{
    if (!isClientAvailable())
        enterDetachedState();
}

MediaStreamFrameController::~MediaStreamFrameController()
{
}

SecurityOrigin* MediaStreamFrameController::securityOrigin() const
{
    return m_frame ? m_frame->existingDOMWindow()->securityOrigin() : 0;
}

ScriptExecutionContext* MediaStreamFrameController::scriptExecutionContext() const
{
    return m_frame ? m_frame->existingDOMWindow()->scriptExecutionContext() : 0;
}

MediaStreamController* MediaStreamFrameController::pageController() const
{
    return m_frame && m_frame->page() ? m_frame->page()->mediaStreamController() : 0;
}

void MediaStreamFrameController::unregister(StreamClient* client)
{
    ASSERT(m_streams.contains(client->clientId()));

    // Assuming we should stop any live streams when losing access to the embedder.
    if (client->isGeneratedStream()) {
        GeneratedStream* stream = static_cast<GeneratedStream*>(client);
        if (stream->readyState() == Stream::LIVE)
            stopGeneratedStream(stream->label());
    }

    m_streams.remove(client->clientId());
}

void MediaStreamFrameController::unregister(GenericClient* client)
{
    ASSERT(m_clients.contains(client->clientId()));
    m_clients.remove(client->clientId());
}

Stream* MediaStreamFrameController::getStreamFromLabel(const String& label) const
{
    ASSERT(m_streams.contains(label));
    ASSERT(m_streams.get(label)->isStream());
    return static_cast<Stream*>(m_streams.get(label));
}

void MediaStreamFrameController::enterDetachedState()
{
    if (m_isInDetachedState) {
        ASSERT(m_requests.isEmpty());
        return;
    }

    m_requests.abortAll();
    m_streams.detachEmbedder();
    m_clients.detachEmbedder();
    m_isInDetachedState = true;
}

bool MediaStreamFrameController::isClientAvailable() const
{
    if (m_isInDetachedState)
        return false;

    MediaStreamController* controller = pageController();
    return controller && controller->isClientAvailable();
}

// Called also when the frame is detached from the page, in which case the page controller will remain alive.
void MediaStreamFrameController::disconnectPage()
{
    if (pageController())
        pageController()->unregisterFrameController(this);

    enterDetachedState();
}

// Called when the frame is being destroyed. Since the frame controller is owned by the frame it will die shortly after this.
void MediaStreamFrameController::disconnectFrame()
{
    disconnectPage();

    ASSERT(m_requests.isEmpty());
    m_streams.unregisterAll();
    m_clients.unregisterAll();

    m_frame = 0;
}

void MediaStreamFrameController::transferToNewPage(Page*)
{
    // FIXME: In the future we should keep running the media stream services while transfering frames between pages.
    // However, until a proper way to do this is decided, we're shutting down services.
    disconnectPage();
}

GenerateStreamOptionFlags MediaStreamFrameController::parseGenerateStreamOptions(const String& options)
{
    GenerateStreamOptionFlags flags = 0;
    Vector<String> optionList;
    options.split(',', optionList);

    for (Vector<String>::const_iterator option = optionList.begin(); option != optionList.end(); ++option) {
        Vector<String> suboptionList;
        option->split(' ', suboptionList);

        if (suboptionList.first() == "audio")
            flags |= GenerateStreamRequestAudio;
        else if (suboptionList.first() == "video") {
            bool videoSuboptions = false;
            Vector<String>::const_iterator suboption = suboptionList.begin();
            for (++suboption; suboption != suboptionList.end(); ++suboption)
                if (*suboption == "user") {
                    flags |= GenerateStreamRequestVideoFacingUser;
                    videoSuboptions = true;
                } else if (*suboption == "environment") {
                    flags |= GenerateStreamRequestVideoFacingEnvironment;
                    videoSuboptions = true;
                }

            // Ask for all kind of cameras if no suboption was specified.
            if (!videoSuboptions)
                flags |= GenerateStreamRequestVideoFacingUser | GenerateStreamRequestVideoFacingEnvironment;
        }
    }

    return flags;
}

// Implements the getUserMedia method from http://www.whatwg.org/specs/web-apps/current-work/#dom-navigator-getusermedia.
void MediaStreamFrameController::generateStream(const String& options,
                                                PassRefPtr<NavigatorUserMediaSuccessCallback> successCallback,
                                                PassRefPtr<NavigatorUserMediaErrorCallback> errorCallback,
                                                ExceptionCode& ec)
{
    ec = 0;
    if (!successCallback)
        return;

    GenerateStreamOptionFlags flags = parseGenerateStreamOptions(options);
    if (!flags) {
        ec = NOT_SUPPORTED_ERR;
        return;
    }

    int requestId = m_requests.getNextId();
    m_requests.add(requestId, GenerateStreamRequest::create(scriptExecutionContext(), successCallback, errorCallback));

    if (!isClientAvailable()) {
        // This makes sure to call the error callback if provided.
        m_requests.abort(requestId);
        return;
    }

    pageController()->generateStream(this, requestId, flags, securityOrigin());
}

void MediaStreamFrameController::stopGeneratedStream(const String& streamLabel)
{
    ASSERT(m_streams.contains(streamLabel));
    ASSERT(m_streams.get(streamLabel)->isGeneratedStream());

    if (isClientAvailable())
        pageController()->stopGeneratedStream(streamLabel);
}

void MediaStreamFrameController::enableAudioTrack(const String& streamLabel, unsigned long index)
{
    // Don't assert since the audio tracks don't necessarily keep alive their associated generated stream object.
    if (!m_streams.contains(streamLabel))
        return;

    if (isClientAvailable())
        pageController()->enableAudioTrack(streamLabel, index);
}

void MediaStreamFrameController::disableAudioTrack(const String& streamLabel, unsigned long index)
{
    // Don't assert since the audio tracks don't necessarily keep alive their associated generated stream object.
    if (!m_streams.contains(streamLabel))
        return;

    if (isClientAvailable())
        pageController()->disableAudioTrack(streamLabel, index);
}

void MediaStreamFrameController::selectVideoTrack(const String& streamLabel, long index)
{
    // Don't assert since the audio tracks don't necessarily keep alive their associated generated stream object.
    if (!m_streams.contains(streamLabel))
        return;

    if (isClientAvailable())
        pageController()->selectVideoTrack(streamLabel, index);
}

void MediaStreamFrameController::streamGenerated(int requestId, const String& label, PassRefPtr<MultipleTrackList> audioTracksParam, PassRefPtr<ExclusiveTrackList> videoTracksParam)
{
    // Don't assert since the request can have been aborted as a result of embedder detachment.
    if (!m_requests.contains(requestId))
        return;

    ASSERT(m_requests.get(requestId)->isGenerateStreamRequest());
    ASSERT(!label.isNull());
    ASSERT(audioTracksParam);
    ASSERT(videoTracksParam);

    RefPtr<MultipleTrackList> audioTracks = audioTracksParam;
    RefPtr<ExclusiveTrackList> videoTracks = videoTracksParam;

    int audioTracksClientId = m_clients.getNextId();
    audioTracks->associateFrameController(this, audioTracksClientId);
    m_clients.add(audioTracksClientId, audioTracks.get());

    int videoTracksClientId = m_clients.getNextId();
    videoTracks->associateFrameController(this, videoTracksClientId);
    m_clients.add(videoTracksClientId, videoTracks.get());

    RefPtr<GenerateStreamRequest> streamRequest = static_cast<GenerateStreamRequest*>(m_requests.get(requestId).get());
    RefPtr<GeneratedStream> generatedStream = GeneratedStream::create(this, label, audioTracks.release(), videoTracks.release());
    m_streams.add(label, generatedStream.get());
    m_requests.remove(requestId);
    streamRequest->successCallback()->handleEvent(generatedStream.get());
}

void MediaStreamFrameController::streamGenerationFailed(int requestId, NavigatorUserMediaError::ErrorCode code)
{
    // Don't assert since the request can have been aborted as a result of embedder detachment.
    if (!m_requests.contains(requestId))
        return;

    ASSERT(m_requests.get(requestId)->isGenerateStreamRequest());

    RefPtr<GenerateStreamRequest> streamRequest = static_cast<GenerateStreamRequest*>(m_requests.get(requestId).get());
    m_requests.remove(requestId);

    if (streamRequest->errorCallback()) {
        RefPtr<NavigatorUserMediaError> error = NavigatorUserMediaError::create(code);
        streamRequest->errorCallback()->handleEvent(error.get());
    }
}

void MediaStreamFrameController::streamFailed(const String& label)
{
    getStreamFromLabel(label)->streamEnded();
}

void MediaStreamFrameController::audioTrackFailed(const String& label, unsigned long index)
{
    Stream* stream = getStreamFromLabel(label);
    ASSERT(stream->isGeneratedStream());
    static_cast<GeneratedStream*>(stream)->audioTracks()->trackFailed(index);
}

void MediaStreamFrameController::videoTrackFailed(const String& label, unsigned long index)
{
    Stream* stream = getStreamFromLabel(label);
    ASSERT(stream->isGeneratedStream());
    static_cast<GeneratedStream*>(stream)->videoTracks()->trackFailed(index);
}

} // namespace WebCore

#endif // ENABLE(MEDIA_STREAM)
