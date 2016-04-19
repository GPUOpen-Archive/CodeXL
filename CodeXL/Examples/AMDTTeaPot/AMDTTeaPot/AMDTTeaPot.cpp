//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTTeaPot.cpp
///
//==================================================================================

//------------------------------ AMDTTeaPot.cpp ----------------------------

#include <windows.h>
#include <commctrl.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include "AMDTTeaPot.h"
#include "AMDTTeapotConstants.h"
#include "AMDTTeapotOGLCanvas.h"
#include "AMDTTeapotOCLSmokeSystem.h"
#include "AMDTMisc.h"
#include "AMDTDebug.h"
#include "AMDTMainWin.h"
#include "GL\gl.h"
#include "GL\glu.h"

// Some helpful macros
#define MAX_STATUSBAR_LEN       (1<<8)
#define MAX_LOADSTRING          (1<<8)

// Timer related
#define ID_IDLE_TIMER           (1<<15)
#define TIMER_UELASPSE          (0)

// Menus related
#define FILE_MENU_NPOS          (0)
#define COMMANDS_MENU_NPOS      (1)
#define VIEW_MENU_NPOS          (2)
#define SMOKE_MENU_NPOS         (3)
#define HELP_MENU_NPOS          (4)

#define SMOKE_SIM_SUBMENU_NPOS  (9)
#define SMOKE_SLC_SUBMENU_NPOS  (10)

// Main GUI related
#define ID_STATUSBAR            (ID_IDLE_TIMER)
#define ID_TEAPOT_WIN           (ID_STATUSBAR+1)

// Global Variables:
HINSTANCE hInst;                                // current instance
TCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
TCHAR szTeapotClass[MAX_LOADSTRING];            // the teapot window class name
static WNDPROC stOriginalWindowProc;            // keep running on main window while about is open
static HCURSOR stHandCursor;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int, HWND*);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    AboutWndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    AboutDialog1(HWND, UINT, WPARAM, LPARAM);
inline void createSmokeDeviceSubMenu(HMENU, int , int, const OCLInfo* oclInfo, SmokeSystemCommandId);

// Instance of the connection between GUI and Teapot logics
MainWin* mainWin = NULL;

// Some useful macros
#define MAIN_WIN_DO(func, ...)\
    do {                                                                      \
        if (mainWin != NULL)                                                \
        {                                                                   \
            mainWin->func(__VA_ARGS__);                                     \
        }                                                                   \
        __pragma(warning(push))                                                     \
        __pragma(warning(disable : 4127))                                           \
    } while (0)                                                             \
        __pragma(warning(pop))


#define PAINT_WRAP(CODE)                                                    \
    do {                                                                    \
        PAINTSTRUCT __ps;                                                   \
        HWND __hWndTP = GetDlgItem(hWnd, ID_TEAPOT_WIN);                    \
        HDC __hdc = BeginPaint(__hWndTP, &__ps);                            \
        { CODE }                                                            \
        SwapBuffers( __hdc );                                               \
        EndPaint(__hWndTP, &__ps);                                          \
        __pragma(warning(push))                                             \
        __pragma(warning(disable : 4127))                                   \
    } while (0)                                                \
        __pragma(warning(pop))
#define COMMAND_TP_CASE_DO(CASE, CODE)      case CASE: { CODE } break

#define SET_STATUSBAR_PARTS_SIZES(SZ)                                                       \
    do {                                                                                    \
        int sbConf[3] = { 50, 260, -1 };                                                    \
        if (SZ > 470) {                                                                     \
            sbConf[1] = SZ - 210;                                                           \
        } else if (SZ <= 250) {                                                             \
            sbConf[1] = 150;                                                                \
        } else if (SZ <= 360) {                                                             \
            sbConf[1] = SZ - 100;                                                           \
        }                                                                                   \
        ::SendMessage(::GetDlgItem(hWnd, ID_STATUSBAR), SB_SETPARTS, 3, (LPARAM)sbConf);    \
        __pragma(warning(push))                                                             \
        __pragma(warning(disable : 4127))                                                   \
    } while (0)                                                                \
        __pragma(warning(pop))

#define SET_STATUSBAR_PART_TEXT(PART, FMT, ...)                                                     \
    do {                                                                                            \
        wchar_t __sbMessage[MAX_STATUSBAR_LEN];                                                     \
        ::_snwprintf(__sbMessage, MAX_STATUSBAR_LEN, FMT, __VA_ARGS__);                             \
        ::SendMessage(::GetDlgItem(hWnd, ID_STATUSBAR), SB_SETTEXT, (PART), (LPARAM)__sbMessage);   \
        __pragma(warning(push))                                                                     \
        __pragma(warning(disable : 4127))                                                           \
    } while (0)                                                                        \
        __pragma(warning(pop))

#define SET_STATUSBAR_STR_PART_TEXT(PART, MSG)                                                      \
    do {                                                                                            \
        if (MSG != NULL) {                                                                          \
            wchar_t     ___sbMessage[MAX_STATUSBAR_LEN];                                            \
            ::mbstowcs(___sbMessage, MSG, MAX_STATUSBAR_LEN);                                       \
            SET_STATUSBAR_PART_TEXT((PART), ___sbMessage);                                          \
        }                                                                                           \
        __pragma(warning(push))                                                                     \
        __pragma(warning(disable : 4127))                                                           \
    } while (0)                                                                        \
        __pragma(warning(pop))

#define REMOVE_CL_MENU()                                                \
    do {                                                                \
        ::Sleep(3);                                                     \
        ::RemoveMenu(::GetMenu(hWnd), SMOKE_MENU_NPOS, MF_BYPOSITION);  \
        ::DrawMenuBar(hWnd);                                            \
        __pragma(warning(push))                                         \
        __pragma(warning(disable : 4127))                               \
    } while (0)                                            \
        __pragma(warning(pop))


void createSmokeDeviceSubMenu(
    HMENU subMenu,
    int minMenuId,
    int maxMenuId,
    const OCLInfo* oclInfo,
    SmokeSystemCommandId cmdId,
    const OCLDevice* defDev
)
{
    const int MAX_NAME_SIZE = 80;
    char name[MAX_NAME_SIZE];
    int id = minMenuId;

    for (int i = 0; i < oclInfo->getNumPlatforms() && id < maxMenuId; ++i)
    {
        const OCLPlatform* platform = oclInfo->getPlatform(i);

        for (int j = 0; j < platform->getNumDevices() && id <= maxMenuId; ++j)
        {
            const OCLDevice* device = platform->getDevice(j);
            strncpy(name, device->getName(), MAX_NAME_SIZE);
            name[sizeof(name) - 1] = '\0';
            wchar_t* wname = new wchar_t[MAX_NAME_SIZE];
            ::mbstowcs(wname, name, MAX_NAME_SIZE);
            ASSERT(::AppendMenu(subMenu, MF_BYPOSITION | MF_STRING, id, &wname[4]), "Failed to append device menu item properly");
            MENUITEMINFO mii =
            {
                sizeof(MENUITEMINFO),
                MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE,
                MFT_STRING | MFT_RADIOCHECK,
                0,
                UINT(id),
                0,
                0,
                0,
                (ULONG_PTR) new SmokeSystemCommandChangeDevice(cmdId, device),
                wname,
                strlen(name),
                0
            };
            ASSERT(::SetMenuItemInfo(subMenu, j, TRUE, &mii), "Failed to set device menu item properly");

            if (device == defDev)
            {
                ::CheckMenuRadioItem(subMenu,
                                     minMenuId,
                                     maxMenuId,
                                     id,
                                     MF_BYCOMMAND);
            }

            id++;
        }
    }
}

int APIENTRY _tWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPTSTR    lpCmdLine,
    int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    HWND  hWnd;
    HDC   hDC;
    HGLRC hRC;


    // TODO: Place code here.
    MSG msg;
    HACCEL hAccelTable;

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, szTitle,       MAX_LOADSTRING);
    LoadString(hInstance, IDC_WRAPPER,   szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    ASSERT(InitInstance(hInstance, nCmdShow, &hWnd), "Failed to perform application initializations");

    hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WRAPPER));

    // Add the Teapot window and status bar
    RECT sbRect, clientRect;
    HWND  hWndStatus = ::CreateWindowEx(0L,                                 // no extended styles
                                        STATUSCLASSNAME,                    // status bar
                                        NULL,                               // no text
                                        WS_CHILD | WS_BORDER | WS_VISIBLE,  // styles
                                        0, 0, 0, 0,                         // x, y, cx, cy
                                        hWnd,                               // parent window
                                        (HMENU)ID_STATUSBAR,                // window ID
                                        hInst,                              // instance
                                        NULL);                              // window data

    SET_STATUSBAR_PART_TEXT(1, L"Setting up GUI...");
    ::GetWindowRect(hWndStatus, &sbRect);
    ::GetClientRect(hWnd, &clientRect);
    HWND hWndTeapot = ::CreateWindowEx(0L,                                  // no extended styles
                                       szWindowClass,                       // Window class
                                       NULL,                                // no text
                                       WS_CHILD | WS_VISIBLE,               // styles
                                       0, 0,                                // Relative to client rect of the parent
                                       clientRect.right - clientRect.left, clientRect.bottom - clientRect.top - (sbRect.bottom - sbRect.top),                   // cx, cy
                                       hWnd,                                // parent window
                                       (HMENU)ID_TEAPOT_WIN,                // window ID
                                       hInst,                               // instance
                                       NULL);                               // window data

    // Add the CL device menus
    HMENU hMenu       = ::GetMenu(hWnd);
    HMENU hMenu_smoke = ::GetSubMenu(hMenu, SMOKE_MENU_NPOS);
    HMENU hMenu_sim   = ::CreatePopupMenu();
    HMENU hMenu_slc   = ::CreatePopupMenu();

    // set default
    ::CheckMenuRadioItem(hMenu_smoke,
                         ID_SMOKE_GRID_32_32_64,
                         ID_SMOKE_GRID_128_128_256,
                         ID_SMOKE_GRID_64_64_64,
                         MF_BYCOMMAND);

    ::InsertMenu(hMenu_smoke, SMOKE_SIM_SUBMENU_NPOS, MF_BYPOSITION | MF_STRING | MF_POPUP, PtrToUint(ULongToPtr(HandleToULong(hMenu_sim))), TP_SUBMENU_SMOKE_SIM);
    ::InsertMenu(hMenu_smoke, SMOKE_SLC_SUBMENU_NPOS, MF_BYPOSITION | MF_STRING | MF_POPUP, PtrToUint(ULongToPtr(HandleToULong(hMenu_slc))), TP_SUBMENU_VOL_SLICE);

    SET_STATUSBAR_PART_TEXT(1, L"Loading Teapot...");
    mainWin = MainWin::instance();
    mainWin->init(hWndTeapot, &hDC, &hRC);
    SET_STATUSBAR_PARTS_SIZES(470);

    SET_STATUSBAR_PART_TEXT(0, L"%0.1f FPS", 0.0f);
    SET_STATUSBAR_PART_TEXT(1, L"Adding CL devices...");
    SET_STATUSBAR_PART_TEXT(2, L"OpenCL-OpenGL Sharing is %s", mainWin->usingGLCLSharing() ? L"Active" : L"Inactive");
    // Add OCL devices menus, and set the radio button on the picked device
    AMDTOpenCLHelper*  oclh  = AMDTOpenCLHelper::GetInstance();
    const OCLInfo*   OCLInfo = oclh->getOpenCLInfo();

    if (OCLInfo != NULL)
    {
        createSmokeDeviceSubMenu(hMenu_sim,
                                 ID_SMOKE_SIM_DEVICE_MIN_ID,
                                 ID_SMOKE_SIM_DEVICE_MAX_ID,
                                 OCLInfo,
                                 SSCID_CHANGE_SMOKE_SIM_DEVICE,
                                 mainWin->getOCLSmokeSimDevice()
                                );
        createSmokeDeviceSubMenu(hMenu_slc,
                                 ID_VOL_SLICE_DEVICE_MIN_ID,
                                 ID_VOL_SLICE_DEVICE_MAX_ID,
                                 OCLInfo,
                                 SSCID_CHANGE_VOL_SLICE_DEVICE,
                                 mainWin->getOCLVolSliceDevice()
                                );
    }
    else
    {
        SET_STATUSBAR_PART_TEXT(1, L"No CL devices found!");
        REMOVE_CL_MENU();
    }

    SET_STATUSBAR_STR_PART_TEXT(1, mainWin->getProgressMessage());

    if (mainWin->usingCL())
    {
        SET_STATUSBAR_PART_TEXT(2, L"OpenCL-OpenGL Sharing is %s", mainWin->usingGLCLSharing() ? L"Active" : L"Inactive");
    }
    else
    {
        SET_STATUSBAR_PART_TEXT(2, L"OpenCL is Disabled");
    }

    // Add idle timer
    UINT_PTR timerID = ::SetTimer(hWnd, ID_IDLE_TIMER, TIMER_UELASPSE, NULL);

    // Main message loop:
    while (::GetMessage(&msg, NULL, 0, 0))
    {
        if (!::TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }

    ::KillTimer(hWnd, timerID);
    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(
    HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = ::LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WRAPPER));
    wcex.hCursor        = ::LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName   = MAKEINTRESOURCE(IDC_WRAPPER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = ::LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return ::RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(
    HINSTANCE hInstance,
    int nCmdShow,
    HWND* hWnd)
{
    BOOL retVal = TRUE;
    hInst = hInstance; // Store instance handle in our global variable

    *hWnd = ::CreateWindow(szWindowClass,
                           szTitle,
                           WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                           TP_MAIN_WINDOW_POS_X, TP_MAIN_WINDOW_POS_Y,
                           TP_MAIN_WINDOW_WIDTH, TP_MAIN_WINDOW_HEIGHT,
                           NULL, NULL, hInstance, NULL);

    if (!*hWnd)
    {
        retVal = FALSE;
    }
    else
    {
        ::ShowWindow(*hWnd, nCmdShow);
        ::UpdateWindow(*hWnd);
    }

    return retVal;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    LRESULT retVal = 0;
    int wmId, wmEvent;

    static bool firstPaint = true;

    switch (message)
    {
        case WM_SIZE:
        {
            Event e;
            wchar_t errormsg[1024];
            HWND hWndTP, hWndSB;

            if ((hWndSB = ::GetDlgItem(hWnd, ID_STATUSBAR)) == NULL)
            {
                ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, errormsg, sizeof(errormsg), NULL);
            }
            else
            {
                RECT sbRect;
                ::GetWindowRect(hWndSB, &sbRect);
                ::SendMessage(GetDlgItem(hWnd, ID_STATUSBAR), WM_SIZE, 0, 0);
                SET_STATUSBAR_PARTS_SIZES(sbRect.right - sbRect.left);

                if ((hWndTP = GetDlgItem(hWnd, ID_TEAPOT_WIN)) == NULL)
                {
                    ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, errormsg, sizeof(errormsg), NULL);
                }
                else
                {
                    e.sizeEvent.width  = LOWORD(lParam);
                    e.sizeEvent.height = HIWORD(lParam) - (sbRect.bottom - sbRect.top);

                    if (!::SetWindowPos(hWndTP, HWND_TOP, 0, 0, e.sizeEvent.width, e.sizeEvent.height, SWP_NOZORDER))
                    {
                        ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, errormsg, sizeof(errormsg), NULL);
                    }
                    else
                    {
                        PAINT_WRAP(MAIN_WIN_DO(onSize, e););
                    }
                }
            }
        }
        break;

        case WM_TIMER:
            switch (wParam)
            {
                    COMMAND_TP_CASE_DO(ID_IDLE_TIMER,

                                       try
                    {
                        PAINT_WRAP(MAIN_WIN_DO(idle_event););
                    }
                    catch (...)
                    {}
                    if (mainWin != NULL)
                {
                    float fps = mainWin->getFrameRate();

                        if (fps < 10)
                        {
                            SET_STATUSBAR_PART_TEXT(0, L"%.1f FPS", fps);
                        }
                        else
                        {
                            SET_STATUSBAR_PART_TEXT(0, L"%d FPS", (int)fps);
                        }

                        if (mainWin->usingCL())
                        {
                            SET_STATUSBAR_PART_TEXT(2, L"OpenCL-OpenGL Sharing is %s", mainWin->usingGLCLSharing() ? L"Active" : L"Inactive");
                        }
                        else
                        {
                            SET_STATUSBAR_PART_TEXT(2, L"OpenCL is Disabled");
                        }
                    }
                                      );
            }

            break;

        case WM_PAINT:
            PAINT_WRAP(MAIN_WIN_DO(onPaint););

            if (firstPaint && (mainWin != NULL))
            {
                firstPaint = false;

                if (!mainWin->canUseCL())
                {
                    REMOVE_CL_MENU();
                }
            }

            break;

        case WM_DESTROY:
            ::PostQuitMessage(0);
            break;

        case WM_LBUTTONDOWN:
            PAINT_WRAP(
                Event e;
                e.mouse.x = LOWORD(lParam);
                e.mouse.y = HIWORD(lParam);
                MAIN_WIN_DO(mouse_down_event, e);
            );
            break;

        case WM_LBUTTONUP:
            PAINT_WRAP(
                Event e;
                e.mouse.x = LOWORD(lParam);
                e.mouse.y = HIWORD(lParam);
                MAIN_WIN_DO(mouse_up_event, e);
            );
            break;

        case WM_MOUSEMOVE:
            PAINT_WRAP(
                Event e;
                e.mouse.x = LOWORD(lParam);
                e.mouse.y = HIWORD(lParam);
                MAIN_WIN_DO(mouse_move_event, e);
            );
            break;

        case WM_ERASEBKGND:
            break;

        case WM_COMMAND:
        {
            wmId    = LOWORD(wParam);
            wmEvent = HIWORD(wParam);

            MENUITEMINFO mii =
            {
                sizeof(MENUITEMINFO),
                MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0
            };
            HMENU hMenu     = ::GetMenu(hWnd);
            HMENU hMenu_cmd = ::GetSubMenu(hMenu, COMMANDS_MENU_NPOS);
            HMENU hMenu_viw = ::GetSubMenu(hMenu, VIEW_MENU_NPOS);
            HMENU hMenu_smk = ::GetSubMenu(hMenu, SMOKE_MENU_NPOS);
            Event e;

            // Parse the menu selections:
            switch (wmId)
            {
                    COMMAND_TP_CASE_DO(ID_COMMAND_MAKE_CRASH,
                                       MAIN_WIN_DO(ID_COMMAND_MAKE_CRASH_handler,                   e);
                                      );
                    COMMAND_TP_CASE_DO(ID_COMMAND_OUTPUT_SAMPLE_DEBUG,
                                       MAIN_WIN_DO(ID_COMMAND_OUTPUT_SAMPLE_DEBUG_handler,          e);
                                      );
                    COMMAND_TP_CASE_DO(ID_COMMAND_GENERATE_OPENGL_ERROR,
                                       MAIN_WIN_DO(ID_COMMAND_GENERATE_OPENGL_ERROR_handler,        e);
                                      );
                    COMMAND_TP_CASE_DO(ID_COMMAND_GENERATE_OPENCL_ERROR,
                                       MAIN_WIN_DO(ID_COMMAND_GENERATE_OPENCL_ERROR_handler,       e);
                                      );
                    COMMAND_TP_CASE_DO(ID_COMMAND_GENERATE_BREAK_POINT,
                                       MAIN_WIN_DO(ID_COMMAND_GENERATE_BREAK_POINT_handler,         e);
                                      );
                    COMMAND_TP_CASE_DO(ID_COMMAND_DETECTED_ERROR,
                                       MAIN_WIN_DO(ID_COMMAND_DETECTED_ERROR_handler,               e);
                                      );
                    COMMAND_TP_CASE_DO(ID_COMMAND_TOGGLE_SHADING_PARAMETERS,

                                       e.toggle.checked =       ::CheckMenuItem(hMenu_cmd, wmId, MF_BYCOMMAND) & MF_CHECKED;

                                       if (!e.toggle.checked)   ::CheckMenuItem(hMenu_cmd, wmId, MF_BYCOMMAND | MF_CHECKED);
                    else { ::CheckMenuItem(hMenu_cmd, wmId, MF_BYCOMMAND | MF_UNCHECKED); }
                    MAIN_WIN_DO(ID_COMMAND_TOGGLE_SHADING_PARAMETERS_handler,    e);
                                      );
                    COMMAND_TP_CASE_DO(ID_COMMAND_TOGGLE_MODEL_VIEW_MATRIX,

                                       e.toggle.checked =       ::CheckMenuItem(hMenu_cmd, wmId, MF_BYCOMMAND) & MF_CHECKED;

                                       if (!e.toggle.checked)   ::CheckMenuItem(hMenu_cmd, wmId, MF_BYCOMMAND | MF_CHECKED);
                    else { ::CheckMenuItem(hMenu_cmd, wmId, MF_BYCOMMAND | MF_UNCHECKED); }
                    MAIN_WIN_DO(ID_COMMAND_TOGGLE_MODEL_VIEW_MATRIX_handler, e);
                                      );
                    COMMAND_TP_CASE_DO(ID_COMMAND_TOGGLE_FRAGMENT_SHADERS,

                                       e.toggle.checked =       ::CheckMenuItem(hMenu_cmd, wmId, MF_BYCOMMAND) & MF_CHECKED;

                                       if (!e.toggle.checked)   ::CheckMenuItem(hMenu_cmd, wmId, MF_BYCOMMAND | MF_CHECKED);
                    else { ::CheckMenuItem(hMenu_cmd, wmId, MF_BYCOMMAND | MF_UNCHECKED); }
                    MAIN_WIN_DO(ID_COMMAND_TOGGLE_FRAGMENT_SHADERS_handler,  e);
                                      );
                    COMMAND_TP_CASE_DO(ID_VIEW_TOGGLE_BACKGROUND_COLOR,

                                       e.toggle.checked =       ::CheckMenuItem(hMenu_viw, wmId, MF_BYCOMMAND) & MF_CHECKED;

                                       if (!e.toggle.checked)   ::CheckMenuItem(hMenu_viw, wmId, MF_BYCOMMAND | MF_CHECKED);
                    else { ::CheckMenuItem(hMenu_viw, wmId, MF_BYCOMMAND | MF_UNCHECKED); }
                    MAIN_WIN_DO(ID_VIEW_TOGGLE_BACKGROUND_COLOR_handler, e);
                                      );
                    COMMAND_TP_CASE_DO(ID_VIEW_TOGGLE_SHADING_PROGRAM,

                                       e.toggle.checked =       ::CheckMenuItem(hMenu_viw, wmId, MF_BYCOMMAND) & MF_CHECKED;

                                       if (!e.toggle.checked)   ::CheckMenuItem(hMenu_viw, wmId, MF_BYCOMMAND | MF_CHECKED);
                    else { ::CheckMenuItem(hMenu_viw, wmId, MF_BYCOMMAND | MF_UNCHECKED); }
                    MAIN_WIN_DO(ID_COMMAND_TOGGLE_SHADING_PARAMETERS_handler,    e);
                                      );
                    COMMAND_TP_CASE_DO(ID_VIEW_TOGGLE_WIREFRAME_MODE,

                                       e.toggle.checked =       ::CheckMenuItem(hMenu_viw, wmId, MF_BYCOMMAND) & MF_CHECKED;

                                       if (!e.toggle.checked)   ::CheckMenuItem(hMenu_viw, wmId, MF_BYCOMMAND | MF_CHECKED);
                    else { ::CheckMenuItem(hMenu_viw, wmId, MF_BYCOMMAND | MF_UNCHECKED); }
                    MAIN_WIN_DO(ID_VIEW_TOGGLE_WIREFRAME_MODE_handler,   e);
                                      );
                    COMMAND_TP_CASE_DO(ID_VIEW_INCREASE_TEXTURE_INFLUENCE,
                                       MAIN_WIN_DO(ID_VIEW_INCREASE_TEXTURE_INFLUENCE_handler,      e);
                                      );
                    COMMAND_TP_CASE_DO(ID_VIEW_DECREASE_TEXTURE_INFLUENCE,
                                       MAIN_WIN_DO(ID_VIEW_DECREASE_TEXTURE_INFLUENCE_handler,     e);
                                      );
                    COMMAND_TP_CASE_DO(ID_VIEW_TOGGLE_GEOMETRY_SHADER,

                                       e.toggle.checked =       ::CheckMenuItem(hMenu_viw, wmId, MF_BYCOMMAND) & MF_CHECKED;

                                       if (!e.toggle.checked)   ::CheckMenuItem(hMenu_viw, wmId, MF_BYCOMMAND | MF_CHECKED);
                    else { ::CheckMenuItem(hMenu_viw, wmId, MF_BYCOMMAND | MF_UNCHECKED); }
                    MAIN_WIN_DO(ID_COMMAND_TOGGLE_SHADING_PARAMETERS_handler,    e);
                                      );
                    COMMAND_TP_CASE_DO(ID_VIEW_INCREASE_SPIKINESS,
                                       MAIN_WIN_DO(ID_VIEW_INCREASE_SPIKINESS_handler,              e);
                                      );
                    COMMAND_TP_CASE_DO(ID_VIEW_DECREASE_SPIKINESS,
                                       MAIN_WIN_DO(ID_VIEW_DECREASE_SPIKINESS_handler,             e);
                                      );
                    COMMAND_TP_CASE_DO(ID_SMOKE_ENABLED,

                                       e.toggle.checked =       ::CheckMenuItem(hMenu_smk, wmId, MF_BYCOMMAND) & MF_CHECKED;

                                       if (!e.toggle.checked)   ::CheckMenuItem(hMenu_smk, wmId, MF_BYCOMMAND | MF_CHECKED);
                    else { ::CheckMenuItem(hMenu_smk, wmId, MF_BYCOMMAND | MF_UNCHECKED); }
                    MAIN_WIN_DO(ID_SMOKE_ENABLED_handler,    e);
                    long uEnable = MF_BYCOMMAND | ((e.toggle.checked) ? MF_GRAYED : MF_ENABLED);

                                   for (int i = ID_SMOKE_USE_GLCL_SHARING ; i <= ID_SMOKE_SHOW_ERROR ; i++)
                {
                    ::EnableMenuItem(hMenu_smk, i, uEnable);
                    }
                    for (int i = ID_SMOKE_SIM_DEVICE_MIN_ID ; i <= ID_SMOKE_SIM_DEVICE_MAX_ID ; i++)
                {
                    ::EnableMenuItem(hMenu_smk, i, uEnable);
                    }
                    for (int i = ID_VOL_SLICE_DEVICE_MIN_ID ; i <= ID_VOL_SLICE_DEVICE_MAX_ID ; i++)
                {
                    ::EnableMenuItem(hMenu_smk, i, uEnable);
                    }
                                      );
                    COMMAND_TP_CASE_DO(ID_SMOKE_USE_GLCL_SHARING,

                                       e.toggle.checked =       ::CheckMenuItem(hMenu_smk, wmId, MF_BYCOMMAND) & MF_CHECKED;

                                       if (!e.toggle.checked)   ::CheckMenuItem(hMenu_smk, wmId, MF_BYCOMMAND | MF_CHECKED);
                    else { ::CheckMenuItem(hMenu_smk, wmId, MF_BYCOMMAND | MF_UNCHECKED); }
                    MAIN_WIN_DO(ID_SMOKE_USE_GLCL_SHARING_handler,   e);
                                      );
                    COMMAND_TP_CASE_DO(ID_SMOKE_SHOW_GRID,

                                       e.toggle.checked =       ::CheckMenuItem(hMenu_smk, wmId, MF_BYCOMMAND) & MF_CHECKED;

                                       if (!e.toggle.checked)   ::CheckMenuItem(hMenu_smk, wmId, MF_BYCOMMAND | MF_CHECKED);
                    else { ::CheckMenuItem(hMenu_smk, wmId, MF_BYCOMMAND | MF_UNCHECKED); }
                    MAIN_WIN_DO(ID_SMOKE_SHOW_GRID_handler,  e);
                                      );
                    COMMAND_TP_CASE_DO(ID_SMOKE_RESET,
                                       MAIN_WIN_DO(ID_SMOKE_RESET_handler,                         e);
                                      );
                    COMMAND_TP_CASE_DO(ID_SMOKE_GRID_32_32_64,
                                       ::CheckMenuRadioItem(hMenu_smk,
                                                            ID_SMOKE_GRID_32_32_64,
                                                            ID_SMOKE_GRID_128_128_256,
                                                            wmId,
                                                            MF_BYCOMMAND);
                                       MAIN_WIN_DO(ID_SMOKE_GRID_32_32_64_handler,                 e);
                                      );
                    COMMAND_TP_CASE_DO(ID_SMOKE_GRID_64_64_64,
                                       ::CheckMenuRadioItem(hMenu_smk,
                                                            ID_SMOKE_GRID_32_32_64,
                                                            ID_SMOKE_GRID_128_128_256,
                                                            wmId,
                                                            MF_BYCOMMAND);
                                       MAIN_WIN_DO(ID_SMOKE_GRID_64_64_64_handler,                  e);
                                      );
                    COMMAND_TP_CASE_DO(ID_SMOKE_GRID_64_64_128,
                                       ::CheckMenuRadioItem(hMenu_smk,
                                                            ID_SMOKE_GRID_32_32_64,
                                                            ID_SMOKE_GRID_128_128_256,
                                                            wmId,
                                                            MF_BYCOMMAND);
                                       MAIN_WIN_DO(ID_SMOKE_GRID_64_64_128_handler,                 e);
                                      );
                    COMMAND_TP_CASE_DO(ID_SMOKE_GRID_128_128_128,
                                       ::CheckMenuRadioItem(hMenu_smk,
                                                            ID_SMOKE_GRID_32_32_64,
                                                            ID_SMOKE_GRID_128_128_256,
                                                            wmId,
                                                            MF_BYCOMMAND);
                                       MAIN_WIN_DO(ID_SMOKE_GRID_128_128_128_handler,               e);
                                      );
                    COMMAND_TP_CASE_DO(ID_SMOKE_GRID_128_128_256,
                                       ::CheckMenuRadioItem(hMenu_smk,
                                                            ID_SMOKE_GRID_32_32_64,
                                                            ID_SMOKE_GRID_128_128_256,
                                                            wmId,
                                                            MF_BYCOMMAND);
                                       MAIN_WIN_DO(ID_SMOKE_GRID_128_128_256_handler,               e);
                                      );
                    COMMAND_TP_CASE_DO(ID_SMOKE_SHOW_ERROR,
                    {
                        const char* errStr = NULL;
                        MAIN_WIN_DO(getSmokeSystemLastError, &errStr);

                        if (errStr == NULL)
                        {
                            ::MessageBox(hWnd,
                            TP_SMOKE_DIALOG_NO_ERROR,
                            TP_ERROR_TITLE,
                            MB_OK | MB_ICONINFORMATION);
                        }
                        else
                        {
                            int len = ::strlen(errStr);
                            wchar_t* wc_errStr = new wchar_t[len];
                            ::mbstowcs(wc_errStr, errStr, len);
                            ::MessageBox(hWnd,
                            wc_errStr,
                            TP_ERROR_TITLE,
                            MB_OK | MB_ICONERROR);
                            delete [] wc_errStr;
                        }
                    }
                                      );
                    COMMAND_TP_CASE_DO(IDM_ABOUT,
                                       ::DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                                      );
                    COMMAND_TP_CASE_DO(IDM_EXIT,
                                       ::DestroyWindow(hWnd);
                                      );

                default:
                    ASSERT(::GetMenuItemInfo(hMenu_smk, wmId, FALSE, &mii), "Failed to get device menu item properly");
                    HMENU hMenu_sim = ::GetSubMenu(hMenu_smk, SMOKE_SIM_SUBMENU_NPOS);
                    HMENU hMenu_slc = ::GetSubMenu(hMenu_smk, SMOKE_SLC_SUBMENU_NPOS);

                    if ((ID_SMOKE_SIM_DEVICE_MIN_ID <= wmId) && (wmId < ID_SMOKE_SIM_DEVICE_MAX_ID))
                    {
                        MAIN_WIN_DO(smokeDevHandler, (void*)mii.dwItemData);
                        ::CheckMenuRadioItem(hMenu_sim,
                                             ID_SMOKE_SIM_DEVICE_MIN_ID,
                                             ID_SMOKE_SIM_DEVICE_MAX_ID,
                                             wmId,
                                             MF_BYCOMMAND);
                    }

                    if ((ID_VOL_SLICE_DEVICE_MIN_ID <= wmId) && (wmId < ID_VOL_SLICE_DEVICE_MAX_ID))
                    {
                        MAIN_WIN_DO(smokeDevHandler, (void*)mii.dwItemData);
                        ::CheckMenuRadioItem(hMenu_slc,
                                             ID_VOL_SLICE_DEVICE_MIN_ID,
                                             ID_VOL_SLICE_DEVICE_MAX_ID,
                                             wmId,
                                             MF_BYCOMMAND);
                    }

                    break;
            }

            if (mainWin != NULL)
            {
                SET_STATUSBAR_STR_PART_TEXT(1, mainWin->getProgressMessage());

                if (mainWin->usingCL())
                {
                    SET_STATUSBAR_PART_TEXT(2, L"OpenCL-OpenGL Sharing is %s", mainWin->usingGLCLSharing() ? L"Active" : L"Inactive");
                }
                else
                {
                    SET_STATUSBAR_PART_TEXT(2, L"OpenCL is Disabled");
                }
            }
        }
        break;

        default:
            break;
    }

    if (message != WM_ERASEBKGND)
    {
        retVal = ::DefWindowProc(hWnd, message, wParam, lParam);
    }

    return retVal;
}

// Message handler for about box.
INT_PTR CALLBACK About(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    INT_PTR retVal = FALSE;
    HWND urlTextWnd;
    HCURSOR handCursor;
    UNREFERENCED_PARAMETER(lParam);

    switch (message)
    {
        case WM_INITDIALOG:
        {
            urlTextWnd = ::GetDlgItem(hDlg, IDC_URL);
            stOriginalWindowProc = (WNDPROC)::SetWindowLongPtr(urlTextWnd, GWLP_WNDPROC, (LONG_PTR)&AboutWndProc);
            handCursor = ::LoadCursor(NULL, IDC_HAND);
            stHandCursor = (HCURSOR)::SetClassLong(urlTextWnd, GCL_HCURSOR, (LONG_PTR)handCursor);
            retVal = TRUE;
        }
        break;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            {
                ::EndDialog(hDlg, LOWORD(wParam));
                retVal = TRUE;
            }

            break;
    }

    return retVal;
}

//
//  FUNCTION: AboutWndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the About window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK AboutWndProc(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    LRESULT retVal = 0;

    if (message == WM_LBUTTONDOWN)
    {
        ::ShellExecuteA(NULL, NULL, TP_ABOUT_DIALOG_WEBSITE_URL, NULL, NULL, SW_SHOWNORMAL);
    }
    else
    {
        retVal = stOriginalWindowProc(hWnd, message, wParam, lParam);
    }

    return retVal;
}
