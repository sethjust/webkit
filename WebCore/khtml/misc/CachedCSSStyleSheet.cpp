/*
    This file is part of the KDE libraries

    Copyright (C) 1998 Lars Knoll (knoll@mpi-hd.mpg.de)
    Copyright (C) 2001 Dirk Mueller (mueller@kde.org)
    Copyright (C) 2002 Waldo Bastian (bastian@kde.org)
    Copyright (C) 2004, 2005, 2006 Apple Computer, Inc.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    This class provides all functionality needed for loading images, style sheets and html
    pages from the web. It has a memory cache for these objects.
*/

#include "config.h"
#include "CachedCSSStyleSheet.h"
#include "Cache.h"
#include "loader.h"
#include "CachedObjectClientWalker.h"

#include "KWQLoader.h"

using namespace DOM;

namespace khtml {

CachedCSSStyleSheet::CachedCSSStyleSheet(DocLoader* dl, const DOMString &url, KIO::CacheControl _cachePolicy, time_t _expireDate, const QString& charset)
    : CachedObject(url, CSSStyleSheet, _cachePolicy, _expireDate), m_codec(0)
{
    // It's css we want.
    setAccept("text/css");
    // load the file
    Cache::loader()->load(dl, this, false);
    m_loading = true;
    if (!charset.isEmpty())
        m_codec = QTextCodec::codecForName(charset.latin1());
    if (!m_codec)
        m_codec = QTextCodec::codecForName("iso8859-1");
}

CachedCSSStyleSheet::CachedCSSStyleSheet(const DOMString &url, const QString &stylesheet_data)
    : CachedObject(url, CSSStyleSheet, KIO::CC_Verify, 0, stylesheet_data.length())
{
    m_loading = false;
    m_status = Persistent;
    m_codec = 0;
    m_sheet = DOMString(stylesheet_data);
}

CachedCSSStyleSheet::~CachedCSSStyleSheet()
{
}

void CachedCSSStyleSheet::ref(CachedObjectClient *c)
{
    CachedObject::ref(c);

    if(!m_loading) c->setStyleSheet( m_url, m_sheet );
}

void CachedCSSStyleSheet::deref(CachedObjectClient *c)
{
    Cache::flush();
    CachedObject::deref(c);
    if ( canDelete() && m_free )
      delete this;
}

void CachedCSSStyleSheet::setCharset( const QString &chs )
{
    if (!chs.isEmpty()) {
        QTextCodec *codec = QTextCodec::codecForName(chs.latin1());
        if (codec)
            m_codec = codec;
    }
}

void CachedCSSStyleSheet::data( QBuffer &buffer, bool eof )
{
    if(!eof) return;
    buffer.close();
    setSize(buffer.buffer().size());
    QString data = m_codec->toUnicode( buffer.buffer().data(), size() );
    m_sheet = DOMString(data);
    m_loading = false;

    checkNotify();
}

void CachedCSSStyleSheet::checkNotify()
{
    if (m_loading)
        return;

    CachedObjectClientWalker w(m_clients);
    while (CachedObjectClient *c = w.next()) {
        if (m_response && !KWQIsResponseURLEqualToURL(m_response, m_url))
            c->setStyleSheet(DOMString(KWQResponseURL(m_response)), m_sheet);
        else
            c->setStyleSheet(m_url, m_sheet);
    }
}

void CachedCSSStyleSheet::error( int /*err*/, const char */*text*/ )
{
    m_loading = false;
    checkNotify();
}

};
