//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdGetLicenseInformationCommand.cpp
///
//==================================================================================

//------------------------------ gdGetLicenseInformationCommand.cpp ------------------------------




// Infra
#include <VersionInfo/VersionInfo.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osProductVersion.h>
#include <GRLicenseManager/lmLicenseManagerFunctions.h>

// Local:
#include <CodeXLAppCode/commands/gdGetLicenseInformationCommand.h>
#include <CodeXLAppCode/gdCodeXLGlobalVariablesManager.h>
#include <CodeXLAppCode/gdStringConstants.h>
#include <CodeXLAppCode/gdCommandIDs.h>

// ---------------------------------------------------------------------------
// Name:        gdGetLicenseInformationCommand::gdGetLicenseInformationCommand
// Description: Constructor.
// Author:      Eran Zinman
// Date:        22/4/2007
// ---------------------------------------------------------------------------
gdGetLicenseInformationCommand::gdGetLicenseInformationCommand(lmProductId productId)
    : _currentProductID(productId), _licenseStatus(LIC_FREE_LICENSE_FAILED)
{
}

// ---------------------------------------------------------------------------
// Name:        gdGetLicenseInformationCommand::~gdGetLicenseInformationCommand
// Description: Destructor.
// Author:      Eran Zinman
// Date:        22/4/2007
// ---------------------------------------------------------------------------
gdGetLicenseInformationCommand::~gdGetLicenseInformationCommand()
{
}

// ---------------------------------------------------------------------------
// Name:        gdGetLicenseInformationCommand::canExecuteSpecificCommand
// Description: Answers the question - can we get license information.
// Author:      Eran Zinman
// Date:        22/4/2007
// ---------------------------------------------------------------------------
bool gdGetLicenseInformationCommand::canExecuteSpecificCommand()
{
    bool retVal = true;

    // Get the gdCodeXLGlobalVariablesManager instance
    gdCodeXLGlobalVariablesManager& theStateManager = gdCodeXLGlobalVariablesManager::instance();

    // Get the license parameters:
    if ((_currentProductID == LM_CodeXL_GL_WIN_PRODUCT) || (_currentProductID == LM_CodeXL_GL_LNX_PRODUCT) ||
        (_currentProductID == LM_CodeXL_GL_MAC_PRODUCT))
    {
        _licenseStatus = theStateManager.CodeXLGLLicenseStatus();
        _licenseParameters = theStateManager.CodeXLGLLicenseParams();
    }
    else if ((_currentProductID == LM_CodeXL_GL_ES_WIN_PRODUCT) || (_currentProductID == LM_CodeXL_IPHONE_PRODUCT))
    {
        _licenseStatus = theStateManager.CodeXLGLESLicenseStatus();
        _licenseParameters = theStateManager.CodeXLGLESLicenseParams();
    }
    else if ((_currentProductID == LM_CodeXL_CL_WIN_PRODUCT) || (_currentProductID == LM_CodeXL_CL_LNX_PRODUCT) ||
             (_currentProductID == LM_CodeXL_CL_MAC_PRODUCT))
    {
        _licenseStatus = theStateManager.CodeXLCLLicenseStatus();
        _licenseParameters = theStateManager.CodeXLCLLicenseParams();
    }
    else
    {
        // We don't recognize the product ID
        retVal = false;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdGetLicenseInformationCommand::executeSpecificCommand
// Description: Get license information and saves it into a string
// Return Val:  bool - Success / failure.
// Author:      Eran Zinman
// Date:        22/4/2007
// ---------------------------------------------------------------------------
bool gdGetLicenseInformationCommand::executeSpecificCommand()
{
    bool retVal = true;

    // Get the product name:
    gtASCIIString productNameString;
    lmProductIdsToProductString(_currentProductID, productNameString);

    // Convert the product name to a unicode string:
    gtString productNameStringUnicode;
    productNameStringUnicode.fromASCIIString(productNameString.asCharArray());

    // If we have a valid license, expired license, expired maintenance
    // or no available floating license for the input products:
    if (LM_LIC_IS_LICENSE_STATUS_VALID(_licenseStatus) || (_licenseStatus == LIC_FREE_LICENSE_EXPIRED) ||
        (_licenseStatus == LIC_VERSION_EXPIRY_FAILED) || (_licenseStatus == LIC_SUPPORTED_UPGRADES_FAILED) ||
        (_licenseStatus == LIC_LICENSE_SERVER_NO_LICENSES_LEFT))
    {
        gtASCIIString licenseeName = _licenseParameters._licenseeName;
        bool isAcademicLicense = _licenseParameters._isAcademicLicense;
        bool isIndieLicense = _licenseParameters._isIndieLicense;
        bool isFreeLicense = _licenseParameters._isFreeLicense;
        osTime licenseExpiryDate = _licenseParameters._licenseExpirationDate;
        osTime supportAndUpgradesDate = _licenseParameters._upgradesExpirationDate;

        // Get the current time:
        osTime currentTime;
        currentTime.setFromCurrentTime();

        // Get the license expiration date string:
        int licenseSecondsLeft = licenseExpiryDate.secondsFrom1970() - currentTime.secondsFrom1970();
        int licenseDaysLeft = (((licenseSecondsLeft / 60) / 60) / 24) + 1;
        gtString licenseExpiryDateString = GD_STR_Empty;
        licenseExpiryDate.dateAsString(licenseExpiryDateString, osTime::WINDOWS_STYLE, osTime::LOCAL);

        if (licenseSecondsLeft < 0)
        {
            licenseExpiryDateString.appendFormattedString(L" (%ls)", GD_STR_HelpAboutLicenseExpired);
        }
        else if (licenseDaysLeft < 61)
        {
            licenseExpiryDateString.appendFormattedString(L" (%d days left)", licenseDaysLeft);
        }

        // Get the support and upgrades expiration date string:
        int upgradesSecondsLeft = supportAndUpgradesDate.secondsFrom1970() - currentTime.secondsFrom1970();
        int upgradesDaysLeft = (((upgradesSecondsLeft / 60) / 60) / 24) + 1;
        gtString supportAndUpgradesDateString;
        supportAndUpgradesDate.dateAsString(supportAndUpgradesDateString, osTime::WINDOWS_STYLE, osTime::LOCAL);

        if (upgradesSecondsLeft < 0)
        {
            supportAndUpgradesDateString.appendFormattedString(L" (%ls)", GD_STR_HelpAboutLicenseExpired);
        }
        else if (upgradesDaysLeft < 61)
        {
            supportAndUpgradesDateString.appendFormattedString(L" (%d days left)", upgradesDaysLeft);
        }

        // Create the license string:
        gtString licenseDetailsString;

        // Add the product name:
        if (isFreeLicense)
        {
            licenseDetailsString = _T(GD_STR_CodeXLProductLineName);
        }
        else
        {
            gtString productNameUnicode;
            productNameUnicode.fromASCIIString(productNameString.asCharArray());
            licenseDetailsString.append(productNameUnicode);
        }

        licenseDetailsString.append(L":\n");

        if (isFreeLicense)
        {
            // Set the Free License details:
            licenseDetailsString.append(L"      ");
            licenseDetailsString.append(GD_STR_HelpAboutFreeLicense);
            licenseDetailsString.append(GD_STR_NewLine);
            licenseDetailsString.append(L"      ");
            licenseDetailsString.append(GD_STR_HelpAboutLicenseExpirationDate);

            //  If the free license is still valid:
            if (currentTime < _licenseParameters._licenseExpirationDate)
            {
                licenseDetailsString.append(licenseExpiryDateString);
            }
            else
            {
                licenseDetailsString.append(GD_STR_HelpAboutLicenseExpired);
            }
        }
        else
        {
            // Set the Licensee details:
            gtString licenseeNameUnicode;
            licenseeNameUnicode.fromASCIIString(licenseeName.asCharArray());

            // Add the Licensee name:
            licenseDetailsString.append(L"      ");
            licenseDetailsString.append(GD_STR_HelpAboutLicenseeName);
            licenseDetailsString.append(licenseeNameUnicode);
            licenseDetailsString.append(GD_STR_NewLine);

            // Add the License type (Academic/Indie/Commercial):
            licenseDetailsString.append(L"      ");
            licenseDetailsString.append(GD_STR_HelpAboutLicenseeType);

            if (isAcademicLicense)
            {
                licenseDetailsString.append(GD_STR_HelpAboutLicenseTypeAcademic);
            }
            else if (isIndieLicense)
            {
                licenseDetailsString.append(GD_STR_HelpAboutLicenseTypeIndie);
            }
            else
            {
                licenseDetailsString.append(GD_STR_HelpAboutLicenseTypeCommercial);
            }

            licenseDetailsString.append(GD_STR_NewLine);

            // Add the License expiration date:
            licenseDetailsString.append(L"      ");
            licenseDetailsString.append(GD_STR_HelpAboutLicenseExpirationDate);
            licenseDetailsString.append(licenseExpiryDateString);
            licenseDetailsString.append(GD_STR_NewLine);

            // Add the upgrades expiration date:
            licenseDetailsString.append(L"      ");
            licenseDetailsString.append(GD_STR_HelpAboutSupportExpirationDate);
            licenseDetailsString.append(supportAndUpgradesDateString);

            // Add the Academic warning:
            if (isAcademicLicense)
            {
                licenseDetailsString.append(L"\n\n");
                licenseDetailsString.append(GD_STR_HelpAboutLicenseAcademicWarning1);
                licenseDetailsString.append(GD_STR_NewLine);
                licenseDetailsString.append(L"      ");
                licenseDetailsString.append(GD_STR_HelpAboutLicenseAcademicWarning2);
            }
            else if (isIndieLicense)
            {
                // Add the Indie license warning:
                licenseDetailsString.append(L"\n\n");
                licenseDetailsString.append(GD_STR_HelpAboutLicenseIndieWarning1);
                licenseDetailsString.append(GD_STR_NewLine);
                licenseDetailsString.append(L"      ");
                licenseDetailsString.append(GD_STR_HelpAboutLicenseIndieWarning2);
            }
        }

        _licenseInformationString = licenseDetailsString;
    }
    else
    {
        // We don't have an installed license for the input product:
        _licenseInformationString = productNameStringUnicode;
        _licenseInformationString += L": ";
        _licenseInformationString += GD_STR_HelpAboutNoLicense;
    }


    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdGetLicenseInformationCommand::getLicenseString
// Description: Get the license string
// Return Val:  bool - Success / failure.
// Author:      Eran Zinman
// Date:        22/4/2007
// ---------------------------------------------------------------------------
bool gdGetLicenseInformationCommand::getLicenseString(gtString& licenseString)
{
    bool retVal = true;

    // If we got license information
    if (!_licenseInformationString.isEmpty())
    {
        licenseString = _licenseInformationString;
    }
    else
    {
        retVal = false;
    }

    return retVal;
}
