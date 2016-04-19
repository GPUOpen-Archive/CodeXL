//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Hooks the OutputDebugString function and adds the output to the API Trace
//==============================================================================

#ifndef TRACE_ANALYZER_OUTPUTDEBUGSTRING_H
#define TRACE_ANALYZER_OUTPUTDEBUGSTRING_H

class TraceAnalyzer;

//-----------------------------------------------------------
// Attaches to the function
//-----------------------------------------------------------
long Hook_OutputDebugString(TraceAnalyzer* pTraceAnalyzer);

//-----------------------------------------------------------
// Detaches from the function
//-----------------------------------------------------------
long Unhook_OutputDebugString();

#endif //TRACE_ANALYZER_OUTPUTDEBUGSTRING_H