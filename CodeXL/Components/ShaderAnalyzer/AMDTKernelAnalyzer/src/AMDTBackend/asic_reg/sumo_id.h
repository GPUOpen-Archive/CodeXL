/*****************************************************************************\
*
*  Module Name    Sumo_id.h
*  Project        Llano/Ontario definitions (SuperSumo, Sumo , Wrestler)
*  Device         Sumo, Wrestler
*
*  Description    Identifier file for SUMO driver
*
*
*  (c) 2009 AMD (unpublished)
*
*  All rights reserved.  This notice is intended as a precaution against
*  inadvertent publication and does not imply publication or any waiver
*  of confidentiality.  The year included in the foregoing notice is the
*  year of creation of the work.
*
*  LOG OF CHANGES
*
*  1.0    07/09/09 [ilitchma] - initial revision
*  1.1    01/20/10 [jamesmar] - add Wrestler ID 0x9802 for Ontario.
*
\*****************************************************************************/

#ifndef _SUMO_ID_H
#define _SUMO_ID_H

//SUMO section

enum
{
    SUPERSUMO_A0    = 0x01,     //Sumo 4-5-5-2
    SUPERSUMO_B0    = 0x02,
    SUMO_A0         = 0x11,     //Sumo 4-2-2-1
    SUMO_B0         = 0x12,
    WRESTLER_A0     = 0x21,     //Sumo-lite 2-2-2-1
    WRESTLER_A1     = 0x22,     //Sumo-lite 2-2-2-1
    WRESTLER_B0     = 0x23,     //Sumo-lite 2-2-2-1
    WRESTLER_C0     = 0x24,     //Sumo-lite 2-2-2-1
    BHEEM_A0        = 0x41,     //Sumo-lite 2-2-2-1
    SUMO_UNKNOWN = 0xFF
};

#define ASICREV_IS_SUPERSUMO(eChipRev)    ((eChipRev >= SUPERSUMO_A0) && (eChipRev < SUMO_A0))
#define ASICREV_IS_SUMO(eChipRev)         ((eChipRev >= SUMO_A0) && (eChipRev < WRESTLER_A0))
#define ASICREV_IS_WRESTLER(eChipRev)     ((eChipRev >= WRESTLER_A0) && (eChipRev < BHEEM_A0))
#define ASICREV_IS_WRESTLER_Cx(eChipRev) ((eChipRev >= WRESTLER_C0) && (eChipRev < BHEEM_A0))
#define ASICREV_IS_BHEEM(eChipRev)        ((eChipRev >= BHEEM_A0) && (eChipRev < SUMO_UNKNOWN))

#define ASIC_IS_SSTRIPPED_SUMO(ChipID)   ( (ChipID == DEVICE_ID_SUMO_SSTRIPPED_DESKTOP) ||  \
                                           (ChipID == DEVICE_ID_SUMO_SSTRIPPED_MOBILE) )

#define ASICREV_IS_SUMO_B0_OR_NEWER(eChipRev) ( ((eChipRev & 0x0f) >= SUPERSUMO_B0) && (eChipRev < WRESTLER_A0) )  // Low 4 bits to identify ASIC Rev, 
// high 4 bits to identify SuperSumo/Sumo/Wrestler.


//
// Define Chip ID's for SUMO family
//
#define DEVICE_ID_SUMO_SUPER_DESKTOP        0x9640      //FM1 socket,4-5-5-2 (QP-SIMD-TP-RB)
#define DEVICE_ID_SUMO_SUPER_MOBILE         0x9641      //FS1 4-5-5-2
#define DEVICE_ID_SUMO_SSTRIPPED_DESKTOP    0x9642      //FM1 socket,4-2-2-1, Same die as SuperSumo fused to Sumo GFX configuration
#define DEVICE_ID_SUMO_SSTRIPPED_MOBILE     0x9643      //FS1 4-2-2-1, Same die as SuperSumo fused to Sumo GFX configuration
#define DEVICE_ID_SUMO_DESKTOP              0x9644      //FM1 socket,4-2-2-1
#define DEVICE_ID_SUMO_MOBILE               0x9645      //To be removed later.
#define DEVICE_ID_SUMO_DESKTOP_9645         0x9645      //FM1 socket,4-2-2-1
#define DEVICE_ID_SUMO_SUPER_MOBILE_9647    0x9647      //FS1 4-4-4-2
#define DEVICE_ID_SUMO_MOBILE_9648          0x9648      //To be removed later.
#define DEVICE_ID_SUMO_SUPER_MOBILE_9648    0x9648      //FS1 4-3-3-2
#define DEVICE_ID_SUMO_MOBILE_9649          0x9649      //FS1 4-2-2-1
#define DEVICE_ID_SUMO_SUPER_DESKTOP_964A   0x964A      //FM1 socket,4-4-4-2 (QP-SIMD-TP-RB)
#define DEVICE_ID_SUMO_SUPER_MOBILE_964E    0x964E      //FP1F 4-5-5-2
#define DEVICE_ID_SUMO_SUPER_MOBILE_964F    0x964F      //FP1F 4-5-5-2


#define DEVICE_ID_WRESTLER                  0x9802      //FT1 2-2-2-1
#define DEVICE_ID_WRESTLER_9803             0x9803      //FT1 2-2-2-1
#define DEVICE_ID_WRESTLER_9804             0x9804      //FT1 2-2-2-1
#define DEVICE_ID_WRESTLER_9805             0x9805      //FT1 2-2-2-1
#define DEVICE_ID_WRESTLER_9806             0x9806      //FT1 2-2-2-1
#define DEVICE_ID_WRESTLER_9807             0x9807      //FT1 2-2-2-1
#define DEVICE_ID_WRESTLER_9808             0x9808      //FT1 2-2-2-1
#define DEVICE_ID_WRESTLER_9809             0x9809      //FT1 2-2-2-1
#define DEVICE_ID_WRESTLER_980A             0x980A      //FT1 2-2-2-1

#define DEVICE_ID_BHEEM                     0x1300      //FT1 2-2-2-1
#define DEVICE_ID_BHEEM_1301                0x1301      //FT1 2-2-2-1
#define DEVICE_ID_BHEEM_1302                0x1302      //FT1 2-2-2-1

//
// Define AMD's internal revision numbers.
//
#define INTERNAL_REV_SUMO_A11       0x00  // The First revision of SUMO ASIC.
#define INTERNAL_REV_SUMO_A0        0x00  // The First revision of SUMO ASIC.
#define INTERNAL_REV_SUMO_A1        0x01  // The Second revision of SUMO ASIC.
#define INTERNAL_REV_SUMO_B0        0x02  // The Third revision of SUMO ASIC.

//
// Define AMD's external revision numbers.
//
#define EXTERNAL_REV_SUMO       0x00
//
// Define AMD's Wrestler internal revision numbers.
//
#define INTERNAL_REV_WRESTLER_A0    0x00  // The First revision of Wrestler ASIC.
#define INTERNAL_REV_WRESTLER_A1    0x01  // The Second revision of Wrestler ASIC.
#define INTERNAL_REV_WRESTLER_B0    0x02  // The Third  revision of Wrestler ASIC.
#define INTERNAL_REV_WRESTLER_C0    0x03  // The Fourth  revision of Wrestler ASIC.

//
// Define AMD's Wrestler external revision numbers.
//
#define EXTERNAL_REV_WRESTLER   0x00

// Define AMD's Bheem internal revision numbers.
//
#define INTERNAL_REV_BHEEM_A0    0x00  // The First revision of Bheem ASIC.

//
// Define AMD's Bheem external revision numbers.
//
#define EXTERNAL_REV_BHEEM   0x00

#endif  // _SUMO_ID_H
