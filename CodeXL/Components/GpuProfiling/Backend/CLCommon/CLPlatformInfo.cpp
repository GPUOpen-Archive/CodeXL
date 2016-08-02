//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file defines the structure used to store platform info.
//==============================================================================

#include "CLPlatformInfo.h"

namespace CLPlatformInfo
{
bool CLPlatformInfoCompare::operator()(const CLPlatformInfo::platform_info pi1, const CLPlatformInfo::platform_info pi2) const
{
    if (pi1.strDeviceName < pi2.strDeviceName)
    {
        return true;
    }
    else
    {
        if (pi1.strDeviceName > pi2.strDeviceName)
        {
            return false;
        }
        else
        {
            if (pi1.strPlatformName < pi2.strPlatformName)
            {
                return true;
            }
            else
            {
                if (pi1.strPlatformName > pi2.strPlatformName)
                {
                    return false;
                }
                else  //case that pi1.strPlatformName == pi2.strPlatformName
                {
                    if (pi1.strDriverVersion < pi2.strDriverVersion)
                    {
                        return true;
                    }
                    else //driver ver pi1 >= driver ver. pi2
                    {
                        if (pi1.strDriverVersion > pi2.strDriverVersion)
                        {
                            return false;
                        }
                        else //pi1.strDriverVersion == pi2.strDriverVersion
                        {
                            if (pi1.uiNbrAddressBits < pi2.uiNbrAddressBits)
                            {
                                return true;
                            }
                            else
                            {
                                if (pi1.uiNbrAddressBits > pi2.uiNbrAddressBits)
                                {
                                    return false;
                                }
                                else
                                {
                                    if (pi1.strCLRuntime < pi2.strCLRuntime)
                                    {
                                        return true;
                                    }
                                    else
                                    {
                                        if (pi1.strCLRuntime > pi2.strCLRuntime)
                                        {
                                            return false;
                                        }
                                        else
                                        {
                                            if (pi1.strBoardName < pi2.strBoardName)
                                            {
                                                return true;
                                            }
                                            else
                                            {
                                                return false;
                                            }
                                        }
                                    } // end if - pi1.strCLRuntime
                                }
                            } //end if - pi1.uiNbrAddressBits
                        }
                    }//end if - pi1.strDriverVersion
                }
            }// end if - pi1.strPlatformName
        }
    }// end if - pi1.strDeviceName
}

}
