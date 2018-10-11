// Copyright (c) 2011-2017 Advanced Micro Devices, Inc. All rights reserved.
#ifndef _CI_ID_H
#define _CI_ID_H

enum
{
    CI_TIRAN_P_A0   = 1,  // Tiran is obsolete, please do not use

    CI_BONAIRE_M_A0 = 20,
    CI_BONAIRE_M_A1 = 21,

    CI_HAWAII_P_A0  = 40,

    CI_MAUI_P_A0    = 60,

    CI_UNKNOWN      = 0xFF
};

#define ASICREV_IS_TIRAN_P(eChipRev)    (eChipRev < CI_BONAIRE_M_A0)
#define ASICREV_IS_BONAIRE_M(eChipRev)  ((eChipRev >= CI_BONAIRE_M_A0) && (eChipRev < CI_HAWAII_P_A0))
#define ASICREV_IS_HAWAII_P(eChipRev)   ((eChipRev >= CI_HAWAII_P_A0) && (eChipRev < CI_MAUI_P_A0))
#define ASICREV_IS_MAUI_P(eChipRev)     (eChipRev >= CI_MAUI_P_A0)


//
// TIRAN/TIRAN MOBILE device IDs (Performance segment)
//
#define DEVICE_ID_CI_TIRAN_P_6600               0x6600  // unfused

// TIRAN ASIC internal revision number
#define INTERNAL_REV_CI_TIRAN_P_A0              0x00    // First spin of Tiran

#define DEVICE_ID_CI_TIRAN_P_PALLADIUM          0x37    // Palladium ID
#define DEVICE_ID_CI_TIRAN_P_LITE_PALLADIUM     0x40    // Palladium ID


//
// BONAIRE/SATURN/EMERALD/STRATO device IDs (Performance to Mainstream segment)
//
#define DEVICE_ID_CI_BONAIRE_M_6640             0x6640  // Saturn XT 
#define DEVICE_ID_CI_BONAIRE_M_6641             0x6641  // Saturn  PRO
#define DEVICE_ID_CI_BONAIRE_M_6646             0x6646  // Emerald XT; Strato XT
#define DEVICE_ID_CI_BONAIRE_M_6647             0x6647  // Emerald PRO; Strato Pro
#define DEVICE_ID_CI_BONAIRE_M_6649             0x6649  // Bonaire  GL Pro 
#define DEVICE_ID_CI_BONAIRE_M_664E             0x664E  // Strato PRO 
#define DEVICE_ID_CI_BONAIRE_M_6650             0x6650  // Bonaire XT
#define DEVICE_ID_CI_BONAIRE_M_6651             0x6651  // Bonaire Pro 
#define DEVICE_ID_CI_BONAIRE_M_6658             0x6658  // Bonaire XTX 
#define DEVICE_ID_CI_BONAIRE_M_665C             0x665C  // Bonaire XT
#define DEVICE_ID_CI_BONAIRE_M_665D             0x665D  // Bonaire Pro 

// STRATO PCI Reivsion IDs
#define PRID_CI_BONAIRE_STRATO_80               0x80      // 0x6646: Strato XT; 0x6647: Strato Pro

#define ASICID_IS_STRATO(wDID, bRID)            ((bRID == PRID_CI_BONAIRE_STRATO_80) && \
                                                 ((wDID == DEVICE_ID_CI_BONAIRE_M_6646) || (wDID == DEVICE_ID_CI_BONAIRE_M_6647)))

#define DEVICE_ID_CI_BONAIRE_M_PALLADIUM        0x45    // Palladium ID
#define DEVICE_ID_CI_BONAIRE_M_LITE_PALLADIUM   0x46    // Palladium ID

// BONAIRE ASIC internal revision number
#define INTERNAL_REV_CI_BONAIRE_M_A0            0x00    // First spin of Bonaire
#define INTERNAL_REV_CI_BONAIRE_M_A1            0x01    // Second spin of Bonaire

//
// HAWAII device IDs (Performance segment)
//
#define DEVICE_ID_CI_HAWAII_P_66A0              0x66A0  // Obsolete
#define DEVICE_ID_CI_HAWAII_P_67A0              0x67A0  // Hawaii GL44
#define DEVICE_ID_CI_HAWAII_P_67A1              0x67A1  // Hawaii GL40
#define DEVICE_ID_CI_HAWAII_P_67A2              0x67A2  // Hawaii GL Gemini
#define DEVICE_ID_CI_HAWAII_P_67A8              0x67A8  // Hawaii GL XT
#define DEVICE_ID_CI_HAWAII_P_67A9              0x67A9  // Hawaii GL Gemini
#define DEVICE_ID_CI_HAWAII_P_67AA              0x67AA  // Hawaii GL Pro
#define DEVICE_ID_CI_HAWAII_P_67B0              0x67B0  // Hawaii XT
#define DEVICE_ID_CI_HAWAII_P_67B1              0x67B1  // Hawaii Pro
#define DEVICE_ID_CI_HAWAII_P_67B8              0x67B8  // Obsolete
#define DEVICE_ID_CI_HAWAII_P_67B9              0x67B9  // Hawaii Gemini
#define DEVICE_ID_CI_HAWAII_P_67BA              0x67BA  // Obsolete
#define DEVICE_ID_CI_HAWAII_P_67BE              0x67BE  // Hawaii LE

#define DEVICE_ID_CI_HAWAII_P_PALLADIUM         0x00    // Palladium ID
#define DEVICE_ID_CI_HAWAII_P_LITE_PALLADIUM    0x0052  // Palladium ID

// HAWAII ASIC internal revision number
#define INTERNAL_REV_CI_HAWAII_P_A0             0x00    // First spin of Hawaii


//
// MAUI device IDs (Performance segment)
//
#define DEVICE_ID_CI_MAUI_P_66E0                0x66E0  // unfused

#define DEVICE_ID_CI_MAUI_P_PALLADIUM           0x00    // Palladium ID
#define DEVICE_ID_CI_MAUI_P_LITE_PALLADIUM      0x00    // Palladium ID

// HAWAII ASIC internal revision number
#define INTERNAL_REV_CI_MAUI_P_A0               0x00    // First spin of Maui

#endif  // _CI_ID_H
