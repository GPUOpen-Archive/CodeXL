
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


#ifndef __CAL_EXT_D3D9_H__
#define __CAL_EXT_D3D9_H__

#include "cal.h"
#include <windows.h>
#include <d3d9.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CALAPIENTRYP
#define CALAPIENTRYP CALAPIENTRY *
#endif

/*
 * calD3D9Associate
 * Enable a CAL device and a D3D device to share resources. 
 * The CALdevice specified by device must be open. 
 * The D3D device specified by d3dDevice must be created. 
 * A D3D device must be associated with a CAL device prior to any resource sharing.
 * 
 * On success, CAL_RESULT_OK is returned. 
 * On error, CAL_RESULT_BAD_HANDLE is returned if device is an invalid handle. 
 */
typedef CALresult (CALAPIENTRYP PFNCALD3D9ASSOCIATE) (CALdevice device, IDirect3DDevice9* d3dDevice);

/*
 * calD3D9MapTexture
 * Return,  in d3dTex, a pointer to a created IDirect3DTexture9 interface for the supplied CAL resource res. 
 * The IDirect3DTexture9 is created on the supplied D3D device specified by d3dDevice. 
 * Use IDirect3DTexture9::GetLevelDesc() to get the description of the vertex buffer created. 
 * 
 * On success, CAL_RESULT_OK is returned. 
 * On error, CAL_RESULT_BAD_HANDLE is returned if res is an invalid handle. 
 * CAL_RESULT_INVALID_PARAMETER is returned if d3dTex or d3dDevice is null, 
 * CAL_RESULT_ERROR is returned if the memory resource res  is not accessible
 * by the physical adapter associated with the D3D Device d3DDevice. 
 */
typedef CALresult (CALAPIENTRYP PFNCALD3D9MAPTEXTUREFUNC) (IDirect3DTexture9 **d3dTex, IDirect3DDevice9* d3dDevice, CALresource res);

/*
 * calD3D9MapSurface 
 * 
 * On success, CAL_RESULT_OK is returned. 
 * On error, CAL_RESULT_BAD_HANDLE is returned if res is an invalid handle. 
 * CAL_RESULT_INVALID_PARAMETER is returned if d3dTex or d3dDevice is null, 
 * CAL_RESULT_ERROR is returned if the memory resource res  is not accessible
 * by the physical adapter associated with the D3D Device d3DDevice. 
 */
typedef CALresult (CALAPIENTRYP PFNCALD3D9MAPSURFACEFUNC) (CALresource* res, CALdevice dev, IDirect3DSurface9* tex, HANDLE shareHandle);

/*
 * calD3D9UnmapTexture
 * Destroy the created Direct3D texture. 
 * calD3D9UnmapTexture should only be called on textures created with calD3D9MapTexture.
 * 
 * On success, CAL_RESULT_OK is returned. 
 * On error, CAL_RESULT_BAD_HANDLE is returned if res is an invalid handle, 
 * CAL_RESULT_ERROR is returned if d3dTex wasn’t created by calD3D9CreateTexture.
 */
typedef CALresult (CALAPIENTRYP PFNCALD3D9UNMAPTEXTUREFUNC) (IDirect3DTexture9 *d3dTex);

/*
 * calD3D9MapVertexBuffer 
 * 
 * On success, CAL_RESULT_OK is returned. 
 * On error, CAL_RESULT_BAD_HANDLE is returned if res is an invalid handle. 
 * CAL_RESULT_INVALID_PARAMETER is returned if d3dVB or d3dDevice is null, 
 * CAL_RESULT_ERROR is returned if the memory resource res  is not accessible
 * by the physical adapter associated with the D3D Device d3DDevice. 
 */
typedef CALresult (CALAPIENTRYP PFNCALD3D9MAPVERTEXBUFFERFUNC) (CALresource* res, CALdevice dev, IDirect3DVertexBuffer9* d3dVB, HANDLE shareHandle);

/*
 * calD3D9MapIndexBuffer 
 * 
 * On success, CAL_RESULT_OK is returned. 
 * On error, CAL_RESULT_BAD_HANDLE is returned if res is an invalid handle. 
 * CAL_RESULT_INVALID_PARAMETER is returned if d3dIB or d3dDevice is null, 
 * CAL_RESULT_ERROR is returned if the memory resource res  is not accessible
 * by the physical adapter associated with the D3D Device d3DDevice. 
 */
typedef CALresult (CALAPIENTRYP PFNCALD3D9MAPINDEXBUFFERFUNC) (CALresource* res, CALdevice dev, IDirect3DIndexBuffer9* d3dIB, HANDLE shareHandle);

#ifdef __cplusplus
}
#endif
#endif // __CAL_EXT_D3D9_H__
