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

//==========================Cocoa===============================
#ifdef VK_USE_PLATFORM_METAL_EXT

#ifndef WINDOW_COCOA
#define WINDOW_COCOA
//-------------------------------------------------
#include "WindowImpl.h"

#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
#else
typedef void* id;
#endif

//#pragma warning(disable:4996)
// clang-format off
// Convert native Cocoa keyboard scancode to cross-platform USB HID code.
const unsigned char COCOA_TO_HID[256] = {
    //TODO
};
// clang-format on
//=============================Cocoa============================
class Window_cocoa : public WindowImpl {
    id appDelegate;
    id window;
    id windowDelegate;
    id view;
    id layer;

    void SetTitle(const char* title);
    void SetWinPos (uint x, uint y);
    void SetWinSize(uint w, uint h);
    void CreateSurface(VkInstance instance);

  public:
    Window_cocoa(const char* title, uint width, uint height);
    virtual ~Window_cocoa();
    EventType GetEvent(bool wait_for_event = false);
    bool CanPresent(VkPhysicalDevice phy, uint32_t queue_family);  // check if this window can present this queue type

    inline id getNativeAppDelegate() {
        return appDelegate;
    }

    inline id getNativeWindow() {
        return window;
    }

    inline id getNativeWindowDelegate() {
        return windowDelegate;
    }

    inline id getNativeView() {
        return view;
    }
};
//==============================================================
#endif

#endif  // VK_USE_PLATFORM_COCOA_KHR
//==============================================================
