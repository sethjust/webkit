/*	WebHTMLViewPrivate.h
	Copyright 2002, Apple, Inc. All rights reserved.
        
        Private header file.  This file may reference classes (both ObjectiveC and C++)
        in WebCore.  Instances of this class are referenced by _private in 
        WebHTMLView.
*/

#import <WebKit/WebHTMLView.h>

@class WebBridge;
@class WebController;
@class WebFrame;
@class WebPluginController;

@interface WebHTMLViewPrivate : NSObject
{
@public
    BOOL needsLayout;
    BOOL needsToApplyStyles;
    BOOL canDragTo;
    BOOL canDragFrom;
    BOOL liveAllowsScrolling;
    BOOL inWindow;
    BOOL inNextValidKeyView;
    
    id savedSubviews;
    BOOL subviewsSetAside;

    NSEvent *mouseDownEvent;
    NSDictionary *dragElement;

    NSURL *draggingImageURL;
    
    NSSize lastLayoutSize;
    
    WebPluginController *pluginController;
}
@end

@interface WebHTMLView (WebPrivate)

- (void)_reset;
- (WebController *)_controller;
- (WebFrame *)_frame;
- (WebBridge *)_bridge;
- (void)_adjustFrames;

// Modifier (flagsChanged) tracking SPI
+ (void)_postFlagsChangedEvent:(NSEvent *)flagsChangedEvent;
- (void)_updateMouseoverWithFakeEvent;

- (NSDictionary *)_elementAtPoint:(NSPoint)point;

- (void)_setAsideSubviews;
- (void)_restoreSubviews;

- (BOOL)_insideAnotherHTMLView;
- (void)_updateMouseoverWithEvent:(NSEvent *)event;
- (BOOL)_interceptKeyEvent:(NSEvent *)event toView:(NSView *)view;

+ (NSArray *)_pasteboardTypes;
- (void)_writeSelectionToPasteboard:(NSPasteboard *)pasteboard;

- (void)_frameOrBoundsChanged;

- (NSImage *)_dragImageForElement:(NSDictionary *)element;
- (void)_handleMouseDragged:(NSEvent *)event;
- (void)_handleAutoscrollForMouseDragged:(NSEvent *)event;
- (BOOL)_mayStartDragWithMouseDragged:(NSEvent *)event;

- (WebPluginController *)_pluginController;

@end
