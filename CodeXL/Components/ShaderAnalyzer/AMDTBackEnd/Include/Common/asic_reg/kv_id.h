// Copyright (c) 2011-2017 Advanced Micro Devices, Inc. All rights reserved.
#ifndef KV_ID_H
#define KV_ID_H

// SW revision section
enum
{
    KV_SPECTRE_A0      = 0x01,       // KV1 with Spectre GFX core, 8-8-1-2 (CU-Pix-Primitive-RB)
    KV_SPOOKY_A0       = 0x41,       // KV2 with Spooky GFX core, including downgraded from Spectre core, 3-4-1-1 (CU-Pix-Primitive-RB)
    KB_KALINDI_A0      = 0x81,       // KB with Kalindi GFX core, 2-4-1-1 (CU-Pix-Primitive-RB)
    KB_KALINDI_A1      = 0x82,       // KB with Kalindi GFX core, 2-4-1-1 (CU-Pix-Primitive-RB)
    BV_KALINDI_A2      = 0x85,       // BV with Kalindi GFX core, 2-4-1-1 (CU-Pix-Primitive-RB)
    ML_GODAVARI_A0     = 0xa1,      // ML with Godavari GFX core, 2-4-1-1 (CU-Pix-Primitive-RB)
    ML_GODAVARI_A1     = 0xa2,      // ML with Godavari GFX core, 2-4-1-1 (CU-Pix-Primitive-RB)
    KV_UNKNOWN = 0xFF
};

#define ASICREV_IS_SPECTRE(eChipRev) ((eChipRev >= KV_SPECTRE_A0) && (eChipRev < KV_SPOOKY_A0))         // identify all versions of SPRECTRE and supported features set
#define ASICREV_IS_SPOOKY(eChipRev) ((eChipRev >= KV_SPOOKY_A0) && (eChipRev < KB_KALINDI_A0))          // identify all versions of SPOOKY and supported features set
#define ASICREV_IS_KALINDI(eChipRev) ((eChipRev >= KB_KALINDI_A0) && (eChipRev < KV_UNKNOWN))           // identify all versions of KALINDI and supported features set

// Following macros are subset of ASICREV_IS_KALINDI macro
#define ASICREV_IS_KALINDI_BHAVANI(eChipRev) ((eChipRev >= BV_KALINDI_A2) && (eChipRev < ML_GODAVARI_A0))   // identify all versions of BHAVANI and supported features set
#define ASICREV_IS_KALINDI_GODAVARI(eChipRev) ((eChipRev >= ML_GODAVARI_A0) && (eChipRev < KV_UNKNOWN)) // identify all versions of GODAVARI and supported features set

//
// Define Chip ID's for Kaveri family
//
#define DEVICE_ID_SPECTRE_MOBILE                  0x1304     // OBSOLETE - TO BE REMOVED    //FP3   soldered, 8-8-1-2 (CU-Pix-Primitive-RB)
#define DEVICE_ID_SPECTRE_MOBILE_1304             0x1304     // FP3   soldered, 8-8-1-2 (CU-Pix-Primitive-RB)
#define DEVICE_ID_SPECTRE_DESKTOP                 0x1305     // OBSOLETE - TO BE REMOVED    //FM2r2 socket,   8-8-1-2 (CU-Pix-Primitive-RB)
#define DEVICE_ID_SPECTRE_DESKTOP_1305            0x1305     // FM2r2 socket,   8-8-1-2 (CU-Pix-Primitive-RB)
#define DEVICE_ID_SPOOKY_MOBILE                   0x1306     // OBSOLETE - TO BE REMOVED    //FS2 socket, 3-4-1-1 (CU-Pix-Primitive-RB)
#define DEVICE_ID_SPECTRE_SL_MOBILE_1306          0x1306     // FP3   soldered, 4-4-1-1 (CU-Pix-Primitive-RB)
#define DEVICE_ID_SPOOKY_DESKTOP                  0x1307     // OBSOLETE - TO BE REMOVED    //FM3 socket,3-4-1-1 (CU-Pix-Primitive-RB) 
#define DEVICE_ID_SPECTRE_SL_DESKTOP_1307         0x1307     // FM2r2 socket,   4-4-1-1 (CU-Pix-Primitive-RB)
#define DEVICE_ID_SPECTRE_LITE_MOBILE_1309        0x1309     // FP3   soldered, 6-8-1-2 (CU-Pix-Primitive-RB)
#define DEVICE_ID_SPECTRE_LITE_MOBILE_130A        0x130A     // FP3   soldered, 6-8-1-2 (CU-Pix-Primitive-RB)
#define DEVICE_ID_SPECTRE_SL_MOBILE_130B          0x130B     // FP3   soldered, 4-4-1-1 (CU-Pix-Primitive-RB)
#define DEVICE_ID_SPECTRE_MOBILE_130C             0x130C     // FP3   soldered, 8-8-1-2 (CU-Pix-Primitive-RB)
#define DEVICE_ID_SPECTRE_LITE_MOBILE_130D        0x130D     // FP3   soldered, 6-8-1-2 (CU-Pix-Primitive-RB)
#define DEVICE_ID_SPECTRE_SL_MOBILE_130E          0x130E     // FP3   soldered, 4-4-1-1 (CU-Pix-Primitive-RB)
#define DEVICE_ID_SPECTRE_DESKTOP_130F            0x130F     // FM2r2 socket,   8-8-1-2 (CU-Pix-Primitive-RB)
#define DEVICE_ID_SPECTRE_WORKSTATION_1310        0x1310     // FM2r2 socket,   8-8-1-2 (CU-Pix-Primitive-RB)
#define DEVICE_ID_SPECTRE_WORKSTATION_1311        0x1311     // FM2r2 socket,   8-8-1-2 (CU-Pix-Primitive-RB)
#define DEVICE_ID_SPOOKY_DESKTOP_1312             0x1312     // FM2r2,          3-4-1-1 (CU-Pix-Primitive-RB)
#define DEVICE_ID_SPECTRE_LITE_DESKTOP_1313       0x1313     // FM2r2 socket,   6-8-1-2 (CU-Pix-Primitive-RB)
#define DEVICE_ID_SPECTRE_SL_DESKTOP_1315         0x1315     // FM2r2 socket,   4-4-1-1 (CU-Pix-Primitive-RB)
#define DEVICE_ID_SPOOKY_DESKTOP_1316             0x1316     // FM2r2 socket,   3-4-1-1 (CU-Pix-Primitive-RB)
#define DEVICE_ID_SPOOKY_MOBILE_1317              0x1317     // FP3,            3-4-1-1 (CU-Pix-Primitive-RB) 
#define DEVICE_ID_SPECTRE_SL_MOBILE_1318          0x1318     // FP3,            4-4-1-1 (CU-Pix-Primitive-RB), ACP disabled in SW
#define DEVICE_ID_SPECTRE_SL_EMBEDDED_131B        0x131B     // FP3 soldered,   4-4-1-1 (CU-Pix-Primitive-RB), ACP disabled in SW
#define DEVICE_ID_SPECTRE_EMBEDDED_131C           0x131C     // FP3 soldered,   8-8-1-2 (CU-Pix-Primitive-RB)
#define DEVICE_ID_SPECTRE_LITE_EMBEDDED_131D      0x131D     // FP3,            6-8-1-2 (CU-Pix-Primitive-RB)

// Define AMD's internal revision numbers.
#define INTERNAL_REV_SPECTRE_A0       0x00       // The First revision of SPECTRE GFX.

// Define AMD's external revision numbers.
#define EXTERNAL_REV_SPECTRE         0x00


//
// Define Chip ID's for Kabini
//
#define DEVICE_ID_KALINDI__9830         0x9830      // FT3 socket and FS1b socket
#define DEVICE_ID_KALINDI__9831         0x9831      // FT3 socket,  
#define DEVICE_ID_KALINDI__9832         0x9832      // FT3 socket,  
#define DEVICE_ID_KALINDI__9833         0x9833      // FT3 socket, 
#define DEVICE_ID_KALINDI__9834         0x9834      // FT3 socket,  
#define DEVICE_ID_KALINDI__9835         0x9835      // FT3 socket,  
#define DEVICE_ID_KALINDI__9836         0x9836      // FT3 socket and FS1b socket 
#define DEVICE_ID_KALINDI__9837         0x9837      // FT3 socket,  
#define DEVICE_ID_KALINDI__9838         0x9838      // FT3 socket and FS1b socket  



//
// Define Chip ID's for Temash
//
#define DEVICE_ID_TEMASH__9839          0x9839      // FT3 socket,  
#define DEVICE_ID_TEMASH__983a          0x983a      // FT3 socket,  
#define DEVICE_ID_TEMASH__983b          0x983b      // FT3 socket,  
#define DEVICE_ID_TEMASH__983c          0x983c      // FT3 socket,  
#define DEVICE_ID_TEMASH__983d          0x983d      // FT3 socket,  
#define DEVICE_ID_TEMASH__983e          0x983e      // FT3 socket,  
#define DEVICE_ID_TEMASH__983f          0x983f      // FT3 socket,  

//
// Define Chip ID's for Bhavani Desktop
// PLEASE DO NOT USE THESE DID! BV DOESN'T SUPPORT THEM!
//
#define DEVICE_ID_BHAVANI__98b0         0x98b0      // OBSOLETE - don't use 
#define DEVICE_ID_BHAVANI__98b1         0x98b1      // OBSOLETE - don't use 


// Define AMD's internal revision numbers.
#define INTERNAL_REV_KALINDI_A0       0x00       // The First revision of KALINDI GFX.
#define INTERNAL_REV_KALINDI_A1       0x01       // The Second revision of KALINDI GFX.
#define INTERNAL_REV_KALINDI_A2       0x02       // GF version of KALINDI GFX which is BHAVANI.

// Define AMD's external revision numbers.
#define EXTERNAL_REV_KALINDI         0x00

//
// Define Chip ID's for Godavari
//

// Mobile
#define DEVICE_ID_GODAVARI__9850         0x9850      // FT3 socket,  
#define DEVICE_ID_GODAVARI__9851         0x9851      // FT3 socket,  
#define DEVICE_ID_GODAVARI__9852         0x9852      // FT3 socket,  
#define DEVICE_ID_GODAVARI__9853         0x9853      // FT3 socket,  
#define DEVICE_ID_GODAVARI__9854         0x9854      // FT3 socket,  
#define DEVICE_ID_GODAVARI__9855         0x9855      // FT3 socket,  
#define DEVICE_ID_GODAVARI__9856         0x9856      // FT3 socket,  
#define DEVICE_ID_GODAVARI__9857         0x9857      // FT3 socket,  
#define DEVICE_ID_GODAVARI__9858         0x9858      // FT3 socket,  
#define DEVICE_ID_GODAVARI__9859         0x9859      // FT3 socket,  
#define DEVICE_ID_GODAVARI__985a         0x985a      // FT3 socket,  
#define DEVICE_ID_GODAVARI__985b         0x985b      // FT3 socket,  
#define DEVICE_ID_GODAVARI__985c         0x985c      // FT3 socket,  
#define DEVICE_ID_GODAVARI__985d         0x985d      // FT3 socket,  
#define DEVICE_ID_GODAVARI__985e         0x985e      // FT3 socket,  
#define DEVICE_ID_GODAVARI__985f         0x985f      // FT3 socket,  

// Define AMD's internal revision numbers.
#define INTERNAL_REV_GODAVARI_A0      0x00       // The First revision of GODAVARI GFX.
#define INTERNAL_REV_GODAVARI_A1      0x01       // The Second revision of GODAVARI GFX.

// Define AMD's external revision numbers.
#define EXTERNAL_REV_GODAVARI         0x00



// This macro will be remove later on (keep it for now for for capability purpose)
#define ASICREV_IS_GODAVARI(eChipRev) ((eChipRev >= ML_GODAVARI_A0) && (eChipRev < KV_UNKNOWN))       // identify all versions of KALINDI and supported features set



#endif  // KV_ID_H
