//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SupportedDeviceList.h
///
//==================================================================================

#ifndef _SUPPORTEDDEVICELIST_H_
#include <AMDTPowerProfileInternal.h>
PciDeviceInfo g_deviceTable[PWR_MAX_DEVICE_LIST_SIZE] =
{
    // Kaveri -- will probably need multiple entries in g_deviceInfo for these
    { GDT_SPECTRE, 0x1304, DEVICE_TYPE_APU, "Spectre", "KV SPECTRE MOBILE 35W (1304)", "KV", SMU_IPVERSION_7_0}, // OK tested
    { GDT_SPECTRE, 0x1305, DEVICE_TYPE_APU, "Spectre", "KV SPECTRE DESKTOP 95W (1305)", "KV", SMU_IPVERSION_7_0},
    { GDT_SPECTRE_SL, 0x1306, DEVICE_TYPE_APU, "Spectre", "KV SPECTRE SL MOBILE 35W (1306)", "KV", SMU_IPVERSION_7_0},
    { GDT_SPECTRE_SL, 0x1307, DEVICE_TYPE_APU, "Spectre", "KV SPECTRE SL DESKTOP 95W (1307)", "KV", SMU_IPVERSION_7_0},
    { GDT_SPECTRE_LITE, 0x1309, DEVICE_TYPE_APU, "Spectre", "AMD Radeon(TM) R7 Graphics", "KV", SMU_IPVERSION_7_0},
    { GDT_SPECTRE_LITE, 0x130A, DEVICE_TYPE_APU, "Spectre", "AMD Radeon(TM) R6 Graphics", "KV", SMU_IPVERSION_7_0},
    { GDT_SPECTRE_SL, 0x130B, DEVICE_TYPE_APU, "Spectre", "AMD Radeon(TM) R4 Graphics", "KV", SMU_IPVERSION_7_0},
    { GDT_SPECTRE, 0x130C, DEVICE_TYPE_APU, "Spectre", "AMD Radeon(TM) R7 Graphics", "KV", SMU_IPVERSION_7_0},
    { GDT_SPECTRE_LITE, 0x130D, DEVICE_TYPE_APU, "Spectre", "AMD Radeon(TM) R6 Graphics", "KV", SMU_IPVERSION_7_0},
    { GDT_SPECTRE_SL, 0x130E, DEVICE_TYPE_APU, "Spectre", "AMD Radeon(TM) R5 Graphics", "KV", SMU_IPVERSION_7_0},
    { GDT_SPECTRE, 0x130F, DEVICE_TYPE_APU, "Spectre", "AMD Radeon(TM) R7 Graphics", "KV", SMU_IPVERSION_7_0},
    { GDT_SPECTRE, 0x1310, DEVICE_TYPE_APU,  "Spectre", "KV SPECTRE WORKSTATION 65W (1310)", "KV", SMU_IPVERSION_7_0},
    { GDT_SPECTRE, 0x1311, DEVICE_TYPE_APU,  "Spectre", "KV SPECTRE WORKSTATION 95W (1311)", "KV", SMU_IPVERSION_7_0},
    { GDT_SPECTRE_LITE, 0x1313, DEVICE_TYPE_APU,  "Spectre", "AMD Radeon(TM) R7 Graphics", "KV", SMU_IPVERSION_7_0},
    { GDT_SPECTRE_SL, 0x1315, DEVICE_TYPE_APU,  "Spectre", "AMD Radeon(TM) R5 Graphics", "KV", SMU_IPVERSION_7_0},
    { GDT_SPECTRE_SL, 0x1318, DEVICE_TYPE_APU,  "Spectre", "AMD Radeon(TM) R5 Graphics", "KV", SMU_IPVERSION_7_0},
    { GDT_SPECTRE_SL, 0x131B, DEVICE_TYPE_APU,  "Spectre", "AMD Radeon(TM) R4 Graphics", "KV", SMU_IPVERSION_7_0},
    { GDT_SPECTRE, 0x131C, DEVICE_TYPE_APU,  "Spectre", "AMD Radeon(TM) R7 Graphics", "KV", SMU_IPVERSION_7_0},
    { GDT_SPECTRE_LITE, 0x131D, DEVICE_TYPE_APU,  "Spectre", "AMD Radeon(TM) R6 Graphics", "KV", SMU_IPVERSION_7_0},
    { GDT_SPOOKY, 0x1312, DEVICE_TYPE_APU,  "Spooky", "KV SPOOKY DESKTOP 95W (1312)", "Fusion", SMU_IPVERSION_7_0},
    { GDT_SPOOKY, 0x1316, DEVICE_TYPE_APU,  "Spooky", "AMD Radeon(TM) R5 Graphics", "Fusion", SMU_IPVERSION_7_0},
    { GDT_SPOOKY, 0x1317, DEVICE_TYPE_APU,  "Spooky", "KV SPOOKY MOBILE 35W (1317)", "Fusion", SMU_IPVERSION_7_0},

    // Temash
    { GDT_KALINDI, 0x9839, DEVICE_TYPE_APU,  "Kalindi", "AMD Radeon HD 8180", "FT3 socket", SMU_IPVERSION_7_0 },
    { GDT_KALINDI, 0x983A, DEVICE_TYPE_APU,  "Kalindi", "Not Used", "FT3 socket", SMU_IPVERSION_7_0},
    { GDT_KALINDI, 0x983B, DEVICE_TYPE_APU,  "Kalindi", "Not Used", "FT3 socket", SMU_IPVERSION_7_0},
    { GDT_KALINDI, 0x983C, DEVICE_TYPE_APU,  "Kalindi", "Not Used", "FT3 socket", SMU_IPVERSION_7_0},
    { GDT_KALINDI, 0x983D, DEVICE_TYPE_APU,  "Kalindi", "AMD Radeon HD 8250", "FT3 socket", SMU_IPVERSION_7_0 },
    { GDT_KALINDI, 0x983E, DEVICE_TYPE_APU,  "Kalindi", "Not Used", "FT3 socket", SMU_IPVERSION_7_0},
    { GDT_KALINDI, 0x983F, DEVICE_TYPE_APU,  "Kalindi", "Not Used", "FT3 socket", SMU_IPVERSION_7_0},

    // Godavari
    { GDT_SPECTRE, 0x1422, DEVICE_TYPE_APU,  "Spectre", "Kaveri", "GV", SMU_IPVERSION_7_0},

    // Beema
    { GDT_KALINDI, 0x9850, DEVICE_TYPE_APU,  "Mullins", "AMD Radeon(TM) R3 Graphics", "ML", SMU_IPVERSION_7_0},
    { GDT_KALINDI, 0x9851, DEVICE_TYPE_APU,  "Mullins", "AMD Radeon(TM) R4 Graphics", "ML", SMU_IPVERSION_7_0},
    { GDT_KALINDI, 0x9852, DEVICE_TYPE_APU,  "Mullins", "AMD Radeon(TM) R2 Graphics", "ML", SMU_IPVERSION_7_0},
    { GDT_KALINDI, 0x9853, DEVICE_TYPE_APU,  "Mullins", "AMD Radeon(TM) R2 Graphics", "ML", SMU_IPVERSION_7_0},
    { GDT_KALINDI, 0x9854, DEVICE_TYPE_APU,  "Mullins", "AMD Radeon(TM) R3 Graphics", "ML", SMU_IPVERSION_7_0},
    { GDT_KALINDI, 0x9855, DEVICE_TYPE_APU,  "Mullins", "AMD Radeon(TM) R6 Graphics", "ML", SMU_IPVERSION_7_0},
    { GDT_KALINDI, 0x9856, DEVICE_TYPE_APU,  "Mullins", "AMD Radeon(TM) R2 Graphics", "ML", SMU_IPVERSION_7_0},
    { GDT_KALINDI, 0x9857, DEVICE_TYPE_APU,  "Mullins", "AMD Radeon APU XX-2200M with R2 Graphics", "ML", SMU_IPVERSION_7_0},
    { GDT_KALINDI, 0x9858, DEVICE_TYPE_APU,  "Mullins", "MULLINS (9858)", "ML", SMU_IPVERSION_7_0},
    { GDT_KALINDI, 0x9859, DEVICE_TYPE_APU,  "Mullins", "MULLINS (9859)", "ML", SMU_IPVERSION_7_0},
    { GDT_KALINDI, 0x985A, DEVICE_TYPE_APU,  "Mullins", "MULLINS (985A)", "ML", SMU_IPVERSION_7_0},
    { GDT_KALINDI, 0x985B, DEVICE_TYPE_APU,  "Mullins", "MULLINS (985B)", "ML", SMU_IPVERSION_7_0},
    { GDT_KALINDI, 0x985C, DEVICE_TYPE_APU,  "Mullins", "MULLINS (985C)", "ML", SMU_IPVERSION_7_0},
    { GDT_KALINDI, 0x985D, DEVICE_TYPE_APU,  "Mullins", "MULLINS (985D)", "ML", SMU_IPVERSION_7_0},
    { GDT_KALINDI, 0x985E, DEVICE_TYPE_APU,  "Mullins", "MULLINS (985E)", "ML", SMU_IPVERSION_7_0},
    { GDT_KALINDI, 0x985F, DEVICE_TYPE_APU,  "Mullins", "MULLINS (985F)", "ML", SMU_IPVERSION_7_0},

    // Carrizo -- OCL uses "Peacock" on mainline currently
    { GDT_CARRIZO, 0x9870, DEVICE_TYPE_APU, "Carrizo Palladium", "CARRIZO 9870", "CZ", SMU_IPVERSION_8_0},
    { GDT_CARRIZO, 0x9874, DEVICE_TYPE_APU, "Carrizo 9874", "CARRIZO 9874", "CZ", SMU_IPVERSION_8_0},
    { GDT_CARRIZO, 0x9875, DEVICE_TYPE_APU, "Carrizo 9875", "CARRIZO 9875", "CZ", SMU_IPVERSION_8_0},
    { GDT_CARRIZO, 0x9876, DEVICE_TYPE_APU, "Carrizo 9876", "CARRIZO 9876", "CZ", SMU_IPVERSION_8_0},
    { GDT_CARRIZO, 0x9877, DEVICE_TYPE_APU, "Carrizo 9877", "CARRIZO 9877", "CZ", SMU_IPVERSION_8_0},

    // Saturn (mobile Bonaire)
    { GDT_BONAIRE, 0x6640, DEVICE_TYPE_DGPU, "Bonaire", "AMD Radeon HD 8950", "Saturn XT", SMU_IPVERSION_7_0},
    { GDT_BONAIRE, 0x6641, DEVICE_TYPE_DGPU, "Bonaire", "SATURN (6641)", "Saturn PRO", SMU_IPVERSION_7_0},
    { GDT_BONAIRE, 0x6646, DEVICE_TYPE_DGPU, "Bonaire", "AMD Radeon R9 M280X", "Emerald XT", SMU_IPVERSION_7_0},
    { GDT_BONAIRE, 0x6647, DEVICE_TYPE_DGPU, "Bonaire", "AMD Radeon R9 M270X", "Emerald PRO", SMU_IPVERSION_7_0},
    // Bonaire
    { GDT_BONAIRE, 0x6649, DEVICE_TYPE_DGPU, "Bonaire", "AMD FirePro W5100", "Bonaire GL Pro", SMU_IPVERSION_7_0}, // 0x6640 - 0x665F Bonaire GL Pro
    { GDT_BONAIRE, 0x6650, DEVICE_TYPE_DGPU, "Bonaire", "BONAIRE (6650)", "Bonaire XT", SMU_IPVERSION_7_0}, // 0x6640 - 0x665F Bonaire XT
    { GDT_BONAIRE, 0x6651, DEVICE_TYPE_DGPU, "Bonaire", "BONAIRE (6651)", "Bonaire Pro", SMU_IPVERSION_7_0}, // 0x6640 - 0x665F Bonaire Pro
    { GDT_BONAIRE, 0x6658, DEVICE_TYPE_DGPU, "Bonaire", "AMD Radeon R7 200 Series", "Bonaire XTX", SMU_IPVERSION_7_0}, // 0x6640 - 0x665F Bonaire XT
    { GDT_BONAIRE, 0x665C, DEVICE_TYPE_DGPU, "Bonaire", "AMD Radeon HD 7700 Series", "Bonaire XT", SMU_IPVERSION_7_0}, // 0x6640 - 0x665F Bonaire XT
    { GDT_BONAIRE, 0x665D, DEVICE_TYPE_DGPU, "Bonaire", "AMD Radeon R7 200 Series", "Bonaire Pro", SMU_IPVERSION_7_0}, // 0x6640 - 0x665F Bonaire Pro
    // Hawaii
    { GDT_HAWAII, 0x66A0, DEVICE_TYPE_DGPU, "Hawaii", "HAWAII XTGL (67A0)", "Obsolete", SMU_IPVERSION_7_1},
    { GDT_HAWAII, 0x67A0, DEVICE_TYPE_DGPU, "Hawaii", "HAWAII XTGL (67A0)", "Hawaii GL44", SMU_IPVERSION_7_1},
    { GDT_HAWAII, 0x67A1, DEVICE_TYPE_DGPU, "Hawaii", "HAWAII GL40 (67A1)", "Hawaii GL40", SMU_IPVERSION_7_1},
    { GDT_HAWAII, 0x67A2, DEVICE_TYPE_DGPU, "Hawaii", "HAWAII GL Gemini (67A2)", "Hawaii GL Gemini", SMU_IPVERSION_7_1},
    { GDT_HAWAII, 0x67A8, DEVICE_TYPE_DGPU, "Hawaii", "", "Hawaii GL XT", SMU_IPVERSION_7_1},
    { GDT_HAWAII, 0x67A9, DEVICE_TYPE_DGPU, "Hawaii", "", "Hawaii GL Gemini", SMU_IPVERSION_7_1},
    { GDT_HAWAII, 0x67AA, DEVICE_TYPE_DGPU, "Hawaii", "", "Hawaii GL Pro", SMU_IPVERSION_7_1},
    { GDT_HAWAII, 0x67B0, DEVICE_TYPE_DGPU, "Hawaii", "AMD Radeon R9 200 Series", "Hawaii XT", SMU_IPVERSION_7_1},
    { GDT_HAWAII, 0x67B1, DEVICE_TYPE_DGPU, "Hawaii", "AMD Radeon R9 200 Series", "Hawaii Pro", SMU_IPVERSION_7_1},
    { GDT_HAWAII, 0x67B8, DEVICE_TYPE_DGPU, "Hawaii", "Not used", "Obsolete", SMU_IPVERSION_7_1},
    { GDT_HAWAII, 0x67B9, DEVICE_TYPE_DGPU, "Hawaii", "AMD Radeon R9 200 Series", "Hawaii Gemini", SMU_IPVERSION_7_1},
    { GDT_HAWAII, 0x67BA, DEVICE_TYPE_DGPU, "Hawaii", "Not used", "Obsolete", SMU_IPVERSION_7_1},
    { GDT_HAWAII, 0x67BE, DEVICE_TYPE_DGPU, "Hawaii", "HAWAII LE (67BE)", "Hawaii LE", SMU_IPVERSION_7_1},



    // Iceland/Topaz -disabling all Topaz series as we are getting invalid values
    //{ GDT_ICELAND, 0x6900, DEVICE_TYPE_DGPU, "Iceland", "AMD Radeon R7 M260", "Topaz XT", SMU_IPVERSION_7_1},
    //{ GDT_ICELAND, 0x6901, DEVICE_TYPE_DGPU, "Iceland", "AMD Radeon R5 M255", "Topaz PRO", SMU_IPVERSION_7_1},
    //{ GDT_ICELAND, 0x6902, DEVICE_TYPE_DGPU, "Iceland", "AMD Radeon Series", "Topaz XTL", SMU_IPVERSION_7_1},
    //{ GDT_ICELAND, 0x6903, DEVICE_TYPE_DGPU, "Iceland", "Not Used", "Unused - Previous Topaz LE", SMU_IPVERSION_7_1},
    //{ GDT_ICELAND, 0x6907, DEVICE_TYPE_DGPU, "Iceland", "AMD Radeon R5 M255", "Topaz LE", SMU_IPVERSION_7_1},

    // Tonga
    { GDT_TONGA, 0x6920, DEVICE_TYPE_DGPU, "Tonga", "Not Used", "unfused", SMU_IPVERSION_7_1 },
    { GDT_TONGA, 0x6921, DEVICE_TYPE_DGPU, "Tonga", "AMD Radeon R9 M295X", "Amethyst XT", SMU_IPVERSION_7_1}, // OK tested
    { GDT_TONGA, 0x6928, DEVICE_TYPE_DGPU, "Tonga", "", "Tonga GL XT", SMU_IPVERSION_7_1},
    { GDT_TONGA, 0x6929, DEVICE_TYPE_DGPU, "Tonga", "TONGA GL32 PRO (6929)", "Tonga GL32 PRO", SMU_IPVERSION_7_1},
    { GDT_TONGA, 0x692B, DEVICE_TYPE_DGPU, "Tonga", "AMD FirePro W7100", "Tonga GL PRO", SMU_IPVERSION_7_1},
    { GDT_TONGA, 0x692F, DEVICE_TYPE_DGPU, "Tonga", "AMD FirePro W7100", "Tonga GL PRO VF", SMU_IPVERSION_7_1},
    { GDT_TONGA, 0x6938, DEVICE_TYPE_DGPU, "Tonga", "AMD Radeon R9 200 Series", "Tonga XT", SMU_IPVERSION_7_1},
    { GDT_TONGA, 0x6939, DEVICE_TYPE_DGPU, "Tonga", "AMD Radeon R9 200 Series", "Tonga PRO", SMU_IPVERSION_7_1},
    { GDT_FIJI, 0x7300, DEVICE_TYPE_DGPU, "Fiji", "AMD Radeon Graphics Processor", "Fiji DID", SMU_IPVERSION_7_1},

    // Stoney
    { GDT_STONEY, 0x98E4, DEVICE_TYPE_APU, "Stoney", "AMD Radeon Series", "STONEY", SMU_IPVERSION_8_1},
    { GDT_INVALID, 0xFFFF, DEVICE_TYPE_OTHERS, "Reserver", "Reserved", "", SMU_IPVERSION_INVALID},
    { GDT_INVALID, 0xFFFF, DEVICE_TYPE_OTHERS, "Reserver", "Reserved", "", SMU_IPVERSION_INVALID},
    { GDT_INVALID, 0xFFFF, DEVICE_TYPE_OTHERS, "Reserver", "Reserved", "", SMU_IPVERSION_INVALID},
    { GDT_INVALID, 0xFFFF, DEVICE_TYPE_OTHERS, "Reserver", "Reserved", "", SMU_IPVERSION_INVALID},
    { GDT_INVALID, 0xFFFF, DEVICE_TYPE_OTHERS, "Reserver", "Reserved", "", SMU_IPVERSION_INVALID},
    { GDT_INVALID, 0xFFFF, DEVICE_TYPE_OTHERS, "Reserver", "Reserved", "", SMU_IPVERSION_INVALID},
    { GDT_INVALID, 0xFFFF, DEVICE_TYPE_OTHERS, "Reserver", "Reserved", "", SMU_IPVERSION_INVALID},
    { GDT_INVALID, 0xFFFF, DEVICE_TYPE_OTHERS, "Reserver", "Reserved", "", SMU_IPVERSION_INVALID},
    { GDT_INVALID, 0xFFFF, DEVICE_TYPE_OTHERS, "Reserver", "Reserved", "", SMU_IPVERSION_INVALID},
    { GDT_INVALID, 0xFFFF, DEVICE_TYPE_OTHERS, "Reserver", "Reserved", "", SMU_IPVERSION_INVALID},
    { GDT_INVALID, 0xFFFF, DEVICE_TYPE_OTHERS, "Reserver", "Reserved", "", SMU_IPVERSION_INVALID},
    { GDT_INVALID, 0xFFFF, DEVICE_TYPE_OTHERS, "Reserver", "Reserved", "", SMU_IPVERSION_INVALID},
    { GDT_INVALID, 0xFFFF, DEVICE_TYPE_OTHERS, "Reserver", "Reserved", "", SMU_IPVERSION_INVALID},
    { GDT_INVALID, 0xFFFF, DEVICE_TYPE_OTHERS, "Reserver", "Reserved", "", SMU_IPVERSION_INVALID},
    { GDT_INVALID, 0xFFFF, DEVICE_TYPE_OTHERS, "Reserver", "Reserved", "", SMU_IPVERSION_INVALID},
    { GDT_INVALID, 0xFFFF, DEVICE_TYPE_OTHERS, "Reserver", "Reserved", "", SMU_IPVERSION_INVALID},
};

PlatformInfo g_platformTable[] =
{
    // Kaveri -- will probably need multiple entries in g_deviceInfo for these
    { GDT_OROCHI, 0x15, 0x0, 0xf, DEVICE_TYPE_CPU_NO_SMU, "Orochi", "FX series"},
    { GDT_SPECTRE, 0x15, 0x30, 0x3f, DEVICE_TYPE_NPU_NO_SMU, "Spectre ", "Kaveri"},
};
#endif //_SUPPORTEDDEVICELIST_H_
