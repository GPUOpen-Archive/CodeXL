//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file Teapot.cpp
///
//==================================================================================

//------------------------------ Teapot.cpp ------------------------------

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/fl_ask.H>
#include <FL/Enumerations.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Group.H>
#include "GLWindow.h"

/// Enumerates menu items IDs
enum
{
    FILE_EXIT,
    COMMAND_MAKE_CRASH,
    COMMAND_OUTPUT_SAMPLE_DEBUG,
    COMMAND_GENERATE_OPENGL_ERROR,
    COMMAND_GENERATE_OPENCL_ERROR,
    COMMAND_GENERATE_BREAK_POINT,
    COMMAND_DETECTED_ERROR,
    COMMAND_TOGGLE_SHADING_PARAMETERS,
    COMMAND_TOGGLE_MODEL_VIEW_MATRIX,
    COMMAND_TOGGLE_FRAGMENT_SHADERS,
    VIEW_TOGGLE_BACKGROUND_COLOR,
    VIEW_TOGGLE_SHADING_PROGRAM,
    VIEW_TOGGLE_GEOMETRY_SHADER,
    VIEW_TOGGLE_WIREFRAME_MODE,
    VIEW_INCREASE_TEXTURE_INFLUENCE,
    VIEW_DECREASE_TEXTURE_INFLUENCE,
    VIEW_INCREASE_SPIKINESS,
    VIEW_DECREASE_SPIKINESS,
    SMOKE_ENABLED,
    SMOKE_USE_GLCL_SHARING,
    SMOKE_SHOW_GRID,
    SMOKE_RESET,
    SMOKE_GRID_32_32_64,
    SMOKE_GRID_64_64_64,
    SMOKE_GRID_64_64_128,
    SMOKE_GRID_128_128_128,
    SMOKE_GRID_128_128_256,
    SMOKE_DEVICE,
    SMOKE_SHOW_ERROR,
    HELP_ABOUT,
    ITEMS_COUNT = HELP_ABOUT + 1
};

/// Label font size
const int STATUS_LABEL_SIZE = 12;

/// Status bar and menu bar height
const int BAR_HEIGHT = 20;


/// Status bar indent
const int STATUS_BAR_INDENT = 30;


/// MenuHandler passed as void* to menu callback
/// Holds necessary information that should be passed
/// forward to GLWindow and then to AMDTTeapotOGLCanvas
typedef struct MenuHandler
{
    MenuHandler(GLWindow* pWin, void* pData) : m_pMainWin(pWin), m_pData(pData), m_pMenuBar(NULL) {}
    GLWindow* m_pMainWin;
    void* m_pData;
    Event m_Event;
    Fl_Menu_Bar* m_pMenuBar;
    char m_strItemName[256];
    int m_ID;

} menuHandler;

/// Array of MenuHandler structures used in common callback
menuHandler* arrMenuHandlers[ITEMS_COUNT];

/// Returns menu item index by passed name
/// \param pointer to Fl_Menu_Bar
/// \param menu item text to search in menu bar
/// \return index of the menu bar item
int GetIndexByName(Fl_Menu_Bar* pMenuBar, const char* findname)
{
    char menupath[1024] = "";            // File/Export

    for (int t = 0; t < pMenuBar->size(); t++)
    {
        Fl_Menu_Item* pMenuItem = (Fl_Menu_Item*) & (pMenuBar->menu()[t]);

        if (pMenuItem->submenu())
        {
            // Submenu?
            if (menupath[0]) { strcat(menupath, "/"); }

            strcat(menupath, pMenuItem->label());

            if (strcmp(menupath, findname) == 0) { return (t); }
        }
        else
        {
            if (pMenuItem->label() == NULL)
            {
                // End of submenu? Pop back one level.
                char* ss = strrchr(menupath, '/');

                if (ss) { *ss = 0; }
                else { menupath[0] = '\0'; }

                continue;
            }

            // Menu item?
            char itempath[1024];         // eg. Edit/Copy
            strcpy(itempath, menupath);

            if (itempath[0]) { strcat(itempath, "/"); }

            strcat(itempath, pMenuItem->label());

            if (strcmp(itempath, findname) == 0)
            {
                return (t);
            }
        }
    }

    return (-1);
}

Fl_Menu_Item* GetMenuItemByName(Fl_Menu_Bar* pMenuBar, const char* findname)
{
    int index = GetIndexByName(pMenuBar, findname);

    if (index == -1)
    {
        return (NULL);
    }

    Fl_Menu_Item* pMenuItem = (Fl_Menu_Item*) & (pMenuBar->menu()[index]);
    return (pMenuItem);
}

/// Static common callback - actual action depends on MenuHandler contained data
/// \param pointer to Fl_Widget (mandatory fltk argument)
/// \param void * - pointer to MenuHandler added at menu bar creation
static void common_cb(Fl_Widget*, void* w)
{
    MenuHandler* pMenuHandler = (MenuHandler*)w;

    if (pMenuHandler != 0)
    {
        GLWindow* pWin = dynamic_cast<GLWindow*>(pMenuHandler->m_pMainWin);

        if (pWin != 0)
        {
            Event e;
            Fl_Menu_Bar* pMenuBar = pMenuHandler->m_pMenuBar;
            int id = pMenuHandler->m_ID;

            switch (id)
            {
                case SMOKE_DEVICE:
                    pWin->smokeDevHandler(pMenuHandler->m_pData);
                    break;

                case FILE_EXIT:
                    pWin->ID_FILE_EXIT_handler(e);
                    break;

                case COMMAND_MAKE_CRASH:
                    pWin->ID_COMMAND_MAKE_CRASH_handler(e);
                    break;

                case COMMAND_OUTPUT_SAMPLE_DEBUG:
                    pWin->ID_COMMAND_OUTPUT_SAMPLE_DEBUG_handler(e);
                    break;

                case COMMAND_GENERATE_OPENGL_ERROR:
                    pWin->ID_COMMAND_GENERATE_OPENGL_ERROR_handler(e);
                    break;

                case COMMAND_GENERATE_OPENCL_ERROR:
                    pWin->ID_COMMAND_GENERATE_OPENCL_ERROR_handler(e);
                    break;

                case COMMAND_GENERATE_BREAK_POINT:
                    pWin->ID_COMMAND_GENERATE_BREAK_POINT_handler(e);
                    break;

                case COMMAND_DETECTED_ERROR:
                    pWin->ID_COMMAND_DETECTED_ERROR_handler(e);
                    break;

                case COMMAND_TOGGLE_SHADING_PARAMETERS:
                    pWin->ID_COMMAND_TOGGLE_SHADING_PARAMETERS_handler(e);
                    break;

                case COMMAND_TOGGLE_MODEL_VIEW_MATRIX:
                    pWin->ID_COMMAND_TOGGLE_MODEL_VIEW_MATRIX_handler(e);
                    break;

                case COMMAND_TOGGLE_FRAGMENT_SHADERS:
                    pWin->ID_COMMAND_TOGGLE_FRAGMENT_SHADERS_handler(e);
                    break;

                case VIEW_TOGGLE_BACKGROUND_COLOR:
                    pWin->ID_VIEW_TOGGLE_BACKGROUND_COLOR_handler(e);
                    break;

                case VIEW_TOGGLE_SHADING_PROGRAM:
                    pWin->ID_VIEW_TOGGLE_SHADING_PROGRAM_handler(e);
                    break;

                case VIEW_TOGGLE_GEOMETRY_SHADER:
                    pWin->ID_VIEW_TOGGLE_GEOMETRY_SHADER_handler(e);
                    break;

                case  VIEW_TOGGLE_WIREFRAME_MODE:
                    pWin->ID_VIEW_TOGGLE_WIREFRAME_MODE_handler(e);
                    break;

                case VIEW_INCREASE_TEXTURE_INFLUENCE:
                    pWin->ID_VIEW_INCREASE_TEXTURE_INFLUENCE_handler(e);
                    break;

                case VIEW_DECREASE_TEXTURE_INFLUENCE:
                    pWin->ID_VIEW_DECREASE_TEXTURE_INFLUENCE_handler(e);
                    break;

                case  ID_VIEW_INCREASE_SPIKINESS:
                    pWin->ID_VIEW_INCREASE_SPIKINESS_handler(e);
                    break;

                case VIEW_DECREASE_SPIKINESS:
                    pWin->ID_VIEW_DECREASE_SPIKINESS_handler(e);
                    break;

                case SMOKE_ENABLED:
                    if (pMenuBar != NULL)
                    {
                        const Fl_Menu_Item* m = GetMenuItemByName(pMenuBar, arrMenuHandlers[SMOKE_ENABLED]->m_strItemName);
                        e.toggle.checked = m->value();
                        pWin->ID_SMOKE_ENABLED_handler(e);
                    }

                    break;

                case SMOKE_USE_GLCL_SHARING:
                    if (pMenuBar != NULL)
                    {
                        const Fl_Menu_Item* m = GetMenuItemByName(pMenuBar, arrMenuHandlers[SMOKE_USE_GLCL_SHARING]->m_strItemName);
                        e.toggle.checked = m->value();
                        pWin->ID_SMOKE_USE_GLCL_SHARING_handler(e);
                    }

                    break;

                case SMOKE_SHOW_GRID:
                    if (pMenuBar != NULL)
                    {
                        const Fl_Menu_Item* m = GetMenuItemByName(pMenuBar, arrMenuHandlers[SMOKE_SHOW_GRID]->m_strItemName);
                        e.toggle.checked = m->value();
                        pWin->ID_SMOKE_SHOW_GRID_handler(e);
                    }

                    break;

                case SMOKE_RESET:
                    pWin->ID_SMOKE_RESET_handler(e);
                    break;

                case SMOKE_GRID_32_32_64:
                    pWin->ID_SMOKE_GRID_32_32_64_handler(e);
                    break;

                case SMOKE_GRID_64_64_64:
                    pWin->ID_SMOKE_GRID_64_64_64_handler(e);
                    break;

                case SMOKE_GRID_64_64_128:
                    pWin->ID_SMOKE_GRID_64_64_128_handler(e);
                    break;

                case SMOKE_GRID_128_128_128:
                    pWin->ID_SMOKE_GRID_128_128_128_handler(e);
                    break;

                case SMOKE_GRID_128_128_256:
                    pWin->ID_SMOKE_GRID_128_128_256_handler(e);
                    break;

                case SMOKE_SHOW_ERROR:
                    pWin->ID_SMOKE_SHOW_ERROR_handler(e);
                    break;

                case HELP_ABOUT:
                    pWin->ID_HELP_ABOUT_handler(e);
                    break;
            }
        }
    }
}


inline void createSmokeDeviceSubMenu(
    Fl_Menu_Bar*           pMenuBar,
    char*                strMenuBar,
    int                  minMenuId,
    const OCLInfo*       oclInfo,
    SmokeSystemCommandId cmdId,
    GLWindow* pWin)
{
    char  tmpMenuBar[256];
    int id = minMenuId;
    int numOfPlatforms = oclInfo->getNumPlatforms();
    int numOfDevices = -1;
    size_t strMenuBarLen = strlen(strMenuBar);
    strMenuBarLen = strMenuBarLen > sizeof(tmpMenuBar) ? sizeof(tmpMenuBar) : strMenuBarLen;

    for (int i = 0; i < numOfPlatforms; ++i)
    {
        const OCLPlatform* platform = oclInfo->getPlatform(i);
        numOfDevices = platform->getNumDevices();

        for (int j = 0; j < numOfDevices; ++j, ++id)
        {
            strncpy(tmpMenuBar, strMenuBar, strMenuBarLen);
            const OCLDevice* device = platform->getDevice(j);
            strncpy(&tmpMenuBar[strMenuBarLen], device->getName(), sizeof(tmpMenuBar) - strMenuBarLen);
            MenuHandler* pHandlerStruct = new MenuHandler(pWin, new SmokeSystemCommandChangeDevice(SSCID_CHANGE_SMOKE_SIM_DEVICE, device));
            pHandlerStruct->m_ID = SMOKE_DEVICE;

            if (id == minMenuId)
            {
                pMenuBar->add(tmpMenuBar, 0, common_cb, pHandlerStruct, FL_MENU_RADIO | FL_MENU_VALUE);
            }
            else
            {
                pMenuBar->add(tmpMenuBar, 0, common_cb, pHandlerStruct, FL_MENU_RADIO);
            }
        }
    }
}// createSmokeDevicepMenuBar

int main(int argc, char* argv[])
{
    XInitThreads();
    // mainWindow
    Fl_Double_Window win(420, 460, "AMD Teapot Example");
    win.size_range(400, 400);

    //Double buffering and full color
    Fl::visual(FL_DOUBLE | FL_RGB);

    /// Grouping status bar boxes to make first 2 boxes fixed sized
    Fl_Group* pStatusBoxesGroup = new Fl_Group(10, win.h() - STATUS_BAR_INDENT, 400, BAR_HEIGHT);

    Fl_Box* pStatusBoxFPS = new Fl_Box(FL_DOWN_BOX, 10, win.h() - STATUS_BAR_INDENT, 100, 20, NULL);
    pStatusBoxFPS->labelsize(STATUS_LABEL_SIZE);

    Fl_Box* pStatusBoxCL_GL = new Fl_Box(FL_DOWN_BOX, 110, win.h() - STATUS_BAR_INDENT, 250, BAR_HEIGHT, NULL);
    pStatusBoxCL_GL->labelsize(STATUS_LABEL_SIZE);

    Fl_Box* pStatusBoxError = new Fl_Box(FL_DOWN_BOX, 360, win.h() - STATUS_BAR_INDENT, 50, BAR_HEIGHT, NULL);
    pStatusBoxError->labelsize(STATUS_LABEL_SIZE);

    /// Invisible box that can be resized  - this makes first 2 boxes fixed sized
    Fl_Box* pResizable = new Fl_Box(400, win.h() - STATUS_BAR_INDENT, 10, BAR_HEIGHT);
    pResizable->hide();

    pStatusBoxesGroup->resizable(pResizable);

    pStatusBoxesGroup->end();

    GLWindow* pGLWin = new GLWindow(10, 30, win.w() - 20, win.h() - 60, NULL, pStatusBoxFPS, pStatusBoxCL_GL, pStatusBoxError);

    Fl_Menu_Bar* pMenuBar = new Fl_Menu_Bar(10, 10, pGLWin->w(), BAR_HEIGHT);
    pMenuBar->color(fl_rgb_color(200, 200, 205));

    // Populating menu bar with items and callbacks (additional info for callbacks is passed with MenuHandler structure)
    pMenuBar->add("&File", 0, 0, 0, FL_SUBMENU);
    arrMenuHandlers[FILE_EXIT] = new MenuHandler(pGLWin, NULL);
    arrMenuHandlers[FILE_EXIT]->m_ID = FILE_EXIT;
    strcpy(arrMenuHandlers[FILE_EXIT]->m_strItemName, "&File/E&xit\t\tEsc");
    pMenuBar->add(arrMenuHandlers[FILE_EXIT]->m_strItemName, "FL_Escape", (Fl_Callback*)common_cb, arrMenuHandlers[FILE_EXIT]);

    arrMenuHandlers[COMMAND_MAKE_CRASH] = new MenuHandler(pGLWin, NULL);
    arrMenuHandlers[COMMAND_MAKE_CRASH]->m_ID = COMMAND_MAKE_CRASH;
    strcpy(arrMenuHandlers[COMMAND_MAKE_CRASH]->m_strItemName, "&Commands/C&rash Application");
    pMenuBar->add(arrMenuHandlers[COMMAND_MAKE_CRASH]->m_strItemName, 0, common_cb, arrMenuHandlers[COMMAND_MAKE_CRASH]);

    arrMenuHandlers[COMMAND_OUTPUT_SAMPLE_DEBUG] = new MenuHandler(pGLWin, NULL);
    arrMenuHandlers[COMMAND_OUTPUT_SAMPLE_DEBUG]->m_ID = COMMAND_OUTPUT_SAMPLE_DEBUG;
    strcpy(arrMenuHandlers[COMMAND_OUTPUT_SAMPLE_DEBUG]->m_strItemName, "&Commands/Output a Sample Debug &String");
    pMenuBar->add(arrMenuHandlers[COMMAND_OUTPUT_SAMPLE_DEBUG]->m_strItemName, "^d", common_cb, arrMenuHandlers[COMMAND_OUTPUT_SAMPLE_DEBUG]);

    arrMenuHandlers[COMMAND_GENERATE_OPENGL_ERROR] = new MenuHandler(pGLWin, NULL);
    arrMenuHandlers[COMMAND_GENERATE_OPENGL_ERROR]->m_ID = COMMAND_GENERATE_OPENGL_ERROR;
    strcpy(arrMenuHandlers[COMMAND_GENERATE_OPENGL_ERROR]->m_strItemName, "&Commands/Generate an Open&GL Error");
    pMenuBar->add(arrMenuHandlers[COMMAND_GENERATE_OPENGL_ERROR]->m_strItemName, "^e", common_cb, arrMenuHandlers[COMMAND_GENERATE_OPENGL_ERROR]);

    arrMenuHandlers[COMMAND_GENERATE_OPENCL_ERROR] = new MenuHandler(pGLWin, NULL);
    arrMenuHandlers[COMMAND_GENERATE_OPENCL_ERROR]->m_ID = COMMAND_GENERATE_OPENCL_ERROR;
    strcpy(arrMenuHandlers[COMMAND_GENERATE_OPENCL_ERROR]->m_strItemName, "&Commands/Generate an Open&CL Error");
    pMenuBar->add(arrMenuHandlers[COMMAND_GENERATE_OPENCL_ERROR]->m_strItemName, "^l", common_cb, arrMenuHandlers[COMMAND_GENERATE_OPENCL_ERROR]);


    arrMenuHandlers[COMMAND_GENERATE_BREAK_POINT] = new MenuHandler(pGLWin, NULL);
    arrMenuHandlers[COMMAND_GENERATE_BREAK_POINT]->m_ID = COMMAND_GENERATE_BREAK_POINT;
    strcpy(arrMenuHandlers[COMMAND_GENERATE_BREAK_POINT]->m_strItemName, "&Commands/Generate &Breakpoint");
    pMenuBar->add(arrMenuHandlers[COMMAND_GENERATE_BREAK_POINT]->m_strItemName, "^p", common_cb, arrMenuHandlers[COMMAND_GENERATE_BREAK_POINT]);

    arrMenuHandlers[COMMAND_DETECTED_ERROR] = new MenuHandler(pGLWin, NULL);
    arrMenuHandlers[COMMAND_DETECTED_ERROR]->m_ID = COMMAND_DETECTED_ERROR;
    strcpy(arrMenuHandlers[COMMAND_DETECTED_ERROR]->m_strItemName, "&Commands/Generate a &Detected Error");
    pMenuBar->add(arrMenuHandlers[COMMAND_DETECTED_ERROR]->m_strItemName, "^t", common_cb, arrMenuHandlers[COMMAND_DETECTED_ERROR]);

    arrMenuHandlers[COMMAND_TOGGLE_SHADING_PARAMETERS] = new MenuHandler(pGLWin, NULL);
    arrMenuHandlers[COMMAND_TOGGLE_SHADING_PARAMETERS]->m_ID = COMMAND_TOGGLE_SHADING_PARAMETERS;
    strcpy(arrMenuHandlers[COMMAND_TOGGLE_SHADING_PARAMETERS]->m_strItemName, "&Commands/Replace S&hading Parameters");
    pMenuBar->add(arrMenuHandlers[COMMAND_TOGGLE_SHADING_PARAMETERS]->m_strItemName, "^o", common_cb, arrMenuHandlers[COMMAND_TOGGLE_SHADING_PARAMETERS], FL_MENU_TOGGLE);

    arrMenuHandlers[COMMAND_TOGGLE_MODEL_VIEW_MATRIX] = new MenuHandler(pGLWin, NULL);
    arrMenuHandlers[COMMAND_TOGGLE_MODEL_VIEW_MATRIX]->m_ID = COMMAND_TOGGLE_MODEL_VIEW_MATRIX;
    strcpy(arrMenuHandlers[COMMAND_TOGGLE_MODEL_VIEW_MATRIX]->m_strItemName, "&Commands/Replace Model V&iew Matrix");
    pMenuBar->add(arrMenuHandlers[COMMAND_TOGGLE_MODEL_VIEW_MATRIX]->m_strItemName, "^m", common_cb, arrMenuHandlers[COMMAND_TOGGLE_MODEL_VIEW_MATRIX], FL_MENU_TOGGLE);

    arrMenuHandlers[COMMAND_TOGGLE_FRAGMENT_SHADERS] = new MenuHandler(pGLWin, NULL);
    arrMenuHandlers[COMMAND_TOGGLE_FRAGMENT_SHADERS]->m_ID = COMMAND_TOGGLE_FRAGMENT_SHADERS;
    strcpy(arrMenuHandlers[COMMAND_TOGGLE_FRAGMENT_SHADERS]->m_strItemName, "&Commands/Replace &Fragment Shader");
    pMenuBar->add(arrMenuHandlers[COMMAND_TOGGLE_FRAGMENT_SHADERS]->m_strItemName, "^z", common_cb, arrMenuHandlers[COMMAND_TOGGLE_FRAGMENT_SHADERS], FL_MENU_TOGGLE);

    arrMenuHandlers[VIEW_TOGGLE_BACKGROUND_COLOR] = new MenuHandler(pGLWin, NULL);
    arrMenuHandlers[VIEW_TOGGLE_BACKGROUND_COLOR]->m_ID = VIEW_TOGGLE_BACKGROUND_COLOR;
    strcpy(arrMenuHandlers[VIEW_TOGGLE_BACKGROUND_COLOR]->m_strItemName, "&View/&Replace Background Color");
    pMenuBar->add(arrMenuHandlers[VIEW_TOGGLE_BACKGROUND_COLOR]->m_strItemName, "^b", common_cb, arrMenuHandlers[VIEW_TOGGLE_BACKGROUND_COLOR], FL_MENU_TOGGLE);

    arrMenuHandlers[VIEW_TOGGLE_SHADING_PROGRAM] = new MenuHandler(pGLWin, NULL);
    arrMenuHandlers[VIEW_TOGGLE_SHADING_PROGRAM]->m_ID = VIEW_TOGGLE_SHADING_PROGRAM;
    strcpy(arrMenuHandlers[VIEW_TOGGLE_SHADING_PROGRAM]->m_strItemName, "&View/Use &Shading Program");
    pMenuBar->add(arrMenuHandlers[VIEW_TOGGLE_SHADING_PROGRAM]->m_strItemName, "^s", common_cb, arrMenuHandlers[VIEW_TOGGLE_SHADING_PROGRAM], FL_MENU_TOGGLE | FL_MENU_VALUE);

    arrMenuHandlers[VIEW_TOGGLE_GEOMETRY_SHADER] = new MenuHandler(pGLWin, NULL);
    arrMenuHandlers[VIEW_TOGGLE_GEOMETRY_SHADER]->m_ID = VIEW_TOGGLE_GEOMETRY_SHADER;
    strcpy(arrMenuHandlers[VIEW_TOGGLE_GEOMETRY_SHADER]->m_strItemName, "&View/Use &Geometry Shader");
    pMenuBar->add(arrMenuHandlers[VIEW_TOGGLE_GEOMETRY_SHADER]->m_strItemName, FL_CTRL + 'g', common_cb, arrMenuHandlers[VIEW_TOGGLE_GEOMETRY_SHADER], FL_MENU_TOGGLE);

    arrMenuHandlers[VIEW_TOGGLE_WIREFRAME_MODE] = new MenuHandler(pGLWin, NULL);
    arrMenuHandlers[VIEW_TOGGLE_WIREFRAME_MODE]->m_ID = VIEW_TOGGLE_WIREFRAME_MODE;
    strcpy(arrMenuHandlers[VIEW_TOGGLE_WIREFRAME_MODE]->m_strItemName, "&View/&Wire-frame mode");
    pMenuBar->add(arrMenuHandlers[VIEW_TOGGLE_WIREFRAME_MODE]->m_strItemName, "^w", common_cb, arrMenuHandlers[VIEW_TOGGLE_WIREFRAME_MODE], FL_MENU_TOGGLE);

    arrMenuHandlers[VIEW_INCREASE_TEXTURE_INFLUENCE] = new MenuHandler(pGLWin, NULL);
    arrMenuHandlers[VIEW_INCREASE_TEXTURE_INFLUENCE]->m_ID = VIEW_INCREASE_TEXTURE_INFLUENCE;
    strcpy(arrMenuHandlers[VIEW_INCREASE_TEXTURE_INFLUENCE]->m_strItemName, "&View/&Increases Texture Influence");
    pMenuBar->add(arrMenuHandlers[VIEW_INCREASE_TEXTURE_INFLUENCE]->m_strItemName, "43", common_cb, arrMenuHandlers[VIEW_INCREASE_TEXTURE_INFLUENCE]);

    arrMenuHandlers[VIEW_DECREASE_TEXTURE_INFLUENCE] = new MenuHandler(pGLWin, NULL);
    arrMenuHandlers[VIEW_DECREASE_TEXTURE_INFLUENCE]->m_ID = VIEW_DECREASE_TEXTURE_INFLUENCE;
    strcpy(arrMenuHandlers[VIEW_DECREASE_TEXTURE_INFLUENCE]->m_strItemName, "&View/&Decreases Texture Influence");
    pMenuBar->add(arrMenuHandlers[VIEW_DECREASE_TEXTURE_INFLUENCE]->m_strItemName, "45", common_cb, arrMenuHandlers[VIEW_DECREASE_TEXTURE_INFLUENCE]);

    arrMenuHandlers[SMOKE_ENABLED] = new MenuHandler(pGLWin, NULL);
    arrMenuHandlers[SMOKE_ENABLED]->m_ID = SMOKE_ENABLED;
    arrMenuHandlers[SMOKE_ENABLED]->m_pMenuBar = pMenuBar;
    strcpy(arrMenuHandlers[SMOKE_ENABLED]->m_strItemName, "&OpenCL Smoke/Enabled");
    pMenuBar->add(arrMenuHandlers[SMOKE_ENABLED]->m_strItemName, 0, common_cb, arrMenuHandlers[SMOKE_ENABLED], FL_MENU_TOGGLE | FL_MENU_VALUE);


    arrMenuHandlers[SMOKE_USE_GLCL_SHARING] = new MenuHandler(pGLWin, NULL);
    arrMenuHandlers[SMOKE_USE_GLCL_SHARING]->m_ID = SMOKE_USE_GLCL_SHARING;
    arrMenuHandlers[SMOKE_USE_GLCL_SHARING]->m_pMenuBar = pMenuBar;
    strcpy(arrMenuHandlers[SMOKE_USE_GLCL_SHARING]->m_strItemName, "&OpenCL Smoke/Use GL-CL sharing");
    pMenuBar->add(arrMenuHandlers[SMOKE_USE_GLCL_SHARING]->m_strItemName, 0, common_cb, arrMenuHandlers[SMOKE_USE_GLCL_SHARING], FL_MENU_TOGGLE | FL_MENU_VALUE);


    arrMenuHandlers[SMOKE_SHOW_GRID] = new MenuHandler(pGLWin, NULL);
    arrMenuHandlers[SMOKE_SHOW_GRID]->m_ID = SMOKE_SHOW_GRID;
    arrMenuHandlers[SMOKE_SHOW_GRID]->m_pMenuBar = pMenuBar;
    strcpy(arrMenuHandlers[SMOKE_SHOW_GRID]->m_strItemName, "&OpenCL Smoke/Show grid");
    pMenuBar->add(arrMenuHandlers[SMOKE_SHOW_GRID]->m_strItemName, 0, common_cb, arrMenuHandlers[SMOKE_SHOW_GRID], FL_MENU_TOGGLE);

    arrMenuHandlers[SMOKE_RESET] = new MenuHandler(pGLWin, NULL);
    arrMenuHandlers[SMOKE_RESET]->m_ID = SMOKE_RESET;
    strcpy(arrMenuHandlers[SMOKE_RESET]->m_strItemName, "&OpenCL Smoke/Reset");
    pMenuBar->add(arrMenuHandlers[SMOKE_RESET]->m_strItemName, 0, common_cb, arrMenuHandlers[SMOKE_RESET]);

    arrMenuHandlers[SMOKE_GRID_32_32_64] = new MenuHandler(pGLWin, NULL);
    arrMenuHandlers[SMOKE_GRID_32_32_64]->m_ID = SMOKE_GRID_32_32_64;
    strcpy(arrMenuHandlers[SMOKE_GRID_32_32_64]->m_strItemName, "&OpenCL Smoke/32x32x64");
    pMenuBar->add(arrMenuHandlers[SMOKE_GRID_32_32_64]->m_strItemName, 0, common_cb, arrMenuHandlers[SMOKE_GRID_32_32_64], FL_MENU_RADIO);

    arrMenuHandlers[SMOKE_GRID_64_64_64] = new MenuHandler(pGLWin, NULL);
    arrMenuHandlers[SMOKE_GRID_64_64_64]->m_ID = SMOKE_GRID_64_64_64;
    strcpy(arrMenuHandlers[SMOKE_GRID_64_64_64]->m_strItemName, "&OpenCL Smoke/64x64x64");
    pMenuBar->add(arrMenuHandlers[SMOKE_GRID_64_64_64]->m_strItemName, 0, common_cb, arrMenuHandlers[SMOKE_GRID_64_64_64], FL_MENU_RADIO | FL_MENU_VALUE);

    arrMenuHandlers[SMOKE_GRID_64_64_128] = new MenuHandler(pGLWin, NULL);
    arrMenuHandlers[SMOKE_GRID_64_64_128]->m_ID = SMOKE_GRID_64_64_128;
    strcpy(arrMenuHandlers[SMOKE_GRID_64_64_128]->m_strItemName, "&OpenCL Smoke/64x64x128");
    pMenuBar->add(arrMenuHandlers[SMOKE_GRID_64_64_128]->m_strItemName, 0, common_cb, arrMenuHandlers[SMOKE_GRID_64_64_128], FL_MENU_RADIO);

    arrMenuHandlers[SMOKE_GRID_128_128_128] = new MenuHandler(pGLWin, NULL);
    arrMenuHandlers[SMOKE_GRID_128_128_128]->m_ID = SMOKE_GRID_128_128_128;
    strcpy(arrMenuHandlers[SMOKE_GRID_128_128_128]->m_strItemName, "&OpenCL Smoke/128x128x128");
    pMenuBar->add(arrMenuHandlers[SMOKE_GRID_128_128_128]->m_strItemName, 0, common_cb, arrMenuHandlers[SMOKE_GRID_128_128_128], FL_MENU_RADIO);

    arrMenuHandlers[SMOKE_GRID_128_128_256] = new MenuHandler(pGLWin, NULL);
    arrMenuHandlers[SMOKE_GRID_128_128_256]->m_ID = SMOKE_GRID_128_128_256;
    strcpy(arrMenuHandlers[SMOKE_GRID_128_128_256]->m_strItemName, "&OpenCL Smoke/128x128x256");
    pMenuBar->add(arrMenuHandlers[SMOKE_GRID_128_128_256]->m_strItemName, 0, common_cb, arrMenuHandlers[SMOKE_GRID_128_128_256], FL_MENU_RADIO);

    AMDTOpenCLHelper* pOclH = AMDTOpenCLHelper::GetInstance();

    if (NULL != pOclH)
    {
        const OCLInfo* pOclInfo = pOclH->getOpenCLInfo();

        if (NULL != pOclInfo)
        {
            int smokeSIMDeviceSubmenuIndex = 0;
            char strSmokeSIMDeviceSubmenu[80];
            strncpy(strSmokeSIMDeviceSubmenu, "&OpenCL Smoke/Choose Smoke SIM Device.../", sizeof(strSmokeSIMDeviceSubmenu));
            createSmokeDeviceSubMenu(pMenuBar,
                                     strSmokeSIMDeviceSubmenu,
                                     smokeSIMDeviceSubmenuIndex,
                                     pOclInfo,
                                     SSCID_CHANGE_SMOKE_SIM_DEVICE,
                                     pGLWin);
            int volumeSliceDeviceSubmenuIndex = 0;
            char strVolumeSliceDeviceSubmenu[80];

            strncpy(strVolumeSliceDeviceSubmenu, "&OpenCL Smoke/Choose Volume Slice Device.../", sizeof(strVolumeSliceDeviceSubmenu));
            createSmokeDeviceSubMenu(pMenuBar,
                                     strVolumeSliceDeviceSubmenu,
                                     volumeSliceDeviceSubmenuIndex,
                                     pOclInfo,
                                     SSCID_CHANGE_VOL_SLICE_DEVICE,
                                     pGLWin);
        }
    }

    arrMenuHandlers[SMOKE_SHOW_ERROR] = new MenuHandler(pGLWin, NULL);
    arrMenuHandlers[SMOKE_SHOW_ERROR]->m_ID = SMOKE_SHOW_ERROR;
    strcpy(arrMenuHandlers[SMOKE_SHOW_ERROR]->m_strItemName, "&OpenCL Smoke/Show Errors");
    pMenuBar->add(arrMenuHandlers[SMOKE_SHOW_ERROR]->m_strItemName, FL_COMMAND + 'h', common_cb, arrMenuHandlers[SMOKE_SHOW_ERROR]);

    arrMenuHandlers[HELP_ABOUT] = new MenuHandler(pGLWin, NULL);
    arrMenuHandlers[HELP_ABOUT]->m_ID = HELP_ABOUT;
    strcpy(arrMenuHandlers[HELP_ABOUT]->m_strItemName, "&Help/&About AMD Teapot Example...");
    pMenuBar->add(arrMenuHandlers[HELP_ABOUT]->m_strItemName, 0, common_cb, arrMenuHandlers[HELP_ABOUT]);
    win.resizable(pGLWin);
    win.end();
    win.show();
    return (Fl::run());
}

