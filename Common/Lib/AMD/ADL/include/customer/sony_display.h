///
///  Copyright (c) 2008 - 2009 Advanced Micro Devices, Inc.
 
///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

/// \file sony_display.h
/// \brief Contains Sony display-related functions exposed by ADL for \ALL platforms.
///
/// This file contains Sony specific LCD Refresh display-related functions exposed by ADL for \ALL platforms.
/// All functions found in this file can be used as a reference to ensure
/// the specified function pointers can be used by the appropriate runtime
/// dynamic library loaders. This header file not for public release

#ifndef SONY_DISPLAY_H_
#define SONY_DISPLAY_H_

#ifndef ADL_EXTERNC
#ifdef __cplusplus
#define ADL_EXTERNC extern "C"
#else
#define ADL_EXTERNC
#endif
#endif

/// \addtogroup DISPLAY
// @{
///
///\brief Function to get the LCD refresh rate capability. Not a Public API
/// 
/// This function retrieves the LCD refresh rate capability. ASIC Specific API
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpLcdRefreshRateCap The pointer to the retrieved LCD refresh rate capability.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_LCDRefreshRateCapability_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLLcdRefreshRateCap *lpLcdRefreshRateCap);

///
///\brief Function to get the LCD refresh rate capability. Not a Public API
/// 
/// This function retrieves the LCD refresh rate capability. ASIC Specific API
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpLcdRefreshRateCap The pointer to the retrieved LCD refresh rate capability.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_LCDRefreshRateCapability_Get(int iAdapterIndex, ADLLcdRefreshRateCap *lpLcdRefreshRateCap);

///
///\brief Function to get the LCD refresh rate. Not a Public API
/// 
/// This function retrieves the LCD refresh rate. ASIC Specific API
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpADLMode The pointer to the retrieved LCD refresh rate. Only ADLMode.iRefreshRate is used.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_LCDRefreshRate_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLMode * lpADLMode);   

///
///\brief Function to get the LCD refresh rate. Not a Public API
/// 
/// This function retrieves the LCD refresh rate. ASIC Specific API
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpADLMode The pointer to the retrieved LCD refresh rate. Only ADLMode.iRefreshRate is used.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_LCDRefreshRate_Get(int iAdapterIndex, ADLMode * lpADLMode);   

///
///\brief Function to set LCD refresh rate. Not a Public API
/// 
/// This function sets LCD refresh rate. ASIC Specific API
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  lpADLMode The pointer to the LCD refresh rate. Only ADLMode.iRefreshRate is used.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_LCDRefreshRate_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLMode * lpADLMode);

///
///\brief Function to set LCD refresh rate. Not a Public API
/// 
/// This function sets LCD refresh rate. ASIC Specific API
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  lpADLMode The pointer to the LCD refresh rate. Only ADLMode.iRefreshRate is used.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL_Display_LCDRefreshRate_Set(int iAdapterIndex, ADLMode * lpADLMode);

///
///\brief Function to retrieve LCD refresh rate options. Not a Public API
/// 
/// This function retrieves LCD refresh rate options. ASIC Specific API
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpLcdRefreshRateOptions The pointer to the LCD refresh rate options.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_LCDRefreshRateOptions_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLLcdRefreshRateOptions* lpLcdRefreshRateOptions); 

///
///\brief Function to retrieve LCD refresh rate options. Not a Public API
/// 
/// This function retrieves LCD refresh rate options. ASIC Specific API
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpLcdRefreshRateOptions The pointer to the LCD refresh rate options.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_LCDRefreshRateOptions_Get(int iAdapterIndex, ADLLcdRefreshRateOptions* lpLcdRefreshRateOptions); 

///
///\brief Function to set LCD refresh rate options. Not a Public API
/// 
/// This function sets LCD refresh rate options. ASIC Specific API
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  lpLcdRefreshRateOptions The pointer to the LCD refresh rate options.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_LCDRefreshRateOptions_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLLcdRefreshRateOptions* lpLcdRefreshRateOptions);

///
///\brief Function to set LCD refresh rate options. Not a Public API
/// 
/// This function sets LCD refresh rate options. ASIC Specific API
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  lpLcdRefreshRateOptions The pointer to the LCD refresh rate options.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL_Display_LCDRefreshRateOptions_Set(int iAdapterIndex, ADLLcdRefreshRateOptions* lpLcdRefreshRateOptions);

// @}

#endif /* SONY_DISPLAY_H_ */
