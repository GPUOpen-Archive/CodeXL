// Copyright (c) 2009-2017 Advanced Micro Devices, Inc. All rights reserved.
#ifndef _SI_ID_H
#define _SI_ID_H

enum
{
    SI_TAHITI_P_A11      = 1,
    SI_TAHITI_P_A0       = SI_TAHITI_P_A11,      //A0 is alias of A11
    SI_TAHITI_P_A21      = 5,
    SI_TAHITI_P_B0       = SI_TAHITI_P_A21,      //B0 is alias of A21
    SI_TAHITI_P_A22      = 6,
    SI_TAHITI_P_B1       = SI_TAHITI_P_A22,      //B1 is alias of A22

    SI_PITCAIRN_PM_A11   = 20,
    SI_PITCAIRN_PM_A0    = SI_PITCAIRN_PM_A11,   //A0 is alias of A11
    SI_PITCAIRN_PM_A12   = 21,
    SI_PITCAIRN_PM_A1    = SI_PITCAIRN_PM_A12,   //A1 is alias of A12

    SI_CAPEVERDE_M_A11   = 40,
    SI_CAPEVERDE_M_A0    = SI_CAPEVERDE_M_A11,   //A0 is alias of A11
    SI_CAPEVERDE_M_A12   = 41,
    SI_CAPEVERDE_M_A1    = SI_CAPEVERDE_M_A12,   //A1 is alias of A12

    SI_OLAND_M_A0        = 60,

    SI_HAINAN_V_A0       = 70,

    SI_UNKNOWN           = 0xFF
};


#define ASICREV_IS_TAHITI_P(eChipRev)     (eChipRev < SI_PITCAIRN_PM_A11)
#define ASICREV_IS_PITCAIRN_PM(eChipRev)  ((eChipRev >= SI_PITCAIRN_PM_A11) && (eChipRev < SI_CAPEVERDE_M_A11))
#define ASICREV_IS_CAPEVERDE_M(eChipRev)  ((eChipRev >= SI_CAPEVERDE_M_A11) && (eChipRev < SI_OLAND_M_A0))
#define ASICREV_IS_OLAND_M(eChipRev)      ((eChipRev >= SI_OLAND_M_A0) && (eChipRev < SI_HAINAN_V_A0))
#define ASICREV_IS_HAINAN_V(eChipRev)     (eChipRev >= SI_HAINAN_V_A0)


//
// TAHITI ISLAND device IDs (Performance segment)
//
#define DEVICE_ID_SI_TAHITI_P_6780              0x6780    //Tahiti XT GL
#define DEVICE_ID_SI_TAHITI_P_6784              0x6784    //obsolete
#define DEVICE_ID_SI_TAHITI_P_6788              0x6788    //Tahiti XT GL (FirePro)
#define DEVICE_ID_SI_TAHITI_P_678A              0x678A    //Tahiti PRO GL
#define DEVICE_ID_SI_TAHITI_P_6790              0x6790    //Aruba XT
#define DEVICE_ID_SI_TAHITI_P_6791              0x6791    //Malta
#define DEVICE_ID_SI_TAHITI_P_6792              0x6792    //Aruba PRO
#define DEVICE_ID_SI_TAHITI_P_6798              0x6798    //Tahiti XT
#define DEVICE_ID_SI_TAHITI_P_6799              0x6799    //New Zeland
#define DEVICE_ID_SI_TAHITI_P_679A              0x679A    //Tahiti PRO
#define DEVICE_ID_SI_TAHITI_P_679B              0x679B    //Malta
#define DEVICE_ID_SI_TAHITI_P_679E              0x679E    //Tahiti LE
#define DEVICE_ID_SI_TAHITI_P_679F              0x679F    //Aruba Pro

// TAHITI ASIC internal revision number
#define INTERNAL_REV_SI_TAHITI_P_A11            0x00      // First spin of Tahiti
#define INTERNAL_REV_SI_TAHITI_P_A0             INTERNAL_REV_SI_TAHITI_P_A11
#define INTERNAL_REV_SI_TAHITI_P_A21            0x01      // Second spin of Tahiti
#define INTERNAL_REV_SI_TAHITI_P_B0             INTERNAL_REV_SI_TAHITI_P_A21
#define INTERNAL_REV_SI_TAHITI_P_A22            0x02      // Third spin of Tahiti
#define INTERNAL_REV_SI_TAHITI_P_B1             INTERNAL_REV_SI_TAHITI_P_A22

#define DEVICE_ID_SI_TAHITI_P_LITE_PALLADIUM    0x0022    // Palladium ID


//
// PITCAIRN/WIMBLEDON device IDs (Performance to Mainstream segment)
//
#define DEVICE_ID_SI_PITCAIRN_PM_6808           0x6808    //Pitcairn GL1
#define DEVICE_ID_SI_PITCAIRN_PM_6809           0x6809    //Pitcairn GL2
#define DEVICE_ID_SI_PITCAIRN_PM_6810           0x6810    //Curacao XT; Trinidad XT
#define DEVICE_ID_SI_PITCAIRN_PM_6811           0x6811    //Curacao Pro; Trinidad Pro
#define DEVICE_ID_SI_PITCAIRN_PM_6816           0x6816    //Curacao XT
#define DEVICE_ID_SI_PITCAIRN_PM_6817           0x6817    //Curacao PRO
#define DEVICE_ID_SI_PITCAIRN_PM_6818           0x6818    //Pitcairn XT
#define DEVICE_ID_SI_PITCAIRN_PM_6819           0x6819    //Pitcairn PRO
#define DEVICE_ID_SI_PITCAIRN_PM_684C           0x684C    //Pitcairn GL

#define DEVICE_ID_SI_PITCAIRN_PM_6800           0x6800    //Wimbledon XT
#define DEVICE_ID_SI_PITCAIRN_PM_6801           0x6801    //Wimbledon PRO/LP
#define DEVICE_ID_SI_PITCAIRN_PM_6802           0x6802    //Wimbledon GL
#define DEVICE_ID_SI_PITCAIRN_PM_6806           0x6806    //Neptune XT

// TRINIDAD PCI Reivsion IDs
#define PRID_SI_PITCAIRN_TRINIDAD_81            0x81      // 0x6810: Trinidad XT; 0x6811: Trinidad Pro

#define ASICDID_IS_NEPTUNE(wDID)                ((wDID == DEVICE_ID_SI_PITCAIRN_PM_6806))

#define ASICID_IS_TRINIDAD(wDID, bRID)          ((bRID == PRID_SI_PITCAIRN_TRINIDAD_81) && \
                                                 ((wDID == DEVICE_ID_SI_PITCAIRN_PM_6810) || (wDID == DEVICE_ID_SI_PITCAIRN_PM_6811)))

// PITCAIRN ASIC internal revision number
#define INTERNAL_REV_SI_PITCAIRN_PM_A11         0x00      // First spin of Pitcairn
#define INTERNAL_REV_SI_PITCAIRN_PM_A0          INTERNAL_REV_SI_PITCAIRN_PM_A11
#define INTERNAL_REV_SI_PITCAIRN_PM_A12         0x01      // Second spin of Pitcairn
#define INTERNAL_REV_SI_PITCAIRN_PM_A1          INTERNAL_REV_SI_PITCAIRN_PM_A12


//
// CAPE VERDE/HEATHROW/CHELSEA/VENUS/TROPO device IDs (Mainstream segment)
//
#define DEVICE_ID_SI_CAPEVERDE_M_6828           0x6828    //Cape Verde GL XT
#define DEVICE_ID_SI_CAPEVERDE_M_6829           0x6829    //unused
#define DEVICE_ID_SI_CAPEVERDE_M_682C           0x682C    //Cape Verde GL PRO
#define DEVICE_ID_SI_CAPEVERDE_M_6830           0x6830    //Summer Palace XT AIO
#define DEVICE_ID_SI_CAPEVERDE_M_6831           0x6831    //AIO Great Wall XT
#define DEVICE_ID_SI_CAPEVERDE_M_6835           0x6835    //Cape Verde PRX
#define DEVICE_ID_SI_CAPEVERDE_M_6837           0x6837    //Cape Verde LE
#define DEVICE_ID_SI_CAPEVERDE_M_6838           0x6838    //Cape Verde XTX
#define DEVICE_ID_SI_CAPEVERDE_M_6839           0x6839    //Cape Verde XT
#define DEVICE_ID_SI_CAPEVERDE_M_683B           0x683B    //Cape Verde PRO
#define DEVICE_ID_SI_CAPEVERDE_M_683D           0x683D    //Cape Verde XT
#define DEVICE_ID_SI_CAPEVERDE_M_683F           0x683F    //Cape Verde PRO

#define DEVICE_ID_SI_CAPEVERDE_M_6820           0x6820    //Venus XTX; Tropo XTX/XT
#define DEVICE_ID_SI_CAPEVERDE_M_6821           0x6821    //Venus XT; Tropo PRO
#define DEVICE_ID_SI_CAPEVERDE_M_6822           0x6822    //Venus Pro MCM
#define DEVICE_ID_SI_CAPEVERDE_M_6823           0x6823    //Venus PRO; Tropo UL
#define DEVICE_ID_SI_CAPEVERDE_M_6824           0x6824    //Chelsea PROA
#define DEVICE_ID_SI_CAPEVERDE_M_6825           0x6825    //Heathrow XT
#define DEVICE_ID_SI_CAPEVERDE_M_6826           0x6826    //Chelsea LP
#define DEVICE_ID_SI_CAPEVERDE_M_6827           0x6827    //Heathrow PRO
#define DEVICE_ID_SI_CAPEVERDE_M_682A           0x682A    //Venus Pro MCM
#define DEVICE_ID_SI_CAPEVERDE_M_682B           0x682B    //Venus LE; Tropo PROL
#define DEVICE_ID_SI_CAPEVERDE_M_682D           0x682D    //Chelsea XT
#define DEVICE_ID_SI_CAPEVERDE_M_682F           0x682F    //Chelsea PRO

// TROPO PCI Reivsion IDs
#define PRID_SI_CAPEVERDE_TROPO_81                0x81      // 0x6820: Tropo XTX
#define PRID_SI_CAPEVERDE_TROPO_83                0x83      // 0x6820: Tropo XT; 0x6821: Tropo PRO; 0x6823: Tropo UL; 0x682B: Tropo PROL

#define ASICDID_IS_VENUS(wDID)                  ((wDID == DEVICE_ID_SI_CAPEVERDE_M_6820) || \
                                                 (wDID == DEVICE_ID_SI_CAPEVERDE_M_6821) || \
                                                 (wDID == DEVICE_ID_SI_CAPEVERDE_M_6822) || \
                                                 (wDID == DEVICE_ID_SI_CAPEVERDE_M_6823) || \
                                                 (wDID == DEVICE_ID_SI_CAPEVERDE_M_682A) || \
                                                 (wDID == DEVICE_ID_SI_CAPEVERDE_M_682B))

#define ASICID_IS_TROPO(wDID, bRID)             (((wDID == DEVICE_ID_SI_CAPEVERDE_M_6820) && ((bRID == PRID_SI_CAPEVERDE_TROPO_81) || (bRID == PRID_SI_CAPEVERDE_TROPO_83))) || \
                                                 (((wDID == DEVICE_ID_SI_CAPEVERDE_M_6821) || (wDID == DEVICE_ID_SI_CAPEVERDE_M_6823) || (wDID == DEVICE_ID_SI_CAPEVERDE_M_682B)) && (bRID == PRID_SI_CAPEVERDE_TROPO_83)))

// CAPE VERDE ASIC internal revision number
#define INTERNAL_REV_SI_CAPEVERDE_M_A11         0x00      // First spin of Cape Verde
#define INTERNAL_REV_SI_CAPEVERDE_M_A0          INTERNAL_REV_SI_CAPEVERDE_M_A11
#define INTERNAL_REV_SI_CAPEVERDE_M_A12         0x01      // Second spin of Cape Verde
#define INTERNAL_REV_SI_CAPEVERDE_M_A1          INTERNAL_REV_SI_CAPEVERDE_M_A12

#define DEVICE_ID_SI_CAPEVERDE_M_LITE_PALLADIUM 0x0026    // Palladium ID


//
// OLAND/MARS/OPAL/LITHO device IDs
//
#define DEVICE_ID_SI_OLAND_M_6608               0x6608    // Oland PRO GL
#define DEVICE_ID_SI_OLAND_M_6610               0x6610    // Oland 128 XT
#define DEVICE_ID_SI_OLAND_M_6611               0x6611    // Oland 128 PRO (6CU)
#define DEVICE_ID_SI_OLAND_M_6613               0x6613    // Oland 128 PRO (5CU)
#define DEVICE_ID_SI_OLAND_M_6631               0x6631    // Oland 64 LE

#define DEVICE_ID_SI_OLAND_M_6600               0x6600    // Mars 128/64 XT; Litho XT
#define DEVICE_ID_SI_OLAND_M_6601               0x6601    // Mars 128/64 PRO, Mars 128 LP
#define DEVICE_ID_SI_OLAND_M_6602               0x6602    // Mars 128 XTX
#define DEVICE_ID_SI_OLAND_M_6603               0x6603    // obsolete
#define DEVICE_ID_SI_OLAND_M_6604               0x6604    // Opal 128 XT; Litho Pro
#define DEVICE_ID_SI_OLAND_M_6605               0x6605    // Opal 128 PRO; Litho UL
#define DEVICE_ID_SI_OLAND_M_6606               0x6606    // Mars 128 XTX (Generic)
#define DEVICE_ID_SI_OLAND_M_6607               0x6607    // Mars LE
#define DEVICE_ID_SI_OLAND_M_6620               0x6620    // obsolete
#define DEVICE_ID_SI_OLAND_M_6621               0x6621    // Mars 64 PRO
#define DEVICE_ID_SI_OLAND_M_6623               0x6623    // Mars 64 LE

// LITHO PCI Reivsion IDs
#define PRID_SI_OLAND_LITHO_80                    0x80      // 
#define PRID_SI_OLAND_LITHO_81                    0x81      // 0x6600: Litho XT; 0x6604: Litho Pro; 0x6605: Litho UL

#define ASICDID_IS_MARS(wDID)                   ((wDID == DEVICE_ID_SI_OLAND_M_6600) || \
                                                 (wDID == DEVICE_ID_SI_OLAND_M_6601) || \
                                                 (wDID == DEVICE_ID_SI_OLAND_M_6602) || \
                                                 (wDID == DEVICE_ID_SI_OLAND_M_6606) || \
                                                 (wDID == DEVICE_ID_SI_OLAND_M_6607) || \
                                                 (wDID == DEVICE_ID_SI_OLAND_M_6621) || \
                                                 (wDID == DEVICE_ID_SI_OLAND_M_6623))

#define ASICDID_IS_OPAL(wDID)                   ((wDID == DEVICE_ID_SI_OLAND_M_6604) || \
                                                 (wDID == DEVICE_ID_SI_OLAND_M_6605))

#define ASICID_IS_LITHO(wDID, bRID)             (((wDID == DEVICE_ID_SI_OLAND_M_6600) || (wDID == DEVICE_ID_SI_OLAND_M_6604) || (wDID == DEVICE_ID_SI_OLAND_M_6605)) && \
                                                 (bRID == PRID_SI_OLAND_LITHO_81))

// OLAND ASIC internal revision number
#define INTERNAL_REV_SI_OLAND_M_A0              0x00      // First spin of Oland

//
// HAINAN/SUN/EXO device IDs
//
#define DEVICE_ID_SI_HAINAN_V_6660              0x6660    //Sun XT; Exo XT/PRO
#define DEVICE_ID_SI_HAINAN_V_6663              0x6663    //Sun PRO; Exo ULT
#define DEVICE_ID_SI_HAINAN_V_6664              0x6664    //Jet XT
#define DEVICE_ID_SI_HAINAN_V_6665              0x6665    //Jet PRO; Exo UL
#define DEVICE_ID_SI_HAINAN_V_6667              0x6667    //Jet ULT; Exo ULP
#define DEVICE_ID_SI_HAINAN_V_666F              0x666F    //Sun LE

// EXO PCI Reivsion IDs
#define PRID_SI_HAINAN_EXO_81                     0x81      // 0x6660: Exo XT
#define PRID_SI_HAINAN_EXO_83                     0x83      // 0x6660: Exo Pro; 0x6663: Exo ULT; 0x6665: Exo UL; 0x6667: Exo ULP

#define ASICDID_IS_JET(wDID)                    ((wDID == DEVICE_ID_SI_HAINAN_V_6664) || \
                                                 (wDID == DEVICE_ID_SI_HAINAN_V_6665) || \
                                                 (wDID == DEVICE_ID_SI_HAINAN_V_6667))

#define ASICID_IS_EXO(wDID, bRID)               (((wDID == DEVICE_ID_SI_HAINAN_V_6660) && ((bRID == PRID_SI_HAINAN_EXO_81) || (bRID == PRID_SI_HAINAN_EXO_83))) || \
                                                 (((wDID == DEVICE_ID_SI_HAINAN_V_6663) || (wDID == DEVICE_ID_SI_HAINAN_V_6665) || (wDID == DEVICE_ID_SI_HAINAN_V_6667)) && (bRID == PRID_SI_HAINAN_EXO_83)))

// HAINAN ASIC internal revision number
#define INTERNAL_REV_SI_HAINAN_V_A0             0x00      // First spin of Hainan

#endif  // _SI_ID_H
