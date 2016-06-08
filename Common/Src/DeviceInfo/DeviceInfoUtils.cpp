//==============================================================================
// Copyright (c) 2010-2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Device info utils class
//==============================================================================

#ifdef _WIN32
    #include <windows.h>
#endif
#ifdef _LINUX
    #include <dlfcn.h>
#endif

#include "DeviceInfoUtils.h"

using namespace std;

static GDT_GfxCardInfo gs_cardInfo[] =
{
    { GDT_TAHITI_XT,  0x6790, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Tahiti", "Not Used" },
    { GDT_TAHITI_PRO, 0x6792, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Tahiti", "Not Used" },
    { GDT_TAHITI_XT,  0x6798, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Tahiti", "AMD Radeon R9 200 / HD 7900 Series" },
    { GDT_TAHITI_XT,  0x6799, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Tahiti", "AMD Radeon HD 7900 Series" },
    { GDT_TAHITI_PRO, 0x679A, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Tahiti", "AMD Radeon HD 7900 Series" },
    { GDT_TAHITI_PRO, 0x679B, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Tahiti", "AMD Radeon HD 7900 Series" },
    { GDT_TAHITI_PRO, 0x679E, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Tahiti", "AMD Radeon HD 7800 Series" },
    { GDT_TAHITI_XT,  0x6780, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Tahiti", "AMD FirePro W9000" },
    { GDT_TAHITI_PRO, 0x6784, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Tahiti", "ATI FirePro V (FireGL V) Graphics Adapter" },
    { GDT_TAHITI_XT,  0x6788, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Tahiti", "ATI FirePro V (FireGL V) Graphics Adapter" },
    { GDT_TAHITI_PRO, 0x678A, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Tahiti", "AMD FirePro W8000" },

    { GDT_PITCAIRN_XT,  0x6818, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Pitcairn", "AMD Radeon HD 7800 Series" },
    { GDT_PITCAIRN_PRO, 0x6819, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Pitcairn", "AMD Radeon HD 7800 Series" },
    { GDT_PITCAIRN_XT,  0x6808, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Pitcairn", "AMD FirePro W7000" },
    { GDT_PITCAIRN_XT,  0x6809, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Pitcairn", "ATI FirePro W5000" },
    { GDT_PITCAIRN_XT,  0x684C, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Pitcairn", "ATI FirePro V(FireGL V) Graphics Adapter" },
    { GDT_PITCAIRN_XT,  0x6800, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Pitcairn", "AMD Radeon HD 7970M" },
    { GDT_PITCAIRN_PRO, 0x6801, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Pitcairn", "AMD Radeon(TM) HD8970M" },
    { GDT_PITCAIRN_XT,  0x6806, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Pitcairn", "AMD Radeon (TM) R9 M290X" },
    { GDT_PITCAIRN_XT,  0x6810, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Pitcairn", "AMD Radeon R9 200 Series" },
    { GDT_PITCAIRN_XT,  0x6810, 0x81, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Pitcairn", "AMD Radeon (TM) R9 370 Series" },
    { GDT_PITCAIRN_PRO, 0x6811, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Pitcairn", "AMD Radeon R9 200 Series" },
    { GDT_PITCAIRN_PRO, 0x6811, 0x81, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Pitcairn", "AMD Radeon (TM) R7 370 Series" },

    { GDT_CAPEVERDE_XT,  0x6820, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Capeverde", "AMD Radeon R9 M275X" },
    { GDT_CAPEVERDE_XT,  0x6820, 0x81, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Capeverde", "AMD Radeon (TM) R9 M375" },
    { GDT_CAPEVERDE_XT,  0x6820, 0x83, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Capeverde", "AMD Radeon (TM) R9 M375X" },
    { GDT_CAPEVERDE_XT,  0x6821, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Capeverde", "AMD Radeon R9 M200X Series" },
    { GDT_CAPEVERDE_XT,  0x6821, 0x83, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Capeverde", "AMD Radeon R9 (TM) M370X" },
    { GDT_CAPEVERDE_XT,  0x6821, 0x87, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Capeverde", "AMD Radeon (TM) R7 M380" },
    { GDT_CAPEVERDE_XT,  0x6821, 0x8B, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Capeverde", "DID:6821 RID:8B" },
    { GDT_CAPEVERDE_PRO, 0x6822, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Capeverde", "AMD Radeon E8860" },
    { GDT_CAPEVERDE_PRO, 0x6823, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Capeverde", "AMD Radeon R9 M200X Series" },
    { GDT_CAPEVERDE_PRO, 0x6824, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Capeverde", "Not Used" },
    { GDT_CAPEVERDE_XT,  0x6825, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Capeverde", "AMD Radeon HD 7800M Series" },
    { GDT_CAPEVERDE_PRO, 0x6826, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Capeverde", "AMD Radeon HD 7700M Series" },
    { GDT_CAPEVERDE_PRO, 0x6827, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Capeverde", "AMD Radeon HD 7800M Series" },
    { GDT_CAPEVERDE_PRO, 0x682A, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Capeverde", "VENUS PRO MCM (682A)" },
    { GDT_CAPEVERDE_XT,  0x682B, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Capeverde", "AMD Radeon HD 8800M Series" },
    { GDT_CAPEVERDE_XT,  0x682B, 0x87, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Capeverde", "AMD Radeon (TM) R9 M360" },
    { GDT_CAPEVERDE_XT,  0x682D, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Capeverde", "AMD Radeon HD 7700M Series" },
    { GDT_CAPEVERDE_PRO, 0x682F, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Capeverde", "AMD Radeon HD 7700M Series" },

    { GDT_CAPEVERDE_XT,  0x6828, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Capeverde", "AMD FirePro W600" },
    { GDT_CAPEVERDE_PRO, 0x682C, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Capeverde", "AMD FirePro W4100" },
    { GDT_CAPEVERDE_XT,  0x6830, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Capeverde", "AMD Radeon 7800M Series" },
    { GDT_CAPEVERDE_XT,  0x6831, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Capeverde", "AMD Radeon 7700M Series" },
    { GDT_CAPEVERDE_PRO, 0x6835, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Capeverde", "AMD Radeon R7 Series / HD 9000 Series" },
    { GDT_CAPEVERDE_XT,  0x6837, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Capeverde", "AMD Radeon HD 7700 Series" },
    { GDT_CAPEVERDE_XT,  0x6838, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Capeverde", "Not Used" },
    { GDT_CAPEVERDE_XT,  0x6839, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Capeverde", "Not Used" },
    { GDT_CAPEVERDE_PRO, 0x683B, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Capeverde", "Not Used" },
    { GDT_CAPEVERDE_XT,  0x683D, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Capeverde", "AMD Radeon HD 7700 Series" },
    { GDT_CAPEVERDE_PRO, 0x683F, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Capeverde", "AMD Radeon HD 7700 Series" },

    // Oland
    { GDT_OLAND, 0x6608, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Oland", "AMD FirePro W2100" },
    { GDT_OLAND, 0x6610, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Oland", "AMD Radeon R7 200 Series" },
    { GDT_OLAND, 0x6610, 0x81, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Oland", "AMD Radeon (TM) R7 350" },
    { GDT_OLAND, 0x6610, 0x83, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Oland", "AMD Radeon (TM) R5 340" },
    { GDT_OLAND, 0x6610, 0x87, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Oland", "AMD Radeon R7 200 Series" },
    { GDT_OLAND, 0x6611, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Oland", "AMD Radeon R7 200 Series" },
    { GDT_OLAND, 0x6611, 0x87, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Oland", "AMD Radeon R7 200 Series" },
    { GDT_OLAND, 0x6613, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Oland", "AMD Radeon R7 200 Series" },
    { GDT_OLAND, 0x6617, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Oland", "AMD Radeon R7 240 Series" },
    { GDT_OLAND, 0x6617, 0x87, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Oland", "AMD Radeon R7 200 Series" },
    { GDT_OLAND, 0x6617, 0xC7, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Oland", "AMD Radeon R7 240 Series" },
    { GDT_OLAND, 0x6631, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Oland", "OLAND (6631)" },

    // Mars (Mobile Oland)
    { GDT_OLAND, 0x6600, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Oland", "AMD Radeon HD 8600/8700M" },
    { GDT_OLAND, 0x6600, 0x81, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Oland", "AMD Radeon (TM) R7 M370" },
    { GDT_OLAND, 0x6601, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Oland", "AMD Radeon (TM) HD 8500M/8700M" },
    { GDT_OLAND, 0x6602, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Oland", "MARS (6602)" },
    { GDT_OLAND, 0x6603, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Oland", "MARS (6603)" },
    { GDT_OLAND, 0x6604, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Oland", "AMD Radeon R7 M265 Series" },
    { GDT_OLAND, 0x6604, 0x81, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Oland", "AMD Radeon (TM) R7 M350" },
    { GDT_OLAND, 0x6605, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Oland", "AMD Radeon R7 M260 Series" },
    { GDT_OLAND, 0x6605, 0x81, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Oland", "AMD Radeon (TM) R7 M340" },
    { GDT_OLAND, 0x6606, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Oland", "AMD Radeon HD 8790M" },
    { GDT_OLAND, 0x6607, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Oland", "AMD Radeon R5 M240" },
    { GDT_OLAND, 0x6620, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Oland", "MARS (6620)" },
    { GDT_OLAND, 0x6621, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Oland", "MARS (6621)" },
    { GDT_OLAND, 0x6623, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Oland", "MARS (6623)" },

    // Hainan
    { GDT_HAINAN, 0x6660, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Hainan", "AMD Radeon HD 8600M Series" },
    { GDT_HAINAN, 0x6660, 0x81, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Hainan", "AMD Radeon (TM) R5 M335" },
    { GDT_HAINAN, 0x6660, 0x83, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Hainan", "AMD Radeon (TM) R5 M330" },
    { GDT_HAINAN, 0x6663, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Hainan", "AMD Radeon HD 8500M Series" },
    { GDT_HAINAN, 0x6663, 0x83, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Hainan", "AMD Radeon (TM) R5 M320" },
    { GDT_HAINAN, 0x6664, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Hainan", "AMD Radeon R5 M200 Series" },
    { GDT_HAINAN, 0x6665, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Hainan", "AMD Radeon R5 M230 Series" },
    { GDT_HAINAN, 0x6665, 0x83, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Hainan", "AMD Radeon (TM) R5 M320" },
    { GDT_HAINAN, 0x6665, 0xC3, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Hainan", "AMD Radeon R5 M430" },
    { GDT_HAINAN, 0x6666, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Hainan", "AMD Radeon R5 M200 Series" },
    { GDT_HAINAN, 0x6667, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Hainan", "AMD Radeon R5 M200 Series" },
    { GDT_HAINAN, 0x6667, 0x83, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Hainan", "EXO ULP (6667)" },
    { GDT_HAINAN, 0x666F, 0x00, GDT_HW_GENERATION_SOUTHERNISLAND, false, "Hainan", "AMD Radeon HD 8500M" },

    // Bonaire
    { GDT_BONAIRE, 0x6649, 0x00, GDT_HW_GENERATION_SEAISLAND, false, "Bonaire", "AMD FirePro W5100" },
    { GDT_BONAIRE, 0x6650, 0x00, GDT_HW_GENERATION_SEAISLAND, false, "Bonaire", "BONAIRE (6650)" },
    { GDT_BONAIRE, 0x6651, 0x00, GDT_HW_GENERATION_SEAISLAND, false, "Bonaire", "BONAIRE (6651)" },
    { GDT_BONAIRE, 0x6658, 0x00, GDT_HW_GENERATION_SEAISLAND, false, "Bonaire", "AMD Radeon R7 200 Series" },
    { GDT_BONAIRE, 0x665C, 0x00, GDT_HW_GENERATION_SEAISLAND, false, "Bonaire", "AMD Radeon HD 7700 Series" },
    { GDT_BONAIRE, 0x665D, 0x00, GDT_HW_GENERATION_SEAISLAND, false, "Bonaire", "AMD Radeon R7 200 Series" },
    { GDT_BONAIRE, 0x665F, 0x81, GDT_HW_GENERATION_SEAISLAND, false, "Bonaire", "AMD Radeon (TM) R7 360 Series" },
    { GDT_BONAIRE, 0x665F, 0x00, GDT_HW_GENERATION_SEAISLAND, false, "Bonaire", "DID:665F RID:00" },
    { GDT_BONAIRE, 0x665F, 0x81, GDT_HW_GENERATION_SEAISLAND, false, "Bonaire", "AMD Radeon (TM) R7 360 Series" },

    // Saturn (mobile Bonaire)
    { GDT_BONAIRE, 0x6640, 0x00, GDT_HW_GENERATION_SEAISLAND, false, "Bonaire", "AMD Radeon HD 8950" },
    { GDT_BONAIRE, 0x6640, 0x80, GDT_HW_GENERATION_SEAISLAND, false, "Bonaire", "AMD Radeon (TM) R9 M380" },
    { GDT_BONAIRE, 0x6641, 0x00, GDT_HW_GENERATION_SEAISLAND, false, "Bonaire", "SATURN (6641)" },
    { GDT_BONAIRE, 0x6646, 0x00, GDT_HW_GENERATION_SEAISLAND, false, "Bonaire", "AMD Radeon R9 M280X" },
    { GDT_BONAIRE, 0x6646, 0x80, GDT_HW_GENERATION_SEAISLAND, false, "Bonaire", "AMD Radeon (TM) R9 M385" },
    { GDT_BONAIRE, 0x6647, 0x00, GDT_HW_GENERATION_SEAISLAND, false, "Bonaire", "AMD Radeon R9 M200X Series" },
    { GDT_BONAIRE, 0x6647, 0x80, GDT_HW_GENERATION_SEAISLAND, false, "Bonaire", "AMD Radeon (TM) R9 M380" },

    // Hawaii
    { GDT_HAWAII, 0x66A0, 0x00, GDT_HW_GENERATION_SEAISLAND, false, "Hawaii", "HAWAII XTGL (67A0)" },
    { GDT_HAWAII, 0x67A0, 0x00, GDT_HW_GENERATION_SEAISLAND, false, "Hawaii", "AMD FirePro W9100" },
    { GDT_HAWAII, 0x67A1, 0x00, GDT_HW_GENERATION_SEAISLAND, false, "Hawaii", "AMD FirePro W8100" },
    { GDT_HAWAII, 0x67A2, 0x00, GDT_HW_GENERATION_SEAISLAND, false, "Hawaii", "HAWAII GL Gemini (67A2)" },
    { GDT_HAWAII, 0x67A8, 0x00, GDT_HW_GENERATION_SEAISLAND, false, "Hawaii", "" },
    { GDT_HAWAII, 0x67A9, 0x00, GDT_HW_GENERATION_SEAISLAND, false, "Hawaii", "" },
    { GDT_HAWAII, 0x67AA, 0x00, GDT_HW_GENERATION_SEAISLAND, false, "Hawaii", "" },
    { GDT_HAWAII, 0x67B0, 0x00, GDT_HW_GENERATION_SEAISLAND, false, "Hawaii", "AMD Radeon R9 200 Series" },
    { GDT_HAWAII, 0x67B0, 0x80, GDT_HW_GENERATION_SEAISLAND, false, "Hawaii", "AMD Radeon (TM) R9 390 Series" },
    { GDT_HAWAII, 0x67B1, 0x00, GDT_HW_GENERATION_SEAISLAND, false, "Hawaii", "AMD Radeon R9 200 Series" },
    { GDT_HAWAII, 0x67B1, 0x80, GDT_HW_GENERATION_SEAISLAND, false, "Hawaii", "AMD Radeon (TM) R9 390 Series" },
    { GDT_HAWAII, 0x67B8, 0x00, GDT_HW_GENERATION_SEAISLAND, false, "Hawaii", "Not used" },
    { GDT_HAWAII, 0x67B9, 0x00, GDT_HW_GENERATION_SEAISLAND, false, "Hawaii", "AMD Radeon R9 200 Series" },
    { GDT_HAWAII, 0x67BA, 0x00, GDT_HW_GENERATION_SEAISLAND, false, "Hawaii", "Not used" },
    { GDT_HAWAII, 0x67BE, 0x00, GDT_HW_GENERATION_SEAISLAND, false, "Hawaii", "HAWAII LE (67BE)" },

    // Kaveri -- will probably need multiple entries in g_deviceInfo for these
    { GDT_SPECTRE,      0x1304, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Spectre", "KV SPECTRE MOBILE 35W (1304)" },
    { GDT_SPECTRE,      0x1305, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Spectre", "KV SPECTRE DESKTOP 95W (1305)" },
    { GDT_SPECTRE_SL,   0x1306, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Spectre", "KV SPECTRE SL MOBILE 35W (1306)" },
    { GDT_SPECTRE_SL,   0x1307, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Spectre", "KV SPECTRE SL DESKTOP 95W (1307)" },
    { GDT_SPECTRE_LITE, 0x1309, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Spectre", "AMD Radeon(TM) R7 Graphics" },
    { GDT_SPECTRE_LITE, 0x130A, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Spectre", "AMD Radeon(TM) R6 Graphics" },
    { GDT_SPECTRE,      0x130C, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Spectre", "AMD Radeon(TM) R7 Graphics" },
    { GDT_SPECTRE_LITE, 0x130D, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Spectre", "AMD Radeon(TM) R6 Graphics" },
    { GDT_SPECTRE_SL,   0x130E, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Spectre", "AMD Radeon(TM) R5 Graphics" },
    { GDT_SPECTRE,      0x130F, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Spectre", "AMD Radeon(TM) R7 Graphics" },
    { GDT_SPECTRE,      0x130F, 0xD4, GDT_HW_GENERATION_SEAISLAND, true, "Spectre", "AMD Radeon(TM) R7 Graphics" },
    { GDT_SPECTRE,      0x130F, 0xD5, GDT_HW_GENERATION_SEAISLAND, true, "Spectre", "AMD Radeon(TM) R7 Graphics" },
    { GDT_SPECTRE,      0x130F, 0xD6, GDT_HW_GENERATION_SEAISLAND, true, "Spectre", "AMD Radeon(TM) R7 Graphics" },
    { GDT_SPECTRE,      0x130F, 0xD7, GDT_HW_GENERATION_SEAISLAND, true, "Spectre", "AMD Radeon(TM) R7 Graphics" },
    { GDT_SPECTRE,      0x1310, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Spectre", "KV SPECTRE WORKSTATION 65W (1310)" },
    { GDT_SPECTRE,      0x1311, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Spectre", "KV SPECTRE WORKSTATION 95W (1311)" },
    { GDT_SPECTRE_LITE, 0x1313, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Spectre", "AMD Radeon(TM) R7 Graphics" },
    { GDT_SPECTRE_LITE, 0x1313, 0xD4, GDT_HW_GENERATION_SEAISLAND, true, "Spectre", "AMD Radeon(TM) R7 Graphics" },
    { GDT_SPECTRE_LITE, 0x1313, 0xD5, GDT_HW_GENERATION_SEAISLAND, true, "Spectre", "AMD Radeon(TM) R7 Graphics" },
    { GDT_SPECTRE_LITE, 0x1313, 0xD6, GDT_HW_GENERATION_SEAISLAND, true, "Spectre", "AMD Radeon(TM) R7 Graphics" },
    { GDT_SPECTRE_SL,   0x1315, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Spectre", "AMD Radeon(TM) R5 Graphics" },
    { GDT_SPECTRE_SL,   0x1315, 0xD4, GDT_HW_GENERATION_SEAISLAND, true, "Spectre", "AMD Radeon(TM) R5 Graphics" },
    { GDT_SPECTRE_SL,   0x1315, 0xD5, GDT_HW_GENERATION_SEAISLAND, true, "Spectre", "AMD Radeon(TM) R5 Graphics" },
    { GDT_SPECTRE_SL,   0x1315, 0xD6, GDT_HW_GENERATION_SEAISLAND, true, "Spectre", "AMD Radeon(TM) R5 Graphics" },
    { GDT_SPECTRE_SL,   0x1315, 0xD7, GDT_HW_GENERATION_SEAISLAND, true, "Spectre", "AMD Radeon(TM) R5 Graphics" },
    { GDT_SPECTRE_SL,   0x1318, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Spectre", "AMD Radeon(TM) R5 Graphics" },
    { GDT_SPECTRE,      0x131C, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Spectre", "AMD Radeon(TM) R7 Graphics" },
    { GDT_SPECTRE_LITE, 0x131D, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Spectre", "AMD Radeon(TM) R6 Graphics" },
    { GDT_SPOOKY,       0x130B, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Spectre", "AMD Radeon(TM) R4 Graphics" },
    { GDT_SPOOKY,       0x1312, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Spooky", "KV SPOOKY DESKTOP 95W (1312)" },
    { GDT_SPOOKY,       0x1316, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Spooky", "AMD Radeon(TM) R5 Graphics" },
    { GDT_SPOOKY,       0x1317, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Spooky", "KV SPOOKY MOBILE 35W (1317)" },
    { GDT_SPOOKY,       0x131B, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Spectre", "AMD Radeon(TM) R4 Graphics" },

    // Kabini
    { GDT_KALINDI, 0x9830, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Kalindi", "AMD Radeon HD 8400 / R3 Series" },
    { GDT_KALINDI, 0x9831, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Kalindi", "AMD Radeon(TM) HD 8400E" },
    { GDT_KALINDI, 0x9832, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Kalindi", "AMD Radeon HD 8330" },
    { GDT_KALINDI, 0x9833, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Kalindi", "AMD Radeon(TM) HD 8330E" },
    { GDT_KALINDI, 0x9834, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Kalindi", "AMD Radeon HD 8210" },
    { GDT_KALINDI, 0x9835, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Kalindi", "AMD Radeon(TM) HD 8210E" },
    { GDT_KALINDI, 0x9836, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Kalindi", "AMD Radeon HD 8200 / R3 Series" },
    { GDT_KALINDI, 0x9837, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Kalindi", "AMD Radeon(TM) HD 8280E" },
    { GDT_KALINDI, 0x9838, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Kalindi", "AMD Radeon HD 8200 / R3 series" },

    // Temash
    { GDT_KALINDI, 0x9839, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Kalindi", "AMD Radeon HD 8180" },
    { GDT_KALINDI, 0x983A, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Kalindi", "Not Used" },
    { GDT_KALINDI, 0x983B, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Kalindi", "Not Used" },
    { GDT_KALINDI, 0x983C, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Kalindi", "Not Used" },
    { GDT_KALINDI, 0x983D, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Kalindi", "AMD Radeon HD 8250" },
    { GDT_KALINDI, 0x983E, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Kalindi", "Not Used" },
    { GDT_KALINDI, 0x983F, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Kalindi", "Not Used" },

    // Beema
    { GDT_KALINDI, 0x9850, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "AMD Radeon(TM) R3 Graphics" },
    { GDT_KALINDI, 0x9850, 0x03, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "AMD Radeon(TM) R3 Graphics" },
    { GDT_KALINDI, 0x9850, 0x40, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "AMD Radeon(TM) R2 Graphics" },
    { GDT_KALINDI, 0x9850, 0x45, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "AMD Radeon(TM) R3 Graphics" },
    { GDT_KALINDI, 0x9851, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "AMD Radeon(TM) R4 Graphics" },
    { GDT_KALINDI, 0x9851, 0x01, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "AMD Radeon(TM) R5E Graphics" },
    { GDT_KALINDI, 0x9851, 0x05, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "AMD Radeon(TM) R5 Graphics" },
    { GDT_KALINDI, 0x9851, 0x06, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "AMD Radeon(TM) R5E Graphics" },
    { GDT_KALINDI, 0x9851, 0x40, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "AMD Radeon(TM) R4 Graphics" },
    { GDT_KALINDI, 0x9851, 0x45, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "AMD Radeon(TM) R5 Graphics" },
    { GDT_KALINDI, 0x9852, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "AMD Radeon(TM) R2 Graphics" },
    { GDT_KALINDI, 0x9852, 0x40, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "AMD Radeon(TM) E1 Graphics" },
    { GDT_KALINDI, 0x9853, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "AMD Radeon(TM) R2 Graphics" },
    { GDT_KALINDI, 0x9853, 0x01, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "AMD Radeon(TM) R4E Graphics" },
    { GDT_KALINDI, 0x9853, 0x03, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "AMD Radeon(TM) R2 Graphics" },
    { GDT_KALINDI, 0x9853, 0x05, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "AMD Radeon(TM) R1E Graphics" },
    { GDT_KALINDI, 0x9853, 0x06, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "AMD Radeon(TM) R1E Graphics" },
    { GDT_KALINDI, 0x9853, 0x40, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "AMD Radeon(TM) R2 Graphics" },

    // Mullins
    { GDT_KALINDI, 0x9854, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "AMD Radeon(TM) R3 Graphics" },
    { GDT_KALINDI, 0x9854, 0x01, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "AMD Radeon(TM) R3E Graphics" },
    { GDT_KALINDI, 0x9854, 0x02, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "AMD Radeon(TM) R3 Graphics" },
    { GDT_KALINDI, 0x9854, 0x05, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "AMD Radeon(TM) R2 Graphics" },
    { GDT_KALINDI, 0x9854, 0x06, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "AMD Radeon(TM) R4 Graphics" },
    { GDT_KALINDI, 0x9854, 0x07, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "AMD Radeon(TM) R3 Graphics" },
    { GDT_KALINDI, 0x9855, 0x02, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "AMD Radeon(TM) R6 Graphics" },
    { GDT_KALINDI, 0x9855, 0x05, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "AMD Radeon(TM) R4 Graphics" },
    { GDT_KALINDI, 0x9856, 0x07, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "AMD Radeon(TM) R1E Graphics" },
    { GDT_KALINDI, 0x9856, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "AMD Radeon(TM) R2 Graphics" },
    { GDT_KALINDI, 0x9856, 0x01, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "AMD Radeon(TM) R2E Graphics" },
    { GDT_KALINDI, 0x9856, 0x02, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "AMD Radeon(TM) R2 Graphics" },
    { GDT_KALINDI, 0x9856, 0x05, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "AMD Radeon(TM) R1E Graphics" },
    { GDT_KALINDI, 0x9856, 0x06, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "AMD Radeon(TM) R2 Graphics" },
    { GDT_KALINDI, 0x9856, 0x07, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "AMD Radeon(TM) R1E Graphics" },
    { GDT_KALINDI, 0x9857, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "MULLINS (9857)" },
    { GDT_KALINDI, 0x9858, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "MULLINS (9858)" },
    { GDT_KALINDI, 0x9859, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "MULLINS (9859)" },
    { GDT_KALINDI, 0x985A, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "MULLINS (985A)" },
    { GDT_KALINDI, 0x985B, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "MULLINS (985B)" },
    { GDT_KALINDI, 0x985C, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "MULLINS (985C)" },
    { GDT_KALINDI, 0x985D, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "MULLINS (985D)" },
    { GDT_KALINDI, 0x985E, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "MULLINS (985E)" },
    { GDT_KALINDI, 0x985F, 0x00, GDT_HW_GENERATION_SEAISLAND, true, "Mullins", "MULLINS (985F)" },

    // Iceland/Topaz
    { GDT_ICELAND, 0x6900, 0x00, GDT_HW_GENERATION_VOLCANICISLAND, false, "Iceland", "AMD Radeon R7 M260" },
    { GDT_ICELAND, 0x6900, 0x81, GDT_HW_GENERATION_VOLCANICISLAND, false, "Iceland", "AMD Radeon (TM) R7 M360" },
    { GDT_ICELAND, 0x6900, 0x83, GDT_HW_GENERATION_VOLCANICISLAND, false, "Iceland", "AMD Radeon (TM) R7 M340" },
    { GDT_ICELAND, 0x6900, 0x87, GDT_HW_GENERATION_VOLCANICISLAND, false, "Iceland", "MESO UL (6900)" },
    { GDT_ICELAND, 0x6900, 0xC1, GDT_HW_GENERATION_VOLCANICISLAND, false, "Iceland", "AMD Radeon R5 M465 Series" },
    { GDT_ICELAND, 0x6900, 0xC3, GDT_HW_GENERATION_VOLCANICISLAND, false, "Iceland", "AMD Radeon R5 M445 Series" },
    { GDT_ICELAND, 0x6901, 0x00, GDT_HW_GENERATION_VOLCANICISLAND, false, "Iceland", "AMD Radeon R5 M255" },
    { GDT_ICELAND, 0x6902, 0x00, GDT_HW_GENERATION_VOLCANICISLAND, false, "Iceland", "AMD Radeon Series" },
    { GDT_ICELAND, 0x6903, 0x00, GDT_HW_GENERATION_VOLCANICISLAND, false, "Iceland", "Not Used" },
    { GDT_ICELAND, 0x6907, 0x00, GDT_HW_GENERATION_VOLCANICISLAND, false, "Iceland", "AMD Radeon R5 M255" },
    { GDT_ICELAND, 0x6907, 0x87, GDT_HW_GENERATION_VOLCANICISLAND, false, "Iceland", "AMD Radeon (TM) R5 M315" },

    // Tonga
    { GDT_TONGA, 0x6920, 0x00, GDT_HW_GENERATION_VOLCANICISLAND, false, "Tonga", "AMD RADEON R9 M395X" },
    { GDT_TONGA, 0x6920, 0x01, GDT_HW_GENERATION_VOLCANICISLAND, false, "Tonga", "AMD RADEON R9 M390X" },
    { GDT_TONGA, 0x6921, 0x00, GDT_HW_GENERATION_VOLCANICISLAND, false, "Tonga", "AMD Radeon (TM) R9 M390X" },
    //{ GDT_TONGA, 0x6928, 0x00, GDT_HW_GENERATION_VOLCANICISLAND, false, "Tonga", "" },
    { GDT_TONGA, 0x6929, 0x00, GDT_HW_GENERATION_VOLCANICISLAND, false, "Tonga", "AMD FirePro S7150" },
    { GDT_TONGA, 0x6929, 0x01, GDT_HW_GENERATION_VOLCANICISLAND, false, "Tonga", "AMD FirePro S7100" },
    { GDT_TONGA, 0x692B, 0x00, GDT_HW_GENERATION_VOLCANICISLAND, false, "Tonga", "AMD FirePro W7100" },
    { GDT_TONGA, 0x692F, 0x00, GDT_HW_GENERATION_VOLCANICISLAND, false, "Tonga", "AMD FirePro S7150VF" },
    { GDT_TONGA, 0x6930, 0xF0, GDT_HW_GENERATION_VOLCANICISLAND, false, "Tonga", "DID:6930 RID:F0" },
    { GDT_TONGA, 0x6930, 0xF1, GDT_HW_GENERATION_VOLCANICISLAND, false, "Tonga", "DID:6930 RID:F1" },
    { GDT_TONGA, 0x6930, 0xFF, GDT_HW_GENERATION_VOLCANICISLAND, false, "Tonga", "DID:6930 RID:FF" },
    { GDT_TONGA, 0x6938, 0x00, GDT_HW_GENERATION_VOLCANICISLAND, false, "Tonga", "AMD Radeon R9 200 Series" },
    { GDT_TONGA, 0x6938, 0xF1, GDT_HW_GENERATION_VOLCANICISLAND, false, "Tonga", "AMD Radeon (TM) R9 380 Series" },
    { GDT_TONGA, 0x6938, 0xF0, GDT_HW_GENERATION_VOLCANICISLAND, false, "Tonga", "AMD Radeon R9 200 Series" },
    { GDT_TONGA, 0x6939, 0x00, GDT_HW_GENERATION_VOLCANICISLAND, false, "Tonga", "AMD Radeon R9 200 Series" },
    { GDT_TONGA, 0x6939, 0xF0, GDT_HW_GENERATION_VOLCANICISLAND, false, "Tonga", "AMD Radeon R9 200 Series" },
    { GDT_TONGA, 0x6939, 0xF1, GDT_HW_GENERATION_VOLCANICISLAND, false, "Tonga", "AMD Radeon (TM) R9 380 Series" },

    // Carrizo
    { GDT_CARRIZO, 0x9870, 0x00, GDT_HW_GENERATION_VOLCANICISLAND, true, "Carrizo", "CARRIZO 9870" },
    { GDT_CARRIZO, 0x9874, 0x00, GDT_HW_GENERATION_VOLCANICISLAND, true, "Carrizo", "CARRIZO 9874" },
    { GDT_CARRIZO, 0x9874, 0xC4, GDT_HW_GENERATION_VOLCANICISLAND, true, "Carrizo", "AMD Radeon R7 Graphics" },
    { GDT_CARRIZO, 0x9874, 0xC5, GDT_HW_GENERATION_VOLCANICISLAND, true, "Carrizo", "AMD Radeon R6 Graphics" },
    { GDT_CARRIZO, 0x9874, 0xC6, GDT_HW_GENERATION_VOLCANICISLAND, true, "Carrizo", "AMD Radeon R6 Graphics" },
    { GDT_CARRIZO, 0x9874, 0xC7, GDT_HW_GENERATION_VOLCANICISLAND, true, "Carrizo", "AMD Radeon R5 Graphics" },
    { GDT_CARRIZO_EMB, 0x9874, 0x81, GDT_HW_GENERATION_VOLCANICISLAND, true, "Carrizo", "AMD Radeon R6 Graphics" },
    { GDT_CARRIZO_EMB, 0x9874, 0x84, GDT_HW_GENERATION_VOLCANICISLAND, true, "Carrizo", "AMD Radeon R7 Graphics" },
    { GDT_CARRIZO_EMB, 0x9874, 0x85, GDT_HW_GENERATION_VOLCANICISLAND, true, "Carrizo", "AMD Radeon R6 Graphics" },
    { GDT_CARRIZO_EMB, 0x9874, 0x87, GDT_HW_GENERATION_VOLCANICISLAND, true, "Carrizo", "AMD Radeon R5 Graphics" },
    { GDT_CARRIZO_EMB, 0x9874, 0x88, GDT_HW_GENERATION_VOLCANICISLAND, true, "Carrizo", "AMD Radeon R7E Graphics" },
    { GDT_CARRIZO_EMB, 0x9874, 0x89, GDT_HW_GENERATION_VOLCANICISLAND, true, "Carrizo", "AMD Radeon R6E Graphics" },
    { GDT_CARRIZO, 0x9874, 0xC8, GDT_HW_GENERATION_VOLCANICISLAND, true, "Carrizo", "AMD Radeon R7 Graphics" },
    { GDT_CARRIZO, 0x9874, 0xC9, GDT_HW_GENERATION_VOLCANICISLAND, true, "Carrizo", "AMD Radeon R7 Graphics" },
    { GDT_CARRIZO, 0x9874, 0xCA, GDT_HW_GENERATION_VOLCANICISLAND, true, "Carrizo", "AMD Radeon R5 Graphics" },
    { GDT_CARRIZO, 0x9874, 0xCB, GDT_HW_GENERATION_VOLCANICISLAND, true, "Carrizo", "AMD Radeon R5 Graphics" },
    { GDT_CARRIZO, 0x9874, 0xCC, GDT_HW_GENERATION_VOLCANICISLAND, true, "Carrizo", "AMD Radeon R7 Graphics" },
    { GDT_CARRIZO, 0x9874, 0xCD, GDT_HW_GENERATION_VOLCANICISLAND, true, "Carrizo", "AMD Radeon R7 Graphics" },
    { GDT_CARRIZO, 0x9874, 0xCE, GDT_HW_GENERATION_VOLCANICISLAND, true, "Carrizo", "AMD Radeon R5 Graphics" },
    { GDT_CARRIZO, 0x9874, 0xE1, GDT_HW_GENERATION_VOLCANICISLAND, true, "Carrizo", "AMD Radeon R7 Graphics" },
    { GDT_CARRIZO, 0x9874, 0xE2, GDT_HW_GENERATION_VOLCANICISLAND, true, "Carrizo", "AMD Radeon R7 Graphics" },
    { GDT_CARRIZO, 0x9874, 0xE3, GDT_HW_GENERATION_VOLCANICISLAND, true, "Carrizo", "AMD Radeon R7 Graphics" },
    { GDT_CARRIZO, 0x9874, 0xE4, GDT_HW_GENERATION_VOLCANICISLAND, true, "Carrizo", "AMD Radeon R7 Graphics" },
    { GDT_CARRIZO, 0x9874, 0xE5, GDT_HW_GENERATION_VOLCANICISLAND, true, "Carrizo", "AMD Radeon R5 Graphics" },
    { GDT_CARRIZO, 0x9874, 0xE6, GDT_HW_GENERATION_VOLCANICISLAND, true, "Carrizo", "AMD Radeon R5 Graphics" },

    { GDT_CARRIZO, 0x9875, 0x00, GDT_HW_GENERATION_VOLCANICISLAND, true, "Carrizo", "CARRIZO 9875" },
    { GDT_CARRIZO, 0x9876, 0x00, GDT_HW_GENERATION_VOLCANICISLAND, true, "Carrizo", "CARRIZO 9876" },
    { GDT_CARRIZO, 0x9877, 0x00, GDT_HW_GENERATION_VOLCANICISLAND, true, "Carrizo", "CARRIZO 9877" },

    // Fiji
    { GDT_FIJI, 0x7300, 0x00, GDT_HW_GENERATION_VOLCANICISLAND, false, "Fiji", "AMD Radeon (TM) Graphics Processor" },
    { GDT_FIJI, 0x7300, 0xC0, GDT_HW_GENERATION_VOLCANICISLAND, false, "Fiji", "AMD Radeon Graphics Processor" },
    { GDT_FIJI, 0x7300, 0xC1, GDT_HW_GENERATION_VOLCANICISLAND, false, "Fiji", "AMD FirePro Processor" },
    { GDT_FIJI, 0x7300, 0xC8, GDT_HW_GENERATION_VOLCANICISLAND, false, "Fiji", "AMD Radeon (TM) R9 Fury Series" },
    { GDT_FIJI, 0x7300, 0xC9, GDT_HW_GENERATION_VOLCANICISLAND, false, "Fiji", "Radeon(TM) Pro Duo" },
    { GDT_FIJI, 0x7300, 0xCB, GDT_HW_GENERATION_VOLCANICISLAND, false, "Fiji", "AMD Radeon (TM) R9 Fury Series" },
    { GDT_FIJI, 0x7300, 0xCA, GDT_HW_GENERATION_VOLCANICISLAND, false, "Fiji", "AMD Radeon (TM) R9 Fury Series" },
};

/// NOTE: Don't update the table below, it's generated from the csv file.
/// How to update this table:
/// 1. Update device info in the CSV file
/// 2. Run deviceinfogen.exe
/// 3. Replace GDT_DeviceInfo gs_deviceInfo[] in DeviceInfo.cpp with the content of generated file device_info_table.txt
static GDT_DeviceInfo gs_deviceInfo[] =
{
    { 2, 10, 1, 8, 2, 64, 2, 7, 4, true }, // GDT_TAHITI_PRO
    { 2, 10, 1, 8, 2, 64, 2, 8, 4, true }, // GDT_TAHITI_XT
    { 2, 10, 1, 8, 2, 64, 2, 4, 4, true }, // GDT_PITCAIRN_PRO
    { 2, 10, 1, 8, 2, 64, 2, 5, 4, true }, // GDT_PITCAIRN_XT
    { 1, 10, 1, 8, 1, 64, 2, 4, 4, true }, // GDT_CAPEVERDE_PRO
    { 1, 10, 1, 8, 1, 64, 2, 5, 4, true }, // GDT_CAPEVERDE_XT
    { 1, 10, 1, 8, 1, 64, 1, 6, 4, true }, // GDT_OLAND
    { 1, 10, 1, 8, 1, 64, 1, 5, 4, true }, // GDT_HAINAN
    { 2, 10, 1, 8, 2, 64, 1, 7, 4, true }, // GDT_BONAIRE
    { 4, 10, 1, 8, 4, 64, 1, 11, 4, true }, // GDT_HAWAII
    { 1, 10, 1, 8, 1, 64, 1, 2, 4, true }, // GDT_KALINDI
    { 1, 10, 1, 8, 1, 64, 1, 8, 4, true }, // GDT_SPECTRE
    { 1, 10, 1, 8, 1, 64, 1, 4, 4, true }, // GDT_SPECTRE_SL
    { 1, 10, 1, 8, 1, 64, 1, 6, 4, true }, // GDT_SPECTRE_LITE
    { 1, 10, 1, 8, 1, 64, 1, 3, 4, true }, // GDT_SPOOKY
    { 1, 10, 1, 8, 1, 64, 1, 6, 4, true }, // GDT_ICELAND
    { 4, 10, 1, 8, 4, 64, 1, 8, 4, true }, // GDT_TONGA
    { 1, 10, 1, 8, 1, 64, 1, 8, 4, true }, // GDT_CARRIZO
    { 1, 10, 1, 8, 1, 64, 1, 3, 4, true }, // GDT_CARRIZO_EM
    { 4, 10, 1, 8, 4, 64, 1, 16, 4, true }, // GDT_FIJI
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, false }, // GDT_STONEY placeholder
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, false }, // GDT_ELLESMERE placeholder
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, false }, // GDT_BAFFIN placeholder
};

static size_t gs_cardInfoSize = sizeof(gs_cardInfo) / sizeof(GDT_GfxCardInfo);
static size_t gs_deviceInfoSize = sizeof(gs_deviceInfo) / sizeof(GDT_DeviceInfo);

AMDTDeviceInfoManager::AMDTDeviceInfoManager()
{
    AMDTDeviceInfoUtils* pDeviceInfoUtils = AMDTDeviceInfoUtils::Instance();

    for (size_t i = 0; i < gs_cardInfoSize; ++i)
    {
        pDeviceInfoUtils->AddDevice(gs_cardInfo[i]);
    }

    for (size_t i = 0; i < gs_deviceInfoSize; ++i)
    {
        pDeviceInfoUtils->AddDeviceInfo(static_cast<GDT_HW_ASIC_TYPE>(i), gs_deviceInfo[i]);
    }

    CallInitInternalDeviceInfo();
}

AMDTDeviceInfoManager::~AMDTDeviceInfoManager()
{}

#ifdef _WIN32
    extern "C" IMAGE_DOS_HEADER __ImageBase;
    #define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

void AMDTDeviceInfoManager::CallInitInternalDeviceInfo() const
{
    static const char* initFuncName = "InitInternalDeviceInfo";
    static bool initFuncCalled = false;

    if (!initFuncCalled)
    {
        typedef void(*DeviceInfoUtilsReadyFunc)();
        DeviceInfoUtilsReadyFunc func = nullptr;
#ifdef _WIN32
        func = (DeviceInfoUtilsReadyFunc)GetProcAddress(HINST_THISCOMPONENT, initFuncName);
#endif
#ifdef _LINUX
        func = (DeviceInfoUtilsReadyFunc)dlsym(nullptr, initFuncName);
#endif

        if (nullptr != func)
        {
            func();
            initFuncCalled = true;
        }
    }
}

bool AMDTDeviceInfoUtils::GetDeviceInfo(size_t deviceID, size_t revisionID, GDT_DeviceInfo& deviceInfo) const
{
    bool found = false;

    for (auto it = m_deviceIDMap.find(deviceID); it != m_deviceIDMap.end() && !found; ++it)
    {
        size_t thisRevId = (*it).second.m_revID;

        if (thisRevId == revisionID)
        {
            for (auto itr = m_asicTypeDeviceInfoMap.find((*it).second.m_asicType); itr != m_asicTypeDeviceInfoMap.end() && !found; ++itr)
            {
                deviceInfo = itr->second;

                if (deviceInfo.m_deviceInfoValid)
                {
                    found = true;
                }
            }
        }
    }

    return found;
}

/// NOTE: this might not return the correct GDT_DeviceInfo instance, since some devices with the same CAL name might have different GDT_DeviceInfo instances
bool AMDTDeviceInfoUtils::GetDeviceInfo(const char* szCALDeviceName, GDT_DeviceInfo& deviceInfo) const
{
    std::string strDeviceName = TranslateDeviceName(szCALDeviceName);

    auto it = m_deviceNameMap.find(strDeviceName.c_str());

    if (it != m_deviceNameMap.end())
    {
        auto deviceIt = m_asicTypeDeviceInfoMap.find(it->second.m_asicType);

        if (m_asicTypeDeviceInfoMap.end() != deviceIt)
        {
            deviceInfo = deviceIt->second;

            if (deviceInfo.m_deviceInfoValid)
            {
                return true;
            }
        }
    }

    return false;
}

bool AMDTDeviceInfoUtils::GetDeviceInfo(size_t deviceID, size_t revisionID, GDT_GfxCardInfo& cardInfo) const
{
    bool found = false;

    for (auto it = m_deviceIDMap.find(deviceID); it != m_deviceIDMap.end() && !found; ++it)
    {
        size_t thisRevId = (*it).second.m_revID;

        if (thisRevId == revisionID)
        {
            cardInfo = (*it).second;
            found = true;
        }
    }

    return found;
}

bool AMDTDeviceInfoUtils::GetDeviceInfo(const char* szCalName, vector<GDT_GfxCardInfo>& cardList) const
{
    std::string strDeviceName = TranslateDeviceName(szCalName);

    cardList.clear();
    pair<DeviceNameMap::const_iterator, DeviceNameMap::const_iterator> matches;
    matches = m_deviceNameMap.equal_range(strDeviceName.c_str());

    for (auto it = matches.first; it != matches.second; ++it)
    {
        cardList.push_back((*it).second);
    }

    return !cardList.empty();
}

bool AMDTDeviceInfoUtils::GetDeviceInfoMarketingName(const char* szMarketingName, vector<GDT_GfxCardInfo>& cardList) const
{
    cardList.clear();
    pair<DeviceNameMap::const_iterator, DeviceNameMap::const_iterator> matches;
    matches = m_deviceMarketingNameMap.equal_range(szMarketingName);

    for (auto it = matches.first; it != matches.second; ++it)
    {
        cardList.push_back((*it).second);
    }

    return !cardList.empty();
}

bool AMDTDeviceInfoUtils::IsAPU(const char* szCALDeviceName, bool& bIsAPU) const
{
    std::string strDeviceName = TranslateDeviceName(szCALDeviceName);

    auto it = m_deviceNameMap.find(strDeviceName.c_str());

    if (it != m_deviceNameMap.end())
    {
        bIsAPU = it->second.m_bAPU;
        return true;
    }
    else
    {
        return false;
    }
}

bool AMDTDeviceInfoUtils::GetHardwareGeneration(size_t deviceID, GDT_HW_GENERATION& gen) const
{
    // revId not needed here, since all revs will have the same hardware family
    auto it = m_deviceIDMap.find(deviceID);

    if (it != m_deviceIDMap.end())
    {
        gen = it->second.m_generation;
        return true;
    }
    else
    {
        return false;
    }
}

bool AMDTDeviceInfoUtils::GetHardwareGeneration(const char* szName, GDT_HW_GENERATION& gen) const
{
    std::string strDeviceName = TranslateDeviceName(szName);

    auto it = m_deviceNameMap.find(strDeviceName.c_str());

    if (it != m_deviceNameMap.end())
    {
        gen = it->second.m_generation;
        return true;
    }
    else
    {
        return false;
    }
}

bool AMDTDeviceInfoUtils::GetAllCardsInHardwareGeneration(GDT_HW_GENERATION gen, std::vector<GDT_GfxCardInfo>& cardList) const
{
    cardList.clear();
    pair<DeviceHWGenerationMap::const_iterator, DeviceHWGenerationMap::const_iterator> matches;
    matches = m_deviceHwGenerationMap.equal_range(gen);

    for (auto it = matches.first; it != matches.second; ++it)
    {
        cardList.push_back((*it).second);
    }

    return !cardList.empty();
}

bool AMDTDeviceInfoUtils::GetAllCardsWithDeviceId(size_t deviceID, std::vector<GDT_GfxCardInfo>& cardList) const
{
    cardList.clear();
    pair<DeviceIDMap::const_iterator, DeviceIDMap::const_iterator> matches;
    matches = m_deviceIDMap.equal_range(deviceID);

    for (auto it = matches.first; it != matches.second; ++it)
    {
        cardList.push_back((*it).second);
    }

    return !cardList.empty();
}

bool AMDTDeviceInfoUtils::GetHardwareGenerationDisplayName(GDT_HW_GENERATION gen, std::string& strGenerationDisplayName) const
{
    static const std::string s_SI_FAMILY_NAME = "Graphics IP v6";
    static const std::string s_CI_FAMILY_NAME = "Graphics IP v7";
    static const std::string s_VI_FAMILY_NAME = "Graphics IP v8";

    bool retVal = true;

    switch (gen)
    {
        case GDT_HW_GENERATION_SOUTHERNISLAND:
            strGenerationDisplayName = s_SI_FAMILY_NAME;
            break;

        case GDT_HW_GENERATION_SEAISLAND:
            strGenerationDisplayName = s_CI_FAMILY_NAME;
            break;

        case GDT_HW_GENERATION_VOLCANICISLAND:
            strGenerationDisplayName = s_VI_FAMILY_NAME;
            break;

        default:
            strGenerationDisplayName.clear();
            retVal = false;
            break;
    }

    return retVal;
}

std::string AMDTDeviceInfoUtils::TranslateDeviceName(const char* strDeviceName) const
{
    std::string retVal(strDeviceName);

    if (nullptr != m_pDeviceNameTranslatorFunction)
    {
        retVal = m_pDeviceNameTranslatorFunction(strDeviceName);
    }

    return retVal;
}

AMDTDeviceInfoUtils* AMDTDeviceInfoUtils::ms_pInstance = nullptr;
AMDTDeviceInfoManager AMDTDeviceInfoManager::ms_instance;

void AMDTDeviceInfoUtils::AddDeviceInfo(GDT_HW_ASIC_TYPE asicType, const GDT_DeviceInfo& deviceInfo)
{
    if (m_asicTypeDeviceInfoMap.end() == m_asicTypeDeviceInfoMap.find(asicType))
    {
        m_asicTypeDeviceInfoMap.insert(ASICTypeDeviceInfoMapPair(asicType, deviceInfo));
    }
    else
    {
        m_asicTypeDeviceInfoMap[asicType] = deviceInfo;
    }
}

void AMDTDeviceInfoUtils::AddDevice(const GDT_GfxCardInfo& cardInfo)
{
    m_deviceIDMap.insert(DeviceIDMapPair(cardInfo.m_deviceID, cardInfo));
    m_deviceNameMap.insert(DeviceNameMapPair(cardInfo.m_szCALName, cardInfo));
    m_deviceMarketingNameMap.insert(DeviceNameMapPair(cardInfo.m_szMarketingName, cardInfo));
    m_deviceHwGenerationMap.insert(DeviceHWGenerationMapPair(cardInfo.m_generation, cardInfo));
}

void AMDTDeviceInfoUtils::SetDeviceNameTranslator(DeviceNameTranslatorFunction deviceNametranslatorFunction)
{
    m_pDeviceNameTranslatorFunction = deviceNametranslatorFunction;
}
