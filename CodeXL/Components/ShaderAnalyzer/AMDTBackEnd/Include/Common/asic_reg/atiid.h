// Copyright (c) 1999-2017 Advanced Micro Devices, Inc. All rights reserved.
#ifndef _ATIID_H
#define _ATIID_H


//
// Define Asic Family IDs for different asic family.
//
//[espiritu - 06/21/00]
//TO DO: change enumeration of all asic families including RageProDerivatives, as
//these are in conflict (used especially in TVOut).

#define FAMILY_UNKNOWN                  0
#define FAMILY_RAGE_128                 1
#define FAMILY_RAGE_128_4X              2
#define FAMILY_RAGE_128_PRO             3
#define FAMILY_M3                       4
#define FAMILY_RAGE_6                   5
#define FAMILY_RADEON100                FAMILY_RAGE_6
#define FAMILY_RAGE_6LITE               6
#define FAMILY_M6                       7
#define FAMILY_RADEON200                8
#define FAMILY_RAGE_128_PROII          21 /*EPR#40430 temporary only, in conflict with LTPro asic in TVOut.*/

#define FAMILY_U1                      22  // CABO family; including A3, A4 and U1
#define FAMILY_RS200                   23  // including RS250, RS250M
#define FAMILY_RS200M                  24
#define FAMILY_RS300                   25  // including Device ID: RS300,RS300VE,RS300M,RS300ML
#define FAMILY_RS400                   26  // including Device ID: RS400,RS400M
#define FAMILY_RS480                   27  // including Device ID: RS480,RS480M
#define FAMILY_RC410                   28  // including Device ID: RC410,RC410M
#define FAMILY_RS600                   29  // including Device ID: RS600,RS600M
//Integrated ASICs continued from 65

//
// *** CHIP FAMILIES ***
//  from rprod.h plus 30
// Chip family - sorted by growing capability of the chip.
//
//#define FAMILY_UNKNOWN               30
//#define FAMILY_VTB,                  31       // ATI-264VT3 family
//#define FAMILY_VTB_PLUS,             32       // ATI-264VT3 UMC family
//#define FAMILY_VTB_VT4,              33       // ATI-264VT4 family
//#define FAMILY_GTB,                  34       // 3D RAGE II family
//#define FAMILY_LTG,                  35       // 3D RAGE LT-G family
//#define FAMILY_GTB_PLUS,             36       // 3D RAGE II+ family
//#define FAMILY_GTB_IIC,              37       // 3D RAGE IIC family
//#define FAMILY_GTC,                  38       // 3D RAGE PRO family
//#define FAMILY_LT_PRO,               39       // 3D RAGE LT PRO family
//#define FAMILY_RAGE_XL,              40       // 3D RAGE XL
//#define FAMILY_RAGE_XC               41       // 3D RAGE XC
//
// Define Vendor ID.
//

#define FAMILY_MORPHEUS                 42
#define FAMILY_RV200                    FAMILY_MORPHEUS
#define FAMILY_M7                       43

#define FAMILY_KHAN                     44
#define FAMILY_R300                     FAMILY_KHAN
#define FAMILY_RV350                    45
#define FAMILY_M10                      46

#define FAMILY_IRIS                     47
#define FAMILY_RV250                    FAMILY_IRIS
#define FAMILY_M9                       48

#define FAMILY_ARGUS                    49
#define FAMILY_RV280                    FAMILY_ARGUS

#define FAMILY_R400                     50

#define FAMILY_M9PLUS                   51

#define FAMILY_R350                     52

#define FAMILY_R360                     53

#define FAMILY_RV380                    54
#define FAMILY_M24                      55

#define FAMILY_LOKI                     56
#define FAMILY_R420                     FAMILY_LOKI // R423 shares the same family id with LOKI

#define FAMILY_M18                      57          // M28 shares the same family id with M18

#define FAMILY_ALTO                     58
#define FAMILY_RV410                    FAMILY_ALTO

#define FAMILY_M26                      59


#define FAMILY_R520                     60

#define FAMILY_M58                      61          // mobile version of R520

#define FAMILY_RV5XX                    62          // RV530, RV535, RV515

#define FAMILY_MV5X                     63          // mobile version of RV5xx

//Integrated ASICS
#define FAMILY_RS690                    65          // including Device ID: RS690,RS690C,RS690M,RS690MC,RS690T; RS740, RS740M

#define FAMILY_R600                     70          // Pele

#define FAMILY_RV6XX                    71          // RV6xx

#define FAMILY_MV6X                     72          // Mobile version of RV6xx

//R600 based integrated ASICs Family ID start from 75
#define FAMILY_RS780                    75          //RS780/RS780M Functionality level will be defined by revision ID.


#define FAMILY_R700                     80          //FAMILY_RV7XX    // FAMILY_R700 will be removed later

#define FAMILY_RV7XX                    81          // WEKIVA/RV7xx

#define FAMILY_MV7X                     82          // Mobile version of RV7xx

#define FAMILY_KONG                     85          // Fusion Roadrunner project - SwiftGPU100/200/300

#define FAMILY_EVERGREEN                90          // EVERGREEN
#define FAMILY_RV8XX                    90          // Deprecated

#define FAMILY_MANHATTAN                91          // MANHATTAN
#define FAMILY_MV8X                     91          // Deprecated

#define FAMILY_SUMO                     95          // Fusion Llano/Ontario project - SuperSumo/Sumo/Wrestler variants

#define FAMILY_NI                      100          // Northern Islands: Ibiza, Cozumel, Kauai
#define FAMILY_NORTHERNISLAND          FAMILY_NI    // Keeping the original name for backward compatibility.  FAMILY_NI follows the new naming convention

#define FAMILY_SI                      110          // Southern Islands: Tahiti (P), Pitcairn (PM), Cape Verde (M), Bali (V)

#define FAMILY_TN                      105          // Fusion Trinity: Devastator - DVST (M), Scrapper (V)

#define FAMILY_CI                      120          // Sea Islands: Hawaii (P), Maui (P), Bonaire (M)

#define FAMILY_KV                      125          // Fusion Kaveri: Spectre, Spooky; Fusion Kabini: Kalindi

#define FAMILY_VI                      130          // Volcanic Islands: Iceland (V), Tonga (M)

#define FAMILY_CZ                      135          // Carrizo, Nolan, Amur

#define FAMILY_PI                      140          // Pirate Islands

#define ATI_VENDOR_ID                   0x1002


#endif  // _ATIID_H
