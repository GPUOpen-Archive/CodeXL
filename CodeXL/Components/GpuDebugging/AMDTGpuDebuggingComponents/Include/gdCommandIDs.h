//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdCommandIDs.h
///
//==================================================================================

//------------------------------ gdCommandIDs.h ------------------------------

#ifndef __GDCOMMANDIDS
#define __GDCOMMANDIDS

// AMDTApplicationFramework:

// ------ This file contains wxWindows IDs for CodeXLApp and CodeXL commands: ------

// Command id's should be defined according to the following ranges:
// gdMainMenuItemCommands - enumeration for the menu item command ids:
//  gdCommandIDs                    (0    - 1999)
//  wxID_LOWEST - wxID_HIGHEST      (4999 - 5999) Currently.
//  acCommandIDs                    (7001 - 7300)
//  ahCommandIDs                    (8001 - 8300)

enum gdMainMenuItemCommands
{
    ID_SAVE_STATE_VARIABLES = 100,  // Instead of 100 we should have used the GPU_DEBUGGER_FIRST_COMMAND_ID enumerated
    // value from <AMDTApplicationFramework/Include/afCommandIds.h>.
    // However, adding an include statement for afCommandIds.h breaks the Visual Studio Command Table
    // (VSCT) compilation, and I could not find where is the proper place to add CommonProjects to the
    // VSCT compiler's list of AdditionalIncludeDirectories.
    // The VSCT compiler reads the \CodeXL\CodeXLVSPackage\CodeXLVSPackageUi\PackageCommands.vsct file
    // which includes this file.

    // Debug menu:
    ID_START_DEBUGGING,
    ID_BREAK_DEBUGGING,
    ID_STOP_DEBUGGING,
    ID_DEBUG_MODE,
    ID_API_STEP_DEBUGGING,
    ID_DRAW_STEP_DEBUGGING,
    ID_FRAME_STEP_DEBUGGING,
    ID_STEP_IN_DEBUGGING,
    ID_STEP_OVER_DEBUGGING,
    ID_STEP_OUT_DEBUGGING,

    //  ID_MARKER_NEXT,
    //  ID_MARKER_PREVIOUS,

    // Breakpoints menu:
    ID_BREAKPOINTS_MENU_BREAK_ON_OPENGL_ERROR,
    ID_BREAKPOINTS_MENU_BREAK_ON_OPENCL_ERROR,
    ID_BREAKPOINTS_MENU_BREAK_ON_DETECTED_ERROR,
#if defined (__APPLE__)
    ID_BREAKPOINTS_MENU_BREAK_ON_SOFTWARE_FALLBACKS,
#endif
    ID_BREAKPOINTS_MENU_BREAK_ON_REDUNDANT_STATE_CHANGES,
    ID_BREAKPOINTS_MENU_BREAK_ON_DEPRECATED_FUNCTIONS,
    ID_BREAKPOINTS_MENU_BREAK_ON_MEMORY_LEAKS,

#if defined(_WIN32)
    ID_BREAKPOINTS_MENU_GL_DEBUG_OUTPUT_BREAK,
    ID_BREAKPOINTS_MENU_GL_DEBUG_OUTPUT_SETTINGS,
#endif

    //  ID_LOAD_BREAKPOINTS,
    //  ID_SAVE_BREAKPOINTS_AS,
    ID_BREAKPOINTS_MENU_ADD_REMOVE_BREAKPOINTS,
    ID_BREAKPOINTS_MENU_ENABLE_DISABLE_ALL_BREAKPOINTS,

    ID_IMAGES_AND_BUFFERS_VIEWER_SAVE_ALL_AS_IMAGE,
    ID_IMAGES_AND_BUFFERS_VIEWER_SAVE_IMAGE_AS,
    ID_IMAGES_AND_BUFFERS_VIEWER_SAVE_RAWDATA_AS,
    ID_IMAGES_AND_BUFFERS_VIEWER_SAVE_ALL_AS_RAWDATA,

    ID_DEBUG_SETTINGS,

    gdAmountOfMainMenuCommands = ID_DEBUG_SETTINGS + 1
};

// "View" menu:
#define ID_TOOLBAR_VIEW_MENU                    140
#define ID_VIEWS_VIEW_MENU                      141
#define ID_VIEWERS_VIEW_MENU                    142
#define ID_PROCESS_EVENTS_VIEW_MENU             143
#define ID_CALLS_HISTORY_VIEW_MENU              144
#define ID_PROPERTIES_VIEW_MENU                 145
#define ID_CALLS_STACK_VIEW_MENU                146
#define ID_SOURCE_CODE_VIEWER_MENU              147
#define ID_STATE_VARIABLES_COMPARE_VIEWER_MENU  148
#define ID_SHADERS_SOURCE_CODE_VIEWER_MENU      149
#define ID_TEXTURES_VIEWER_MENU                 150
#define ID_STATE_VARIABLES_VIEW_MENU            151
#define ID_DEBUG_TOOLBAR_MENU                   153
#define ID_HISTORY_TOOLBAR_MENU                 154
#define ID_RASTER_MODE_TOOLBAR_MENU             155
#define ID_INTERACTIVE_MODE_TOOLBAR_MENU        156
#define ID_THREADS_AND_CONTEXTS_TOOLBAR_MENU    157
#define ID_VIEWERS_TOOLBAR_MENU                 158
#define ID_PERFORMANCE_TOOLBAR_MENU             159
#define ID_PERFORMANCE_GRAPH_VIEW_MENU          161
#define ID_PERFORMANCE_DASHBOARD_VIEW_MENU      162
#define ID_QUEUE_REALTIME_STATISTICS_VIEW_MENU  163
#define ID_EXECUTION_MODE_TOOLBAR_MENU          164
#define ID_STATISTICS_VIEWER_MENU               165
#define ID_MEMORY_ANALYSIS_VIEWER_MENU          169
#define ID_COMMAND_QUEUES_VIEWER_MENU           170
#define ID_OPENCL_PERFORMANCE_TOOLBAR_MENU      171

// "Information" menu:
#define ID_SYSTEM_INFO              196
#define ID_DISPLAY_INFO             197
#define ID_GRAPHIC_CARD_INFO        198
#define ID_PIXEL_FORMATS_INFO       199
#define ID_OPENGL_EXTENSIONS_INFO   200
#define ID_CONTEXT_INFO             201
#define ID_OPENGL_STATE_VAR_INFO    202

// "Break points" menu:
#define ID_BREAK_POINTS_WILL_BE_DEFINE 203

// "Additional directories" dialog
#define ID_ADDITIONAL_DIRECTORIES_DIALOG                            210
#define ID_ADDITIONAL_DIRECTORIES_LIST_FOR_DIRECTORIES              211

// "Tools" menu:
#define ID_TOOLS_OPTIONS_NB_CONTROL                                 217
#define ID_TOOLS_OPTIONS_CALL_STACK_CHECKBOX1                       218
#define ID_TOOLS_OPTIONS_CALL_STACK_CHECKBOX2                       219
#define ID_TOOLS_OPTIONS_CALL_STACK_COLLECT_CHECKBOX                220
#define ID_TOOLS_OPTIONS_CALL_STACK_TEXT1                           221
#define ID_TOOLS_OPTIONS_CALL_STACK_TEXT2                           222
#define ID_TOOLS_OPTIONS_CALL_STACK_ADDITIONAL_DIRECTORIES_BUTTON   223
#define ID_TOOLS_OPTIONS_CALL_STACK_SOURCE_CODE_ROOT_BUTTON         224
#define ID_TOOLS_OPTIONS_BROWSE_FOR_LOG_FILES_DIR                   225
#define ID_TOOLS_OPTIONS_TEXTURES_CHECKBOX                          226
#define ID_TOOLS_OPTIONS_TEXTURES_JPG                               227
#define ID_TOOLS_OPTIONS_TEXTURES_BMP                               228
#define ID_TOOLS_OPTIONS_TEXTURES_PNG                               229
#define ID_TOOLS_OPTIONS_TEXTURES_TIF                               230
#define ID_TOOLS_OPTIONS_ADVANCED_FLUSH_LOG_CHECKBOX                231
#define ID_TOOLS_OPTIONS_ADVANCED_DEBUG_LOG_LEVEL_COMBO             233
#define ID_TOOLS_OPTIONS_ADVANCED_PERFORMANCE_COUNTERS_SAMPLE_SPIN  234
#define ID_TOOLS_OPTIONS_ADVANCED_FLOATING_POINT_PRECISION_SPIN     235
#define ID_TOOLS_OPTIONS_CONNECTION_USING_PROXY_CHECKBOX            236
#define ID_TOOLS_OPTIONS_CONNECTION_PROXY_SERVER                    237
#define ID_TOOLS_OPTIONS_CONNECTION_PROXY_PORT                      238
#define ID_TOOLS_OPTIONS_LOGGING_OPENGL_CALLS_TEXTCTRL              239
#define ID_TOOLS_OPTIONS_LOGGING_OPENCL_CALLS_TEXTCTRL              240
#define ID_TOOLS_OPTIONS_LOGGING_OPENCL_QUEUE_COMMANDS_TEXTCTRL     241


// Generic Context menu items that doesn't appear in the menu bar
#define ID_REMOVE_SELECTED          252
#define ID_ADD_VARIABLES_STATE      253
#define ID_ADD_PERFORMANCE_COUNTER  254
#define ID_SELECT_DIFFERENT         255

// "Debug" toolbar:
#define ID_DEBUG_TOOLBAR            260
#define ID_DEBUG_TOOLBAR_BREAK      261
#define ID_DEBUG_TOOLBAR_ONE_STEP   262
#define ID_DEBUG_TOOLBAR_DRAW_STEP  263
#define ID_DEBUG_TOOLBAR_FRAME_STEP 264
#define ID_DEBUG_TOOLBAR_RUN        265
#define ID_DEBUG_TOOLBAR_STOP       266

// "Threads And Contexts" toolbar:
#define ID_THREADS_AND_CONTEXTS_TOOLBAR_CONTEXT_COMBO   280
#define ID_THREADS_AND_CONTEXTS_TOOLBAR_THREAD_COMBO    281
#define ID_THREADS_AND_CONTEXTS_TOOLBAR_CONTEXT_INFO    282

// "Interactive mode" toolbar:
#define ID_INTERACTIVE_MODE_TOOLBAR                             300
#define ID_INTERACTIVE_MODE_TOOLBAR_COMBO_BOX                   301
#define ID_INTERACTIVE_MODE_TOOLBAR_DELAY_INTERVAL_SPIN_CTRL    302
#define ID_INTERACTIVE_MODE_TOOLBAR_DELAY_INTERVAL_CHECKBOX     303

// "Raster mode" toolbar
#define ID_DISPLAY_TOOLBAR_FILL                     311
#define ID_DISPLAY_TOOLBAR_POINTS                   312
#define ID_DISPLAY_TOOLBAR_WIREFRAME                313

// "Viewers" toolbar
#define ID_VIEWERS_TOOLBAR_SOURCE_CODE              330
#define ID_VIEWERS_TOOLBAR_STATE_VARIABLES_COMPARE  331
#define ID_VIEWERS_TOOLBAR_SHADERS_SOURCE_CODE      332
#define ID_VIEWERS_TOOLBAR_TEXTURES                 333
#define ID_VIEWERS_TOOLBAR_STATISTICS               334
#define ID_VIEWERS_TOOLBAR_MEMORY_ANALYSIS          335
#define ID_VIEWERS_TOOLBAR_COMMAND_QUEUES           336

// "Performance" toolbar
#define ID_PERFORMANCE_TOOLBAR                      339
#define ID_PERFORMANCE_TOOLBAR_SAVE_PROFILING_DATA  340
#define ID_PERFORMANCE_TOOLBAR_NULL_DRIVER          341
#define ID_PERFORMANCE_TOOLBAR_NULL_TEXTURES        342
#define ID_PERFORMANCE_TOOLBAR_NULL_GEOMETRY_SHADERS 343
#define ID_PERFORMANCE_TOOLBAR_NULL_FRAGMENT_SHADERS 344
#define ID_PERFORMANCE_TOOLBAR_NULL_PORT_VIEW       345
#define ID_PERFORMANCE_TOOLBAR_NULL_LIGHTS          346

// "Execution Mode" toolbar
#define ID_EXECUTION_MODE_TOOLBAR_DEBUG             348
#define ID_EXECUTION_MODE_TOOLBAR_PROFILE           349
#define ID_EXECUTION_MODE_TOOLBAR_ANALYZE           350

// State Variables Compare Viewer
#define ID_STATE_VARIABLES_COMPARISON_VIEWER                        379
#define ID_COMPARE_STATE_VAR_LIST                                   380
#define ID_COMPARE_STATE_VAR_ONLINE_COMPARISON                      381
#define ID_COMPARE_STATE_VAR_ONLINE_COMPARISON_SNAPSHOT_FILE        382
#define ID_COMPARE_STATE_VAR_ONLINE_COMPARISON_PREVIOUS_SUSPENSION  383
#define ID_COMPARE_STATE_VAR_ONLINE_COMPARISON_DEFAULT_VALUES       384
#define ID_COMPARE_STATE_VAR_SNAPSHOT_BROWSE1                       385
#define ID_COMPARE_STATE_VAR_SNAPSHOT_TEXT1                         386
#define ID_COMPARE_STATE_VAR_SNAPSHOT_TEXT2                         387
#define ID_COMPARE_STATE_VAR_SNAPSHOT_BROWSE2                       388
#define ID_COMPARE_STATE_VAR_SHOW_ONLY_CHANGES                      389
#define ID_COMPARE_STATE_VAR_DIFFERENCE_NEXT                        390
#define ID_COMPARE_STATE_VAR_DIFFERENCE_PREVIOUS                    391
#define ID_COMPARE_STATE_VAR_FIND                                   392
#define ID_COMPARE_STATE_VAR_SAVE_SNAPSHOT                          393
#define ID_COMPARE_STATE_VAR_CONTEXT_COMBO                          394

// Debug Settings Dialog
#define ID_DEBUG_SETTINGS_LINUX_LOADER_PRELOAD_CHECKBOX             395
#define ID_DEBUG_SETTINGS_LOCAL_COMPUTER_RADIO_BUTTON               400
#define ID_DEBUG_SETTINGS_IPHONE_SIMULATOR_RADIO_BUTTON             401
#define ID_DEBUG_SETTINGS_IPHONE_DEVICE_RADIO_BUTTON                402
#define ID_DEBUG_SETTINGS_GLFLUSH_CHECKBOX                          403
#define ID_DEBUG_SETTINGS_SWAP_BUFFERS_CHECKBOX                     404
#define ID_DEBUG_SETTINGS_GLFINISH_CHECKBOX                         405
#define ID_DEBUG_SETTINGS_SWAP_LAYER_BUFFERS_CHECKBOX               406
#define ID_DEBUG_SETTINGS_MAKE_CURRENT_CHECKBOX                     407
#define ID_DEBUG_SETTINGS_GL_CLEAR_CHECKBOX                         408
#define ID_DEBUG_SETTINGS_GL_FRAME_TERMINATOR_GREMEDY_CHECKBOX      409
#define ID_DEBUG_SETTINGS_CL_GREMEDY_COMPUTATION_FRAME_CHECKBOX     410
#define ID_DEBUG_SETTINGS_CLFLUSH_CHECKBOX                          411
#define ID_DEBUG_SETTINGS_CLFINISH_CHECKBOX                         412
#define ID_DEBUG_SETTINGS_CLWAITFOREVENTS_CHECKBOX                  413
#define ID_DEBUG_SETTINGS_EXPORT_IPHONE_DEVICE_LIBRARIES_BUTTON     414
#define ID_DEBUG_SETTINGS_USE_AUTOMATIC_CONFIGURATION               415
#define ID_DEBUG_SETTINGS_BROWSE_FOR_EXE                            416
#define ID_DEBUG_SETTINGS_BROWSE_FOR_PATH                           417
#define ID_DEBUG_SETTINGS_BROWSE_FOR_KERNEL_SOURCE                  418
#define ID_DEBUG_SETTINGS_KERNEL_SOURCE_PATH                        419
#define ID_DEBUG_SETTINGS_BROWSE_FOR_ENV_VARS                       420
#define ID_DEBUG_SETTINGS_PROGRAM_EXE                               421
#define ID_DEBUG_SETTINGS_PROGRAM_PATH                              422
#define ID_DEBUG_SETTINGS_PROGRAM_ARGS                              423
#define ID_DEBUG_SETTINGS_PROGRAM_ENV_VARS                          424
#define ID_DEBUG_SETTINGS_CALLS_LOG_FILE                            425
#define ID_DEBUG_SETTINGS_IPHONE_DEFAULT_SDKS                       426
#define ID_DEBUG_SETTINGS_IPHONE_SDK_PATH                           427
#define ID_DEBUG_SETTINGS_BROWSE_FOR_IPHONE_SDK                     428
#define ID_DEBUG_SETTINGS_INITIALIZE_DIRECTDRAW_LIBRARY             429

// State Variables Dialog
#define ID_STATE_VAR_LIST           440
#define ID_CHOSEN_STATE_VAR_LIST    441
#define ID_STATE_VAR_ADD            442
#define ID_STATE_VAR_REMOVE         443
#define ID_STATE_VAR_REMOVE_ALL     444
#define ID_STATE_FILTER_TEXT_CTRL   445

// Breakpoints Dialog
#define ID_BREAKPOINTS_LIST                             447
#define ID_CHOSEN_BREAKPOINTS_LIST                      448
#define ID_BREAKPOINTS_ADD                              449
#define ID_BREAKPOINTS_REMOVE                           450
#define ID_BREAKPOINTS_REMOVE_ALL                       451
#define ID_BREAKPOINTS_ENABLE_DISABLE_ALL               452
#define ID_BREAKPOINTS_BREAK_ON_OPENGL_ERROR            453
#define ID_BREAKPOINTS_BREAK_ON_OPENCL_ERROR            454
#define ID_BREAKPOINTS_BREAK_ON_DETECTED_ERROR          455
#define ID_BREAKPOINTS_BREAK_ON_SOFTWARE_FALLBACKS      456
#define ID_BREAKPOINTS_BREAK_ON_REDUNDANT_STATE_CHANGES 457
#define ID_BREAKPOINTS_BREAK_ON_DEPRECATED_FUNCTIONS    458
#define ID_BREAKPOINTS_BREAK_ON_MEMORY_LEAKS            459
#define ID_BREAKPOINTS_FILTER_TEXT_CTRL                 460
#define ID_BREAKPOINTS_NB_CONTROL                       461
#define ID_BREAKPOINTS_LISTS_NB_CONTROL                 462
#define ID_BREAKPOINTS_KERNEL_LIST                      463
#define ID_BREAKPOINTS_GENERIC_LIST                     464
#define ID_BREAKPOINTS_KERNEL_FILTER_TEXT_CTRL          465


#define ID_BREAKPOINTS_SET_FUNCTION_AS_BREAKPOINT           466
#define ID_CALLS_HISTORY_LIST_FND_DIALOG                    467
#define ID_DEBUG_PROCESS_EVENTS_FND_DIALOG                  468
#define ID_CALL_STACK_FND_DIALOG                            469
#define ID_STATE_VAR_FND_DIALOG                             470
#define ID_STATE_VAR_COMPARE_FND_DIALOG                     471
#define ID_FUNCTION_CALLS_STATISTICS_FND_DIALOG             472
#define ID_FUNCTION_CALLS_STATISTICS_LIST_FND_DIALOG        473
#define ID_TOTAL_STATICTICS_VIEWER_FND_DIALOG               474
#define ID_TOTAL_STATICTICS_VIEWER_LIST_FND_DIALOG          475
#define ID_STATE_CHANGE_VIEWER_FND_DIALOG                   476
#define ID_STATE_CHANGE_VIEWER_LIST_FND_DIALOG              477
#define ID_BREAKPOINTS_SET_MULTIPLE_FUNCTIONS_AS_BREAKPOINT 478
#define ID_STATISTICS_VIEWER_EXPORT_FUNCTION_CALLS_STATISTICS 479

// "Debugged process events" View
#define ID_DEBUG_PROCESS_EVENTS_LIST                480
#define ID_DEBUG_PROCESS_EVENTS_SCROLL_DOWN_TIMER   481

// Startup Dialog
#define ID_STARTUP_DIALOG           482
#define ID_STARTUP_LISTCTRL         483

// Survey Dialog
#define ID_SURVEY_DIALOG_REMIND_CHECKBOX    491
#define ID_SURVEY_DIALOG_YES_BUTTON         492
#define ID_SURVEY_DIALOG_NO_BUTTON          493

// Eula Dialog
#define ID_EULA_DIALOG                      495
#define ID_EULA_DIALOG_EXIT_BUTTON          496
#define ID_EULA_DIALOG_NEXT_BUTTON          497
#define ID_EULA_DIALOG_AGREE_RADIO          498
#define ID_EULA_DIALOG_DONOT_AGREE_RADIO    499

// "Properties" View
#define ID_PROPERTIES_VIEW  500

// "Dashboard Graph" View
#define ID_PERFORMANCE_DASHBOARD_VIEW   501

// "Performance Graph" View
#define ID_PERFORMANCE_GRAPH_VIEW 505
#define ID_PERFORMANCE_GRAPH_VIEW_LEFT_CANVAS       506
#define ID_PERFORMANCE_GRAPH_VIEW_RIGHT_LIST_CTRL   507

// "Call stack" View
#define ID_CALL_STACK_LIST_CTRL 510

// Command Queues Realtime View
#define ID_COMMAND_QUEUES_REALTIME_STATISTICS_VIEW  512

// "New Project" wizard
#define ID_NEW_PROJECT_WIZARD                                           520
#define ID_NEW_PROJECT_WIZARD_BROWSE_FOR_EXE                            521
#define ID_NEW_PROJECT_WIZARD_BROWSE_FOR_PATH                           522
#define ID_NEW_PROJECT_WIZARD_PROGRAM_EXE                               523
#define ID_NEW_PROJECT_WIZARD_PROGRAM_PATH                              524
#define ID_NEW_PROJECT_WIZARD_PROGRAM_ARGS                              525
#define ID_NEW_PROJECT_WIZARD_GL_FINISH                                 526
#define ID_NEW_PROJECT_WIZARD_GL_FLUSH                                  527
#define ID_NEW_PROJECT_WIZARD_SWAP_BUFFERS                              528
#define ID_NEW_PROJECT_WIZARD_SWAP_LAYER_BUFFERS                        529
#define ID_NEW_PROJECT_WIZARD_GL_CLEAR                                  530
#define ID_NEW_PROJECT_WIZARD_MAKE_CURRENT                              531
#define ID_NEW_PROJECT_WIZARD_GL_FRAME_TERMINATOR_GREMEDY               532
#define ID_NEW_PROJECT_WIZARD_CL_FLUSH                                  533
#define ID_NEW_PROJECT_WIZARD_CL_FINISH                                 534
#define ID_NEW_PROJECT_WIZARD_CL_GREMEDY_COMPUTATION_FRAME              535
#define ID_NEW_PROJECT_WIZARD_CL_WAIT_FOR_EVENTS                        536
#define ID_NEW_PROJECT_WIZARD_SELECT_BREAKPOINTS_BUTTON                 537
#define ID_NEW_PROJECT_WIZARD_OPENGL_PROJECT_RADIO_BUTTON               538
#define ID_NEW_PROJECT_WIZARD_OPENGL_ES_PROJECT_RADIO_BUTTON            539
#define ID_NEW_PROJECT_WIZARD_OPENCL_PROJECT_RADIO_BUTTON               540
#define ID_NEW_PROJECT_WIZARD_OPENGL_AND_OPENCL_PROJECT_RADIO_BUTTON    541
#define ID_NEW_PROJECT_WIZARD_IPHONE_SIMULATOR_TARGET_RADIO_BUTTON      542
#define ID_NEW_PROJECT_WIZARD_IPHONE_DEVICE_TARGET_RADIO_BUTTON         543


// "Shaders source code" Viewer
#define ID_SHADERS_SOURCE_CODE_VIEWER                       632
#define ID_SHADERS_SOURCE_CODE_VIEWER_COMBO                 633
#define ID_SHADERS_SOURCE_CODE_VIEWER_OBJECTS_TREE          634
#define ID_SHADERS_SOURCE_CODE_PROPERTIES_HTML_WINDOW       635
#define ID_SHADERS_SOURCE_CODE_VIEWER_HTML_WINDOW           636
#define ID_SHADERS_SOURCE_CODE_VIEWER_SPLITTER_WINDOW       637
#define ID_SHADERS_SOURCE_CODE_VIEWER_LEFT_SPLITTER_WINDOW  638
#define ID_SHADERS_SOURCE_CODE_WINDOW                       639
#define ID_SHADERS_SOURCE_CODE_BUILD_PROGRAM_MENU           640
#define ID_SHADERS_SOURCE_CODE_COMPILE_SHADER_MENU          641
#define ID_SHADERS_SOURCE_CODE_VALIDATE_PROGRAM_MENU        642
#define ID_SHADERS_SOURCE_CODE_SHOW_BUILD_LOG_MENU          643
#define ID_SHADERS_SOURCE_CODE_BUILD_PROGRAM_TOOLBAR        644
#define ID_SHADERS_SOURCE_CODE_COMPILE_SHADER_TOOLBAR       645
#define ID_SHADERS_SOURCE_CODE_VALIDATE_PROGRAM_TOOLBAR     646
#define ID_SHADERS_SOURCE_CODE_SHOW_BUILD_LOG_TOOLBAR       647
#define ID_SHADERS_SOURCE_CODE_VIEW_SHOW_HIDE_TOOLBAR       648
#define ID_SHADERS_SOURCE_CODE_VIEW_SHOW_HIDE_PROGRAMS_LIST 649
#define ID_SHADERS_SOURCE_CODE_VIEW_SHOW_HIDE_PROPERTIES    650
#define ID_SHADERS_SOURCE_CODE_VIEW_SCINTILLA_EDITOR        651

// "Shaders source code build log" dialog
#define ID_SHADERS_SOURCE_CODE_BUILD_LOG_HTML_CTRL          653
#define ID_SHADERS_SOURCE_CODE_BUILD_LOG_CLOSE_BUTTON       654

// "Source code" Viewer:
#define ID_SOURCE_CODE_VIEWER                               657

// Timers:
#define ID_PERFORMANCE_COUNTER_TIMER                        660
#define ID_CLIENT_HEARTBEAT_TIMER                           661

// gdGUILayoutsDialog:
#define ID_GUI_LAYOUTS_DIALOG           670
#define ID_GUI_LAYOUTS_LISTCTRL         671
#define ID_GUI_LAYOUTS_ADD_NEW_TEXTCTRL 672
#define ID_GUI_LAYOUTS_APPLY_BUTTON     673
#define ID_GUI_LAYOUTS_RENAME_BUTTON    674
#define ID_GUI_LAYOUTS_DELETE_BUTTON    675
#define ID_GUI_LAYOUTS_ADD_NEW_BUTTON   676


#define ID_EVALUATION_DIALOG                    710
#define ID_EVALUATION_DIALOG_ACCEPT_BUTTON      711
#define ID_EVALUATION_DIALOG_DECLINE_BUTTON     712
#define ID_EVALUATION_DIALOG_PURCHASE_BUTTON    713
#define ID_EVALUATION_DIALOG_ACADEMIC_BUTTON    714
#define ID_EVALUATION_DIALOG_HTML_WINDOW        715

#define ID_EXPIRED_MAINTENANCE_REMINDER_DIALOG_REMIND_ME_RADIO_BUTTON           720
#define ID_EXPIRED_MAINTENANCE_REMINDER_DIALOG_REMIND_ME_ONE_WEEK_RADIO_BUTTON  721
#define ID_EXPIRED_MAINTENANCE_REMINDER_DIALOG_DONT_REMIND_ME_RADIO_BUTTON      722

#define ID_EXPIRED_MAINTENANCE_DIALOG_PURCHASE_BUTTON 730 // Also used in expired maintenance REMINDER dlg.

// Edit Environment Variables Dialog
#define ID_EDIT_ENVIRONMENT_VARS_DIALOG         740
#define ID_EDIT_ENVIRONMENT_VARS_DIALOG_GRID    741

// Performance Counter dialog
#define ID_PERFORMANCE_DIALOG_COUNTERS_DIALOG   800
#define ID_AVAILABLE_COUNTERS_TREE_CTRL         801
#define ID_CHOSEN_COUNTERS_LIST                 802
#define ID_COUNTERS_ADD                         803
#define ID_COUNTERS_REMOVE                      804
#define ID_COUNTERS_REMOVE_ALL                  805
#define ID_COUNTERS_SCALE_COMBO                 806
#define ID_COUNTERS_DISPLAYED_GRAPH_COMBO       807
#define ID_COUNTERS_COLOR_COMBO                 808
#define ID_COUNTERS_WIDTH_COMBO                 809
#define ID_COUNTERS_DESCRIPTION_TEXT            810
#define ID_COUNTERS_ADD_OTHER_OS                811
#define ID_COUNTERS_DELETE_SELECTED             812
#define ID_COUNTERS_ADD_CONTEXT                 813
#define ID_COUNTERS_ADD_QUEUE                   814

// Add Render Context dialog
#define ID_ADD_CONTEXT_OK                       830
#define ID_ADD_CONTEXT_CANCEL                   831
#define ID_CONTEXT_CONTEXT_ID_CHANGED           832
#define ID_QUEUE_CONTEXT_ID_CHANGED             833

// Check for updates Dialog
#define ID_CHECK_FOR_UPDATES_DIALOG             850
#define ID_CHECK_FOR_UPDATES_DOWNLOAD_BUTTON    851
#define ID_CHECK_FOR_UPDATES_EXIT_BUTTON        852
#define ID_CHECK_FOR_UPDATES_HTML_WINDOW        853
#define ID_CHECK_FOR_UPDATES_AUTOCHECK_CHECKBOX 854

// Request Free License dialog:
#define ID_REQUEST_FREE_LICENSE_DIALOG              905
#define ID_REQUEST_FREE_LICENSE_NAME_FIELD          906
#define ID_REQUEST_FREE_LICENSE_COMPANY_FIELD       907
#define ID_REQUEST_FREE_LICENSE_EMAIL_FIELD         908
#define ID_REQUEST_FREE_LICENSE_TEL_NUM_FIELD       909
#define ID_REQUEST_FREE_LICENSE_DISPLAY_DAYA        910
#define ID_REQUEST_FREE_LICENSE_ADVANCED_BUTTON     911
#define ID_REQUEST_FREE_LICENSE_SEND_BUTTON         912
#define ID_REQUEST_FREE_LICENSE_CANCEL_BUTTON       913
#define ID_REQUEST_FREE_LICENSE_CLICK_HERE          914
#define ID_REQUEST_FREE_LICENSE_LICENSE_BY_EMAIL    915
#define ID_REQUEST_FREE_LICENSE_USING_PROXY         916
#define ID_REQUEST_FREE_LICENSE_PRIVACY_POLICY      917
#define ID_REQUEST_FREE_LICENSE_SEND_ME_INFORMATION_CHECKBOX            918
#define ID_REQUEST_FREE_LICENSE_SEND_ME_PARTNER_INFORMATION_CHECKBOX    919

// Statistics viewer:
#define ID_STATISTICS_VIEWER                                    1000
#define ID_STATISTICS_VIEW                                      1001
#define ID_STATISTICS_VIEWER_CLOSE                              1002
#define ID_STATISTICS_VIEWER_NOTEBOOK                           1008
#define ID_STATISTICS_VIEWER_EXPORT_FUNCTION_CALL_STATISTICS    1010
#define ID_STATISTICS_VIEWER_EXPORT_TOTAL_STATISTICS            1011
#define ID_STATISTICS_VIEWER_EXPORT_BATCH_STATISTICS            1012
#define ID_STATISTICS_VIEWER_CLEAR_STATISTICS                   1013
#define ID_STATISTICS_VIEWER_QUIT                               1014
#define ID_STATISTICS_VIEWER_VIEW_SHOW_GRAPH                    1015
#define ID_STATISTICS_VIEWER_VIEW_SHOW_PROPERTIES               1016
#define ID_STATISTICS_VIEWER_VIEW_SHOW_CALLS_HISTORY            1017
#define ID_STATISTICS_VIEWER_VIEW_SHOW_CALLS_STATISTICS         1018
#define ID_STATISTICS_VIEWER_VIEW_SHOW_DEPRECATION_STATISTICS   1019
#define ID_STATISTICS_VIEWER_VIEW_SHOW_TOTAL_STATISTICS         1020
#define ID_STATISTICS_VIEWER_VIEW_SHOW_BATCH_STATISTICS         1021
#define ID_STATISTICS_VIEWER_DETAILED_BATCH_DATA_BUTTON         1022
#define ID_STATISTICS_VIEWER_EXPORT_DEPRECATION_STATISTICS      1023
#define ID_STATISTICS_VIEWER_VIEW_SHOW_DETAILED_BATCH_STATISTICS 1024
#define ID_STATISTICS_VIEWER_COMBO                              1025
#define ID_STATISTICS_VIEWER_BOTTOM_SPLIT_WINDOW                1026


// General statistics viewer:
#define ID_SAVE_TOTAL_STATISTICS                            1052
#define ID_SAVE_STATE_CHANGE_STATISTICS                     1053
#define ID_SAVE_DEPRECATION_STATISTICS                      1055
#define ID_SAVE_BATCH_STATISTICS                            1057
#define ID_SAVE_STATISTICS                                  1058


// Reset GUI layouts dialog:
#define ID_RESET_LAYOUTS_DIALOG                     1065
#define ID_RESET_LAYOUTS_DIALOG_RESET_BUTTON        1066
#define ID_RESET_LAYOUTS_DIALOG_DEBUGGER_CHECKBOX   1067
#define ID_RESET_LAYOUTS_DIALOG_PROFILER_CHECKBOX   1068
#define ID_RESET_LAYOUTS_DIALOG_ANALYZER_CHECKBOX   1069

// Memory analysis viewer:
#define ID_MEMORY_ANALYSIS_VIEWER                               1080
#define ID_MEMORY_VIEWER_CLOSE                                  1081
#define ID_MEMORY_VIEWER_VIEW_SHOW_GRAPH                        1082
#define ID_MEMORY_VIEWER_VIEW_SHOW_PROPERTIES                   1083
#define ID_MEMORY_VIEWER_VIEW_SHOW_CREATION_STACK               1084
#define ID_MEMORY_VIEWER_QUIT                                   1085
#define ID_MEMORY_VIEWER_LIST                                   1086
#define ID_MEMORY_VIEWER_CREATION_STACKS_LIST                   1087
#define ID_MEMORY_VIEWER_EXPORT_DATA                            1088
#define ID_MEMORY_VIEWER_BREAK_CHECKBOX                         1089
#define ID_MEMORY_VIEWER_VIEW_APPLICATION_TREE                  1090


// Render contexts information dialog
#define ID_CONTEXT_INFO_DIALOG                          1105
#define ID_CONTEXT_INFO_DIALOG_CONTEXTS_COMBO           1106
#define ID_CONTEXT_INFO_VIEW                            1107
#define ID_CONTEXT_INFO_DIALOG_CLOSE_BUTTON             1108
#define ID_CONTEXT_INFO_DIALOG_SAVE_BUTTON              1109

// Select iPhone application dialog:
#define ID_SELECT_IPHONE_APP_DIALOG                     1120
#define ID_SELECT_IPHONE_APP_LIST_CTRL                  1121
#define ID_SELECT_IPHONE_APP_MANUAL_BUTTON              1122

// OpenCL Command Queues viewer:
#define ID_COMMAND_QUEUES_VIEWER                            1140
#define ID_COMMAND_QUEUES_VIEWER_QUEUES_TREE_CTRL           1141
#define ID_COMMAND_QUEUES_VIEWER_COMMANDS_LIST_CTRL         1142
#define ID_COMMAND_QUEUES_VIEWER_CALLS_STACK_LIST           1143
#define ID_COMMAND_QUEUES_VIEWER_PROPERTIES_VIEW            1144
#define ID_COMMAND_QUEUES_VIEWER_FILE_QUIT                  1145
#define ID_COMMAND_QUEUES_VIEWER_VIEW_CALLS_STACK_VIEW      1146
#define ID_COMMAND_QUEUES_VIEWER_VIEW_PROPERTIES_VIEW       1147
#define ID_COMMAND_QUEUES_VIEWER_VIEW_GRAPH_VIEW            1148
#define ID_SAVE_COMMAD_QUEUES_DATA                          1149

// Export Device Server Libraries dialog:
#define ID_EXPORT_DEVICE_SERVER_LIBRARIES_DIRECTORY_PATH_TEXT_CTRL  1170
#define ID_EXPORT_DEVICE_SERVER_LIBRARIES_BROWSE_BUTTON             1171
#define ID_EXPORT_DEVICE_SERVER_LIBRARIES_OVERWRITE_EXISTING        1172
#define ID_EXPORT_DEVICE_SERVER_LIBRARIES_CREATE_SUBDIRECTORY       1173

// OpenCL performance toolbar:
#define ID_OPENCL_PERFORMANCE_TOOLBAR                       1190
#define ID_OPENCL_PERFORMANCE_TOOLBAR_KERNELS               1191
#define ID_OPENCL_PERFORMANCE_TOOLBAR_READ                  1192
#define ID_OPENCL_PERFORMANCE_TOOLBAR_WRITE                 1193
#define ID_OPENCL_PERFORMANCE_TOOLBAR_COPY                  1194

// Monitored tree
#define ID_OBJECT_NAVIGATION_TREE                           1220

// "Calls History" View
#define ID_CALLS_HISTORY_LIST                               1223

// MultiWatch views:
#define ID_MULTIWATCH_VIEW1                                 1230
#define ID_MULTIWATCH_VIEW_FIRST                            ID_MULTIWATCH_VIEW1
#define ID_MULTIWATCH_VIEW2                                 1231
#define ID_MULTIWATCH_VIEW3                                 1232

#define ID_MULTIWATCH_DATA_VIEWER1                          1237
#define ID_MULTIWATCH_DATA_VIEWER_FIRST ID_MULTIWATCH_DATA_VIEWER1
#define ID_MULTIWATCH_DATA_VIEWER2                          1238
#define ID_MULTIWATCH_DATA_VIEWER3                          1239

#define ID_MULTIWATCH_IMAGE_VIEWER1                         1240
#define ID_MULTIWATCH_IMAGE_VIEWER_FIRST ID_MULTIWATCH_IMAGE_VIEWER1
#define ID_MULTIWATCH_IMAGE_VIEWER2                         1241
#define ID_MULTIWATCH_IMAGE_VIEWER3                         1242

#define ID_STATE_VARIABLES_VIEW                             1250


#endif  // __GDCOMMANDIDS
