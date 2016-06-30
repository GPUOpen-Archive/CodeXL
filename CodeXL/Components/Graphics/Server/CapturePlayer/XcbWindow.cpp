//==============================================================================
/// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file WindowsWindow.cpp
/// \brief An empty window used for replaying API Frame Capture files.
//==============================================================================

#include <X11/Xlib.h>
#include "XcbWindow.h"

#include "../Common/Logger.h"

const char* kWindowTitle = "Capture Player - [PLACEHOLDER].ACR"; ///< Window title

/// Constructor.
/// \param inWidth The width of the player window
/// \param inHeight The height of the player window
XcbWindow::XcbWindow(UINT windowWidth, UINT windowHeight)
    : WindowBase(windowWidth, windowHeight)
{
}

/// Destructor
XcbWindow::~XcbWindow()
{
}

/// Initialize a connection to XCB
/// \return true if connection can be made, false if error
bool XcbWindow::InitConnection()
{
    const xcb_setup_t* setup;
    xcb_screen_iterator_t iter;
    int scr;

    mConnection = xcb_connect(NULL, &scr);

    if (mConnection == NULL)
    {
        Log(logERROR, "Cannot find a compatible Vulkan installable client driver (ICD).\nExiting ...\n");
        return false;
    }

    setup = xcb_get_setup(mConnection);
    iter = xcb_setup_roots_iterator(setup);

    while (scr-- > 0)
    {
        xcb_screen_next(&iter);
    }

    mScreen = iter.data;
    return true;
}

/// Create a new window and prepare it for use.
/// \returns True if initialization is successful.
bool XcbWindow::Initialize()
{
    uint32_t value_mask, value_list[32];

    if (!InitConnection())
    {
        return false;
    }

    mWindowHandle = xcb_generate_id(mConnection);

    value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    value_list[0] = mScreen->black_pixel;
    value_list[1] = XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_EXPOSURE |
                    XCB_EVENT_MASK_STRUCTURE_NOTIFY;

    xcb_create_window(mConnection, XCB_COPY_FROM_PARENT, mWindowHandle,
                      mScreen->root, 0, 0, m_windowWidth, m_windowHeight, 0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, mScreen->root_visual,
                      value_mask, value_list);

    // Change the title of the window
    xcb_change_property(mConnection, XCB_PROP_MODE_REPLACE, mWindowHandle,
                        XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(kWindowTitle), kWindowTitle);

    // Magic code that will send notification when window is destroyed
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(mConnection, 1, 12, "WM_PROTOCOLS");
    xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(mConnection, cookie, 0);

    xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(mConnection, 0, 16, "WM_DELETE_WINDOW");
    atom_wm_delete_window = xcb_intern_atom_reply(mConnection, cookie2, 0);

    xcb_change_property(mConnection, XCB_PROP_MODE_REPLACE, mWindowHandle,
                        (*reply).atom, 4, 32, 1, &(*atom_wm_delete_window).atom);
    free(reply);

    xcb_map_window(mConnection, mWindowHandle);

    // Force the x/y coordinates to 100,100 results are identical in consecutive
    // runs
    const uint32_t coords[] = { 100, 100 };
    xcb_configure_window(mConnection, mWindowHandle, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, coords);

    return true;
}

/// Shut down and clean up resources associated with a ReplayWindow instance.
/// \returns True if cleanup and shutdown was successful.
bool XcbWindow::Shutdown()
{
    xcb_destroy_window(mConnection, mWindowHandle);
    xcb_disconnect(mConnection);
    return true;
}

/// Open an initialized window in the system UI.
/// \param inNCmdShow Controls how the window is to be shown.
/// \return True if success, false if fail.
bool XcbWindow::OpenAndUpdate(int inNCmdShow)
{
    GT_UNREFERENCED_PARAMETER(inNCmdShow);
    xcb_map_window(mConnection, mWindowHandle);

    return true;
}

/// Update the window. This is the OS-dependent message loop
/// implementation so should be called periodically.
/// \return false if the message loop is to be terminated, true
/// otherwise.
bool XcbWindow::Update()
{
    bool result = true;

    xcb_generic_event_t* event;

    event = xcb_poll_for_event(mConnection);

    if (event)
    {
        uint8_t event_code = event->response_type & 0x7f;

        switch (event_code)
        {
            case XCB_EXPOSE:
            {
                // TODO: Resize window
            }
            break;

            case XCB_CLIENT_MESSAGE:
                if ((*(xcb_client_message_event_t*)event).data.data32[0] == (*atom_wm_delete_window).atom)
                {
                    result = false;
                }

                break;

            case XCB_KEY_RELEASE:
            {
            }
            break;

            case XCB_CONFIGURE_NOTIFY:
            {
                const xcb_configure_notify_event_t* cfg = (const xcb_configure_notify_event_t*)event;

                if ((m_windowWidth != cfg->width) || (m_windowHeight != cfg->height))
                {
                    m_windowWidth = cfg->width;
                    m_windowHeight = cfg->height;
                }
            }
            break;

            default:
            {
            }
            break;
        }
    }

    return result;
}
