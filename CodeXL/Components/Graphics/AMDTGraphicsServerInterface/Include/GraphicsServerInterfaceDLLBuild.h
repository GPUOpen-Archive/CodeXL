//------------------------------ GraphicsServerInterfaceDLLBuild.h ------------------------------

#ifndef _GRAPHICS_SERVER_INTERFACE_DLL_BUILD_H_
#define _GRAPHICS_SERVER_INTERFACE_DLL_BUILD_H_

// Under Win32 builds - define: OW_API to be:
// - When building AMDTGraphicsServerInterface.lib:        default
// - When building AMDTGraphicsServerInterfaceDLL.dll:     __declspec(dllexport).
// - When building other projects:                         __declspec(dllimport).

#if defined(_WIN32)
    #if defined(AMDTGRAPHICSSERVERINTERFACE_EXPORTS)
        #define GSI_API __declspec(dllexport)
    #elif defined(AMDTGRAPHICSSERVERINTERFACE_STATIC)
        #define GSI_API
    #else
        #define GSI_API __declspec(dllimport)
    #endif
#else
    #define GSI_API
#endif


#endif  // __GRAPHICSSERVERINTERFACEDLLBUILD
