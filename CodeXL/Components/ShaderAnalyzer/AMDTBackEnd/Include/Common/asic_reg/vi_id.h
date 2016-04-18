/*****************************************************************************\
*
*  Module Name    vi_id.h
*  Project        VOLCANIC ISLANDS
*  Devices        VOLCANIC ISLANDS
*
*  Description    Defining Device IDs, ASIC revision IDs for VOLCANIC ISLANDS
*
*
*  (c) 2012 Advanced Micro Devices, Inc. (unpublished)
*
*  All rights reserved.  This notice is intended as a precaution against
*  inadvertent publication and does not imply publication or any waiver
*  of confidentiality.  The year included in the foregoing notice is the
*  year of creation of the work.
*
\*****************************************************************************/

#ifndef _VI_ID_H
#define _VI_ID_H

enum
{
    VI_ICELAND_M_A0   = 1,

    VI_TONGA_P_A0     = 20,
    VI_TONGA_P_A1     = 21,

    VI_BERMUDA_P_A0   = 40,

    VI_FIJI_P_A0      = 60,

    VI_UNKNOWN        = 0xFF
};


#define ASICREV_IS_ICELAND_M(eChipRev)  (eChipRev < VI_TONGA_P_A0)
#define ASICREV_IS_TONGA_P(eChipRev)    ((eChipRev >= VI_TONGA_P_A0) && (eChipRev < VI_BERMUDA_P_A0))
#define ASICREV_IS_BERMUDA_P(eChipRev)  ((eChipRev >= VI_BERMUDA_P_A0) && (eChipRev < VI_FIJI_P_A0))
#define ASICREV_IS_FIJI_P(eChipRev)     (eChipRev >= VI_FIJI_P_A0)

//
// TONGA/AMETHYST device IDs (performance segment)
//
#define DEVICE_ID_VI_TONGA_P_6920               0x6920  // unfused
#define DEVICE_ID_VI_TONGA_P_6921               0x6921  // Amethyst XT
#define DEVICE_ID_VI_TONGA_P_6928               0x6928  // Tonga GL XT
#define DEVICE_ID_VI_TONGA_P_692B               0x692B  // Tonga GL PRO
#define DEVICE_ID_VI_TONGA_P_692F               0x692F  // Tonga GL PRO VF
#define DEVICE_ID_VI_TONGA_P_6938               0x6938  // Tonga XT
#define DEVICE_ID_VI_TONGA_P_6939               0x6939  // Tonga PRO

#define DEVICE_ID_VI_TONGA_P_PALLADIUM          0x00    // Palladium ID
#define DEVICE_ID_VI_TONGA_P_LITE_PALLADIUM     0x48    // Palladium ID

// TONGA ASIC internal revision number
#define INTERNAL_REV_VI_TONGA_P_A0              0x00    // First spin of Tonga
#define INTERNAL_REV_VI_TONGA_P_A1              0x01    // Second spin of Tonga

//
// ICELAND/TOPAZ/MESO device IDs (mainstream segment)
//
#define DEVICE_ID_VI_ICELAND_M_6900             0x6900  // Topaz XT; Meso XT
#define DEVICE_ID_VI_ICELAND_M_6901             0x6901  // Topaz Pro
#define DEVICE_ID_VI_ICELAND_M_6902             0x6902  // Topaz XTL
#define DEVICE_ID_VI_ICELAND_M_6903             0x6903  // Unused - Previous Topaz LE
#define DEVICE_ID_VI_ICELAND_M_6907             0x6907  // Topaz LE

// MESO PCI Reivsion IDs
#define PRID_VI_ICELAND_MESO_81                   0x81  // 0x6900: MESO XT

#define ASICID_IS_MESO(wDID, bRID)              ((wDID == DEVICE_ID_VI_ICELAND_M_6900) && (bRID == PRID_VI_ICELAND_MESO_81))

#define DEVICE_ID_VI_ICELAND_M_PALLADIUM        0x47    // Palladium ID
#define DEVICE_ID_VI_ICELAND_M_LITE_PALLADIUM   0x00    // Palladium ID

// ICELAND ASIC internal revision number
#define INTERNAL_REV_VI_ICELAND_M_A0            0x00    // First spin of ICELAND

//
// Bermuda device IDs (performance segment)
//

#define DEVICE_ID_VI_BERMUDA_P_PALLADIUM        0x49    // Palladium ID
#define DEVICE_ID_VI_BERMUDA_P_LITE_PALLADIUM   0x00    // Palladium ID

// BERMUDA ASIC internal revision number
#define INTERNAL_REV_VI_BERMUDA_P_A0            0x00    // First spin of Bermuda


//
// Fiji device IDs (performance segment)
//

#define DEVICE_ID_VI_FIJI_P_LITE_PALLADIUM      0x4A    // Palladium ID

// FIJI ASIC internal revision number
#define INTERNAL_REV_VI_FIJI_P_A0               0x00    // First spin of Fiji

#endif  // _VI_ID_H
