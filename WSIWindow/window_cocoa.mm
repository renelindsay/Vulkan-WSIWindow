/*
*--------------------------------------------------------------------------
* Copyright (c) 2016 Valve Corporation
* Copyright (c) 2016 LunarG, Inc.
* Copyright (c) 2016-2017 Rene Lindsay
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* Author: Rene Lindsay <rjklindsay@gmail.com>
*
*--------------------------------------------------------------------------
*/

#include "window_cocoa.h"

#import <QuartzCore/QuartzCore.h>

//Last registered event
auto lastEvent = EventType::NONE;

//=====================Cocoa OBJECTS=====================
static const NSRange nsEmptyRange = { NSNotFound, 0 };

@interface CocoaAppDelegate : NSObject <NSApplicationDelegate> {
    
}

- (void) menuItemClicked:(id) sender;

@end

@implementation CocoaAppDelegate

- (void) menuItemClicked:(id) sender {
    
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender {
    return NSTerminateCancel;
}

- (void)applicationDidChangeScreenParameters:(NSNotification *) notification {
    
}

- (void)applicationWillFinishLaunching:(NSNotification *)notification {
    
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    [NSApp stop:nil];
}

- (void)applicationDidHide:(NSNotification *)notification {
    
}

@end

@interface CocoaHelper : NSObject
@end

@implementation CocoaHelper

- (void)selectedKeyboardInputSourceChanged:(NSObject* )object {
    
}

- (void)doNothing:(id)object {
}

@end

@interface CocoaWindowDelegate : NSObject {
    Window_cocoa* window;
    bool initFocusFinished;
}

- (instancetype)initWithWSIWindow:(Window_cocoa*)initWindow;

@end

@implementation CocoaWindowDelegate

- (instancetype)initWithWSIWindow:(Window_cocoa*)initWindow {
    self = [super init];
    if (self != nil) {
        window = initWindow;
        initFocusFinished = false;
    }

    return self;
}

- (BOOL)windowShouldClose:(id)sender {
    lastEvent = EventType::CLOSE;

    return NO;
}

- (void)windowDidResize:(NSNotification *)notification {
    const NSRect contentRect = [window->getNativeView() frame];
    const NSRect fbRect = [window->getNativeView() convertRectToBacking:contentRect];

    //TODO
}

- (void)windowDidMove:(NSNotification*)notification {
    
}

- (void)windowDidMiniaturize:(NSNotification*)notification {
    
}

- (void)windowDidDeminiaturize:(NSNotification*)notification {
    
}

- (void)windowDidBecomeKey:(NSNotification*)notification {
    
}

- (void)windowDidResignKey:(NSNotification*)notification {
    
}

- (void)windowDidChangeOcclusionState:(NSNotification*)notification {
    
}

@end

//View

@interface CocoaContentView : NSView <NSTextInputClient> {
    Window_cocoa* window;
    NSTrackingArea* trackingArea;
    NSMutableAttributedString* markedText;
}

- (instancetype)initWithWSIWindow:(Window_cocoa*)initWindow;

@end

@implementation CocoaContentView

- (instancetype)initWithWSIWindow:(Window_cocoa*)initWindow {
    self = [super init];
    if (self != nil) {
        window = initWindow;
        trackingArea = nil;
        markedText = [[NSMutableAttributedString alloc] init];

        [self updateTrackingAreas];
        [self registerForDraggedTypes:@[NSPasteboardTypeURL]];
    }

    return self;
}

- (void)dealloc {
    [trackingArea release];
    [markedText release];
    [super dealloc];
    [super release];
}

- (BOOL)isOpaque {
    return YES;
}

- (BOOL)canBecomeKeyView {
    return YES;
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (BOOL)wantsUpdateLayer {
    return YES;
}

- (void)updateLayer {
    
}

- (void)cursorUpdate:(NSEvent*)event {
    
}

- (BOOL)acceptsFirstMouse:(NSEvent*)event {
    return YES;
}

- (void)mouseDown:(NSEvent*)event {
    //TODO
}

- (void)mouseDragged:(NSEvent*)event {
    [self mouseMoved:event];
}

- (void)mouseUp:(NSEvent*)event {
    //TODO
}

- (void)mouseMoved:(NSEvent*)event {
    //TODO
}

- (void)rightMouseDown:(NSEvent*)event {
    //TODO
}

- (void)rightMouseDragged:(NSEvent*)event {
    [self mouseMoved:event];
}

- (void)rightMouseUp:(NSEvent*)event {
    //TODO
}

- (void)otherMouseDown:(NSEvent*)event {
    
}

- (void)otherMouseDragged:(NSEvent*)event {
    [self mouseMoved:event];
}

- (void)otherMouseUp:(NSEvent*)event {
    
}

- (void)mouseExited:(NSEvent*)event {
    //TODO
}

- (void)mouseEntered:(NSEvent*)event {
    //TODO
}

- (void)viewDidChangeBackingProperties {
    const NSRect contentRect = [window->getNativeView() frame];
    const NSRect fbRect = [window->getNativeView() convertRectToBacking:contentRect];
    //TODO
}

- (void)drawRect:(NSRect)rect {
    
}

- (void)updateTrackingAreas {
    if (trackingArea != nil) {
        [self removeTrackingArea:trackingArea];
        [trackingArea release];
    }

    const NSTrackingAreaOptions options = NSTrackingMouseEnteredAndExited |
                                          NSTrackingActiveInKeyWindow |
                                          NSTrackingEnabledDuringMouseDrag |
                                          NSTrackingCursorUpdate |
                                          NSTrackingInVisibleRect |
                                          NSTrackingAssumeInside;

    trackingArea = [[NSTrackingArea alloc] initWithRect:[self bounds]
                                                options:options
                                                  owner:self
                                               userInfo:nil];

    [self addTrackingArea:trackingArea];
    [super updateTrackingAreas];
}

- (void)keyDown:(NSEvent *)event {
    //TODO
}

- (void)flagsChanged:(NSEvent *)event {
    //TODO
}

- (void)keyUp:(NSEvent*)event {
    //TODO
}

- (void)scrollWheel:(NSEvent*)event {
    //TODO
}

- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender {
    return NSDragOperationGeneric;
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender {
    return YES;
}

- (BOOL)hasMarkedText {
    return [markedText length] > 0;
}

- (NSRange)markedRange {
    if ([markedText length] > 0)
        return NSMakeRange(0, [markedText length] - 1);
    else
        return nsEmptyRange;
}

- (NSRange)selectedRange {
    return nsEmptyRange;
}

- (void)setMarkedText:(id)string
        selectedRange:(NSRange)selectedRange
     replacementRange:(NSRange)replacementRange {
    [markedText release];
    if ([string isKindOfClass:[NSAttributedString class]])
        markedText = [[NSMutableAttributedString alloc] initWithAttributedString:string];
    else
        markedText = [[NSMutableAttributedString alloc] initWithString:string];
}

- (void)unmarkText {
    [[markedText mutableString] setString:@""];
}

- (NSArray*)validAttributesForMarkedText {
    return [NSArray array];
}

- (NSAttributedString*)attributedSubstringForProposedRange:(NSRange)range
                                               actualRange:(NSRangePointer)actualRange {
    return nil;
}

- (NSUInteger)characterIndexForPoint:(NSPoint)point {
    return 0;
}

- (NSRect)firstRectForCharacterRange:(NSRange)range
                         actualRange:(NSRangePointer)actualRange {
    const NSRect frame = [window->getNativeView() frame];
    return NSMakeRect(frame.origin.x, frame.origin.y, 0.0, 0.0);
}

- (void)insertText:(id)string replacementRange:(NSRange)replacementRange {
    //TODO
}

- (void)doCommandBySelector:(SEL)selector {

}

@end

//Window Object

@interface CocoaWindow : NSWindow {}
@end

@implementation CocoaWindow

- (BOOL)canBecomeKeyWindow {
    return YES;
}

- (BOOL)canBecomeMainWindow {
    return YES;
}

@end

//=====================Cocoa IMPLEMENTATION=====================
Window_cocoa::Window_cocoa(const char* title, uint width, uint height) {
    shape.width  = width;
    shape.height = height;
    running      = true;
    LOGI("Creating Cocoa Window...\n");

    //=====================Helper=====================
    CocoaHelper* helper = [[CocoaHelper alloc] init];
    
    [NSThread detachNewThreadSelector:@selector(doNothing:)
                             toTarget:helper
                           withObject:nil];

    [NSApplication sharedApplication];

    //=====================App delegate=====================
    id appDelegate = [[CocoaAppDelegate alloc] init];
    [NSApp setDelegate:appDelegate];

    NSEvent* (^block)(NSEvent*) = ^ NSEvent* (NSEvent* event) {
        if ([event modifierFlags] & NSEventModifierFlagCommand)
            [[NSApp keyWindow] sendEvent:event];

        return event;
    };

    [[NSNotificationCenter defaultCenter]
        addObserver:helper
           selector:@selector(selectedKeyboardInputSourceChanged:)
               name:NSTextInputContextKeyboardSelectionDidChangeNotification
             object:nil];
    
    CGEventSourceSetLocalEventsSuppressionInterval(CGEventSourceCreate(kCGEventSourceStateHIDSystemState), 0.0);

    //if (![[NSRunningApplication currentApplication] isFinishedLaunching])
    //    [NSApp run];

    //=====================Window delegate=====================
    windowDelegate = [[CocoaWindowDelegate alloc] initWithWSIWindow:this];
    //if (windowDelegate == NULL) {
    //    LVND_ERROR("Failed to create NS window delegate");
    //}

    NSRect contentRect = NSMakeRect(0, 0, width, height);

    NSUInteger styleMask = NSWindowStyleMaskMiniaturizable;
    styleMask |= (NSWindowStyleMaskTitled | NSWindowStyleMaskClosable);
    styleMask |= NSWindowStyleMaskResizable;
    window = [[CocoaWindow alloc]
        initWithContentRect:contentRect
                  styleMask:styleMask
                    backing:NSBackingStoreBuffered
                      defer:NO];
    //if (window == NULL) {
    //    LVND_ERROR("Failed to create NS window");
    //}
    
    [(NSWindow*)window center];

    const NSWindowCollectionBehavior behavior =
        NSWindowCollectionBehaviorFullScreenPrimary |
        NSWindowCollectionBehaviorManaged;
    [window setCollectionBehavior:behavior];

    view = [[CocoaContentView alloc] initWithWSIWindow:this];
    //if (view == NULL) {
    //    LVND_ERROR("Failed to create NS view");
    //}

    [window setContentView:view];
    [window makeFirstResponder:view];
    [window setTitle:@(title)];
    [window setDelegate:windowDelegate];
    [window setAcceptsMouseMovedEvents:YES];
    [window setRestorable:NO];

    [NSApp activateIgnoringOtherApps:YES];
    [window makeKeyAndOrderFront:view];

    //Getting some properties
    /*
    const NSRect fbRect = [(id)window->handle->view convertRectToBacking:contentRect];
    window->framebufferWidth = fbRect.size.width;
    window->framebufferHeight = fbRect.size.height;

    //Retina
    if (window->width == window->framebufferWidth && window->height == window->framebufferHeight)
        window->handle->isRetina = false;
    else
        window->handle->isRetina = true;
    
    const NSPoint mousePos = [(id)window->handle->window mouseLocationOutsideOfEventStream];
    window->mouseX = mousePos.x;
    window->mouseY = mousePos.y;
    */

    //[app setDelegate:handle->delegate];

    //[(id)window->handle->window orderFrontRegardless];
    [NSApp run];
}

Window_cocoa::~Window_cocoa() {
    //TODO
}

void Window_cocoa::SetTitle(const char* title) {
    //TODO
}

void Window_cocoa::SetWinPos(uint x, uint y) {
    //TODO
}

void Window_cocoa::SetWinSize(uint w, uint h) {
    //TODO
}

void Window_cocoa::CreateSurface(VkInstance instance) {
    if (surface) return;
    this->instance = instance;
    
    CAMetalLayer* metalLayer = [CAMetalLayer layer];
    //[layer setContentsScale:window->framebufferScaleX];
    layer = metalLayer;

    NSWindow* nswindow = window;
    nswindow.contentView.layer = layer;
    nswindow.contentView.wantsLayer = YES;

    VkMetalSurfaceCreateInfoEXT surfaceCreateInfo;
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    surfaceCreateInfo.pNext = NULL;
    surfaceCreateInfo.pLayer = layer;
    surfaceCreateInfo.flags = 0;

    VKERRCHECK(vkCreateMetalSurfaceEXT(instance, &surfaceCreateInfo, NULL, &surface));

    LOGI("Vulkan Surface created\n");
}

EventType Window_cocoa::GetEvent(bool wait_for_event) {
    @autoreleasepool {
    
    //for (;;) {
    NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                        untilDate:[NSDate distantPast]
                                            inMode:NSDefaultRunLoopMode
                                            dequeue:YES];
    if (event == nil)
        return {EventType::UNKNOWN};

    [NSApp sendEvent:event];
    //}

    } // autoreleasepool

    switch (lastEvent) {
    case EventType::CLOSE:
        return CloseEvent();
    default:
        return {EventType::NONE};
    }
}

// Return true if this window can present the given queue type
bool Window_cocoa::CanPresent(VkPhysicalDevice gpu, uint32_t queue_family) {
    //TODO
}
