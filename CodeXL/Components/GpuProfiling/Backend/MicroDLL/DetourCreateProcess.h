//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file contains the functions to detour CreateProcess.
//==============================================================================

#ifndef _DETOURCREATEPROCESS_H_
#define _DETOURCREATEPROCESS_H_

/// \addtogroup MicroDLL
// @{

/// Start detouring CreateProcess calls
/// \return true if successful, false otherwise
bool DetoursAttachCreateProcess();

/// Stop detouring CreateProcess calls
/// \return true if successful, false otherwise
bool DetoursDetachCreateProcess();

// @}

#endif // _DETOURCREATEPROCESS_H_
