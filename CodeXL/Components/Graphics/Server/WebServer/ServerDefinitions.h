//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Server definitions
//==============================================================================

#ifndef __SERVER_DEFINITIONS_H_
#define __SERVER_DEFINITIONS_H_

#ifdef CODEXL_GRAPHICS
    #define PS_CONFIG_FILE              "CodeXLFrameAnalysisServer.cfg" ///< Default value for command line option
    #define PS_DEFAULT_PORT             "8080"                          ///< Default value for command line option
    #define PS_DEFAULT_LOGFILE          "CodeXLFrameAnalysisServer.log" ///< Default value for command line option
    #define PS_DEFAULT_LOGLEVEL         "0"                             ///< Default value for command line option
    #define PS_DEFAULT_WIREFRAMECOLOR   "1"                             ///< Default value for command line option 1 is pink / purple, 2 is green, 3 is blue
    #define PS_DEFAULT_COUNTERFILE      "CodeXLFrameAnalysisCounters.txt" ///< Default value for command line option
#else
    #define PS_CONFIG_FILE              "GPUPerfServer.cfg" ///< Default value for command line options
    #ifdef _LINUX
        #define PS_DEFAULT_PORT             "8080"          ///< Default value for command line options
    #else
        #define PS_DEFAULT_PORT             "80"            ///< Default value for command line options
    #endif
    #define PS_DEFAULT_LOGFILE          "pslog.txt"         ///< Default value for command line options
    #define PS_DEFAULT_LOGLEVEL         "0"                 ///< Default value for command line options
    #define PS_DEFAULT_WIREFRAMECOLOR   "1"                 ///< 1 is pink / purple, 2 is green, 3 is blue
    #define PS_DEFAULT_COUNTERFILE      "pscounters.txt"    ///< Default value for command line options
#endif

#endif // __SERVER_DEFINITIONS_H_