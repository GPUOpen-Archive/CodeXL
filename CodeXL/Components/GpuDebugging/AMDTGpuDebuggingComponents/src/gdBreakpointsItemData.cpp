//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdBreakpointsItemData.cpp
///
//==================================================================================

//------------------------------ gdBreakpointsItemData.cpp ------------------------------

#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/apKernelFunctionNameBreakpoint.h>
#include <AMDTAPIClasses/Include/apKernelSourceCodeBreakpoint.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionBreakPoint.h>
#include <AMDTAPIClasses/Include/apSourceCodeBreakpoint.h>
#include <AMDTAPIClasses/Include/apHostSourceBreakPoint.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdBreakpointsItemData.h>


// ---------------------------------------------------------------------------
// Name:        gdBreakpointsDialog::gdBreakpointsListItemData::gdBreakpointsListItemData
// Description: Constructor.
// Author:      Yaki Tebeka
// Date:        20/11/2007
// ---------------------------------------------------------------------------
gdBreakpointsItemData::gdBreakpointsItemData()
    : _breakpointType(OS_TOBJ_ID_MONITORED_FUNC_BREAKPOINT), _monitoredFunctionId(apMonitoredFunctionsAmount),
      _genericBreakpointType(AP_BREAK_TYPE_UNKNOWN), _clProgramHandle(OA_CL_NULL_HANDLE),
      _sourceCodeFilePath(L""), _sourceCodeLine(-1), _hitCount(0), _isEnabled(true)
{
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsItemData::gdBreakpointsItemData
// Description: Initialize from breakpoint object
// Arguments:   const gtAutoPtr<apBreakPoint>& aptrBreakpoint
// Author:      Sigal Algranaty
// Date:        7/9/2011
// ---------------------------------------------------------------------------
gdBreakpointsItemData::gdBreakpointsItemData(const gtAutoPtr<apBreakPoint>& aptrBreakpoint)
    : _breakpointType(OS_TOBJ_ID_MONITORED_FUNC_BREAKPOINT), _monitoredFunctionId(apMonitoredFunctionsAmount),
      _genericBreakpointType(AP_BREAK_TYPE_UNKNOWN), _clProgramHandle(OA_CL_NULL_HANDLE),
      _sourceCodeFilePath(L""), _sourceCodeLine(-1), _hitCount(0), _isEnabled(true)
{
    // Get the breakpoint type:
    _breakpointType = aptrBreakpoint->type();

    // Set the enabled flag:
    _isEnabled = aptrBreakpoint->isEnabled();

    // Get the string by breakpoint type:
    switch (_breakpointType)
    {
        case OS_TOBJ_ID_MONITORED_FUNC_BREAKPOINT:
        {
            // Down cast it to apMonitoredFunctionBreakPoint:
            apMonitoredFunctionBreakPoint* pFunctionBreakpoint = (apMonitoredFunctionBreakPoint*)(aptrBreakpoint.pointedObject());
            GT_IF_WITH_ASSERT(pFunctionBreakpoint != NULL)
            {
                _monitoredFunctionId = pFunctionBreakpoint->monitoredFunctionId();
            }
        }
        break;

        case OS_TOBJ_ID_GENERIC_BREAKPOINT:
        {
            // Down cast it to apMonitoredFunctionBreakPoint:
            apGenericBreakpoint* pGenericBreakpoint = (apGenericBreakpoint*)(aptrBreakpoint.pointedObject());
            GT_IF_WITH_ASSERT(pGenericBreakpoint != NULL)
            {
                // Convert the integer into a breakpoint type enumeration:
                _genericBreakpointType = pGenericBreakpoint->breakpointType();
            }
        }
        break;

        case OS_TOBJ_ID_KERNEL_SOURCE_CODE_BREAKPOINT:
        {
            // Down cast it to apKernelSourceCodeBreakpoint:
            apKernelSourceCodeBreakpoint* pKernelBreakpoint = (apKernelSourceCodeBreakpoint*)(aptrBreakpoint.pointedObject());
            GT_IF_WITH_ASSERT(pKernelBreakpoint != NULL)
            {
                if (pKernelBreakpoint->isUnresolved())
                {
                    _sourceCodeLine = pKernelBreakpoint->lineNumber();
                    _sourceCodeFilePath = pKernelBreakpoint->unresolvedPath();
                }
                else
                {
                    gaCodeLocationFromKernelSourceBreakpoint(*pKernelBreakpoint, _sourceCodeFilePath, _sourceCodeLine);

                    // Set the program handle:
                    _clProgramHandle = pKernelBreakpoint->programHandle();
                }
            }
        }
        break;

        case OS_TOBJ_ID_SOURCE_CODE_BREAKPOINT:
        {
            // Down cast it to apSourceCodeBreakpoint:
            apSourceCodeBreakpoint* pSourceCodeBreakpoint = (apSourceCodeBreakpoint*)(aptrBreakpoint.pointedObject());
            GT_IF_WITH_ASSERT(pSourceCodeBreakpoint != NULL)
            {
                // Set the item data file path and line number:
                _sourceCodeFilePath = pSourceCodeBreakpoint->filePath();
                _sourceCodeLine = pSourceCodeBreakpoint->lineNumber();
            }
        }
        break;

        case OS_TOBJ_ID_KERNEL_FUNC_NAME_BREAKPOINT:
        {
            // Down cast it to apKernelFunctionNameBreakpoint:
            apKernelFunctionNameBreakpoint* pFunctionNameBreakpoint = (apKernelFunctionNameBreakpoint*)(aptrBreakpoint.pointedObject());
            GT_IF_WITH_ASSERT(nullptr != pFunctionNameBreakpoint)
            {
                // Get the kernel function's name:
                _kernelFunctionName = pFunctionNameBreakpoint->kernelFunctionName();
            }
        }
        break;

        case OS_TOBJ_ID_HOST_SOURCE_CODE_BREAKPOINT:
        {
            // Down cast it to apSourceCodeBreakpoint:
            apHostSourceCodeBreakpoint* pSourceCodeBreakpoint = (apHostSourceCodeBreakpoint*)(aptrBreakpoint.pointedObject());
            GT_IF_WITH_ASSERT(nullptr != pSourceCodeBreakpoint)
            {
                // Set the item data file path and line number:
                _sourceCodeFilePath = pSourceCodeBreakpoint->filePath();
                _sourceCodeLine = pSourceCodeBreakpoint->lineNumber();
            }
        }
        break;

        default:
        {
            GT_ASSERT(false);
            break;
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdBreakpointsItemData::breakpointAPIIndex
// Description: Find the index of the breakpoints represented by this item data
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        22/9/2011
// ---------------------------------------------------------------------------
int gdBreakpointsItemData::breakpointAPIIndex()
{
    int retVal = -1;

    // Get the amount of API breakpoints:
    int breakpointsAmount = -1;
    bool rc = gaGetAmountOfBreakpoints(breakpointsAmount);
    GT_IF_WITH_ASSERT(rc)
    {
        // Go through the API breakpoints and look for the breakpoint represented in this item data:
        for (int i = 0 ; i < breakpointsAmount; i++)
        {
            // Get the current breakpoint:
            gtAutoPtr<apBreakPoint> aptrBreakpoint;
            rc = gaGetBreakpoint(i, aptrBreakpoint);
            GT_IF_WITH_ASSERT(rc)
            {
                if (this->sameAs(aptrBreakpoint))
                {
                    retVal = i;
                    break;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsItemData::sameAs
// Description: Is this breakpoint data represents the same breakpoint as aptrBreakpoint
// Arguments:   const gtAutoPtr<apBreakPoint>& aptrBreakpoint
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        22/9/2011
// ---------------------------------------------------------------------------
bool gdBreakpointsItemData::sameAs(const gtAutoPtr<apBreakPoint>& aptrBreakpoint)
{
    bool retVal = false;

    // Compare the breakpoint type:
    if (aptrBreakpoint->type() == _breakpointType)
    {
        // Compare the breakpoint according to its type:
        // Get the string by breakpoint type:
        switch (_breakpointType)
        {
            case OS_TOBJ_ID_MONITORED_FUNC_BREAKPOINT:
            {
                // Down cast it to apMonitoredFunctionBreakPoint:
                apMonitoredFunctionBreakPoint* pFunctionBreakpoint = (apMonitoredFunctionBreakPoint*)(aptrBreakpoint.pointedObject());
                GT_IF_WITH_ASSERT(pFunctionBreakpoint != NULL)
                {
                    retVal = (_monitoredFunctionId == pFunctionBreakpoint->monitoredFunctionId());
                }
            }
            break;

            case OS_TOBJ_ID_GENERIC_BREAKPOINT:
            {
                // Down cast it to apMonitoredFunctionBreakPoint:
                apGenericBreakpoint* pGenericBreakpoint = (apGenericBreakpoint*)(aptrBreakpoint.pointedObject());
                GT_IF_WITH_ASSERT(pGenericBreakpoint != NULL)
                {
                    // Convert the integer into a breakpoint type enumeration:
                    retVal = (_genericBreakpointType == pGenericBreakpoint->breakpointType());
                }
            }
            break;

            case OS_TOBJ_ID_KERNEL_SOURCE_CODE_BREAKPOINT:
            {
                // Down cast it to apKernelSourceCodeBreakpoint:
                apKernelSourceCodeBreakpoint* pKernelBreakpoint = (apKernelSourceCodeBreakpoint*)(aptrBreakpoint.pointedObject());
                GT_IF_WITH_ASSERT(pKernelBreakpoint != NULL)
                {
                    if (pKernelBreakpoint->isUnresolved())
                    {
                        retVal = ((pKernelBreakpoint->unresolvedPath() == _sourceCodeFilePath) && (pKernelBreakpoint->lineNumber() == _sourceCodeLine));
                    }
                    else
                    {
                        osFilePath filePath;
                        int lineNumber = -1;
                        gaCodeLocationFromKernelSourceBreakpoint(*pKernelBreakpoint, filePath, lineNumber);
                        retVal = ((filePath == _sourceCodeFilePath) && (lineNumber == _sourceCodeLine) && (pKernelBreakpoint->programHandle() == _clProgramHandle));
                    }
                }
            }
            break;

            case OS_TOBJ_ID_SOURCE_CODE_BREAKPOINT:
            {
                // Down cast it to apSourceCodeBreakpoint:
                apSourceCodeBreakpoint* pSourceCodeBreakpoint = (apSourceCodeBreakpoint*)(aptrBreakpoint.pointedObject());
                GT_IF_WITH_ASSERT(nullptr != pSourceCodeBreakpoint)
                {
                    // Set the item data file path and line number:
                    retVal = ((_sourceCodeFilePath == pSourceCodeBreakpoint->filePath()) && (_sourceCodeLine == pSourceCodeBreakpoint->lineNumber()));
                }
            }
            break;

            case OS_TOBJ_ID_HOST_SOURCE_CODE_BREAKPOINT:
            {
                // Down cast it to apSourceCodeBreakpoint:
                apHostSourceCodeBreakpoint* pSourceCodeBreakpoint = (apHostSourceCodeBreakpoint*)(aptrBreakpoint.pointedObject());
                GT_IF_WITH_ASSERT(nullptr != pSourceCodeBreakpoint)
                {
                    // Set the item data file path and line number:
                    retVal = ((_sourceCodeFilePath == pSourceCodeBreakpoint->filePath()) && (_sourceCodeLine == pSourceCodeBreakpoint->lineNumber()));
                }
            }
            break;

            case OS_TOBJ_ID_KERNEL_FUNC_NAME_BREAKPOINT:
            {
                // Down cast it to apKernelFunctionNameBreakpoint:
                apKernelFunctionNameBreakpoint* pFunctionNameBreakpoint = (apKernelFunctionNameBreakpoint*)(aptrBreakpoint.pointedObject());
                GT_IF_WITH_ASSERT(pFunctionNameBreakpoint != NULL)
                {
                    // Get the kernel function's name:
                    retVal = (_kernelFunctionName == pFunctionNameBreakpoint->kernelFunctionName());
                }
            }
            break;

            default:
            {
                GT_ASSERT(false);
                break;
            }
        }
    }

    return retVal;
}
