//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acIcons.cpp
///
//==================================================================================

//------------------------------ acIcons.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local:
#include <AMDTApplicationComponents/Include/acDisplay.h>
#include <AMDTApplicationComponents/Include/acIcons.h>

static QImage ac_stat_iconsArray[AC_NUMBER_OF_ICON_IDS][AC_NUMBER_OF_ICON_SIZES];

// An array of {icon id, x index, y index} locations in each image_map.
// Generated from acIcons.xls with the formula '=IF(C2="", "", "{" &C2&", "&C$1&", "&$B2&"},")' :
struct acIconCoord
{
    int iconId;
    int x;
    int y;
};

static const acIconCoord ac_stat_icon16_coords[] =
{
    { AC_ICON_EMPTY,                0, 0 },
    { AC_ICON_CODEXL_LOGO,          1, 0 },
    { AC_ICON_CODEXL_LOGO_INVERTED, 2, 0 },
    //-----------------------------------//
    { AC_ICON_VIEW_PROPERTIES,      8, 0 },
    { AC_ICON_VIEW_INFORMATION,     9, 0 },

    { AC_ICON_APPTREE_ROOT, 0, 1 },
    { AC_ICON_APPTREE_BACK, 1, 1 },
    { AC_ICON_APPTREE_FWD,  2, 1 },
    //-----------------------------------//
    { AC_ICON_RIBBON_OPEN,              8, 1 },
    { AC_ICON_RIBBON_OPEN_DISABLED,     9, 1 },
    { AC_ICON_RIBBON_CLOSE,             10, 1 },
    { AC_ICON_RIBBON_CLOSE_DISABLED,    11, 1 },
    { AC_ICON_RIBBON_FILTER,            12, 1 },

    { AC_ICON_EXECUTION_PLAY,           0, 2 },
    { AC_ICON_EXECUTION_PAUSE,          1, 2 },
    { AC_ICON_EXECUTION_STOP,           2, 2 },
    { AC_ICON_EXECUTION_API_STEP,       3, 2 },
    { AC_ICON_EXECUTION_DRAW_STEP,      4, 2 },
    { AC_ICON_EXECUTION_FRAME_STEP,     5, 2 },
    { AC_ICON_EXECUTION_STEP_IN,        6, 2 },
    { AC_ICON_EXECUTION_STEP_OVER,      7, 2 },
    { AC_ICON_EXECUTION_STEP_OUT,       8, 2 },
    { AC_ICON_EXECUTION_BUILD,          9, 2 },
    //      { AC_ICON_EXECUTION_BUILD_ANALYZE, 10, 2 },
    { AC_ICON_EXECUTION_CANCEL_BUILD,  11, 2 },
    { AC_ICON_EXECUTION_CAPTURE,       12, 2 },

    { AC_ICON_SOURCE_C,                      0, 3 },
    { AC_ICON_SOURCE_CL,                     1, 3 },
    { AC_ICON_SOURCE_CPP,                    2, 3 },
    { AC_ICON_SOURCE_GENERIC,                3, 3 },
    { AC_ICON_SOURCE_GLSL,                   4, 3 },
    { AC_ICON_SOURCE_H,                      5, 3 },
    { AC_ICON_SOURCE_READONLY,               6, 3 },
    //--------------------------------------------//
    { AC_ICON_SOURCE_DISABLED_BREAKPOINT,   16, 3 },
    { AC_ICON_SOURCE_ENABLED_BREAKPOINT,    17, 3 },
    { AC_ICON_SOURCE_PROGRAM_COUNTER,       18, 3 },
    { AC_ICON_SOURCE_TOP_PROGRAM_COUNTER,   19, 3 },
    //--------------------------------------------//
    { AC_ICON_FIND_UP,                      24, 3 },
    { AC_ICON_FIND_DOWN,                    25, 3 },
    { AC_ICON_FIND_CLOSE_CLEAR,             26, 3 },
    { AC_ICON_FIND_FIND,                    27, 3 },

    { AC_ICON_DEBUG_MODE,                0, 4 },
    { AC_ICON_WARNING_INFO,              1, 4 },
    { AC_ICON_WARNING_YELLOW,            2, 4 },
    { AC_ICON_WARNING_ORANGE,            3, 4 },
    { AC_ICON_WARNING_RED,               4, 4 },
    { AC_ICON_DEBUG_VIEW_BREAKPOINTS,    8, 4 },
    { AC_ICON_DEBUG_VIEW_CALLSTACK,      9, 4 },
    { AC_ICON_DEBUG_VIEW_CALLHISTORY,   10, 4 },
    { AC_ICON_DEBUG_VIEW_LOCALS,        11, 4 },
    { AC_ICON_DEBUG_VIEW_MEMORY,        12, 4 },
    { AC_ICON_DEBUG_VIEW_EVENTS,        13, 4 },
    { AC_ICON_DEBUG_VIEW_STATISTICS,    14, 4 },
    { AC_ICON_DEBUG_VIEW_STATEVARS,     15, 4 },
    { AC_ICON_DEBUG_VIEW_WATCH,         16, 4 },
    { AC_ICON_DEBUG_VIEW_MULTIWATCH,    17, 4 },

    { AC_ICON_DEBUG_APPTREE_GL_CONTEXT,                  0, 5 },
    { AC_ICON_DEBUG_APPTREE_GL_CONTEXTDELETED,           1, 5 },
    { AC_ICON_DEBUG_APPTREE_GL_CONTEXTSHARED,            2, 5 },
    { AC_ICON_DEBUG_APPTREE_GL_CONTEXTDELETEDSHARED,     3, 5 },
    { AC_ICON_DEBUG_APPTREE_GL_TEXGENERIC,               4, 5 },
    { AC_ICON_DEBUG_APPTREE_GL_TEX1D,                    5, 5 },
    { AC_ICON_DEBUG_APPTREE_GL_TEX2D,                    6, 5 },
    { AC_ICON_DEBUG_APPTREE_GL_TEX2DMS,                  7, 5 },
    { AC_ICON_DEBUG_APPTREE_GL_TEXCUBE,                  8, 5 },
    { AC_ICON_DEBUG_APPTREE_GL_TEX3D,                    9, 5 },
    { AC_ICON_DEBUG_APPTREE_GL_TEXRECT,                 10, 5 },
    { AC_ICON_DEBUG_APPTREE_GL_TEX1DA,                  11, 5 },
    { AC_ICON_DEBUG_APPTREE_GL_TEX2DA,                  12, 5 },
    { AC_ICON_DEBUG_APPTREE_GL_TEX2DMSA,                13, 5 },
    { AC_ICON_DEBUG_APPTREE_GL_TEXCUBEA,                14, 5 },
    { AC_ICON_DEBUG_APPTREE_GL_TEXBUFFER,               15, 5 },
    { AC_ICON_DEBUG_APPTREE_GL_TEXUNKNOWN,              16, 5 },
    // AC_ICON_DEBUG_APPTREE_GL_BUFFER_* are placed at the end
    // of line 6, due to space contraints in the image grid.
    { AC_ICON_DEBUG_APPTREE_GL_SAMPLER,                 17, 5 },
    { AC_ICON_DEBUG_APPTREE_GL_STATICBUFFER,            18, 5 },
    { AC_ICON_DEBUG_APPTREE_GL_STATICBUFFERLINK,        19, 5 },
    { AC_ICON_DEBUG_APPTREE_GL_RENDERBUFFER,            20, 5 },
    { AC_ICON_DEBUG_APPTREE_GL_RENDERBUFFERLINK,        21, 5 },
    { AC_ICON_DEBUG_APPTREE_GL_FBO,                     22, 5 },
    { AC_ICON_DEBUG_APPTREE_GL_PROGRAM,                 23, 5 },
    { AC_ICON_DEBUG_APPTREE_GL_PROGRAMDELETED,          24, 5 },
    { AC_ICON_DEBUG_APPTREE_GL_SHADER,                  25, 5 },
    { AC_ICON_DEBUG_APPTREE_GL_SHADERDELETED,           26, 5 },
    { AC_ICON_DEBUG_APPTREE_GL_PIPELINE,                27, 5 },
    { AC_ICON_DEBUG_APPTREE_GL_DISPLAYLIST,             28, 5 },
    { AC_ICON_DEBUG_APPTREE_GL_PBUFFER,                 29, 5 },
    { AC_ICON_DEBUG_APPTREE_GL_SYNC,                    30, 5 },

    { AC_ICON_DEBUG_APPTREE_CL_CONTEXT,                      0, 6 },
    { AC_ICON_DEBUG_APPTREE_CL_CONTEXTDELETED,               1, 6 },
    { AC_ICON_DEBUG_APPTREE_CL_BUFFER,                       2, 6 },
    { AC_ICON_DEBUG_APPTREE_CL_IMAGEGENERIC,                 3, 6 },
    { AC_ICON_DEBUG_APPTREE_CL_IMAGE2D,                      4, 6 },
    { AC_ICON_DEBUG_APPTREE_CL_IMAGE3D,                      5, 6 },
    { AC_ICON_DEBUG_APPTREE_CL_PIPE,                         6, 6 },
    { AC_ICON_DEBUG_APPTREE_CL_SAMPLER,                      7, 6 },
    { AC_ICON_DEBUG_APPTREE_CL_QUEUE,                        8, 6 },
    { AC_ICON_DEBUG_APPTREE_CL_EVENT,                        9, 6 },
    { AC_ICON_DEBUG_APPTREE_CL_PROGRAM,                     10, 6 },
    { AC_ICON_DEBUG_APPTREE_CL_KERNEL,                      11, 6 },
    //------------------------------------------------------------//
    { AC_ICON_DEBUG_APPTREE_GL_BUFFER_GENERIC,              16, 6 },
    { AC_ICON_DEBUG_APPTREE_GL_BUFFER_ARRAY,                17, 6 },
    { AC_ICON_DEBUG_APPTREE_GL_BUFFER_DRAW_INDIRECT,        18, 6 },
    { AC_ICON_DEBUG_APPTREE_GL_BUFFER_DISPATCH_INDIRECT,    19, 6 },
    { AC_ICON_DEBUG_APPTREE_GL_BUFFER_ELEMENT_ARRAY,        20, 6 },
    { AC_ICON_DEBUG_APPTREE_GL_BUFFER_PIXEL_PACK,           21, 6 },
    { AC_ICON_DEBUG_APPTREE_GL_BUFFER_PIXEL_UNPACK,         22, 6 },
    { AC_ICON_DEBUG_APPTREE_GL_BUFFER_COPY_READ,            23, 6 },
    { AC_ICON_DEBUG_APPTREE_GL_BUFFER_COPY_WRITE,           24, 6 },
    { AC_ICON_DEBUG_APPTREE_GL_BUFFER_TRANSFORM_FEEDBACK,   25, 6 },
    { AC_ICON_DEBUG_APPTREE_GL_BUFFER_UNIFORM,              26, 6 },
    { AC_ICON_DEBUG_APPTREE_GL_BUFFER_ATOMIC_COUNTER,       27, 6 },
    { AC_ICON_DEBUG_APPTREE_GL_BUFFER_SHADER_STORAGE,       28, 6 },
    { AC_ICON_DEBUG_APPTREE_GL_BUFFER_QUERY,                29, 6 },
    { AC_ICON_DEBUG_APPTREE_GL_BUFFER_TEXTURE,              30, 6 },
    { AC_ICON_DEBUG_APPTREE_GL_BUFFER_UNKNOWN,              31, 6 },

    { AC_ICON_DEBUG_CALLSHISTORY_GL,                 0, 7 },
    { AC_ICON_DEBUG_CALLSHISTORY_GLEXT,              1, 7 },
    { AC_ICON_DEBUG_CALLSHISTORY_WGL,                2, 7 },
    { AC_ICON_DEBUG_CALLSHISTORY_GLX,                3, 7 },
    { AC_ICON_DEBUG_CALLSHISTORY_CGL,                4, 7 },
    { AC_ICON_DEBUG_CALLSHISTORY_CL,                 5, 7 },
    //----------------------------------------------------//
    { AC_ICON_DEBUG_CALLSHISTORY_NEXTMARKER,        16, 7 },
    { AC_ICON_DEBUG_CALLSHISTORY_PREVIOUSMARKER,    17, 7 },
    { AC_ICON_DEBUG_CALLSHISTORY_OPENLOG,           18, 7 },
    { AC_ICON_DEBUG_CALLSHISTORY_RECORD,            19, 7 },

    { AC_ICON_DEBUG_EVENTSVIEW_PROCESS_PLUS,         0, 8 },
    { AC_ICON_DEBUG_EVENTSVIEW_PROCESS_RUN,          1, 8 },
    { AC_ICON_DEBUG_EVENTSVIEW_PROCESS_MINUS,        2, 8 },
    { AC_ICON_DEBUG_EVENTSVIEW_MODULE_PLUS,          3, 8 },
    { AC_ICON_DEBUG_EVENTSVIEW_MODULE_MINUS,         4, 8 },
    { AC_ICON_DEBUG_EVENTSVIEW_THREAD_PLUS,          5, 8 },
    { AC_ICON_DEBUG_EVENTSVIEW_THREAD_MINUS,         6, 8 },
    { AC_ICON_DEBUG_EVENTSVIEW_CONNECTION_PLUS,      7, 8 },
    { AC_ICON_DEBUG_EVENTSVIEW_CONNECTION_MINUS,     8, 8 },
    { AC_ICON_DEBUG_EVENTSVIEW_GLCONTEXT_PLUS,       9, 8 },
    { AC_ICON_DEBUG_EVENTSVIEW_GLCONTEXT_MINUS,     10, 8 },
    { AC_ICON_DEBUG_EVENTSVIEW_CLCONTEXT_PLUS,      11, 8 },
    { AC_ICON_DEBUG_EVENTSVIEW_CLCONTEXT_MINUS,     12, 8 },
    { AC_ICON_DEBUG_EVENTSVIEW_CLQUEUE_PLUS,        13, 8 },
    { AC_ICON_DEBUG_EVENTSVIEW_CLQUEUE_MINUS,       14, 8 },
    { AC_ICON_DEBUG_EVENTSVIEW_CLPROGRAM_PLUS,      15, 8 },
    { AC_ICON_DEBUG_EVENTSVIEW_CLPROGRAM_MINUS,     16, 8 },
    { AC_ICON_DEBUG_EVENTSVIEW_CLPROGRAM_BUILD,     17, 8 },
    { AC_ICON_DEBUG_EVENTSVIEW_BREAK,               18, 8 },
    { AC_ICON_DEBUG_EVENTSVIEW_BREAK_POINT,         19, 8 },
    { AC_ICON_DEBUG_EVENTSVIEW_BREAK_APISTEP,       20, 8 },
    { AC_ICON_DEBUG_EVENTSVIEW_BREAK_DRAWSTEP,      21, 8 },
    { AC_ICON_DEBUG_EVENTSVIEW_BREAK_FRAMESTEP,     22, 8 },
    { AC_ICON_DEBUG_EVENTSVIEW_BREAK_GLERROR,       23, 8 },
    { AC_ICON_DEBUG_EVENTSVIEW_BREAK_CLERROR,       24, 8 },
    { AC_ICON_DEBUG_EVENTSVIEW_CLERROR,             25, 8 },
    { AC_ICON_DEBUG_EVENTSVIEW_BREAK_DETECTED,      26, 8 },
    { AC_ICON_DEBUG_EVENTSVIEW_DETECTED,            27, 8 },
    { AC_ICON_DEBUG_EVENTSVIEW_BREAK_REDUNDANT,     28, 8 },
    { AC_ICON_DEBUG_EVENTSVIEW_BREAK_DEPRECATED,    29, 8 },
    { AC_ICON_DEBUG_EVENTSVIEW_BREAK_MEMORY,        30, 8 },
    { AC_ICON_DEBUG_EVENTSVIEW_BREAK_SWFALLBACK,    31, 8 },

    { AC_ICON_DEBUG_EVENTSVIEW_BREAK_UNKNOWN,        0, 9 },
    { AC_ICON_DEBUG_EVENTSVIEW_EXCEPTION,            1, 9 },
    { AC_ICON_DEBUG_EVENTSVIEW_OUTPUT,               2, 9 },
    { AC_ICON_DEBUG_EVENTSVIEW_GDB,                  3, 9 },

    { AC_ICON_DEBUG_TEXVW_TOOLBAR_ZOOMIN,    0, 10 },
    { AC_ICON_DEBUG_TEXVW_TOOLBAR_ZOOMOUT,   1, 10 },
    { AC_ICON_DEBUG_TEXVW_TOOLBAR_PAN,       2, 10 },
    { AC_ICON_DEBUG_TEXVW_TOOLBAR_MOUSE,     3, 10 },
    { AC_ICON_DEBUG_TEXVW_TOOLBAR_RED,       4, 10 },
    { AC_ICON_DEBUG_TEXVW_TOOLBAR_GREEN,     5, 10 },
    { AC_ICON_DEBUG_TEXVW_TOOLBAR_BLUE,      6, 10 },
    { AC_ICON_DEBUG_TEXVW_TOOLBAR_ALPHA,     7, 10 },
    { AC_ICON_DEBUG_TEXVW_TOOLBAR_GRAY,      8, 10 },
    { AC_ICON_DEBUG_TEXVW_TOOLBAR_INVERT,    9, 10 },
    { AC_ICON_DEBUG_TEXVW_TOOLBAR_ROTATEL,  10, 10 },
    { AC_ICON_DEBUG_TEXVW_TOOLBAR_ROTATER,  11, 10 },
    { AC_ICON_DEBUG_TEXVW_TOOLBAR_ORIGINAL, 12, 10 },
    { AC_ICON_DEBUG_TEXVW_TOOLBAR_BEST,     13, 10 },
    //---------------------------------------------//
    { AC_ICON_DEBUG_STATISTICS_CLEAR,       24, 10 },
    { AC_ICON_DEBUG_STATISTICS_BATCH,       25, 10 },

    { AC_ICON_PROFILE_MODE,                     0, 12 },
    //------------------------------------------------//
    { AC_ICON_PROFILE_APPTREE_SESSION_SINGLE,   8, 12 },
    { AC_ICON_PROFILE_APPTREE_SESSION_MULTI,    9, 12 },
    { AC_ICON_PROFILE_APPTREE_KERNEL_SINGLE,    10, 12 },
    { AC_ICON_PROFILE_APPTREE_KERNEL_MULTI,     11, 12 },

    { AC_ICON_PROFILE_GPU_APPTREE_COUNTERS_SINGLE,  0, 16 },
    { AC_ICON_PROFILE_GPU_APPTREE_COUNTERS_MULTI,   1, 16 },
    { AC_ICON_PROFILE_GPU_APPTREE_APPTRACE_SINGLE,  2, 16 },
    { AC_ICON_PROFILE_GPU_APPTREE_APPTRACE_MULTI,   3, 16 },

    { AC_ICON_PROFILE_GPU_APPTREE_SUMMARY,          0, 17 },
    { AC_ICON_PROFILE_GPU_APPTREE_CL_API,           1, 17 },
    { AC_ICON_PROFILE_GPU_APPTREE_HSA_API,          2, 17 },
    { AC_ICON_PROFILE_GPU_APPTREE_CONTEXT,          3, 17 },
    { AC_ICON_PROFILE_GPU_APPTREE_KERNEL,           4, 17 },
    { AC_ICON_PROFILE_GPU_APPTREE_TOP10_TRANSFER,   5, 17 },
    { AC_ICON_PROFILE_GPU_APPTREE_TOP10_KERNEL,     6, 17 },
    { AC_ICON_PROFILE_GPU_APPTREE_BESTPRACTICE,     7, 17 },


    { AC_ICON_PROFILE_CPU_APPTREE_TBS_SINGLE,            0, 20 },
    { AC_ICON_PROFILE_CPU_APPTREE_TBS_MULTI,             1, 20 },
    { AC_ICON_PROFILE_CPU_APPTREE_ASSESS_SINGLE,         2, 20 },
    { AC_ICON_PROFILE_CPU_APPTREE_ASSESS_MULTI,          3, 20 },
    { AC_ICON_PROFILE_CPU_APPTREE_IBS_SINGLE,            4, 20 },
    { AC_ICON_PROFILE_CPU_APPTREE_IBS_MULTI,             5, 20 },
    { AC_ICON_PROFILE_CPU_APPTREE_BRANCHING_SINGLE,      6, 20 },
    { AC_ICON_PROFILE_CPU_APPTREE_BRANCHING_MULTI,       7, 20 },
    { AC_ICON_PROFILE_CPU_APPTREE_DATAACCESS_SINGLE,     8, 20 },
    { AC_ICON_PROFILE_CPU_APPTREE_DATAACCESS_MULTI,      9, 20 },
    { AC_ICON_PROFILE_CPU_APPTREE_INSTACCESS_SINGLE,    10, 20 },
    { AC_ICON_PROFILE_CPU_APPTREE_INSTACCESS_MULTI,     11, 20 },
    { AC_ICON_PROFILE_CPU_APPTREE_L2CACHE_SINGLE,       12, 20 },
    { AC_ICON_PROFILE_CPU_APPTREE_L2CACHE_MULTI,        13, 20 },
    { AC_ICON_PROFILE_CPU_APPTREE_CUSTOM_SINGLE,        14, 20 },
    { AC_ICON_PROFILE_CPU_APPTREE_CUSTOM_MULTI,         15, 20 },
    { AC_ICON_PROFILE_CPU_APPTREE_CLU_SINGLE,           16, 20 },
    { AC_ICON_PROFILE_CPU_APPTREE_CLU_MULTI,            17, 20 },

    { AC_ICON_PROFILE_CPU_APPTREE_OVERVIEW,     0, 21 },
    { AC_ICON_PROFILE_CPU_APPTREE_MODULES,      1, 21 },
    { AC_ICON_PROFILE_CPU_APPTREE_CALLGRAPH,    2, 21 },
    { AC_ICON_PROFILE_CPU_APPTREE_FUNCTIONS,    3, 21 },
    { AC_ICON_PROFILE_CPU_APPTREE_SOURCE,       4, 21 },
    { AC_ICON_PROFILE_CPU_APPTREE_SOURCES,      5, 21 },

    { AC_ICON_PROFILE_CPU_MOUDLE_USER_32,   0, 22 },
    { AC_ICON_PROFILE_CPU_MOUDLE_USER_64,   1, 22 },
    { AC_ICON_PROFILE_CPU_MOUDLE_SYSTEM_32, 2, 22 },
    { AC_ICON_PROFILE_CPU_MOUDLE_SYSTEM_64, 3, 22 },

    { AC_ICON_PROFILE_PWR_APPTREE_SESSION_SINGLE,    0, 24 },
    { AC_ICON_PROFILE_PWR_APPTREE_SESSION_MULTI,     1, 24 },
    //-----------------------------------------------------//
    { AC_ICON_PROFILE_PWR_APPTREE_TIMELINE,         16, 24 },
    { AC_ICON_PROFILE_PWR_APPTREE_SUMMARY,          17, 24 },
    { AC_ICON_PROFILE_PWR_APPTREE_NEW,              18, 24 },

    { AC_ICON_PROFILE_PWR_COUNTER_POWER,        0, 25 },
    { AC_ICON_PROFILE_PWR_COUNTER_FREQUENCY,    1, 25 },
    { AC_ICON_PROFILE_PWR_COUNTER_CURRENT,      2, 25 },
    { AC_ICON_PROFILE_PWR_COUNTER_VOLTAGE,      3, 25 },
    { AC_ICON_PROFILE_PWR_COUNTER_DVFS,         4, 25 },
    { AC_ICON_PROFILE_PWR_COUNTER_PID,          5, 25 },
    { AC_ICON_PROFILE_PWR_COUNTER_TEMPRATURE,   6, 25 },
    { AC_ICON_PROFILE_PWR_COUNTER_COUNT,        7, 25 },

    { AC_ICON_PROFILE_THR_APPTREE_SESSION_SINGLE,    0, 23 },
    { AC_ICON_PROFILE_THR_APPTREE_SESSION_MULTI,     1, 23 },
    //-----------------------------------------------------//
    { AC_ICON_PROFILE_THR_APPTREE_TIMELINE,         16, 23 },
    { AC_ICON_PROFILE_THR_APPTREE_OVERVIEW,         17, 23 },

    { AC_ICON_BUILD_AND_ANALYZE_MODE,            0, 28 },
    //-------------------------------------------------//
    { AC_ICON_ANALYZE_APPTREE_KERNELS,          16, 28 },
    { AC_ICON_ANALYZE_APPTREE_FOLDER_VK,        17, 28 },
    { AC_ICON_ANALYZE_APPTREE_FOLDER_CL,        18, 28 },
    { AC_ICON_ANALYZE_APPTREE_FOLDER_GL,        19, 28 },
    { AC_ICON_ANALYZE_APPTREE_FOLDER_DX,        20, 28 },
    { AC_ICON_ANALYZE_APPTREE_FOLDER,           21, 28 },

    { AC_ICON_ANALYZE_APPTREE_KERNEL,       0, 29 },
    { AC_ICON_ANALYZE_APPTREE_OVERVIEW,     1, 29 },
    { AC_ICON_ANALYZE_APPTREE_ANALYSIS,     2, 29 },
    { AC_ICON_ANALYZE_APPTREE_STATISTICS,   3, 29 },
    { AC_ICON_ANALYZE_APPTREE_IL,           4, 29 },
    { AC_ICON_ANALYZE_APPTREE_ISA,          5, 29 },
    { AC_ICON_ANALYZE_APPTREE_IL_ISA,       6, 29 },
    { AC_ICON_ANALYZE_APPTREE_ADDNEW,       7, 29 },
    { AC_ICON_ANALYZE_APPTREE_OPEN,         8, 29 },
    { AC_ICON_ANALYZE_APPTREE_SOURCE,       9, 29 },

    { AC_ICON_ANALYZE_STATISTICS_MEETS,     0, 30 },
    { AC_ICON_ANALYZE_STATISTICS_DEGRADE,   1, 30 },
    { AC_ICON_ANALYZE_STATISTICS_LIGHTBULB, 2, 30 },

    { AC_ICON_FRAME_ANALYSIS_MODE, 0, 32 },

    { AC_ICON_FRAME_ANALYSIS_APPTREE_SINGLE,    0, 33 },
    { AC_ICON_FRAME_ANALYSIS_APPTREE_MULTI,     1, 33 },

    { AC_ICON_FRAME_ANALYSIS_APP_TREE_DASHBOARD,            0, 34 },
    { AC_ICON_FRAME_ANALYSIS_APP_TREE_FRAME,                1, 34 },
    { AC_ICON_FRAME_ANALYSIS_APP_TREE_OVERVIEW,             2, 34 },
    { AC_ICON_FRAME_ANALYSIS_APP_TREE_TIMELINE,             3, 34 },
    { AC_ICON_FRAME_ANALYSIS_APP_TREE_PERFORMANCE_PROFILE,  4, 34 },
    { AC_ICON_FRAME_ANALYSIS_APP_TREE_IMAGE,                5, 34 },

    { -1, -1, -1 }
};

static const acIconCoord ac_stat_icon32_coords[] =
{
    // { AC_ICON_CODEXL_LOGO,                  0, 0 }, // Icon already present in acIcons_16x2, no need to reread from acIcons_32x1. If we add 32x2, re-add this.
    // { AC_ICON_CODEXL_LOGO_INVERTED,         1, 0 }, // Icon already present in acIcons_16x2, no need to reread from acIcons_32x1. If we add 32x2, re-add this.
    { AC_ICON_AMDCOMPRESS_LOGO,             2, 0 },
    { AC_ICON_AMDCOMPRESS_LOGO_INVERTED,    3, 0 }, // = AC_ICON_AMDCOMPRESS_LOGO

    { AC_ICON_STARTUP_NEW,                  0, 4 },
    { AC_ICON_STARTUP_NEW_DEBUG,            1, 4 },
    { AC_ICON_STARTUP_NEW_PROFILE,          2, 4 },
    { AC_ICON_STARTUP_NEW_PROFILE_ATTACH,   3, 4 },
    { AC_ICON_STARTUP_NEW_PROFILE_SYSTEM,   4, 4 },
    { AC_ICON_STARTUP_NEW_FRAME_ANALYZE,    5, 4 },
    { AC_ICON_STARTUP_NEW_ANALYZE,          6, 4 },
    { AC_ICON_STARTUP_NEW_ANALYZE_FILE,     7, 4 },

    { -1, -1, -1 }
};

/// XPM:
// #include <AMDTApplicationFramework/Include/res/icons/ApplicationIcon_32_xpm.xpm>        // AC_ICON_CODEXL_LOGO, 32
#include <AMDTApplicationFramework/Include/res/icons/ApplicationIcon_64_xpm.xpm>        // AC_ICON_CODEXL_LOGO, 64

// #include <AMDTApplicationFramework/Include/res/icons/AMDCompressIcon_32_xpm.xpm>        // AC_ICON_AMDCOMPRESS_LOGO, 32
#include <AMDTApplicationFramework/Include/res/icons/AMDCompressIcon_64_xpm.xpm>        // AC_ICON_AMDCOMPRESS_LOGO, 64

// Testing values:
// #define AC_TEST_ICON_MARK_SCALED_ICONS
// #define AC_TEST_ICON_FORCE_DEFAULT_SIZE

// #define AC_TEST_ICON_SHOW_ICONS_DLG
// #define AC_TEST_ICON_SHOW_ICONS_DLG_ONLY_DEFINED_SIZES

#ifdef AC_TEST_ICON_SHOW_ICONS_DLG

// The table is too long with 200+ icons, so split it to columns:
#define AC_TEST_ICON_SHOW_ICONS_DLG_NUM_ROWS 25
#define AC_TEST_ICON_SHOW_ICONS_DLG_GROUP_COUNT ((AC_NUMBER_OF_ICON_IDS + AC_TEST_ICON_SHOW_ICONS_DLG_NUM_ROWS - 1) / AC_TEST_ICON_SHOW_ICONS_DLG_NUM_ROWS)
#define AC_TEST_ICON_SHOW_ICONS_DLG_GROUP_WIDTH (AC_NUMBER_OF_ICON_SIZES + 1)
void acShowIconsTestDialog()
{
    QDialog* testDlg = new QDialog;
    QGridLayout* testDlgLayout = new QGridLayout(testDlg);

    // Column headings:
    for (int jj = 0; jj < AC_TEST_ICON_SHOW_ICONS_DLG_GROUP_COUNT; jj++)
    {
        for (int j = 0; j < AC_NUMBER_OF_ICON_SIZES; j++)
        {
            QString rowHeaderText;
            int pxSize = acIconSizeToPixelSize((acIconSize)j);
            rowHeaderText.sprintf("%d", pxSize);
            QLabel* pRowHeader = new QLabel(rowHeaderText);
            testDlgLayout->addWidget(pRowHeader, 0, jj * AC_TEST_ICON_SHOW_ICONS_DLG_GROUP_WIDTH + j + 1);
        }
    }

    // Icon rows:
    for (int i = 0; i < AC_NUMBER_OF_ICON_IDS; i++)
    {
        // Line headings:
        int ii = i % AC_TEST_ICON_SHOW_ICONS_DLG_NUM_ROWS;
        int jj = i / AC_TEST_ICON_SHOW_ICONS_DLG_NUM_ROWS;
        QString lineHeaderText;
        lineHeaderText.sprintf("Icon #%d", i);
        QLabel* pLineHeader = new QLabel(lineHeaderText);
        testDlgLayout->addWidget(pLineHeader, ii + 1, jj * AC_TEST_ICON_SHOW_ICONS_DLG_GROUP_WIDTH);

        for (int j = 0; j < AC_NUMBER_OF_ICON_SIZES; j++)
        {
#ifdef AC_TEST_ICON_SHOW_ICONS_DLG_ONLY_DEFINED_SIZES

            if (!ac_stat_iconsArray[i][j].isNull())
#endif // AC_TEST_ICON_SHOW_ICONS_DLG_ONLY_DEFINED_SIZES
            {
                QPixmap pixmap;
                acSetIconInPixmap(pixmap, (acIconId)i, (acIconSize)j);
                QLabel* pCurrentIcon = new QLabel;
                pCurrentIcon->setPixmap(pixmap);
                testDlgLayout->addWidget(pCurrentIcon, ii + 1, jj * AC_TEST_ICON_SHOW_ICONS_DLG_GROUP_WIDTH + j + 1);
            }
        }
    }

    testDlg->setLayout(testDlgLayout);
    testDlg->exec();

    testDlg->deleteLater();
}

#endif // AC_TEST_ICON_SHOW_ICONS_DLG


// Helper functions:

QImage acGetSubImage(const QImage& master, int x, int y, acIconSize iconSize)
{
    int dim = acIconSizeToPixelSize(iconSize);
    return master.copy(x * dim, y * dim, dim, dim);
}

void acInitializeIconArray()
{
    static bool onlyOnce = true;

    if (onlyOnce)
    {
        onlyOnce = false;

        // Icon image maps:
        QImage imageMap16_x1(":/acIcons_16_x1.png");
        QImage imageMap16_x1_5(":/acIcons_16_x1_5.png");
        QImage imageMap16_x2(":/acIcons_16_x2.png");
        QImage imageMap32_x1(":/acIcons_32_x1.png");

        // ac_stat_iconsArray[AC_ICON_CODEXL_LOGO][AC_32x32_ICON] handled in the loop below
        ac_stat_iconsArray[AC_ICON_CODEXL_LOGO][AC_64x64_ICON] = QImage(ApplicationIcon_64_xpm);

        // ac_stat_iconsArray[AC_ICON_AMDCOMPRESS_LOGO][AC_32x32_ICON] handled in the loop below
        ac_stat_iconsArray[AC_ICON_AMDCOMPRESS_LOGO][AC_64x64_ICON] = QImage(AMDTCompressApplicationIcon_64_xpm);
        // ac_stat_iconsArray[AC_ICON_AMDCOMPRESS_LOGO_INVERTED][AC_32x32_ICON] handled in the loop below
        ac_stat_iconsArray[AC_ICON_AMDCOMPRESS_LOGO_INVERTED][AC_32x32_ICON].invertPixels(QImage::InvertRgb);

        // Add 16x16 icons:
        int i = 0;
        bool goOn = true;

        while (goOn)
        {
            // Get the next item:
            const acIconCoord& currentIconCoord = ac_stat_icon16_coords[i++];

            if (currentIconCoord.iconId < 0)
            {
                goOn = false;
            }
            else
            {
                ac_stat_iconsArray[currentIconCoord.iconId][AC_16x16_ICON] = acGetSubImage(imageMap16_x1, currentIconCoord.x, currentIconCoord.y, AC_16x16_ICON);
                ac_stat_iconsArray[currentIconCoord.iconId][AC_24x24_ICON] = acGetSubImage(imageMap16_x1_5, currentIconCoord.x, currentIconCoord.y, AC_24x24_ICON);
                ac_stat_iconsArray[currentIconCoord.iconId][AC_32x32_ICON] = acGetSubImage(imageMap16_x2, currentIconCoord.x, currentIconCoord.y, AC_32x32_ICON);
            }
        }

        // Add 32x32 icons:
        i = 0;
        goOn = true;

        while (goOn)
        {
            // Get the next item:
            const acIconCoord& currentIconCoord = ac_stat_icon32_coords[i++];

            if (currentIconCoord.iconId < 0)
            {
                goOn = false;
            }
            else
            {
                ac_stat_iconsArray[currentIconCoord.iconId][AC_32x32_ICON] = acGetSubImage(imageMap32_x1, currentIconCoord.x, currentIconCoord.y, AC_32x32_ICON);
            }
        }

#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD

#ifdef AC_TEST_ICON_SHOW_ICONS_DLG
        // Show the test dialog, if required:
        acShowIconsTestDialog();
#endif // AC_TEST_ICON_SHOW_ICONS_DLG

        // On Debug only, validate that we have at least one image for each id:
        gtString missingIds;

        for (int ii = 0; ii < AC_NUMBER_OF_ICON_IDS; ii++)
        {
            bool foundImage = false;

            for (int j = 0; j < AC_NUMBER_OF_ICON_SIZES && !foundImage; j++)
            {
                if (!ac_stat_iconsArray[ii][j].isNull())
                {
                    foundImage = true;
                }
            }

            if (!foundImage)
            {
                missingIds.appendFormattedString(L" %d", ii);
            }
        }

        if (!missingIds.isEmpty())
        {
            missingIds.prepend(L"The following icon IDs have no image attached for any size:");
            GT_ASSERT_EX(false, missingIds.asCharArray());
        }

#endif // AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
    }
}

acIconSize acFindIconSizeMatchForId(acIconId iconId, acIconSize iconSize)
{
    acIconSize retVal = AC_NUMBER_OF_ICON_SIZES;

    acIconSize sizesToTest[AC_NUMBER_OF_ICON_SIZES] = { iconSize };

    switch (iconSize)
    {
        case AC_16x16_ICON:
            sizesToTest[1] = AC_24x24_ICON;
            sizesToTest[2] = AC_32x32_ICON;
            sizesToTest[3] = AC_48x48_ICON;
            sizesToTest[4] = AC_64x64_ICON;
            break;

        case AC_24x24_ICON:
            sizesToTest[1] = AC_32x32_ICON;
            sizesToTest[2] = AC_16x16_ICON;
            sizesToTest[3] = AC_48x48_ICON;
            sizesToTest[4] = AC_64x64_ICON;
            break;

        case AC_32x32_ICON:
            sizesToTest[1] = AC_48x48_ICON;
            sizesToTest[2] = AC_24x24_ICON;
            sizesToTest[3] = AC_64x64_ICON;
            sizesToTest[4] = AC_16x16_ICON;
            break;

        case AC_48x48_ICON:
            sizesToTest[1] = AC_64x64_ICON;
            sizesToTest[2] = AC_32x32_ICON;
            sizesToTest[3] = AC_24x24_ICON;
            sizesToTest[4] = AC_16x16_ICON;
            break;

        case AC_64x64_ICON:
            sizesToTest[1] = AC_48x48_ICON;
            sizesToTest[2] = AC_32x32_ICON;
            sizesToTest[3] = AC_24x24_ICON;
            sizesToTest[4] = AC_16x16_ICON;
            break;

        case AC_NUMBER_OF_ICON_SIZES:
        default:
            GT_ASSERT(false);
            // Return here to avoid accessing an invalid location in the array:
            return AC_NUMBER_OF_ICON_SIZES;
            break;
    }

    for (int i = 0; i < AC_NUMBER_OF_ICON_SIZES; i++)
    {
        acIconSize currentSize = sizesToTest[i];

        if (currentSize < AC_NUMBER_OF_ICON_SIZES)
        {
            if (!ac_stat_iconsArray[iconId][currentSize].isNull())
            {
                retVal = currentSize;
                break;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acIconSizeToPixelSize
// Description: Gets the icon's size in pixels. To be used to initialize image lists, etc.
// Author:      Uri Shomroni
// Date:        22/9/2014
// ---------------------------------------------------------------------------
int acIconSizeToPixelSize(acIconSize iconSize)
{
    int retVal = 16;

    switch (iconSize)
    {
        case AC_16x16_ICON:
            retVal = 16;
            break;

        case AC_24x24_ICON:
            retVal = 24;
            break;

        case AC_32x32_ICON:
            retVal = 32;
            break;

        case AC_48x48_ICON:
            retVal = 48;
            break;

        case AC_64x64_ICON:
            retVal = 64;
            break;

        case AC_NUMBER_OF_ICON_SIZES:
        default:
            // Unexpected value!
            GT_ASSERT(false);
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acGetRecommendedIconSize
// Description: Gets the recommended icon size for toolbars and controls.
//              This value is cached to avoid inconsistencies.
// Author:      Uri Shomroni
// Date:        22/9/2014
// ---------------------------------------------------------------------------
acIconSize acGetRecommendedIconSize()
{
    static acIconSize retVal = AC_16x16_ICON;
    static bool onlyOnce = true;
#ifdef AC_TEST_ICON_FORCE_DEFAULT_SIZE
    onlyOnce = false;
#endif

    if (onlyOnce)
    {
        onlyOnce = false;

        // Get the application DPI:
        unsigned int applicationDPI = acGetApplicationDPI();

        // The optimal DPI for each icon size:
        // +-----------------+----+------+-----+-----+-----+
        // | Icon Size (NxN) | 16 |   24 |  32 |  48 |  64 |
        // | Scale           | 1x | 1.5x |  2x |  3x |  4x |
        // | Best DPI        | 96 |  144 | 192 | 288 | 384 |
        // +-----------------+----+------+-----+-----+-----+
        // Thus, we will limit the values at the mid-point for each range:
        // 96 ~ 120 ~ 144 ~ 168 ~ 192 ~ 240 ~ 288 ~ 336 ~ 384
        if (applicationDPI < 120)
        {
            retVal = AC_16x16_ICON;
        }
        else if (applicationDPI < 168)
        {
            retVal = AC_24x24_ICON;
        }
        else if (applicationDPI < 240)
        {
            retVal = AC_32x32_ICON;
        }
        else if (applicationDPI < 336)
        {
            retVal = AC_48x48_ICON;
        }
        else
        {
            // Make sure the value makes *SOME SORT* of sense, or we'll need bigger icons:
            GT_ASSERT(applicationDPI <= 768);
            retVal = AC_64x64_ICON;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acGetScaledIconSize
// Description: Gets the scaled icon size for toolbars and controls.
//              This value is based on the value of acGetRecommendedIconSize.
// Author:      Uri Shomroni
// Date:        8/2/2016
// ---------------------------------------------------------------------------
acIconSize acGetScaledIconSize(acIconSize x1Size)
{
    // +----------------------+----+------+-----+-----+-----+
    // | Rec. Icon Size (NxN) | 16 |   24 |  32 |  48 |  64 |
    // | Scale                | 1x | 1.5x |  2x |  3x |  4x |
    // +----------------------+----+------+-----+-----+-----+
    // | Scaled 16x16         | 16 |   24 |  32 |  48 |  64 |
    // | Scaled 24x24         | 24 |   32 |  48 |  64 | 64+ |
    // | Scaled 32x32         | 32 |   48 |  64 | 64+ | 64+ |
    // | Scaled 48x48         | 48 |   64 | 64+ | 64+ | 64+ |
    // | Scaled 64x64         | 64 |  64+ | 64+ | 64+ | 64+ |
    // +----------------------+----+------+-----+-----+-----+
    // The "64+" sizes, in order, should be 96, 128, 192, 256.
    // However, we do not support these currently (they would be added to the enum if needed)
    int baseIconSize = (int)acGetRecommendedIconSize();
    int scaledSize = ((int)x1Size - (int)AC_16x16_ICON) + baseIconSize;

    if (AC_NUMBER_OF_ICON_SIZES <= scaledSize)
    {
        scaledSize = (int)AC_NUMBER_OF_ICON_SIZES - 1;
    }

    return (acIconSize)scaledSize;
}

// ---------------------------------------------------------------------------
// Name:        acGetIcon
// Description: Gets the icon image for the required size and id combination.
//              If the image does not yet exist, the function scales it from
//              one that does.
// Author:      Uri Shomroni
// Date:        22/9/2014
// ---------------------------------------------------------------------------
const QImage& acGetIcon(acIconId iconId, acIconSize iconSize)
{
    bool validIndex = (iconId < AC_NUMBER_OF_ICON_IDS) && (iconSize < AC_NUMBER_OF_ICON_SIZES);
    GT_IF_WITH_ASSERT(validIndex)
    {
        // Load the icons if this is the first time this function is called:
        acInitializeIconArray();

        // If the icon we want does not exist:
        QImage& targetArraySlot = ac_stat_iconsArray[iconId][iconSize];

        if (targetArraySlot.isNull())
        {
            // Scale it from another version of the same icon:
            acIconSize bestFitSize = acFindIconSizeMatchForId(iconId, iconSize);
            GT_IF_WITH_ASSERT(bestFitSize < AC_NUMBER_OF_ICON_SIZES)
            {
                // Scale it to the target size and cache it in our array:
                int targetSize = acIconSizeToPixelSize(iconSize);
                targetArraySlot = ac_stat_iconsArray[iconId][bestFitSize].scaled(targetSize, targetSize);

#ifdef AC_TEST_ICON_MARK_SCALED_ICONS
                // Since this is for testing only, avoid doing it in release builds even if the flag was accidentally left on:
#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
                // Mark the icon with a color dot to signify it was scaled:
                // 16x16 = red, 24x24 = green, 32x32 = blue, 48x48 = yellow, 64x64 = magenta, error value = cyan
#define AC_TEST_ICONSIZE_TO_COLOR_UINT(s) ((AC_16x16_ICON == s) ? 0xFFFF0000 : \
                                           (AC_24x24_ICON == s) ? 0xFF00FF00 : \
                                           (AC_32x32_ICON == s) ? 0xFF0000FF : \
                                           (AC_48x48_ICON == s) ? 0xFFFFFF00 : \
                                           (AC_64x64_ICON == s) ? 0xFFFF00FF : 0xFF00FFFF)
                unsigned int fromColor = AC_TEST_ICONSIZE_TO_COLOR_UINT(bestFitSize);
                unsigned int toColor = AC_TEST_ICONSIZE_TO_COLOR_UINT(iconSize);
                targetArraySlot.setPixel(1, 1, fromColor);
                targetArraySlot.setPixel(2, 1, fromColor);
                targetArraySlot.setPixel(1, 2, fromColor);
                targetArraySlot.setPixel(2, 2, fromColor);
                targetArraySlot.setPixel(3, 1, toColor);
                targetArraySlot.setPixel(4, 1, toColor);
                targetArraySlot.setPixel(3, 2, toColor);
                targetArraySlot.setPixel(4, 2, toColor);
#endif
#endif
            }
        }
    }

    static const QImage nullImage;
    return validIndex ? ac_stat_iconsArray[iconId][iconSize] : nullImage;
}

// ---------------------------------------------------------------------------
// Name:        acSetIconInPixmap
// Description: Convenience function, gets the QImage and converts into the QPixmap
// Author:      Uri Shomroni
// Date:        22/9/2014
// ---------------------------------------------------------------------------
bool acSetIconInPixmap(QPixmap& pixmap, acIconId iconId, acIconSize iconSize)
{
    bool retVal = false;
    const QImage& rIconImage = acGetIcon(iconId, iconSize);
    GT_IF_WITH_ASSERT(!rIconImage.isNull())
    {
        retVal = pixmap.convertFromImage(rIconImage);
    }

    return retVal;
}

