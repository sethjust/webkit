/*	
        WebImageRenderer.m
	Copyright (c) 2002, 2003, Apple, Inc. All rights reserved.
*/

#import <WebKit/WebAssertions.h>
#import <WebKit/WebImageRenderer.h>
#import <WebKit/WebImageRendererFactory.h>

#import <WebCore/WebCoreImageRenderer.h>
#import <WebKit/WebAssertions.h>

extern NSString *NSImageLoopCount;

#define MINIMUM_DURATION (1.0/30.0)

// Forward declarations
@interface WebImageRenderer (WebInternal)
- (void)startAnimationIfNecessary;
@end

@implementation WebImageRenderer

static NSMutableSet *activeImageRenderers;

+ (void)stopAnimationsInView:(NSView *)aView
{
    NSEnumerator *objectEnumerator = [activeImageRenderers objectEnumerator];
    WebImageRenderer *renderer;
    NSMutableSet *renderersToStop = [[NSMutableSet alloc] init];

    while ((renderer = [objectEnumerator nextObject])) {
        if (renderer->frameView == aView) {
            [renderersToStop addObject: renderer];
        }
    }

    objectEnumerator = [renderersToStop objectEnumerator];
    while ((renderer = [objectEnumerator nextObject])) {
        if (renderer->frameView == aView) {
            [renderer stopAnimation];
        }
    }
    [renderersToStop release];
}

- (id)initWithMIMEType:(NSString *)MIME
{
    self = [super init];
    if (self != nil) {
        // Work around issue with flipped images and TIFF by never using the image cache.
        // See bug 3344259 and related bugs.
        [self setCacheMode:NSImageCacheNever];

        loadStatus = NSImageRepLoadStatusUnknownType;
        MIMEType = [MIME copy];
        isNull = YES;
        compositeOperator = (int)NSCompositeSourceOver;
    }
    
    return self;
}

- (id)initWithData:(NSData *)data MIMEType:(NSString *)MIME
{
    WebImageRenderer *result = nil;

    NS_DURING
    
        result = [super initWithData:data];
        if (result != nil) {
            // Work around issue with flipped images and TIFF by never using the image cache.
            // See bug 3344259 and related bugs.
            [result setCacheMode:NSImageCacheNever];
    
            result->loadStatus = NSImageRepLoadStatusUnknownType;
            result->MIMEType = [MIME copy];
            result->isNull = [data length] == 0;
            result->compositeOperator = (int)NSCompositeSourceOver;
        }

    NS_HANDLER

        result = nil;

    NS_ENDHANDLER

    return result;
}

- (id)initWithContentsOfFile:(NSString *)filename
{
    NSBundle *bundle = [NSBundle bundleForClass:[self class]];
    NSString *imagePath = [bundle pathForResource:filename ofType:@"tiff"];
    WebImageRenderer *result = nil;

    NS_DURING

        result = [super initWithContentsOfFile:imagePath];
        if (result != nil) {
            // Work around issue with flipped images and TIFF by never using the image cache.
            // See bug 3344259 and related bugs.
            [result setCacheMode:NSImageCacheNever];
    
            result->loadStatus = NSImageRepLoadStatusUnknownType;
            result->compositeOperator = (int)NSCompositeSourceOver;
        }
        
    NS_HANDLER

        result = nil;

    NS_ENDHANDLER

    return result;
}

- (id <WebCoreImageRenderer>)retainOrCopyIfNeeded
{
    WebImageRenderer *copy;

    if (originalData){
        copy = [[[WebImageRendererFactory sharedFactory] imageRendererWithData:originalData MIMEType:MIMEType] retain];
    }
    else {
        copy = [self retain];
    }

    return copy;
}

- copyWithZone:(NSZone *)zone
{
    // FIXME: If we copy while doing an incremental load, it won't work.
    WebImageRenderer *copy;

    copy = [super copyWithZone:zone];
    copy->MIMEType = [MIMEType copy];
    copy->originalData = [originalData retain];
    copy->frameTimer = nil;
    copy->frameView = nil;
    copy->patternColor = nil;
    copy->compositeOperator = compositeOperator;
        
    return copy;
}

- (BOOL)isNull
{
    return isNull;
}

- (void)_adjustSizeToPixelDimensions
{
    // Force the image to use the pixel size and ignore the dpi.
    // Ignore any absolute size in the image and always use pixel dimensions.
    NSBitmapImageRep *imageRep = [[self representations] objectAtIndex:0];
    NSSize size = NSMakeSize([imageRep pixelsWide], [imageRep pixelsHigh]);
    [imageRep setSize:size];
    [self setScalesWhenResized:YES];
    [self setSize:size];
}

- (BOOL)incrementalLoadWithBytes:(const void *)bytes length:(unsigned)length complete:(BOOL)isComplete
{
    NSBitmapImageRep *imageRep = [[self representations] objectAtIndex:0];
    NSData *data = [[NSData alloc] initWithBytes:bytes length:length];

    NS_DURING
        loadStatus = [imageRep incrementalLoadFromData:data complete:isComplete];
    NS_HANDLER
        loadStatus = NSImageRepLoadStatusInvalidData; // Arbitrary choice; any error will do.
    NS_ENDHANDLER

    // Hold onto the original data in case we need to copy this image.  (Workaround for appkit NSImage
    // copy flaw).
    if (isComplete && [self frameCount] > 1)
        originalData = data;
    else
        [data release];

    switch (loadStatus) {
    case NSImageRepLoadStatusUnknownType:       // not enough data to determine image format. please feed me more data
        //printf ("NSImageRepLoadStatusUnknownType size %d, isComplete %d\n", length, isComplete);
        return NO;
    case NSImageRepLoadStatusReadingHeader:     // image format known, reading header. not yet valid. more data needed
        //printf ("NSImageRepLoadStatusReadingHeader size %d, isComplete %d\n", length, isComplete);
        return NO;
    case NSImageRepLoadStatusWillNeedAllData:   // can't read incrementally. will wait for complete data to become avail.
        //printf ("NSImageRepLoadStatusWillNeedAllData size %d, isComplete %d\n", length, isComplete);
        return NO;
    case NSImageRepLoadStatusInvalidData:       // image decompression encountered error.
        //printf ("NSImageRepLoadStatusInvalidData size %d, isComplete %d\n", length, isComplete);
        return NO;
    case NSImageRepLoadStatusUnexpectedEOF:     // ran out of data before full image was decompressed.
        //printf ("NSImageRepLoadStatusUnexpectedEOF size %d, isComplete %d\n", length, isComplete);
        return NO;
    case NSImageRepLoadStatusCompleted:         // all is well, the full pixelsHigh image is valid.
        //printf ("NSImageRepLoadStatusCompleted size %d, isComplete %d\n", length, isComplete);
        [self _adjustSizeToPixelDimensions];        
        isNull = NO;
        return YES;
    default:
        [self _adjustSizeToPixelDimensions];
        //printf ("incrementalLoadWithBytes: size %d, isComplete %d\n", length, isComplete);
        // We have some valid data.  Return YES so we can attempt to draw what we've got.
        isNull = NO;
        return YES;
    }
    
    return NO;
}

- (void)dealloc
{
    ASSERT(frameTimer == nil);
    ASSERT(frameView == nil);
    [patternColor release];
    [MIMEType release];
    [originalData release];
    [super dealloc];
}

- (id)firstRepProperty:(NSString *)propertyName
{
    id firstRep = [[self representations] objectAtIndex:0];
    id property = nil;
    if ([firstRep respondsToSelector:@selector(valueForProperty:)])
        property = [firstRep valueForProperty:propertyName];
    return property;
}

- (int)frameCount
{
    id property = [self firstRepProperty:NSImageFrameCount];
    return property ? [property intValue] : 1;
}

- (int)currentFrame
{
    id property = [self firstRepProperty:NSImageCurrentFrame];
    return property ? [property intValue] : 1;
}

- (void)setCurrentFrame:(int)frame
{
    NSBitmapImageRep *imageRep = [[self representations] objectAtIndex:0];
    [imageRep setProperty:NSImageCurrentFrame withValue:[NSNumber numberWithInt:frame]];
}

- (float)unadjustedFrameDuration
{
    id property = [self firstRepProperty:NSImageCurrentFrameDuration];
    return property ? [property floatValue] : 0.0;
}

- (float)frameDuration
{
    float duration = [self unadjustedFrameDuration];
    if (duration < MINIMUM_DURATION) {
        /*
            Many annoying ads specify a 0 duration to make an image flash
            as quickly as possible.  However a zero duration is faster than
            the refresh rate.  We need to pick a minimum duration.
            
            Browsers handle the minimum time case differently.  IE seems to use something
            close to 1/30th of a second.  Konqueror uses 0.  The ImageMagick library
            uses 1/100th.  The units in the GIF specification are 1/100th of second.
            We will use 1/30th of second as the minimum time.
        */
        duration = MINIMUM_DURATION;
    }
    return duration;
}

- (int)repetitionCount
{
    id property = [self firstRepProperty:NSImageLoopCount];
    int count = property ? [property intValue] : 0;
    return count;
}

- (void)scheduleFrame
{
    if (frameTimer && [frameTimer isValid])
        return;
    frameTimer = [[NSTimer scheduledTimerWithTimeInterval:[self frameDuration]
                                                    target:self
                                                  selector:@selector(nextFrame:)
                                                  userInfo:nil
                                                   repeats:NO] retain];
}

- (void)drawClippedToValidInRect:(NSRect)ir fromRect:(NSRect)fr
{
    if (loadStatus >= 0) {
        // The last line might be a partial line, so the number of complete lines is the number
        // we get from NSImage minus one.
        int numCompleteLines = loadStatus - 1;
        if (numCompleteLines <= 0) {
            return;
        }
        int pixelsHigh = [[[self representations] objectAtIndex:0] pixelsHigh];
        if (pixelsHigh > numCompleteLines) {
            // Figure out how much of the image is OK to draw.  We can't simply
            // use numCompleteLines because the image may be scaled.
            float clippedImageHeight = floor([self size].height * numCompleteLines / pixelsHigh);
            
            // Figure out how much of the source is OK to draw from.
            float clippedSourceHeight = clippedImageHeight - fr.origin.y;
            if (clippedSourceHeight < 1) {
                return;
            }
            
            // Figure out how much of the destination we are going to draw to.
            float clippedDestinationHeight = ir.size.height * clippedSourceHeight / fr.size.height;

            // Reduce heights of both rectangles without changing their positions.
            // In the flipped case, just adjusting the height is sufficient.
            ASSERT([self isFlipped]);
            ASSERT([[NSView focusView] isFlipped]);
            ir.size.height = clippedDestinationHeight;
            fr.size.height = clippedSourceHeight;
        }
    }
    
    // This is the operation that handles transparent portions of the source image correctly.
    [self drawInRect:ir fromRect:fr operation:compositeOperator fraction: 1.0];
}

- (void)nextFrame:(id)context
{
    int currentFrame;
    
    // Release the timer that just fired.
    [frameTimer release];
    frameTimer = nil;
    
    currentFrame = [self currentFrame] + 1;
    if (currentFrame >= [self frameCount]) {
        repetitionsComplete += 1;
        if ([self repetitionCount] && repetitionsComplete >= [self repetitionCount]) {
            animationFinished = YES;
            return;
        }
        currentFrame = 0;
    }
    [self setCurrentFrame:currentFrame];
    
    // Release the tiling pattern so next frame will update the pattern if we're tiling.
    [patternColor release];
    patternColor = nil;
    
    [frameView setNeedsDisplayInRect:targetRect];
}

// Will be called when the containing view is displayed by WebCore RenderImage (via QPainter).
// If the image is an animated image it will begin animating.  If the image is already animating,
// it's frame will have been advanced by nextFrame:.
//
// Also used to draw the image by WebImageView.
- (void)drawImageInRect:(NSRect)ir fromRect:(NSRect)fr
{
    if (animatedTile){
        [self tileInRect:ir fromPoint:tilePoint];
    }
    else {
        [self drawClippedToValidInRect:ir fromRect:fr];
        imageRect = fr;
        targetRect = ir;
        [self startAnimationIfNecessary];
    }
}

- (void)drawImageInRect:(NSRect)ir fromRect:(NSRect)fr compositeOperator:(NSCompositingOperation)operator
{
    compositeOperator = operator;
    [self drawImageInRect:ir fromRect:fr];
}

- (void)startAnimationIfNecessary
{
    if ([self frameCount] > 1 && !animationFinished) {
        NSView *newView = [NSView focusView];
        if (newView != frameView){
            [frameView release];
            frameView = [newView retain];
        }
        [self scheduleFrame];
        if (!activeImageRenderers) {
            activeImageRenderers = [[NSMutableSet alloc] init];
        }
        [activeImageRenderers addObject:self];
    }
}

- (void)stopAnimation
{
    [frameTimer invalidate];
    [frameTimer release];
    frameTimer = nil;
    
    [frameView release];
    frameView = nil;

    [activeImageRenderers removeObject:self];
}

- (void)tileInRect:(NSRect)rect fromPoint:(NSPoint)point
{
    // These calculations are only correct for the flipped case.
    ASSERT([self isFlipped]);
    ASSERT([[NSView focusView] isFlipped]);

    NSSize size = [self size];

    // Check and see if a single draw of the image can cover the entire area we are supposed to tile.
    NSRect oneTileRect;
    oneTileRect.origin.x = rect.origin.x + fmodf(fmodf(-point.x, size.width) - size.width, size.width);
    oneTileRect.origin.y = rect.origin.y + fmodf(fmodf(-point.y, size.height) - size.height, size.height);
    oneTileRect.size = size;

    // Compute the appropriate phase relative to the top level view in the window.
    // Conveniently, the oneTileRect we computed above has the appropriate origin.
    NSPoint originInWindow = [[NSView focusView] convertPoint:oneTileRect.origin toView:nil];
    CGSize phase = CGSizeMake(fmodf(originInWindow.x, size.width), fmodf(originInWindow.y, size.height));
    
    // If the single image draw covers the whole area, then just draw once.
    if (NSContainsRect(oneTileRect, rect)) {
        NSRect fromRect;

        fromRect.origin.x = rect.origin.x - oneTileRect.origin.x;
        fromRect.origin.y = rect.origin.y - oneTileRect.origin.y;
        fromRect.size = rect.size;
        
        [self drawClippedToValidInRect:rect fromRect:fromRect];
        return;
    }

    // If we only have a partial image, just do nothing, because CoreGraphics will not help us tile
    // with a partial image. But maybe later we can fix this by constructing a pattern image that's
    // transparent where needed.
    if (loadStatus != NSImageRepLoadStatusCompleted) {
        return;
    }
    
    // Since we need to tile, construct an NSColor so we can get CoreGraphics to do it for us.
    if (patternColorLoadStatus != loadStatus) {
        [patternColor release];
        patternColor = nil;
    }
    if (patternColor == nil) {
        patternColor = [[NSColor colorWithPatternImage:self] retain];
        patternColorLoadStatus = loadStatus;
    }
        
    [NSGraphicsContext saveGraphicsState];
    
    CGContextSetPatternPhase((CGContextRef)[[NSGraphicsContext currentContext] graphicsPort], phase);    
    [patternColor set];
    [NSBezierPath fillRect:rect];
    
    [NSGraphicsContext restoreGraphicsState];

    animatedTile = YES;
    tilePoint = point;
    targetRect = rect;
    [self startAnimationIfNecessary];
}

- (void)resize:(NSSize)s
{
    [self setScalesWhenResized:YES];
    [self setSize:s];
}

// required by protocol -- apparently inherited methods don't count

- (NSSize)size
{
    return [super size];
}

- (NSString *)MIMEType
{
    return MIMEType;
}

@end
