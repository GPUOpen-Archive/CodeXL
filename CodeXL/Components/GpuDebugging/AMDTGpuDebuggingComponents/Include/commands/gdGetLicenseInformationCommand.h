//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdGetLicenseInformationCommand.h
///
//==================================================================================

//------------------------------ gdGetLicenseInformationCommand.h ------------------------------

#ifndef __GDGETLICENSEINFORMATIONCOMMAND
#define __GDGETLICENSEINFORMATIONCOMMAND

// Infra:
#include <GRLicenseManager/lmLicenseStatus.h>
#include <GRLicenseManager/lmLicenseParameters.h>

// Local:
#include <CodeXLAppCode/commands/gdCommand.h>
#include <CodeXLAppCode/gdCodeXLAppDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:           gdGetLicenseInformationCommand : public gdCommand
// General Description:
// Get license information according to the product ID and return a string
// containing all license information
// Author:               Eran Zinman
// Creation Date:        22/4/2007
// ----------------------------------------------------------------------------------
class GD_API gdGetLicenseInformationCommand : public gdCommand
{
public:
    gdGetLicenseInformationCommand(lmProductId productId);
    virtual ~gdGetLicenseInformationCommand();

    // Overrides gdCommand:
    virtual bool canExecuteSpecificCommand();
    virtual bool executeSpecificCommand();

    bool getLicenseString(gtString& licenseString);
    const lmLicenseStatus& licenseStatus() const { return _licenseStatus; };
    const lmLicenseParameters& licenseParameters() const { return _licenseParameters; };

private:
    gtString _licenseInformationString;
    lmProductId _currentProductID;
    lmLicenseStatus _licenseStatus;
    lmLicenseParameters _licenseParameters;
};


#endif  // __GDGETLICENSEINFORMATIONCOMMAND
