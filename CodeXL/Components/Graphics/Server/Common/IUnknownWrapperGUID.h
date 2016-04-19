//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Defines GUID to use in QueryInterface to determine if object is wrapped
//==============================================================================

#ifndef IUNKNOWNWRAPPER_H
#define IUNKNOWNWRAPPER_H

/// Define a GUID for use in QueryInterface to determine if a COM object has been wrapped or not.
/// {7CFA98C7-AD8F-45EA-A377-4AFF36BE0544}
static const IID IID_IWrappedObject = { 0x7cfa98c7, 0xad8f, 0x45ea, { 0xa3, 0x77, 0x4a, 0xff, 0x36, 0xbe, 0x5, 0x44 } };

#endif IUNKNOWNWRAPPER_H