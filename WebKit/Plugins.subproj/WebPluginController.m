//
//  WebPluginController.m
//  WebKit
//
//  Created by Chris Blumenberg on Wed Oct 23 2002.
//  Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
//

#import <WebKit/WebPluginController.h>

#import <WebKit/WebBridge.h>
#import <WebKit/WebFramePrivate.h>
#import <WebKit/WebFrameView.h>
#import <WebKit/WebHTMLViewPrivate.h>
#import <WebKit/WebKitLogging.h>
#import <WebKit/WebPlugin.h>
#import <WebKit/WebPluginContainer.h>
#import <WebKit/WebViewPrivate.h>
#import <WebKit/WebUIDelegate.h>

#import <WebCore/WebCoreBridge.h>

#import <Foundation/NSURL_NSURLExtras.h>
#import <Foundation/NSURLRequest.h>

@interface NSView (PluginSecrets)
- (void)setContainingWindow:(NSWindow *)w;
@end

// For compatibility only.
@interface NSView (OldPluginAPI)
- (void)pluginInitialize;
- (void)pluginStart;
- (void)pluginStop;
- (void)pluginDestroy;
@end

@implementation WebPluginController

- initWithHTMLView:(WebHTMLView *)HTMLView
{
    [super init];
    _HTMLView = HTMLView;
    _views = [[NSMutableArray alloc] init];
    return self;
}

- (void)startAllPlugins
{
    if (_started) {
        return;
    }
    
    if ([_views count] > 0) {
        LOG(Plugins, "starting WebKit plugins : %@", [_views description]);
    }
    
    int i, count = [_views count];
    for (i = 0; i < count; i++) {
        id aView = [_views objectAtIndex:i];
        if ([aView respondsToSelector:@selector(webPlugInStart)])
            [aView webPlugInStart];
        else if ([aView respondsToSelector:@selector(pluginStart)])
            [aView pluginStart];
    }
    _started = YES;
}

- (void)stopAllPlugins
{
    if (!_started) {
        return;
    }

    if ([_views count] > 0) {
        LOG(Plugins, "stopping WebKit plugins: %@", [_views description]);
    }
    
    int i, count = [_views count];
    for (i = 0; i < count; i++) {
        id aView = [_views objectAtIndex:i];
        if ([aView respondsToSelector:@selector(webPlugInStop)])
            [aView webPlugInStop];
        else if ([aView respondsToSelector:@selector(pluginStop)])
            [aView pluginStop];
    }
    _started = NO;
}

- (void)addPlugin:(NSView *)view
{
    if (!_HTMLView) {
        ERROR("can't add a plug-in to a defunct WebPluginController");
        return;
    }
    
    if (![_views containsObject:view]) {
        [_views addObject:view];
        
        LOG(Plugins, "initializing plug-in %@", view);
        if ([view respondsToSelector:@selector(webPlugInInitialize)])
            [view webPlugInInitialize];
        else if ([view respondsToSelector:@selector(pluginInitialize)])
            [view pluginInitialize];

        if (_started) {
            LOG(Plugins, "starting plug-in %@", view);
            if ([view respondsToSelector:@selector(webPlugInStart)])
                [view webPlugInStart];
            else if ([view respondsToSelector:@selector(pluginStart)])
                [view pluginStart];
            
            if ([view respondsToSelector:@selector(setContainingWindow:)])
                [view setContainingWindow:[_HTMLView window]];
        }
    }
}

- (void)destroyAllPlugins
{    
    [self stopAllPlugins];

    if ([_views count] > 0) {
        LOG(Plugins, "destroying WebKit plugins: %@", [_views description]);
    }
    
    int i, count = [_views count];
    for (i = 0; i < count; i++) {
        id aView = [_views objectAtIndex:i];
        if ([aView respondsToSelector:@selector(webPlugInDestroy)])
            [aView webPlugInDestroy];
        else if ([aView respondsToSelector:@selector(pluginDestroy)])
            [aView pluginDestroy];
    }
    [_views makeObjectsPerformSelector:@selector(removeFromSuperviewWithoutNeedingDisplay)];
    [_views release];
    _views = nil;

    _HTMLView = nil;
}

- (void)webPlugInContainerLoadRequest:(NSURLRequest *)request inFrame:(NSString *)target
{
    if (!request) {
        ERROR("nil URL passed");
        return;
    }
    if (!_HTMLView) {
        ERROR("could not load URL %@ because plug-in has already been destroyed", request);
        return;
    }
    WebFrame *frame = [_HTMLView _frame];
    if (!frame) {
        ERROR("could not load URL %@ because plug-in has already been stopped", request);
        return;
    }
    if (!target) {
        target = @"_top";
    }
    NSString *JSString = [[request URL] _web_scriptIfJavaScriptURL];
    if (JSString) {
        if ([frame findFrameNamed:target] != frame) {
            ERROR("JavaScript requests can only be made on the frame that contains the plug-in");
            return;
        }
        [[frame _bridge] stringByEvaluatingJavaScriptFromString:JSString];
    } else {
        if (!request) {
            ERROR("could not load URL %@", URL);
            return;
        }
        [frame _loadRequest:request inFrameNamed:target];
    }
}

// For compatibility only.
- (void)showURL:(NSURL *)URL inFrame:(NSString *)target
{
    [self webPlugInContainerLoadRequest:[NSURLRequest requestWithURL:URL] inFrame:target];
}

- (void)webPlugInContainerShowStatus:(NSString *)message
{
    if (!message) {
        message = @"";
    }
    if (!_HTMLView) {
        ERROR("could not show status message (%@) because plug-in has already been destroyed", message);
        return;
    }
    WebView *v = [_HTMLView _webView];
    [[v _UIDelegateForwarder] webView:v setStatusText:message];
}

// For compatibility only.
- (void)showStatus:(NSString *)message
{
    [self webPlugInContainerShowStatus:message];
}

- (NSColor *)webPlugInContainerSelectionColor
{
    return [[_HTMLView _bridge] selectionColor];
}

// For compatibility only.
- (NSColor *)selectionColor
{
    return [self webPlugInContainerSelectionColor];
}

- (WebFrame *)webFrame
{
    return [_HTMLView _frame];
}

@end
