#ifndef _WINDRIVERSTACKWALK_CODERANGE_H_
#define _WINDRIVERSTACKWALK_CODERANGE_H_
#pragma once

/// \struct CSS_CodeRange Holds info of the code (executable) section from
/// modules in a target process for call-stack unwinding
typedef struct _CSS_CodeRange
{
    /// The start address of the code (executable) section
    UINT64 startAddr;
    /// The size of the code
    ULONG codeSize;
} CSS_CodeRange, *PCSS_CodeRange;

#endif // _WINDRIVERSTACKWALK_CODERANGE_H_
