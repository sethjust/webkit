/*
 * Copyright (C) 2001, 2002 Apple Computer, Inc.  All rights reserved.
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

#import "KWQCharsets.h"
#import <CoreFoundation/CoreFoundation.h>

struct CharsetEntry {
    const char *name;
    int mib;
    CFStringEncoding encoding;
};


/* The following autogenerated file includes the charset data. */
#import "KWQCharsetData.c"

static CFMutableDictionaryRef nameToEncoding = NULL;
static CFMutableDictionaryRef mibToEncoding = NULL;
static CFMutableDictionaryRef encodingToName = NULL;
static CFMutableDictionaryRef encodingToMIB = NULL;

static void buildDictionaries (void)
{
  int i;

  nameToEncoding = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, NULL);
  mibToEncoding = CFDictionaryCreateMutable(NULL, 0, NULL, NULL);

  encodingToName = CFDictionaryCreateMutable(NULL, 0, NULL, &kCFTypeDictionaryValueCallBacks);
  encodingToMIB = CFDictionaryCreateMutable(NULL, 0, NULL, NULL);

  for (i = 0; table[i].name != NULL; i++) {
    CFStringRef name;
    name = CFStringCreateWithCString(NULL, table[i].name, kCFStringEncodingASCII);

    if (name != NULL) {
      CFDictionarySetValue(nameToEncoding, name, (void *) table[i].encoding);
      CFDictionarySetValue(encodingToName, (void *) table[i].encoding, name);
    }

    if (table[i].mib != -1) {
      CFDictionarySetValue(mibToEncoding, (void *) table[i].mib, (void *) table[i].encoding);
      CFDictionarySetValue(encodingToMIB, (void *) table[i].encoding, (void *) table[i].mib);
    }
  }
  
  // Add special bogus entry to make UTF-16 mean UTF-8.
  // We do this because Internet Explorere does, and because web pages say UTF-16 and mean UTF-8.
  // See bug 2969378 and http://zingermans.com/ for a concrete example.
  CFDictionarySetValue(nameToEncoding, CFSTR("utf-16"), (void *) kCFStringEncodingUTF8);
}

CFStringEncoding KWQCFStringEncodingFromIANACharsetName(CFStringRef charsetName)
{
  const void *value;

  if (nameToEncoding == NULL) {
    buildDictionaries ();
  }
  
  if (CFDictionaryGetValueIfPresent(nameToEncoding, (void *) charsetName, &value)) {
    return (CFStringEncoding) value;
  } else {
    return kCFStringEncodingInvalidId;
  }
}


CFStringEncoding KWQCFStringEncodingFromMIB(int mib)
{
  const void *value;

  if (mibToEncoding == NULL) {
    buildDictionaries ();
  }

  if (CFDictionaryGetValueIfPresent(mibToEncoding, (void *) mib,  &value)) {
    return (CFStringEncoding) value;
  } else {
    return kCFStringEncodingInvalidId;
  }
}



CFStringRef KWQCFStringEncodingToIANACharsetName(CFStringEncoding encoding)
{
  const void *value;

  if (encodingToName == NULL) {
      buildDictionaries ();
  }

  if (CFDictionaryGetValueIfPresent(encodingToName, (void *) encoding, &value)) {
      return (CFStringRef) value;
  } else {
    return NULL;
  }
}


int KWQCFStringEncodingToMIB(CFStringEncoding encoding)
{
  const void *value;

  if (encodingToMIB == NULL) {
      buildDictionaries ();
  }

  if (CFDictionaryGetValueIfPresent(encodingToMIB, (void *) encoding, &value)) {
      return (int) value;
  } else {
      return -1;
  }
}
