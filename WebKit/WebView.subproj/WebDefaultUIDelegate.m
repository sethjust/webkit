/*	
    WebDefaultPolicyDelegate.m
    Copyright 2003, Apple Computer, Inc.
*/
#import <Cocoa/Cocoa.h>

#import <WebFoundation/NSURLRequest.h>

#import <WebKit/WebDefaultWindowOperationsDelegate.h>
#import <WebKit/WebView.h>
#import <WebKit/WebWindowOperationsDelegate.h>


@implementation WebDefaultWindowOperationsDelegate

static WebDefaultWindowOperationsDelegate *sharedDelegate = nil;

// Return a object with vanilla implementations of the protocol's methods
// Note this feature relies on our default delegate being stateless.  This
// is probably an invalid assumption for the WebWindowOperationsDelegate.
// If we add any real functionality to this default delegate we probably
// won't be able to use a singleton.
+ (WebDefaultWindowOperationsDelegate *)sharedWindowOperationsDelegate
{
    if (!sharedDelegate) {
        sharedDelegate = [[WebDefaultWindowOperationsDelegate alloc] init];
    }
    return sharedDelegate;
}


- (WebView *)webView: (WebView *)wv createWindowWithRequest:(NSURLRequest *)request;
{
    return nil;
}

- (void)webViewShowWindow: (WebView *)wv;
{
}

- (void)webViewShowWindowBehindFrontmost: (WebView *)wv;
{
}

- (void)webViewCloseWindow: (WebView *)wv;
{
}

- (void)webViewFocusWindow: (WebView *)wv;
{
}

- (void)webViewUnfocusWindow: (WebView *)wv;
{
}

- (NSResponder *)webViewFirstResponderInWindow: (WebView *)wv;
{
    return nil;
}

- (void)webView: (WebView *)wv makeFirstResponderInWindow:(NSResponder *)responder;
{
}

- (void)webView: (WebView *)wv setStatusText:(NSString *)text;
{
}

- (NSString *)webViewStatusText: (WebView *)wv;
{
    return nil;
}

- (void)webView: (WebView *)wv mouseDidMoveOverElement:(NSDictionary *)elementInformation modifierFlags:(unsigned int)modifierFlags;
{
}

- (BOOL)webViewAreToolbarsVisible: (WebView *)wv;
{
    return NO;
}

- (void)webView: (WebView *)wv setToolbarsVisible:(BOOL)visible;
{
}

- (BOOL)webViewIsStatusBarVisible: (WebView *)wv;
{
    return NO;
}

- (void)webView: (WebView *)wv setStatusBarVisible:(BOOL)visible;
{
}

- (BOOL)webViewIsResizable: (WebView *)wv;
{
    return NO;
}

- (void)webView: (WebView *)wv setResizable:(BOOL)resizable;
{
}

- (void)webView: (WebView *)wv setFrame:(NSRect)frame;
{
    [[wv window] setFrame:frame display:NO];
}

- (NSRect)webViewFrame: (WebView *)wv;
{
    return [[wv window] frame];
}

- (void)webView:(WebView *)webView setContentRect:(NSRect)contentRect
{
    [[webView window] setFrame:[NSWindow frameRectForContentRect:contentRect styleMask:[[webView window] styleMask]] display:NO];
}

- (NSRect)webViewContentRect:(WebView *)webView
{
    return [NSWindow contentRectForFrameRect:[[webView window] frame] styleMask:[[webView window] styleMask]];
}

- (void)webView: (WebView *)wv runJavaScriptAlertPanelWithMessage:(NSString *)message;
{
}

- (BOOL)webView: (WebView *)wv runJavaScriptConfirmPanelWithMessage:(NSString *)message
{
    return NO;
}

- (NSString *)webView: (WebView *)wv runJavaScriptTextInputPanelWithPrompt:(NSString *)prompt defaultText:(NSString *)defaultText
{
    return nil;
}

- (void)webView: (WebView *)wv runOpenPanelForFileButtonWithResultListener:(id<WebOpenPanelResultListener>)resultListener
{
}

@end
