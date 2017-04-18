/*****************************************************************************\
*
*  Module Name    tn_id.h
*  Project        Trinity Family
*  Device         Devastator, Scrapper
*
*  Description    Identifier file for Trinity driver
*
*
*  (c) 2010 AMD (unpublished)
*
*  All rights reserved.  This notice is intended as a precaution against
*  inadvertent publication and does not imply publication or any waiver
*  of confidentiality.  The year included in the foregoing notice is the
*  year of creation of the work.
*
*  LOG OF CHANGES
*
*  1.0    10/21/10 [ilitchma] - initial revision
*  1.1    02/24/11 [ilitchma] - updated IDs based on marketing request
*  1.2    10/01/11 [ilitchma] - Add A1 spin definitions
*  1.3    11/10/11 [ilitchma] - Add TN1 production DIDs
*  1.4    07/12/12 [rdass] - Add RL (Richland - TN refresh) production DIDs
*
\*****************************************************************************/

#ifndef TN_ID_H
#define TN_ID_H

//DEVASTATOR section

enum
{
    TN_DEVASTATOR_M_A0          = 0x01,       //Devastator 4-6-6-2
    TN_DEVASTATOR_M_A1          = 0x02,       //Devastator 4-6-6-2

    TN_DEVASTATOR_W_A0          = 0x11,       //Devastator Workstation 4-6-6-2
    TN_DEVASTATOR_W_A1          = 0x12,       //Devastator Workstation 4-6-6-2

    TN_DEVASTATOR_LITE_MV_A0    = 0x21,       //Devastator Lite 4-4-4-2
    TN_DEVASTATOR_LITE_MV_A1    = 0x22,       //Devastator Lite 4-4-4-2

    TN_DEVASTATOR_V_A0          = 0x41,       //Devastator 4-6-6-2 downconfigured to 4-3-3-1 same as scrapper
    TN_DEVASTATOR_V_A1          = 0x42,       //Devastator 4-6-6-2 downconfigured to 4-3-3-1 same as scrapper
    //DEVASTATOR_V IDs are depricated do not use

    TN_SCRAPPER_V_A0            = 0x41,       //Scrapper  4-3-3-1
    TN_SCRAPPER_V_A1            = 0x42,       //Scrapper  4-3-3-1

    TN_DVST_DUO_V_A0            = 0x61,       //Scrapper  4-2-2-1

    TN_SCRAPPER_LV_A0           = 0x61,       //Scrapper Lite  4-2-2-1
    TN_SCRAPPER_LV_A1           = 0x62,       //Scrapper Lite  4-2-2-1

    TN_UNKNOWN = 0xFF
};

#define ASICREV_IS_DEVASTATOR_M(eChipRev)    ((eChipRev >= TN_DEVASTATOR_M_A0) && (eChipRev < TN_DEVASTATOR_LITE_MV_A0))
//ASICREV_IS_DEVASTATOR_M should be used to identify DEVASTATOR full version features and configuration

#define ASICREV_IS_DEVASTATOR_W(eChipRev)    ((eChipRev >= TN_DEVASTATOR_W_A0) && (eChipRev < TN_DEVASTATOR_LITE_MV_A0))
//ASICREV_IS_DEVASTATOR_W should be used to identify DEVASTATOR Workstation version features and configuration

#define ASICREV_IS_DEVASTATOR_M_MV(eChipRev)    ((eChipRev >= TN_DEVASTATOR_M_A0) && (eChipRev < TN_DEVASTATOR_V_A0))
//ASICREV_IS_DEVASTATOR_M_MV should be used to identify DEVASTATOR 4-6-2 + 4-4-2 features and configuration

#define ASICREV_IS_DEVASTATOR_LITE_MV(eChipRev)    ((eChipRev >= TN_DEVASTATOR_LITE_MV_A0) && (eChipRev < TN_DEVASTATOR_V_A0))
//TN_DEVASTATOR_LITE_MV should be used to identify DEVASTATOR LITE features and configuration

#define ASICREV_IS_DEVASTATOR(eChipRev)    ((eChipRev >= TN_DEVASTATOR_M_A0) && (eChipRev < TN_SCRAPPER_V_A0))
//ASICREV_IS_DEVASTATOR used to identify silicon type including downconfigured parts

//DEVASTATOR_V IDs are depricated do not use
#define ASICREV_IS_DEVASTATOR_V(eChipRev)    ((eChipRev >= TN_DEVASTATOR_V_A0) && (eChipRev < TN_SCRAPPER_V_A0))
//ASICREV_IS_DEVASTATOR_V should be used to identify DEVASTATOR downconfigured to Scrapper

#define ASICREV_IS_TN_V(eChipRev)     ((eChipRev >= TN_DEVASTATOR_V_A0) && (eChipRev < TN_UNKNOWN))
//ASICREV_IS_SCRAPPER_WIDE should be used to identify Scrapper features and configuration

#define ASICREV_IS_SCRAPPER(eChipRev)     ((eChipRev >= TN_SCRAPPER_V_A0) && (eChipRev < TN_UNKNOWN))
//ASICREV_IS_SCRAPPER should be used to identify Scrapper variant

#define ASICREV_IS_SCRAPPER_V(eChipRev)     ((eChipRev >= TN_SCRAPPER_V_A0) && (eChipRev < TN_SCRAPPER_LV_A0))
//ASICREV_IS_SCRAPPERV should be used to identify Scrapper 4-3-3-1 variant

#define ASICREV_IS_SCRAPPER_LITE_LV(eChipRev)     ((eChipRev >= TN_SCRAPPER_LV_A0) && (eChipRev < TN_UNKNOWN))
//ASICREV_IS_SCRAPPER should be used to identify Scrapper Lite 4-2-2-1 variant



#define ASIC_IS_TN_A0(eChipRev) ((eChipRev & 0xF) == 0x1)
// ASIC is Trinity A0
#define ASIC_IS_TN_A1(eChipRev) ((eChipRev & 0xF) == 0x2)
// ASIC is Trinity A1

//
// Define Chip ID's for Trinity family
//
#define DEVICE_ID_DEVASTATOR_MOBILE             0x9900     //FP2,FS1r2 socket,4-6-6-2 (QP-SIMD-TP-RB)
#define DEVICE_ID_DEVASTATOR_DESKTOP            0x9901     //FM2 socket,4-6-6-2 (QP-SIMD-TP-RB)
#define DEVICE_ID_DEVASTATOR_LITE_MOBILE        0x9903     //FP2,FS1r2 socket,4-4-4-2 (QP-SIMD-TP-RB)
#define DEVICE_ID_DEVASTATOR_LITE_DESKTOP       0x9904     //FM2 socket,4-4-4-2 (QP-SIMD-TP-RB)
#define DEVICE_ID_DEVASTATOR_WS_9905            0x9905     //Trinity/DVST - workstation FM2 socket,4-6-6-2 (QP-SIMD-TP-RB)
#define DEVICE_ID_DEVASTATOR_WS_9906            0x9906     //Trinity/DVST - workstation FM2 socket,4-6-6-2 (QP-SIMD-TP-RB)
#define DEVICE_ID_DEVASTATOR_MOBILE_FP2_9907    0x9907     //Trinity/DVST - mobile FP2 25W, 4-6-6-2 (QP-SIMD-TP-RB)
#define DEVICE_ID_DEVASTATOR_MOBILE_FP2_9908    0x9908     //Trinity/DVST - mobile FP2 19W, 4-6-6-2 (QP-SIMD-TP-RB)
#define DEVICE_ID_DEVASTATOR_MOBILE_FP2_9909    0x9909     //Trinity/DVST - mobile FP2 17W, 4-6-6-2 (QP-SIMD-TP-RB)
#define DEVICE_ID_DEVASTATOR_LITE_MOBILE_FP2_990A   0x990A     //FP2 17W, 4-4-4-2 (QP-SIMD-TP-RB)


#define DEVICE_ID_RL_DEVASTATOR_MOBILE          0x990B     //FP2,FS1r2 socket,4-6-6-2 (QP-SIMD-TP-RB) (Richland)
#define DEVICE_ID_RL_DEVASTATOR_DESKTOP         0x990C     //FM2 socket,4-6-6-2 (QP-SIMD-TP-RB) (Richland)
#define DEVICE_ID_RL_DEVASTATOR_LITE_MOBILE     0x990D     //FP2,FS1r2 socket,4-4-4-2 (QP-SIMD-TP-RB) (Richland)
#define DEVICE_ID_RL_DEVASTATOR_LITE_DESKTOP    0x990E     //FM2 socket,4-4-4-2 (QP-SIMD-TP-RB) (Richland)
#define DEVICE_ID_RL_DEVASTATOR_MOBILE_FP2_990F 0x990F     //FP2 25W, 4-6-6-2 (QP-SIMD-TP-RB) (Richland)

#define DEVICE_ID_DEVASTATOR_MOBILE_EMBEDDED        0x9910  //FS1r2 socket,4-6-6-2 (QP-SIMD-TP-RB)
#define DEVICE_ID_DEVASTATOR_LITE_MOBILE_EMBEDDED   0x9913  //FS1r2 socket,4-4-4-2 (QP-SIMD-TP-RB)
#define DEVICE_ID_DEVASTATOR_FP2_EMBEDDED_9917  0x9917  //FP2 25W,  4-6-6-2 (QP-SIMD-TP-RB)
#define DEVICE_ID_DEVASTATOR_FP2_EMBEDDED_9918  0x9918  //FP2 19W, 4-6-6-6 (QP-SIMD-TP-RB)
#define DEVICE_ID_DEVASTATOR_LITE_FP2_EMBEDDED  0x9919  //FP2 17W, 4-4-4-2 (QP-SIMD-TP-RB)

#define DEVICE_ID_SCRAPPER_MOBILE               0x9990     //FP2, FS1r2 socket,4-3-3-1(QP-SIMD-TP-RB)
#define DEVICE_ID_SCRAPPER_DESKTOP              0x9991     //FM2 socket,4-3-3-1 (QP-SIMD-TP-RB)
#define DEVICE_ID_SCRAPPER_LITE_MOBILE          0x9992     //FS1r2 socket,4-2-2-1(QP-SIMD-TP-RB)
#define DEVICE_ID_SCRAPPER_LITE_DESKTOP         0x9993     //FM2 socket,4-2-2-1(QP-SIMD-TP-RB)
#define DEVICE_ID_SCRAPPER_MOBILE_FP2           0x9994     //FP2, FS1r2 socket,4-3-3-1(QP-SIMD-TP-RB)

#define DEVICE_ID_RL_SCRAPPER_MOBILE            0x9995     //FP2, FS1r2 socket,4-3-3-1(QP-SIMD-TP-RB) (Richland)
#define DEVICE_ID_RL_SCRAPPER_DESKTOP           0x9996     //FM2 socket,4-3-3-1 (QP-SIMD-TP-RB) (Richland)
#define DEVICE_ID_RL_SCRAPPER_LITE_MOBILE       0x9997     //FS1r2 socket,4-2-2-1(QP-SIMD-TP-RB) (Richland)
#define DEVICE_ID_RL_SCRAPPER_LITE_DESKTOP      0x9998     //FM2 socket,4-2-2-1(QP-SIMD-TP-RB) (Richland)
#define DEVICE_ID_RL_DEVASTATOR_MOBILE_FP2_9999 0x9999     //FP2 17W, 4-6-6-2 (QP-SIMD-TP-RB) (Richland)

#define DEVICE_ID_RL_SCRAPPER_MOBILE_FP2_999A       0x999A  //FP2 17W, 4-3-3-1 (QP-SIMD-TP-RB) (Richland)
#define DEVICE_ID_RL_SCRAPPER_LITE_MOBILE_FP2_999B  0x999B  //FP2 17W, 4-2-2-1 (QP-SIMD-TP-RB) (Richland)

#define DEVICE_ID_RL_DEVASTATOR_DESKTOP_999C        0x999C  //FM2 45W,4-6-6-2 (QP-SIMD-TP-RB) (Richland)
#define DEVICE_ID_RL_DEVASTATOR_LITE_DESKTOP_999D   0x999D  //FM2 45W,4-4-4-2 (QP-SIMD-TP-RB) (Richland)

#define DEVICE_ID_SCRAPPER_MOBILE_EMBEDDED          0x99A0  //FS1r2 socket,4-3-3-1(QP-SIMD-TP-RB)
#define DEVICE_ID_SCRAPPER_LITE_MOBILE_EMBEDDED     0x99A2  //FS1r2 socket,4-2-2-1(QP-SIMD-TP-RB)
#define DEVICE_ID_SCRAPPER_FP2_EMBEDDED         0x99A4  //FP2, 4-3-3-1(QP-SIMD-TP-RB)

#define DEVICE_ID_DEVASTATOR_DUO            0x990F     //FM2,FP2,FS1r2 socket,4-2-2-1 (QP-SIMD-TP-RB) Non production part
//DEVASTATOR_DUO is depricated do not use

//
// Define AMD's internal revision numbers.
//
#define INTERNAL_REV_DEVASTATOR_A0       0x00  // The First revision of Trinity TN1 GFX.
#define INTERNAL_REV_DEVASTATOR_A1       0x01  // The Second revision of Trinity TN1 GFX.
#define INTERNAL_REV_SCRAPPER_TN2_A0     0x02  // The First revision of Trinity TN2 GFX.

//
// Define AMD's external revision numbers.
//
#define EXTERNAL_REV_DEVASTATOR       0x00


#endif  // TN_ID_H
