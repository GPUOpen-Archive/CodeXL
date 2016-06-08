//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDCLExtensions.h
///
//=====================================================================

//------------------------------ AMDCLExtensions.h ------------------------------

#ifndef __AMDCLEXTENSIONS
#define __AMDCLEXTENSIONS

#ifndef CL_API_ENTRY
    #define CL_API_ENTRY
#endif

#ifndef CL_API_CALL
    #define CL_API_CALL __stdcall
#endif

#ifndef CL_API_SUFFIX__VERSION_1_0
    #define CL_API_SUFFIX__VERSION_1_0
#endif

#ifdef __cplusplus
extern "C" {
#endif



/* ----------------------- cl_amd_computation_frame ----------------------- */


#ifndef cl_amd_computation_frame
#define cl_amd_computation_frame 1
#ifdef CL_EXT_PROTOTYPES

extern CL_API_ENTRY cl_int CL_API_CALL
clBeginComputationFrameAMD(cl_context   /* context */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
clEndComputationFrameAMD(cl_context /* context */) CL_API_SUFFIX__VERSION_1_0;

#endif /* CL_EXT_PROTOTYPES */

typedef cl_int(CL_API_ENTRY CL_API_CALL* PFNCLBEGINCOMPUTATIONFRAMEAMDPROC)(cl_context   /* context */) CL_API_SUFFIX__VERSION_1_0;
typedef cl_int(CL_API_ENTRY CL_API_CALL* PFNCLENDCOMPUTATIONFRAMEAMDPROC)(cl_context   /* context */) CL_API_SUFFIX__VERSION_1_0;

#endif


/* ----------------------- cl_amd_object_naming ----------------------- */
#ifndef cl_amd_object_naming
#define cl_amd_object_naming 1
#ifdef CL_EXT_PROTOTYPES
extern CL_API_ENTRY cl_int CL_API_CALL
clNameContextAMD(cl_context     /* context */,
                 const char*    /* name */,
                 size_t         /* length */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
clNameCommandQueueAMD(cl_command_queue  /* command_queue */,
                      const char*       /* name */,
                      size_t            /* length */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
clNameMemObjectAMD(cl_mem       /* memobj */,
                   const char*  /* name */,
                   size_t       /* length */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
clNameSamplerAMD(cl_sampler     /* sampler */,
                 const char*    /* name */,
                 size_t         /* length */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
clNameProgramAMD(cl_program     /* program */,
                 const char*    /* name */,
                 size_t         /* length */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
clNameKernelAMD(cl_kernel   /* kernel */,
                const char* /* name */,
                size_t      /* length */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
clNameEventAMD(cl_event     /* event */,
               const char*  /* name */,
               size_t       /* length */) CL_API_SUFFIX__VERSION_1_0;
#endif /* CL_EXT_PROTOTYPES */

typedef cl_int(CL_API_ENTRY CL_API_CALL* PFNCLNAMECONTEXTAMDPROC)(cl_context     /* context */, const char*    /* name */, size_t         /* length */) CL_API_SUFFIX__VERSION_1_0;
typedef cl_int(CL_API_ENTRY CL_API_CALL* PFNCLNAMECOMMANDQUEUEAMDPROC)(cl_command_queue  /* command_queue */, const char*       /* name */, size_t            /* length */) CL_API_SUFFIX__VERSION_1_0;
typedef cl_int(CL_API_ENTRY CL_API_CALL* PFNCLNAMEMEMOBJECTAMDPROC)(cl_mem       /* memobj */, const char*  /* name */, size_t       /* length */) CL_API_SUFFIX__VERSION_1_0;
typedef cl_int(CL_API_ENTRY CL_API_CALL* PFNCLNAMESAMPLERAMDPROC)(cl_sampler     /* sampler */, const char*    /* name */, size_t         /* length */) CL_API_SUFFIX__VERSION_1_0;
typedef cl_int(CL_API_ENTRY CL_API_CALL* PFNCLNAMEPROGRAMAMDPROC)(cl_program     /* program */, const char*    /* name */, size_t         /* length */) CL_API_SUFFIX__VERSION_1_0;
typedef cl_int(CL_API_ENTRY CL_API_CALL* PFNCLNAMEKERNELAMDPROC)(cl_kernel   /* kernel */, const char* /* name */, size_t      /* length */) CL_API_SUFFIX__VERSION_1_0;
typedef cl_int(CL_API_ENTRY CL_API_CALL* PFNCLNAMEEVENTAMDPROC)(cl_event     /* event */, const char*  /* name */, size_t       /* length */) CL_API_SUFFIX__VERSION_1_0;


#endif
#ifdef __cplusplus
}
#endif

#endif /* __AMDCLEXTENSIONS */

