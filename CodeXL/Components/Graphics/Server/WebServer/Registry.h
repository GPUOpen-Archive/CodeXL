//==============================================================================
// Copyright (c) 2014 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Responsible for Windows registry interactions
//==============================================================================

#ifndef PS_REGISTRY_H
#define PS_REGISTRY_H

// Function prototypes from Registry.cpp

void SetOpenWithRegistryKey(void);
void DeleteOpenWithRegistryKey(void);

void EnableAppInit();
void RestoreAppInit();

#endif // PS_REGISTRY_H
