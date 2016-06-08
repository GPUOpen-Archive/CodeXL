//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file
///
//=====================================================================

//------------------------------ GROSWrappersTest.cpp ------------------------------

// OpenGL:
#include <AMDTOSWrappers/Include/osOpenGLIncludes.h>

// C++:
#include <iostream>

// GRBaseTools:
#include <AMDTBaseTools/Include/gtString.h>

// GROSWrappers:
/*
#include <AMDTOSWrappers/Include/osClassInstancesFactory.h>
#include <AMDTOSWrappers/Include/osWindow.h>
#include <AMDTOSWrappers/Include/osGraphicDeviceContext.h>
#include <AMDTOSWrappers/Include/osPixelFormat.h>
#include <AMDTOSWrappers/Include/osOpenGLRenderContext.h>
#include <AMDTOSWrappers/Include/osSharedMemorySocketServer.h>
#include <AMDTOSWrappers/Include/osLinuxProcFileSystemReader.h>
*/

#include <AMDTOSWrappers/Include/osCGIInputDataReader.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>

/*
void testWindows()
{
    // Creating a window:
    osClassInstancesFactory& factory = osClassInstancesFactory::instance();
    osWindow* pMyWindow = factory.createWindow("Title", 0, 0, 100, 100);

    if (pMyWindow)
    {
        osGraphicDeviceContext* pMyDeviceContext = pMyWindow->createDeviceContext();

        if (pMyDeviceContext)
        {
            int amountOfPixelFormats = pMyDeviceContext->amountOfAvailablePixelFormats();

            cout << "There are " << amountOfPixelFormats << " pixel formats available\n";

            int chosenPixelFormatIndex = -1;

            for (int i=1; i<=amountOfPixelFormats; i++)
            {
                osPixelFormat* pCurrentPixelFormat = NULL;
                bool rc = pMyDeviceContext->getPixelFormat(i, pCurrentPixelFormat);

                if (rc)
                {
                    cout << "Pixel format" << i << ":\n";

                    bool isDoubleBuffered = pCurrentPixelFormat->isDoubleBuffered();
                    cout << "Is double buffered: " <<  isDoubleBuffered << "\n";

                    int amountOfZBufferBits = pCurrentPixelFormat->amountOfZBufferBits();
                    cout << "Amount of Z buffer bits: " <<  amountOfZBufferBits << "\n";

                    // ...

                    cout << "\n";

                    // Get the hardware support for this pixel format:
                    osPixelFormat::HardwareSupport hardwareSupport = pCurrentPixelFormat->hardwareSupport();

                    // Does it support OpenGL:
                    bool supportsOpenGL = pCurrentPixelFormat->supportsOpenGL();

                    // Look for a pixel format that supports OpenGL and is fully supported
                    // by hardware:
                    if ((hardwareSupport == osPixelFormat::FULL_HARDWARE_ACCELERATION) &&
                        supportsOpenGL)
                    {
                        chosenPixelFormatIndex = i;
                    }

                    // Clean up:
                    delete pCurrentPixelFormat;
                }
            }

            if (chosenPixelFormatIndex != -1)
            {
                // Create an OpenGL render context that will draw into this device context:
                pMyDeviceContext->setActivePixelFormat(chosenPixelFormatIndex);
                osOpenGLRenderContext* pOpenGLRenderContext = pMyDeviceContext->createOpenGLRenderContext();

                if (pOpenGLRenderContext)
                {
                    // Make the render context the current render context:
                    pOpenGLRenderContext->makeCurrent();

                    // Get the OpenGL vendor:
                    gtString vendorString = (char*)glGetString(GL_VENDOR);

                    // Get the OpenGL renderer:
                    gtString rendererString = (char*)glGetString(GL_RENDERER);

                    // Get the OpenGL version:
                    gtString versionString = (char*)glGetString(GL_VERSION);

                    // Get the available OpenGL extensions:
                    gtString extensionsString = (char*)glGetString(GL_EXTENSIONS);

                    cout << "OpenGL implementation: \n";
                    cout << "--------------------- \n";

                    cout << "Vendor: " << vendorString.asCharArray() << endl;
                    cout << "Renderer: " << rendererString.asCharArray() << endl;
                    cout << "Version: " << versionString.asCharArray() << endl;
                    cout << "Available extensions: " << extensionsString.asCharArray() << endl;
                }


            }

            // Clean up:
            delete pMyDeviceContext;
        }

        // Clean up:
        delete pMyWindow;
    }
}
*/

/*
void testSharedMemSockets()
{
    osSharedMemorySocketServer smSocketServer("Kuku");
    smSocketServer.open();
}
*/

/*
void testLinuxProcFileSystemReader()
{
    osLinuxProcFileSystemReader linuxFSReader;
    linuxFSReader.updateCPUsData();
}
*/

void testCGIInputReader()
{
    osCGIInputDataReader cgiInputReader;
    cgiInputReader.readGetInputData();
}


int main(int argc, char* argv[])
{
    cout << "GROSWrappersTest - main function begin\n";

    // testWindows();
    // testSharedMemSockets();
    // testLinuxProcFileSystemReader();
    // testCGIInputReader();

    bool isUnderDebugger = osIsRunningUnderDebugger();

    if (isUnderDebugger)
    {
        cout << "I am running under a debugger\n";
    }
    else
    {
        cout << "I am running in a stand-alone mode\n";
    }

    cout << "GROSWrappersTest - main function end\n";
    cout.flush();

    return 0;
}



