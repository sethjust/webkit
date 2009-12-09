/*
 * Copyright (C) 2009 Google Inc. All rights reserved.
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
#include "ScriptState.h"

#include "Frame.h"
#include "Node.h"
#include "Page.h"
#include "ScriptController.h"

#include <wtf/Assertions.h>
#include <wtf/StdLibExtras.h>

namespace WebCore {

ScriptState::ScriptState(Frame* frame)
    : m_frame(frame)
    , m_context(v8::Persistent<v8::Context>::New(V8Proxy::mainWorldContext(frame)))
{
}

ScriptState::ScriptState(Frame* frame, v8::Handle<v8::Context> context)
    : m_frame(frame)
    , m_context(v8::Persistent<v8::Context>::New(context))
{
}

ScriptState::~ScriptState()
{
    m_context.Dispose();
    m_context.Clear();
}

ScriptState* scriptStateFromNode(DOMWrapperWorld*, Node* node)
{
    // This should be never reached with V8 bindings (WebKit only uses it
    // for non-JS bindings)
    ASSERT_NOT_REACHED();
    return 0;
}

ScriptState* scriptStateFromPage(DOMWrapperWorld*, Page* page)
{
    // This should be only reached with V8 bindings from single process layout tests.
    return page->mainFrame()->script()->mainWorldScriptState();
}

// FIXME: Stub method so we compile.  Currently called from FrameLoader.cpp.
DOMWrapperWorld* mainThreadNormalWorld()
{
    DEFINE_STATIC_LOCAL(DOMWrapperWorld, oneWorld, ());
    return &oneWorld;
}

}
