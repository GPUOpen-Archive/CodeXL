//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file
///
//=====================================================================

//------------------------------ GRemedyCLExtensions.h ------------------------------

#ifndef __GREMEDYCLEXTENSIONS
#define __GREMEDYCLEXTENSIONS

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



/* ----------------------- cl_gremedy_computation_frame ----------------------- */


#ifndef cl_gremedy_computation_frame
#define cl_gremedy_computation_frame 1
#ifdef CL_EXT_PROTOTYPES

extern CL_API_ENTRY cl_int CL_API_CALL
clBeginComputationFrameGREMEDY(cl_context   /* context */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
clEndComputationFrameGREMEDY(cl_context /* context */) CL_API_SUFFIX__VERSION_1_0;

#endif /* CL_EXT_PROTOTYPES */

typedef cl_int(CL_API_ENTRY CL_API_CALL* PFNCLBEGINCOMPUTATIONFRAMEGREMEDYPROC)(cl_context   /* context */) CL_API_SUFFIX__VERSION_1_0;
typedef cl_int(CL_API_ENTRY CL_API_CALL* PFNCLENDCOMPUTATIONFRAMEGREMEDYPROC)(cl_context   /* context */) CL_API_SUFFIX__VERSION_1_0;

#endif


/* ----------------------- cl_gremedy_object_naming ----------------------- */
#ifndef cl_gremedy_object_naming
#define cl_gremedy_object_naming 1
#ifdef CL_EXT_PROTOTYPES
extern CL_API_ENTRY cl_int CL_API_CALL
clNameContextGREMEDY(cl_context     /* context */,
                     const char*    /* name */,
                     size_t         /* length */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
clNameCommandQueueGREMEDY(cl_command_queue  /* command_queue */,
                          const char*       /* name */,
                          size_t            /* length */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
clNameMemObjectGREMEDY(cl_mem       /* memobj */,
                       const char*  /* name */,
                       size_t       /* length */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
clNameSamplerGREMEDY(cl_sampler     /* sampler */,
                     const char*    /* name */,
                     size_t         /* length */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
clNameProgramGREMEDY(cl_program     /* program */,
                     const char*    /* name */,
                     size_t         /* length */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
clNameKernelGREMEDY(cl_kernel   /* kernel */,
                    const char* /* name */,
                    size_t      /* length */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
clNameEventGREMEDY(cl_event     /* event */,
                   const char*  /* name */,
                   size_t       /* length */) CL_API_SUFFIX__VERSION_1_0;
#endif /* CL_EXT_PROTOTYPES */

typedef cl_int(CL_API_ENTRY CL_API_CALL* PFNCLNAMECONTEXTGREMEDYPROC)(cl_context     /* context */, const char*    /* name */, size_t         /* length */) CL_API_SUFFIX__VERSION_1_0;
typedef cl_int(CL_API_ENTRY CL_API_CALL* PFNCLNAMECOMMANDQUEUEGREMEDYPROC)(cl_command_queue  /* command_queue */, const char*       /* name */, size_t            /* length */) CL_API_SUFFIX__VERSION_1_0;
typedef cl_int(CL_API_ENTRY CL_API_CALL* PFNCLNAMEMEMOBJECTGREMEDYPROC)(cl_mem       /* memobj */, const char*  /* name */, size_t       /* length */) CL_API_SUFFIX__VERSION_1_0;
typedef cl_int(CL_API_ENTRY CL_API_CALL* PFNCLNAMESAMPLERGREMEDYPROC)(cl_sampler     /* sampler */, const char*    /* name */, size_t         /* length */) CL_API_SUFFIX__VERSION_1_0;
typedef cl_int(CL_API_ENTRY CL_API_CALL* PFNCLNAMEPROGRAMGREMEDYPROC)(cl_program     /* program */, const char*    /* name */, size_t         /* length */) CL_API_SUFFIX__VERSION_1_0;
typedef cl_int(CL_API_ENTRY CL_API_CALL* PFNCLNAMEKERNELGREMEDYPROC)(cl_kernel   /* kernel */, const char* /* name */, size_t      /* length */) CL_API_SUFFIX__VERSION_1_0;
typedef cl_int(CL_API_ENTRY CL_API_CALL* PFNCLNAMEEVENTGREMEDYPROC)(cl_event     /* event */, const char*  /* name */, size_t       /* length */) CL_API_SUFFIX__VERSION_1_0;


#endif
#ifdef __cplusplus
}
#endif

#endif /* __GREMEDYCLEXTENSIONS */

