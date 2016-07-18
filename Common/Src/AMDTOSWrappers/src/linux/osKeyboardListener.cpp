//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osKeyboardListener.cpp
///
//=====================================================================

//------------------------------ osKeyboardListener.cpp ------------------------------

//C++
#include <future>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <termios.h>
#include <signal.h>

//Infra
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDirectory.h>

//Local
#include <AMDTOSWrappers/Include/osKeyboardListener.h>


#pragma GCC diagnostic ignored "-Wwrite-strings"

osKeyboardListener::osKeyboardListener()
{
    m_pListener.reset(new std::thread(&osKeyboardListener::Listen, this, GetKeyboardDescriptor()));

}

osKeyboardListener::~osKeyboardListener()
{
    m_Stop = true;
    m_pListener->join();
}

osKeyboardListener& osKeyboardListener::Instance()
{
    static osKeyboardListener instance;
    return instance;
}

//Set callback function,which is to be called from keyboard pressed Hook
void   osKeyboardListener::SetOnKbPressedCallback(OnKeyboardPressed callback)
{
    m_callback = callback;
}

int osKeyboardListener::GetKeyboardDescriptor()
{
    int fd(0);

    char name[256] = "Unknown";

    if ((getuid()) != 0)
    {
        GT_ASSERT("You are not root! This may not work...n");
    }

    osDirectory deviceDir;
    deviceDir.setDirectoryFullPathFromString(gtString(L"/dev/input/"));
    gtList<osFilePath> devicePaths;
    deviceDir.getContainedFilePaths(L"*", osDirectory::SORT_BY_DATE_DESCENDING, devicePaths);

    //Open Device
    for (osFilePath devicePath : devicePaths)
    {
        const char* deviceString = devicePath.asString().asASCIICharArray();

        if ((fd = open(deviceString, O_RDONLY)) == -1)
        {
            gtString msg;
            msg.fromASCIIString(deviceString).append(L" is not a vaild device");
            OS_OUTPUT_DEBUG_LOG(msg.asCharArray(), OS_DEBUG_LOG_DEBUG);
        }

        //Print Device Name
        ioctl(fd, EVIOCGNAME(sizeof(name)), name);

        gtString deviceName;
        deviceName.fromASCIIString(name);
        deviceName.toLowerCase();
        deviceName.find(L"keyboard");
        const size_t found = deviceName.find(L"keyboard");

        if (found != std::string::npos)
        {
            //found keyboard device!
            break;
        }
    }

    return fd;
}

void osKeyboardListener::Listen(int fd)
{
    int rv = 0;
    int value = 0;
    int size = sizeof(struct input_event);
    struct timeval timeout;
    fd_set set;
    struct input_event ev[64];

    while (!m_Stop)
    {
        timeout.tv_sec = 0;
        timeout.tv_usec = 50000;//50 ms
        FD_ZERO(&set); // clear the set
        FD_SET(fd, &set); // add our file descriptor to the set
        rv = select(fd + 1, &set, NULL, NULL, &timeout);

        //if no error , no timeout and we not stopped
        if (rv != -1 && rv != 0 && false == m_Stop)
        {
            if ((rv = read(fd, ev, size * 64)) >= size)
            {
                value = ev[0].value;

                // Only read the key press event
                if (value != ' ' && ev[1].value == 1 && ev[1].type == EV_KEY)
                {
                    if (m_callback != nullptr)
                    {
                        m_callback(ev[1].code);
                    }
                }
            }
        }
    } //while
}
