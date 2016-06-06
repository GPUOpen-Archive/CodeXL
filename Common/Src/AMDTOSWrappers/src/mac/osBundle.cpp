//------------------------------ osBundle.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local:
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osBundle.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

#ifdef _GR_IPHONE_BUILD
    #include <dlfcn.h>
    // CoreServices for iPhone:
    #include <CFNetwork/CFNetwork.h>
#else
    // Carbon:
    #include <Carbon/Carbon.h>
#endif

// Will hold the OpenGL ES framework path if needed:
static gtString s_openglesFrameworkPathAsString;

// ---------------------------------------------------------------------------
// Name:        osGetOpenGLFrameworkFunctionAddress
// Description: The MAC OS implementation for getProcAddress
// Arguments: const char* procName
// Return Val: osProcedureAddress
// Author:      Sigal Algranaty
// Date:        8/12/2008
// ---------------------------------------------------------------------------
osProcedureAddress osGetOpenGLFrameworkFunctionAddress(const char* procName)
{
    osProcedureAddress retVal = NULL;

#ifdef _GR_IPHONE_BUILD
    gtString openglesModuleFilePath = osGetOpenGLESFrameworkPath();
    openglesModuleFilePath.append(osFilePath::osPathSeparator).append(OS_OPENGL_ES_MODULE_NAME);
    void* openGLESLibraryHandle = dlopen(openglesModuleFilePath.asCharArray(), RTLD_LAZY);
    retVal = dlsym(openGLESLibraryHandle, procName);
#else
    // Get the OS functions entry points:
    CFBundleRef pBundleRefOpenGL = NULL;
    bool rc = osGetSystemOpenGLFrameworkBundle(pBundleRefOpenGL);
    GT_IF_WITH_ASSERT(rc)
    {
        // Get the function pointer from the bundle:
        retVal = CFBundleGetFunctionPointerForName(pBundleRefOpenGL, CFStringCreateWithCStringNoCopy(NULL, procName, CFStringGetSystemEncoding(), NULL));
    }

    if (pBundleRefOpenGL != NULL)
    {
        // Release the bundle ref:
        CFBundleUnloadExecutable(pBundleRefOpenGL);
        CFRelease(pBundleRefOpenGL);
        pBundleRefOpenGL = NULL;
    }

#endif

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osGetSystemOpenGLFrameworkBundle
// Description: Returns a reference to the system's OpenGL framework bundle.
// Arguments: refOpenGLFrameworkBundle - Will get a reference to the system's OpenGL
//                               framework bundle.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        28/12/2008
// ---------------------------------------------------------------------------
bool osGetSystemOpenGLFrameworkBundle(CFBundleRef& refOpenGLFrameworkBundle)
{
    bool retVal = false;

#ifdef _GR_IPHONE_BUILD
    // This function should not be called on the iPhone:
    GT_ASSERT(false);
#else
    // Get a reference to the System's frameworks folder:
    FSRef frameworksFolderRef;
    OSStatus err = FSFindFolder(kSystemDomain, kFrameworksFolderType, FALSE, &frameworksFolderRef);
    GT_IF_WITH_ASSERT(err == noErr)
    {
        // Convert the reference to a URL:
        CFURLRef baseURL = CFURLCreateFromFSRef(kCFAllocatorSystemDefault, &frameworksFolderRef);
        GT_IF_WITH_ASSERT(baseURL != NULL)
        {
            // Build the OpenGL framework bundle URL (path):
            CFURLRef bundleURL = CFURLCreateCopyAppendingPathComponent(kCFAllocatorSystemDefault, baseURL, CFSTR("OpenGL.framework"), false);

            if (bundleURL != NULL)
            {
                // Get a reference to the OpenGL framework bundle:
                refOpenGLFrameworkBundle = CFBundleCreate(kCFAllocatorSystemDefault, bundleURL);

                if (refOpenGLFrameworkBundle != NULL)
                {
                    // Load the OpenGL fremework bundle into this process address space:
                    Boolean rc1 = CFBundleLoadExecutable(refOpenGLFrameworkBundle);
                    GT_IF_WITH_ASSERT(rc1)
                    {
                        retVal = true;
                    }
                    else
                    {
                        // Failure clean up:
                        CFRelease(refOpenGLFrameworkBundle);
                        refOpenGLFrameworkBundle = NULL;
                    }
                }

                // Cleanup:
                CFRelease(bundleURL);
            }

            // Cleanup:
            CFRelease(baseURL);
        }
    }
#endif // _GR_IPHONE_BUILD

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osSetOpenGLESFrameworkPath
// Description: Sets the OpenGL ES framework path
// Author:      Uri Shomroni
// Date:        8/6/2009
// ---------------------------------------------------------------------------
void osSetOpenGLESFrameworkPath(const char* pFrameworkPath)
{
    GT_IF_WITH_ASSERT(pFrameworkPath != NULL)
    {
        s_openglesFrameworkPathAsString = pFrameworkPath;
    }
}

// ---------------------------------------------------------------------------
// Name:        osGetOpenGLESFrameworkPath
// Description: Returns the previously set OpenGL ES framework path
// Return Val: const char*
// Author:      Uri Shomroni
// Date:        8/6/2009
// ---------------------------------------------------------------------------
const char* osGetOpenGLESFrameworkPath()
{
    return s_openglesFrameworkPathAsString.asCharArray();
}

