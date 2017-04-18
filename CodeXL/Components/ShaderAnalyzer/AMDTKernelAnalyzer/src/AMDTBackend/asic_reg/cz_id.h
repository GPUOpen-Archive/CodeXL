/*****************************************************************************\
*
*  Module Name    cz_id.h
*  Project        CARRIZO
*  Devices        CARRIZO
*
*  Description    Defining Device IDs, ASIC revision IDs for CARRIZO
*
*
*  (c) 2013 Advanced Micro Devices, Inc. (unpublished)
*
*  All rights reserved.  This notice is intended as a precaution against
*  inadvertent publication and does not imply publication or any waiver
*  of confidentiality.  The year included in the foregoing notice is the
*  year of creation of the work.
*
\*****************************************************************************/

#ifndef _CZ_ID_H
#define _CZ_ID_H

enum
{
    CARRIZO_A0      = 0x01,
    CARRIZO_A1      = 0x02,
    NOLAN_A0        = 0x21,
    AMUR_A0         = 0x41,
    STONEY_A0       = 0x61,
    CZ_UNKNOWN      = 0xFF
};


#define ASICREV_IS_CARRIZO(eChipRev)            ( (eChipRev >= CARRIZO_A0) && (eChipRev < NOLAN_A0) )
#define ASICREV_IS_NOLAN(eChipRev)              ( (eChipRev >= NOLAN_A0) && (eChipRev < AMUR_A0) )
#define ASICREV_IS_AMUR(eChipRev)               ( (eChipRev >= AMUR_A0) && (eChipRev < STONEY_A0) )
#define ASICREV_IS_STONEY(eChipRev)             ( (eChipRev >= STONEY_A0) && (eChipRev < CZ_UNKNOWN) )


//
// Carrizo device IDs
//
#define DEVICE_ID_CZ_9870                   0x9870  // Palladium  

#define DEVICE_ID_CZ_9874                   0x9874
#define DEVICE_ID_CZ_9875                   0x9875
#define DEVICE_ID_CZ_9876                   0x9876
#define DEVICE_ID_CZ_9877                   0x9877

// CARRIZO ASIC internal revision number
#define INTERNAL_REV_CARRIZO_A0             0x00    // First spin of CARRIZO
#define INTERNAL_REV_CARRIZO_A1             0x01    // Second spin of CARRIZO

// CARRIZO PCI Reivsion IDs
#define PRID_CZ_C4                          0xC4  // Client B10
#define PRID_CZ_C5                          0xC5  // Client B8
#define PRID_CZ_C6                          0xC6  // Client B6
#define PRID_CZ_C7                          0xC7  // Client B4

//
// Nolan embedded device IDs
//
#define DEVICE_ID_NL_98C0                   0x98C0 // Nolan embedded internal GFX
#define DEVICE_ID_NL_98CC                   0x98CC // Nolan Emb iTemp Prime 4C
#define DEVICE_ID_NL_98CD                   0x98CD // Nolan Emb iTemp Prime 2C

#define PRID_NOLAN_80                       0x80    // Embedded 15W 4CU
#define PRID_NOLAN_81                       0x81    // Embedded 4W 4CU
#define PRID_NOLAN_82                       0x82    // Embedded 5W 2CU
#define PRID_NOLAN_83                       0x83    // Embedded 5W no CU

// NOLAN ASIC internal revision number
#define INTERNAL_REV_NOLAN_A0               0x00    // First spin of NOLAN


//
// Amur device IDs
//
#define DEVICE_ID_AM_9890                   0x9890

// AMUR ASIC internal revision number
#define INTERNAL_REV_AMUR_A0                0x00    // First spin of AMUR

//
// Stoney device IDs
//
#define DEVICE_ID_ST_98E4                   0x98E4

// STONEY ASIC internal revision number
#define INTERNAL_REV_STONEY_A0              0x00    // First spin of STONEY

// STONEY PCI Revision IDs
#define PRID_ST_C0                          0xC0    // 15W 2C N FT4 98E4
#define PRID_ST_C1                          0xC1    // 15W 2C N FP4 98E4
#define PRID_ST_C8                          0xC8    // 10W 2C N-4 FT4 98E4
#define PRID_ST_C9                          0xC9    // 10W 2C N-4 FP4 98E4
#define PRID_ST_D1                          0xD1    // 15W 2C N-1 FP4 98E4
#define PRID_ST_D8                          0xD8    // 4.5W 2C N-1 FT4 98E4
#define PRID_ST_E0                          0xE0    // 15W 2C N-2 FT4 98E4
#define PRID_ST_E1                          0xE1    // 15W 2C N-2 FP4 98E4
#define PRID_ST_F1                          0xF1    // 15W 2C N-3 FP4 98E4

#endif // _CZ_ID_H

