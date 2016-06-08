
/* ============================================================

Copyright (c) 2007 Advanced Micro Devices, Inc.  All rights reserved.

Redistribution and use of this material is permitted under the following
conditions:

Redistributions must retain the above copyright notice and all terms of this
license.

In no event shall anyone redistributing or accessing or using this material
commence or participate in any arbitration or legal action relating to this
material against Advanced Micro Devices, Inc. or any copyright holders or
contributors. The foregoing shall survive any expiration or termination of
this license or any agreement or access or use related to this material.

ANY BREACH OF ANY TERM OF THIS LICENSE SHALL RESULT IN THE IMMEDIATE REVOCATION
OF ALL RIGHTS TO REDISTRIBUTE, ACCESS OR USE THIS MATERIAL.

THIS MATERIAL IS PROVIDED BY ADVANCED MICRO DEVICES, INC. AND ANY COPYRIGHT
HOLDERS AND CONTRIBUTORS "AS IS" IN ITS CURRENT CONDITION AND WITHOUT ANY
REPRESENTATIONS, GUARANTEE, OR WARRANTY OF ANY KIND OR IN ANY WAY RELATED TO
SUPPORT, INDEMNITY, ERROR FREE OR UNINTERRUPTED OPERATION, OR THAT IT IS FREE
FROM DEFECTS OR VIRUSES.  ALL OBLIGATIONS ARE HEREBY DISCLAIMED - WHETHER
EXPRESS, IMPLIED, OR STATUTORY - INCLUDING, BUT NOT LIMITED TO, ANY IMPLIED
WARRANTIES OF TITLE, MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
ACCURACY, COMPLETENESS, OPERABILITY, QUALITY OF SERVICE, OR NON-INFRINGEMENT.
IN NO EVENT SHALL ADVANCED MICRO DEVICES, INC. OR ANY COPYRIGHT HOLDERS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, PUNITIVE,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, REVENUE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED OR BASED ON ANY THEORY OF LIABILITY
ARISING IN ANY WAY RELATED TO THIS MATERIAL, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE. THE ENTIRE AND AGGREGATE LIABILITY OF ADVANCED MICRO DEVICES,
INC. AND ANY COPYRIGHT HOLDERS AND CONTRIBUTORS SHALL NOT EXCEED TEN DOLLARS
(US $10.00). ANYONE REDISTRIBUTING OR ACCESSING OR USING THIS MATERIAL ACCEPTS
THIS ALLOCATION OF RISK AND AGREES TO RELEASE ADVANCED MICRO DEVICES, INC. AND
ANY COPYRIGHT HOLDERS AND CONTRIBUTORS FROM ANY AND ALL LIABILITIES,
OBLIGATIONS, CLAIMS, OR DEMANDS IN EXCESS OF TEN DOLLARS (US $10.00). THE
FOREGOING ARE ESSENTIAL TERMS OF THIS LICENSE AND, IF ANY OF THESE TERMS ARE
CONSTRUED AS UNENFORCEABLE, FAIL IN ESSENTIAL PURPOSE, OR BECOME VOID OR
DETRIMENTAL TO ADVANCED MICRO DEVICES, INC. OR ANY COPYRIGHT HOLDERS OR
CONTRIBUTORS FOR ANY REASON, THEN ALL RIGHTS TO REDISTRIBUTE, ACCESS OR USE
THIS MATERIAL SHALL TERMINATE IMMEDIATELY. MOREOVER, THE FOREGOING SHALL
SURVIVE ANY EXPIRATION OR TERMINATION OF THIS LICENSE OR ANY AGREEMENT OR
ACCESS OR USE RELATED TO THIS MATERIAL.

NOTICE IS HEREBY PROVIDED, AND BY REDISTRIBUTING OR ACCESSING OR USING THIS
MATERIAL SUCH NOTICE IS ACKNOWLEDGED, THAT THIS MATERIAL MAY BE SUBJECT TO
RESTRICTIONS UNDER THE LAWS AND REGULATIONS OF THE UNITED STATES OR OTHER
COUNTRIES, WHICH INCLUDE BUT ARE NOT LIMITED TO, U.S. EXPORT CONTROL LAWS SUCH
AS THE EXPORT ADMINISTRATION REGULATIONS AND NATIONAL SECURITY CONTROLS AS
DEFINED THEREUNDER, AS WELL AS STATE DEPARTMENT CONTROLS UNDER THE U.S.
MUNITIONS LIST. THIS MATERIAL MAY NOT BE USED, RELEASED, TRANSFERRED, IMPORTED,
EXPORTED AND/OR RE-EXPORTED IN ANY MANNER PROHIBITED UNDER ANY APPLICABLE LAWS,
INCLUDING U.S. EXPORT CONTROL LAWS REGARDING SPECIFICALLY DESIGNATED PERSONS,
COUNTRIES AND NATIONALS OF COUNTRIES SUBJECT TO NATIONAL SECURITY CONTROLS.
MOREOVER, THE FOREGOING SHALL SURVIVE ANY EXPIRATION OR TERMINATION OF ANY
LICENSE OR AGREEMENT OR ACCESS OR USE RELATED TO THIS MATERIAL.

NOTICE REGARDING THE U.S. GOVERNMENT AND DOD AGENCIES: This material is
provided with "RESTRICTED RIGHTS" and/or "LIMITED RIGHTS" as applicable to
computer software and technical data, respectively. Use, duplication,
distribution or disclosure by the U.S. Government and/or DOD agencies is
subject to the full extent of restrictions in all applicable regulations,
including those found at FAR52.227 and DFARS252.227 et seq. and any successor
regulations thereof. Use of this material by the U.S. Government and/or DOD
agencies is acknowledgment of the proprietary rights of any copyright holders
and contributors, including those of Advanced Micro Devices, Inc., as well as
the provisions of FAR52.227-14 through 23 regarding privately developed and/or
commercial computer software.

This license forms the entire agreement regarding the subject matter hereof and
supersedes all proposals and prior discussions and writings between the parties
with respect thereto. This license does not affect any ownership, rights, title,
or interest in, or relating to, this material. No terms of this license can be
modified or waived, and no breach of this license can be excused, unless done
so in a writing signed by all affected parties. Each term of this license is
separately enforceable. If any term of this license is determined to be or
becomes unenforceable or illegal, such term shall be reformed to the minimum
extent necessary in order for this license to remain in effect in accordance
with its terms as modified by such reformation. This license shall be governed
by and construed in accordance with the laws of the State of Texas without
regard to rules on conflicts of law of any state or jurisdiction or the United
Nations Convention on the International Sale of Goods. All disputes arising out
of this license shall be subject to the jurisdiction of the federal and state
courts in Austin, Texas, and all defenses are hereby waived concerning personal
jurisdiction and venue of these courts.

============================================================ */



#ifndef __CAL_EXT_H__
#define __CAL_EXT_H__

#include "cal.h"

#ifdef __cplusplus
extern "C" {
#define CALAPI
#else
#define CALAPI extern
#endif

#ifndef CALAPIENTRYP
#define CALAPIENTRYP CALAPIENTRY *
#endif

typedef enum CALextidEnum {
    CAL_EXT_D3D9            = 0x1001,   /* CAL/D3D9 interaction extension    */
    CAL_EXT_RESERVED        = 0x1002,   /* Place Holder  */
    CAL_EXT_D3D10           = 0x1003,   /* CAL/D3D10 interaction extension */
    CAL_EXT_COUNTERS        = 0x1004,   /* CAL counter extension */
    CAL_EXT_DOMAIN_PARAMS   = 0x1005,   /* CAL Domain Param extension */
    CAL_EXT_RES_CREATE      = 0x1006,   /* CAL Create resource extension */
    CAL_EXT_COMPUTE_SHADER  = 0x1007,   /* CAL compute shader extension */
    CAL_EXT_SAMPLER_PARAM   = 0x1008,   /* CAL Sampler extension */
} CALextid;

typedef CALvoid* CALextproc;


CALAPI CALresult CALAPIENTRY calExtSupported(CALextid extid);
CALAPI CALresult CALAPIENTRY calExtGetVersion(CALuint* major, CALuint* minor, CALextid extid);
CALAPI CALresult CALAPIENTRY calExtGetProc(CALextproc* proc, CALextid extid, const CALchar* procname);

/*
 * CAL Domain Parameters extension
 */
typedef struct CALvec4fRec {
    CALfloat x;
    CALfloat y;
    CALfloat z;
    CALfloat w;
} CALvec4f;

typedef struct CALparamRec {
    CALvec4f ul;            /* upper left domain parameter value */
    CALvec4f ll;            /* lower left domain parameter value */
    CALvec4f lr;            /* lower right domain parameter value */
} CALparam;

typedef struct CALdomainparamRec {
    CALparam  param0;
    CALparam  param1;
    CALparam  param2;
    CALparam  param3;
    CALparam  param4;
    CALparam  param5;
    CALparam  param6;
    CALparam  param7;
} CALdomainparam;


typedef CALresult (CALAPIENTRYP PFNCALCTXRUNPROGRAMPARAMS)(CALevent *event, CALcontext ctx, CALfunc func, const CALdomain* domain, const CALdomainparam* params);

/**
 * @fn calCtxRunProgramGrid(CALevent* event,
 *                          CALcontext ctx,
 *                          CALprogramGrid* pProgramGrid)
 *
 * @brief Invoke the kernel over the specified domain.
 *
 *
 * issues a task to invoke the computation of the kernel identified by
 * <i>func</i> within a region <i>domain</i> on the context <i>ctx</i> and
 * returns an associated event token in <i>*event</i> with this task. This
 * method returns CAL_RESULT_ERROR if <i>func</i> is not found in the currently
 * loaded module. This method returns CAL_RESULT_ERROR, if any of the inputs,
 * input references, outputs and constant buffers associated with the kernel
 * are not setup. Completion of this event can be queried by the master process
 * using <i>calIsEventDone</i>
 *
 * Extended contextual information regarding a calCtxRunProgram failure
 * can be obtained with the calGetErrorString function.
 *
 * @param event (out) - event associated with RunProgram instance. On error, event will be zero.
 * @param ctx (in) - context.
 * @param programGrid (in) - description of program information to get kernel and thread counts.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calCtxIsEventDone
 */
typedef CALresult (CALAPIENTRYP PFNCALCTXRUNPROGRAMGRID)(CALevent* event,
                                                  CALcontext ctx,
                                                  CALprogramGrid* pProgramGrid);

/**
 * @fn calModuleGetFuncInfo(CALfuncInfo* info, CALcontext ctx, CALmodule module, CALfunc func)
 *
 * @brief Retrieve information regarding the named func in the
 * named module.
 *
 * returns in <i>*info</i> the information regarding the func.
 * This method returns CAL_RESULT_NOT_INITIALIZED if CAL is not
 * initialied.
 * This method returns CAL_RESULT_INVALID_PARAMETER if info is
 * NULL.
 * This method returns CAL_RESULT_BAD_HANDLE if ctx is invalid
 * or module is not loaded or func is not found.
 * This method returns CAL_RESULT_ERROR if there was an error
 *
 * @param pInfo (out) - pointer to CALmoduleInfo output
 *              structure.
 * @param ctx (in) - context.
 * @param module (in) - handle to the loaded image.
 * @param func (in) - name of the function.
 *
 * @return Returns CAL_RESULT_OK on success,
 *         CAL_RESULT_NOT_INITIALIZED,
 *         CAL_RESULT_INVALID_PARAMETER, CAL_RESULT_BAD_HANDLE,
 *         or CAL_RESULT_ERROR if there was an error.
 *
 */
typedef CALresult (CALAPIENTRYP PFNCALMODULEGETFUNCINFO)(CALfuncInfo* pInfo,
                     CALcontext   ctx,
                     CALmodule    module,
                     CALfunc      func);

/**
 * @fn calCtxRunProgramGridArray(CALevent* event,
 *                               CALcontext ctx,
 *                               CALprogramGridArray* pProgramGridArray)
 *
 * @brief Invoke the kernel array over the specified domain(s).
 *
 *
 * issues a task to invoke the computation of the kernel arrays identified by
 * <i>func</i> within a region <i>domain</i> on the context <i>ctx</i> and
 * returns an associated event token in <i>*event</i> with this task. This
 * method returns CAL_RESULT_ERROR if <i>func</i> is not found in the currently
 * loaded module. This method returns CAL_RESULT_ERROR, if any of the inputs,
 * input references, outputs and constant buffers associated with the kernel
 * are not setup. Completion of this event can be queried by the master process
 * using <i>calIsEventDone</i>
 *
 * Extended contextual information regarding a calCtxRunProgram failure
 * can be obtained with the calGetErrorString function.
 *
 * @param event (out) - event associated with RunProgram instance. On error, event will be zero.
 * @param ctx (in) - context.
 * @param programGridArray (in) - array containing kernel programs and grid information.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calCtxIsEventDone
 */
typedef CALresult (CALAPIENTRYP PFNCALCTXRUNPROGRAMGRIDARRAY)(CALevent* event,
                                                  CALcontext ctx,
                                                  CALprogramGridArray* pGridArray);


/**
 * @fn calResCreate2D(CALresource* res, CALdevice dev, CALvoid* mem, CALuint width, CALuint height, CALformat format, CALuint size, CALuint flags)
 *
 * @brief Create a resource based on memory allocated by the application
 *
 * @param res (out)   - returned resource handle. On error, res will be zero.
 * @param dev (in)    - device the resource should be local.
 * @param mem (in)    - pointer to allocated memory
 * @param width (in)  - width of resource (in elements).
 * @param height (in) - height of resource
 * @param format (in) - format/type of each element of the resource.
 * @param size (in)   - Size of allocation (in bytes)
 * @param flags (in) - currently unused.
 *
 * @return Returns CAL_RESULT_OK on success
 */



typedef CALresult (CALAPIENTRYP PFNCALRESCREATE2D)(CALresource* res,
               CALdevice dev,
               CALvoid* mem,
               CALuint width,
               CALuint height,
               CALformat format,
               CALuint size,
               CALuint flags);
/**
 * @fn calResCreate1D(CALresource* res, CALdevice dev, CALvoid* mem, CALuint width, CALuint height, CALformat format, CALuint size, CALuint flags)
 *
 * @brief Create a resource based on memory allocated by the application
 *
 * @param res (out)   - returned resource handle. On error, res will be zero.
 * @param dev (in)    - device the resource should be local.
 * @param mem (in)    - pointer to allocated memory
 * @param width (in)  - width of resource (in elements).
 * @param format (in) - format/type of each element of the resource.
 * @param size (in)   - Size of allocation (in bytes)
 * @param flags (in) - currently unused.
 *
 * @return Returns CAL_RESULT_OK on success
 */

typedef CALresult (CALAPIENTRYP PFNCALRESCREATE1D)(CALresource* res,
               CALdevice dev,
               CALvoid* mem,
               CALuint width,
               CALformat format,
               CALuint size,
               CALuint flags);

//
// calCtxSetSamplerParams typedefs
//
typedef enum calSamplerParameterEnum {
    CAL_SAMPLER_PARAM_FETCH4 = 0, //DEPRECATED.  should set min/mag filter.
    CAL_SAMPLER_PARAM_DEFAULT = 0,
    CAL_SAMPLER_PARAM_MIN_FILTER,
    CAL_SAMPLER_PARAM_MAG_FILTER,
    CAL_SAMPLER_PARAM_WRAP_S,
    CAL_SAMPLER_PARAM_WRAP_T,
    CAL_SAMPLER_PARAM_WRAP_R,
    CAL_SAMPLER_PARAM_BORDER_COLOR,
    CAL_SAMPLER_PARAM_LAST
} CALsamplerParameter;

typedef enum calSamplerParamMinFilter {
    CAL_SAMPLER_MIN_LINEAR,
    CAL_SAMPLER_MIN_NEAREST,
    CAL_SAMPLER_MIN_NEAREST_MIPMAP_NEAREST,
    CAL_SAMPLER_MIN_NEAREST_MIPMAP_LINEAR,
    CAL_SAMPLER_MIN_LINEAR_MIPMAP_NEAREST,
    CAL_SAMPLER_MIN_LINEAR_MIPMAP_LINEAR,
    reserved_min0,
    CAL_SAMPLER_MIN_LINEAR_FOUR_SAMPLE,
    CAL_SAMPLER_MIN_LINEAR_FOUR_SAMPLE_MIPMAP_NEAREST,
    reserved_min1,
    reserved_min2,
} CALsamplerParamMinFilter;

typedef enum calSamplerParamMagFilter {
    CAL_SAMPLER_MAG_NEAREST,
    CAL_SAMPLER_MAG_LINEAR,
    reserved_mag0,
    reserved_mag1,
    CAL_SAMPLER_MAG_LINEAR_FOUR_SAMPLE
} CALsamplerParamMagFilter;

typedef enum calSamplerParamWrapMode {
    CAL_SAMPLER_WRAP_REPEAT,
    CAL_SAMPLER_WRAP_MIRRORED_REPEAT,
    CAL_SAMPLER_WRAP_CLAMP_TO_EDGE,
    CAL_SAMPLER_WRAP_MIRROR_CLAMP_TO_EDGE_EXT,
    CAL_SAMPLER_WRAP_CLAMP,
    CAL_SAMPLER_WRAP_MIRROR_CLAMP_EXT,
    CAL_SAMPLER_WRAP_CLAMP_TO_BORDER,
    CAL_SAMPLER_WRAP_MIRROR_CLAMP_TO_BORDER_EXT
} CALsamplerParamWrapMode;

/**
 * @fn calCtxSetSamplerParameter(CALcontext ctx, CALname name, CALsamplerParameter param, CALvoid* vals)
 *
 * @brief Set sampler state for the given sampler name.
 *
 * This method returns CAL_RESULT_BAD_HANDLE if ctx or name is invalid
 * or if name is not a sampler type name.
 * This method returns CAL_RESULT_INVALID_PARAMETER if vals is NULL and param
 * is not CAL_SAMPLER_PARAM_DEFAULT.
 * This method returns CAL_RESULT_ERROR if there was an error trying to set
 * the sampler paramter.
 *
 * @param ctx (in) - context.
 * @param name (in) - name.
 * @param param (in) - sampler parameter to change.
 * @param vals (in) - typeless list of values to update the sampler parameter.
 *
 * @return Returns CAL_RESULT_OK on success,
 *         CAL_RESULT_INVALID_PARAMETER, CAL_RESULT_BAD_HANDLE,
 *         or CAL_RESULT_ERROR if there was an error.
 *
 */
typedef CALresult (CALAPIENTRYP PFNCALSETSAMPLERPARAMS) (CALcontext ctx,
                                                         CALname name,
                                                         CALsamplerParameter param,
                                                         CALvoid* vals);

/**
 * @fn calCtxGetSamplerParameter(CALcontext ctx, CALname name, CALsamplerParameter param, CALvoid* vals)
 *
 * @brief Get sampler state for the given sampler name.
 *
 * This method returns CAL_RESULT_BAD_HANDLE if ctx or name is invalid
 * or if name is not a sampler type name.
 * This method returns CAL_RESULT_INVALID_PARAMETER if vals is NULL.
 * This method returns CAL_RESULT_ERROR if there was an error trying to get
 * the sampler paramter.
 *
 * @param ctx (in) - context.
 * @param name (in) - name.
 * @param param (in) - sampler parameter to change.
 * @param vals (out) - typeless list of values for the sampler parameter.
 *
 * @return Returns CAL_RESULT_OK on success,
 *         CAL_RESULT_INVALID_PARAMETER, CAL_RESULT_BAD_HANDLE,
 *         or CAL_RESULT_ERROR if there was an error.
 *
 */
typedef CALresult (CALAPIENTRYP PFNCALGETSAMPLERPARAMS) (CALcontext ctx,
                                                        CALname name,
                                                        CALsamplerParameter param,
                                                        CALvoid* vals);


#ifdef __cplusplus
}      /* extern "C" { */
#endif

#endif


