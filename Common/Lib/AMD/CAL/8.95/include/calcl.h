/**
 *  @file     calcl.h
 *  @brief    CAL Compiler Interface Header
 *  @version  1.00.0 Beta
 */


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

#ifndef __CALCL_H__
#define __CALCL_H__

#include "cal.h"

#ifdef __cplusplus
extern "C" {
#define CALAPI
#else
#define CALAPI extern
#endif

#ifdef _WIN32
#define CALAPIENTRY  __stdcall
#else
#define CALAPIENTRY
#endif

/** Kernel source code language */
typedef enum CALlanguageEnum {
    CAL_LANGUAGE_IL=1,               /**< IL text */
} CALlanguage;

/** Kernel source code type */
typedef enum CALCLprogramTypeEnum {
    CAL_PROGRAM_TYPE_PS,              /**< GPU pixel program text */
    CAL_PROGRAM_TYPE_CS               /**< GPU compute program text */
} CALCLprogramType;

typedef void (*CALLogFunction)(const char* msg);  /**< Callback function for dissassembler */

/*============================================================================
 * CAL Compiler Interface
 *============================================================================*/

/**
 * @fn calclGetVersion(CALuint* major, CALuint* minor, CALuint* imp)
 *
 * @brief Retrieve the CAL compiler version that is loaded
 *
 * CAL version is in the form of API_Major.API_Minor.Implementation where
 * "API_Major" is the major version number of the CAL API. "API_Minor" is the
 * minor version number of the CAL API. "Implementation" is the implementation
 * instance of the supplied API version number.
 *
 * @return Returns CAL_RESULT_OK on success.
 *
 * @sa calInit calShutdown
 *
 */
CALAPI CALresult CALAPIENTRY calclGetVersion(CALuint* major, CALuint* minor, CALuint* imp);

/**
 * @fn calclCompile(CALobject* obj, CALlanguage language, const CALchar* source, CALtarget target)
 *
 * @brief Compile source into an object
 *
 * CAL compiler function. Compile a source language string to the specified target device
 * and return a compiled object.
 *
 * @param obj (out) - created object
 * @param language (in) - source language designation.
 * @param source (in) - string containing kernel source code.
 * @param target (in) - machine target.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calclLink calclFreeObject calclFreeImage
 */
CALAPI CALresult CALAPIENTRY calclCompile(CALobject* obj, CALlanguage language, const CALchar* source, CALtarget target);

/**
 * @fn calclLink(CALimage* image, CALobject* obj, CALuint objCount)
 *
 * @brief Link a list of objects into an image
 *
 * CAL compiler function. Link a list of individual objects into a full image.
 *
 * @param image (out) - created image
 * @param obj (in) - list of objects.
 * @param objCount (in) - number of objects
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calclCompile calclFreeObject calclFreeImage
 */
CALAPI CALresult CALAPIENTRY calclLink(CALimage* image, CALobject* obj, CALuint objCount);

/**
 * @fn calclFreeObject(CALobject obj)
 *
 * @brief Free a CALobject
 *
 * CAL compiler function. Frees a CALobject generated by calCompile
 *
 * @param obj (in) - object to free.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calclCompile calclFreeImage
 */
CALAPI CALresult CALAPIENTRY calclFreeObject(CALobject obj);

/**
 * @fn calclFreeImage(CALimage image)
 *
 * @brief Free a CALimage
 *
 * CAL compiler function. Frees a CALimage generated by calLink
 *
 * @param image (in) - image to free.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calclLink
 */
CALAPI CALresult CALAPIENTRY calclFreeImage(CALimage image);


/**
 * @fn calclDisassembleImage(const CALimage image, CALLogFunction logfunc)
 *
 * @brief Disassemble a CAL image.
 *
 * CAL compiler function. Disassembles the CAL image, outputing on a line by line bases to the supplied log function.
 *
 * @param image (in) - image to disassemble.
 * @param logfunc (in) - user supplied function to invoke for each line of disassembly.
 *
 * @return No return
 *
 * @sa calclCompile calclLink
 */
CALAPI void CALAPIENTRY calclDisassembleImage(const CALimage image, CALLogFunction logfunc);

/**
 * @fn calclAssembleObject(CALobject* obj, CALCLprogramType type, const CALchar* source, CALtarget target)
 *
 * @brief Assemble source into an object
 *
 * CAL compiler function. Assemble a source language string to the specified target device
 * and return a compiled object.
 *
 * @param obj (out) - created object
 * @param type (in) - source program type.
 * @param source (in) - string containing kernel source code.
 * @param target (in) - machine target.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calclLink calclFreeObject calclFreeImage
 */
CALAPI CALresult CALAPIENTRY calclAssembleObject(CALobject* obj, CALCLprogramType type, const CALchar* source, CALtarget target);

/**
 * @fn calclDisassembleObject(const CALobject* obj, CALLogFunction logfunc)
 *
 * @brief Disassemble a CAL object source into an object
 *
 * CAL compiler function. Disassembles the CAL object, outputing on a line by line bases to the supplied log function.
 *
 * @param obj (in) - object to disassemble.
 * @param logfunc (in) - user supplied function to invoke for each line of disassembly.
 *
 * @return No return
 *
 * @sa calclAssemble calclLink
 */
CALAPI void CALAPIENTRY calclDisassembleObject(const CALobject* obj, CALLogFunction logfunc);

/**
 * @fn calclImageGetSize(CALuint* size, CALimage image)
 *
 * @brief Return the size of the buffer needed for use with calclImageWrite.
 *
 * CAL compiler function. Determines the size of the buffer that needs to be
 * allocated for use with calclImageWrite.
 *
 * @param size (out) - returned size
 * @param image (in) - image whose size is computed
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calclImageWrite
 */
CALAPI CALresult CALAPIENTRY calclImageGetSize(CALuint* size, CALimage image);

/**
 * @fn calclImageWrite(CALvoid* buffer, CALuint size, CALimage image)
 *
 * @brief Serialize the supplied CALimage to the supplied buffer.
 *
 * CAL compiler function.  Serializes the contents of the CALimage image to
 * the supplied buffer.  The buffer should be allocated by the user and must
 * be at least as big as the value returned by calclImageGetSize.  The size
 * parameter indicates the size of the supplied buffer and is used to check
 * for buffer overrun.
 *
 * @param buffer (out) - buffer to serialize into
 * @param size (in) - size of allocated buffer
 * @param image (in) - image to serialize
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calclImageGetSize
 */
CALAPI CALresult CALAPIENTRY calclImageWrite(CALvoid* buffer, CALuint size, CALimage image);

/**
 * @fn calclGetErrorString(void)
 *
 * @brief Return details about current error state
 *
 * calGetErrorString returns a text string containing details about the last
 * returned error condition. Calling calGetErrorString does not effect the
 * error state.
 *
 * @return Returns a null terminated string detailing the error condition
 *
 */
CALAPI const CALchar* CALAPIENTRY calclGetErrorString(void);

#ifdef __cplusplus
}      /* extern "C" { */
#endif


#endif /* __CALCL_H__ */


