#include "Include/beProgramBuilder.h"

using namespace std;

void beProgramBuilder::UsePlatformNativeLineEndings(std::string& text)
{
    // If text is empty
    if (text.length() <= 0)
    {
        return;
    }

    // Remove all carriage returns.
    // This will put us into Linux format (from either Mac or Windows).
    // [With the AMD OpenCL stack as of April 2012, this does nothing.]
    text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());

    // Add a linefeed at the end if there's not one there already.
    if (text[text.length() - 1] != '\n')
    {
        text += '\n';
    }

#ifdef _WIN32
    // Now convert all of the \n to \r\n.
    size_t pos = 0;

    while ((pos = text.find('\n', pos)) != string::npos)
    {
        text.replace(pos, 1, "\r\n");
        pos += 2;
    }

#endif
}

bool beProgramBuilder::IsPublishedDevice(const string& sDevice)
{
    bool bRet = false;

    // gaurd
    if (sDevice.empty() == false)
    {
        // the format of the internal driver can be XX.XXX.XXX.XXXX in which every segment can be 1 to 4 digit long.
        // also dodn't want to deal with floating point numbers because they are unpredictables.
        int driverOne = 0, driverTwo = 0, driverThree = 0, driverFour = 0, deviceOne = 0, deviceTwo = 0, deviceThree = 0, deviceFour = 0;
        sscanf(m_DriverVersion.c_str(), "%d.%d.%d.%d", &driverOne, &driverTwo, &driverThree, &driverFour);
        sscanf(sDevice.c_str(), "%d.%d.%d.%d", &deviceOne, &deviceTwo, &deviceThree, &deviceFour);
        // since version 14.4 is bigger then 14.300 need to normalize all to be 4 digits
        MakeNumber4Digit(driverTwo);
        MakeNumber4Digit(driverThree);
        MakeNumber4Digit(driverFour);
        MakeNumber4Digit(deviceTwo);
        MakeNumber4Digit(deviceThree);
        MakeNumber4Digit(deviceFour);

        if (deviceOne < driverOne)
        {
            bRet = true;
        }
        else if (deviceOne == driverOne)
        {
            if (deviceTwo < driverTwo)
            {
                bRet = true;
            }

            else if (deviceTwo == driverTwo)
            {
                if (deviceThree < driverThree)
                {
                    bRet = true;
                }

                else if (deviceThree == driverThree)
                {
                    if (deviceFour <= driverFour)
                    {
                        bRet = true;
                    }
                }
            }
        }
    }

    return bRet;
}

void beProgramBuilder::MakeNumber4Digit(int& iTheNumber)
{
    if (iTheNumber > 0)
    {
        while ((iTheNumber / 1000) == 0)
        {
            iTheNumber *= 10;
        }
    }
}



