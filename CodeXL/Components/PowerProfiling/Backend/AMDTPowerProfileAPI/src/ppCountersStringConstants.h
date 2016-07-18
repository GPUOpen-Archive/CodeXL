//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppCountersStringConstants.h
///
//==================================================================================

//------------------------------ ppCountersStringConstants.h ------------------------------

#ifndef __PPCOUNTERS_STRING_CONSTANTS_H
#define __PPCOUNTERS_STRING_CONSTANTS_H

//Device or platform names
#define PP_STR_Carrizo         "Carrizo"
#define PP_STR_Kaveri          "Kaveri"
#define PP_STR_Temesh          "Temesh"
#define PP_STR_Mullins         "Mulins"
#define PP_STR_Godavari        "Godavari"

// Counter category names
#define PP_STR_PowerCategoryName        "Power"
#define PP_STR_CurrentCategoryName      "Current"
#define PP_STR_VoltageCategoryName      "Voltage"
#define PP_STR_DVFSCategoryName         "CPU Core State (DVFS)"
#define PP_STR_ProcessCategoryName      "Process"
#define PP_STR_TemperatureCategoryName  "Temperature"
#define PP_STR_FrequencyCategoryName    "Frequency"
#define PP_STR_CountCategoryName        "Count"
#define PP_STR_UnsupportedCategoryName  "Unsupported Category"

// Counter names:
#define PP_STR_Counter_Temp_Node          "Node Temp"
#define PP_STR_Counter_Power_TotalAPU     "Total APU Power"
#define PP_STR_Counter_Power_Other        "Other"
#define PP_STR_Counter_Power_IGPU         "iGPU Power"
#define PP_STR_Counter_Power_GFX          "GFX Power"
#define PP_STR_Counter_Power_CU0          "CPU CU0 Power"
#define PP_STR_Counter_Power_CU1          "CPU CU1 Power"
#define PP_STR_Counter_Power_DisplayCtrl  "Display-Ctrl Power"
#define PP_STR_Counter_Power_MemCtrl      "Mem-Ctrl Power"
#define PP_STR_Counter_Power_PCIECtrl     "PCIe-Ctrl Power"
#define PP_STR_Counter_Power_DGPU         "dGPU Power"

#define PP_STR_Counter_Power_IOCtrl       "IO-Ctrl Power"
#define PP_STR_Counter_Power_NB           "NB Power"
#define PP_STR_Counter_Power_GFXCtrl      "GFX-Ctrl Power"
#define PP_STR_Counter_Power_UVD          "UVD Power"
#define PP_STR_Counter_Power_VCE          "VCE Power"
#define PP_STR_Counter_Power_ACP          "ACP Power"
#define PP_STR_Counter_Power_UNB          "UNB Power"
#define PP_STR_Counter_Power_SMU          "SMU Power"
#define PP_STR_Counter_Power_RoC          "RoC Power"
#define PP_STR_Counter_Power_VddIO        "VddIO Power"
#define PP_STR_Counter_Power_VddNB        "VddNB Power"
#define PP_STR_Counter_Power_Vddp         "Vddp Power"
#define PP_STR_Counter_Power_VddGFX       "VddGfx Power"

#define PP_STR_Counter_Temp_CU0           "CPU CU0 Temp"
#define PP_STR_Counter_Temp_CU1           "CPU CU1 Temp"
#define PP_STR_Counter_MeasuredTemp_CU0   "CPU CU0 Measured Temp"
#define PP_STR_Counter_MeasuredTemp_CU1   "CPU CU1 Measured Temp"
#define PP_STR_Counter_Temp_IGPU          "iGPU Temp"
#define PP_STR_Counter_Temp_GFX           "GFX Temp"
#define PP_STR_Counter_Temp_DGPU          "dGPU Temp"
#define PP_STR_Counter_MeasuredTemp_IGPU  "iGPU Measured Temp"
#define PP_STR_Counter_MeasuredTemp_GFX   "GFX Measured Temp"
#define PP_STR_Counter_MeasuredTemp_DGPU  "dGPU Measured Temp"
#define PP_STR_Counter_MeasuredVoltage_DGPU  "dGPU Load Voltage"
#define PP_STR_Counter_MeasuredCurrent_DGPU  "dGPU Current"


#define PP_STR_FrequencyCounterPostfix  "Avg Frequency"
#define PP_STR_Counter_Freq_Core0       "CPU Core0 Frequency"
#define PP_STR_Counter_Freq_Core1       "CPU Core1 Frequency"
#define PP_STR_Counter_Freq_Core2       "CPU Core2 Frequency"
#define PP_STR_Counter_Freq_Core3       "CPU Core3 Frequency"
#define PP_STR_Counter_AvgFreq_Core0    "CPU Core0 " PP_STR_FrequencyCounterPostfix
#define PP_STR_Counter_AvgFreq_Core1    "CPU Core1 " PP_STR_FrequencyCounterPostfix
#define PP_STR_Counter_AvgFreq_Core2    "CPU Core2 " PP_STR_FrequencyCounterPostfix
#define PP_STR_Counter_AvgFreq_Core3    "CPU Core3 " PP_STR_FrequencyCounterPostfix
#define PP_STR_Counter_Freq_IGPU        "iGPU Frequency"
#define PP_STR_Counter_AvgFreq_IGPU     "iGPU " PP_STR_FrequencyCounterPostfix
#define PP_STR_Counter_AvgFreq_DGPU     "dGPU " PP_STR_FrequencyCounterPostfix
#define PP_STR_Counter_AvgFreq_GFX      "GFX Core " PP_STR_FrequencyCounterPostfix
#define PP_STR_Counter_AvgFreq_ACP      "ACP " PP_STR_FrequencyCounterPostfix
#define PP_STR_Counter_Ave_Freq_Core    "Core%1 Avg Frequency"


#define PP_STR_Counter_Voltage_CPUCore  "SVI2 CPU Cores"
#define PP_STR_Counter_Voltage_NB       "SVI2 NB"

#define PP_STR_Counter_Current_CPUCore  "SVI2 CPU Cores"
#define PP_STR_Counter_Current_NB       "SVI2 NB"

#define PP_STR_Counter_PState_Core0       "CPU Core0 P-State"
#define PP_STR_Counter_PState_Core1       "CPU Core1 P-State"
#define PP_STR_Counter_PState_Core2       "CPU Core2 P-State"
#define PP_STR_Counter_PState_Core3       "CPU Core3 P-State"
#define PP_STR_Counter_PState_Core        "Core%1 P-State"

#define PP_STR_Counter_C0Residency_CU0   "CPU CU0 C0 Residency"
#define PP_STR_Counter_C0Residency_CU1   "CPU CU1 C0 Residency"

#define PP_STR_Counter_C1Residency_CU0   "CPU CU0 C1 Residency"
#define PP_STR_Counter_C1Residency_CU1   "CPU CU1 C1 Residency"

#define PP_STR_Counter_CC6Residency_CU0   "CPU CU0 CC6 Residency"
#define PP_STR_Counter_CC6Residency_CU1   "CPU CU1 CC6 Residency"

#define PP_STR_Counter_PC6Residency_CU0   "CPU CU0 PC6 Residency"
#define PP_STR_Counter_PC6Residency_CU1   "CPU CU1 PC6 Residency"

#define PP_STR_Counter_Process_Id_Prefix  "Process Id"
#define PP_STR_Counter_Process_Id_C0      PP_STR_Counter_Process_Id_Prefix "-C0"
#define PP_STR_Counter_Process_Id_C1      PP_STR_Counter_Process_Id_Prefix "-C1"
#define PP_STR_Counter_Process_Id_C2      PP_STR_Counter_Process_Id_Prefix "-C2"
#define PP_STR_Counter_Process_Id_C3      PP_STR_Counter_Process_Id_Prefix "-C3"

#endif //__PPCOUNTERS_STRING_CONSTANTS_H
