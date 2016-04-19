//------------------------------ __KAAMDTKERNELANALYZERDLLBUILD_H.h ------------------------------

#ifndef __KAAMDTKERNELANALYZERDLLBUILD_H
#define __KAAMDTKERNELANALYZERDLLBUILD_H

// Under Win32 builds - define: GW_API to be:
// - When building AMDTKernelAnalyzer.dll:     __declspec(dllexport).
// - When building other projects:     __declspec(dllimport).

#if defined(_WIN32)
    #if defined(AMDTKERNELANALYZER_EXPORTS)
        #define KA_API __declspec(dllexport)
    #else
        #define KA_API __declspec(dllimport)
    #endif
#else
    #define KA_API
#endif


#endif  // __KAAMDTKERNELANALYZERDLLBUILD_H
