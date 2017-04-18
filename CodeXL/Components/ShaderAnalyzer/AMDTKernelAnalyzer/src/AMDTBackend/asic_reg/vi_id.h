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

enum {
    VI_ICELAND_M_A0   = 1,

    VI_TONGA_P_A0     = 20,
    VI_TONGA_P_A1     = 21,

    VI_BERMUDA_P_A0   = 40,

    VI_FIJI_P_A0      = 60,

    VI_ELLESMERE_P_A0 = 80,
    VI_ELLESMERE_P_A1 = 81,

    VI_BAFFIN_M_A0    = 90,
    VI_BAFFIN_M_A1    = 91,

	VI_LEXA_V_A0      = 0xA0,

    VI_UNKNOWN        = 0xFF
};


#define ASICREV_IS_ICELAND_M(eChipRev)   (eChipRev < VI_TONGA_P_A0)
#define ASICREV_IS_TONGA_P(eChipRev)    ((eChipRev >= VI_TONGA_P_A0)     && (eChipRev < VI_BERMUDA_P_A0))
#define ASICREV_IS_BERMUDA_P(eChipRev)  ((eChipRev >= VI_BERMUDA_P_A0)   && (eChipRev < VI_FIJI_P_A0))
#define ASICREV_IS_FIJI_P(eChipRev)     ((eChipRev >= VI_FIJI_P_A0)      && (eChipRev < VI_ELLESMERE_P_A0))
#define ASICREV_IS_ELLESMERE_P(eChipRev)((eChipRev >= VI_ELLESMERE_P_A0) && (eChipRev < VI_BAFFIN_M_A0))
#define ASICREV_IS_BAFFIN_M(eChipRev)   ((eChipRev >= VI_BAFFIN_M_A0)    && (eChipRev < VI_LEXA_V_A0))
#define ASICREV_IS_LEXA_V(eChipRev)      (eChipRev >= VI_LEXA_V_A0)


//
// TONGA/AMETHYST/ANTIGUA/MAGNETO device IDs (performance segment)
//
#define DEVICE_ID_VI_TONGA_P_6920               0x6920  // AmethystP
#define DEVICE_ID_VI_TONGA_P_6921               0x6921  // Amethyst XT; Magneto
#define DEVICE_ID_VI_TONGA_P_6928               0x6928  // Tonga GL XT
#define DEVICE_ID_VI_TONGA_P_6929               0x6929  // Tonga GL32 Pro
#define DEVICE_ID_VI_TONGA_P_692B               0x692B  // Tonga GL PRO
#define DEVICE_ID_VI_TONGA_P_692F               0x692F  // Tonga GL PRO VF
#define DEVICE_ID_VI_TONGA_P_6930               0x6930  // TongaP; TongaP XTA
#define DEVICE_ID_VI_TONGA_P_6938               0x6938  // Tonga XT; Antigua XT
#define DEVICE_ID_VI_TONGA_P_6939               0x6939  // Tonga PRO; Antigua PRO

// TONGA/ANTIGUA PCI Revision IDs 
#define PRID_VI_TONGA_00                         0x00  // AmethystP XTA; Magneto
#define PRID_VI_TONGA_01                         0x01  // AmethystP PROA
#define PRID_VI_TONGA_F0                         0xF0  // TongaP XTA; Tonga
#define PRID_VI_TONGA_F1                         0xF1  // TongaP PROA; 0x6939: Antigua PRO; 0x6938: Antigua XT
#define PRID_VI_TONGA_FF                         0xFF  // TongaP LEA

#define ASICID_IS_AMETHYST(wDID, bRID)			((wDID == DEVICE_ID_VI_TONGA_P_6921) && (bRID == PRID_VI_TONGA_00))

#define ASICID_IS_ANTIGUA(wDID, bRID)           (((wDID == DEVICE_ID_VI_TONGA_P_6939) && (bRID == PRID_VI_TONGA_F1)) || \
                                                 ((wDID == DEVICE_ID_VI_TONGA_P_6938) && (bRID == PRID_VI_TONGA_F1)))

#define ASICID_IS_TONGA_P(wDID, bRID)			 (((wDID == DEVICE_ID_VI_TONGA_P_6930) && ((bRID == PRID_VI_TONGA_F0) || (bRID == PRID_VI_TONGA_F1) || (bRID == PRID_VI_TONGA_FF))) || \
                                                  ((wDID == DEVICE_ID_VI_TONGA_P_6920) && ((bRID == PRID_VI_TONGA_00) || (bRID == PRID_VI_TONGA_01))))

#define DEVICE_ID_VI_TONGA_P_PALLADIUM          0x00    // Palladium ID
#define DEVICE_ID_VI_TONGA_P_LITE_PALLADIUM     0x48    // Palladium ID

// TONGA ASIC internal revision number
#define INTERNAL_REV_VI_TONGA_P_A0              0x00    // First spin of Tonga
#define INTERNAL_REV_VI_TONGA_P_A1              0x01    // Second spin of Tonga

//
// ICELAND/TOPAZ/MESO device IDs (mainstream segment)
//
#define DEVICE_ID_VI_ICELAND_M_6900             0x6900  // Topaz XT; Meso XT/PRO; Weston XT/PRO
#define DEVICE_ID_VI_ICELAND_M_6901             0x6901  // Topaz Pro
#define DEVICE_ID_VI_ICELAND_M_6902             0x6902  // Topaz XTL
#define DEVICE_ID_VI_ICELAND_M_6903             0x6903  // Unused - Previous Topaz LE
#define DEVICE_ID_VI_ICELAND_M_6907             0x6907  // Topaz LE; Meso LE

// MESO PCI Revision IDs 
#define PRID_VI_ICELAND_MESO_81                   0x81  // 0x6900: MESO XT
#define PRID_VI_ICELAND_MESO_83                   0x83  // 0x6900: MESO Pro
#define PRID_VI_ICELAND_MESO_87                   0x87  // 0x6907: MESO LE

// WESTON PCI Revision IDs 
#define PRID_VI_ICELAND_WESTON_C1                 0xC1  // 0x6900: WESTON XT
#define PRID_VI_ICELAND_WESTON_C3                 0xC3  // 0x6900: WESTON Pro

#define ASICID_IS_MESO(wDID, bRID)              (((wDID == DEVICE_ID_VI_ICELAND_M_6900) && ((bRID == PRID_VI_ICELAND_MESO_81) || (bRID == PRID_VI_ICELAND_MESO_83))) || \
                                                 ((wDID == DEVICE_ID_VI_ICELAND_M_6907) &&  (bRID == PRID_VI_ICELAND_MESO_87)))

#define ASICID_IS_WESTON(wDID, bRID)            ((wDID == DEVICE_ID_VI_ICELAND_M_6900) && ((bRID == PRID_VI_ICELAND_WESTON_C1) || (bRID == PRID_VI_ICELAND_WESTON_C3)))

#define DEVICE_ID_VI_ICELAND_M_PALLADIUM        0x47    // Palladium ID
#define DEVICE_ID_VI_ICELAND_M_LITE_PALLADIUM   0x00    // Palladium ID

// ICELAND ASIC internal revision number
#define INTERNAL_REV_VI_ICELAND_M_A0            0x00    // First spin of ICELAND

//
// Bermuda device IDs (performance segment)
//

#define DEVICE_ID_VI_BERMUDA_P_PALLADIUM        0x49    // Palladium ID
#define DEVICE_ID_VI_BERMUDA_P_LITE_PALLADIUM	0x00    // Palladium ID

// BERMUDA ASIC internal revision number
#define INTERNAL_REV_VI_BERMUDA_P_A0            0x00    // First spin of Bermuda


//
// Fiji device IDs (performance segment)
//
#define DEVICE_ID_VI_FIJI_P_7300                0x7300  // Fiji DID
#define DEVICE_ID_VI_FIJI_P_730F                0x730F  // Fiji VF DID

// FIJI PCI Revision IDs 
#define PRID_VI_FIJI_00                         0x00  // unfused
#define PRID_VI_FIJI_C0                         0xC0  // not used
#define PRID_VI_FIJI_C1                         0xC1  // Fiji Server (Gemini)
#define PRID_VI_FIJI_C8                         0xC8  // Fiji XT
#define PRID_VI_FIJI_C9                         0xC9  // Fiji Gemini
#define PRID_VI_FIJI_CA                         0xCA  // Fiji LP
#define PRID_VI_FIJI_CB                         0xCB  // Fiji Pro

#define DEVICE_ID_VI_FIJI_P_LITE_PALLADIUM      0x4A    // Palladium ID

// FIJI ASIC internal revision number
#define INTERNAL_REV_VI_FIJI_P_A0               0x00    // First spin of Fiji


//
// Ellesmere device IDs (performance segment)
//
#define DEVICE_ID_VI_ELLESMERE_P_67C0 		0x67C0   // EllesmereM GL XT
#define DEVICE_ID_VI_ELLESMERE_P_67C1 		0x67C1   // EllesmereM GL PRO
#define DEVICE_ID_VI_ELLESMERE_P_67C2 		0x67C2   // Ellesmere Server XT
#define DEVICE_ID_VI_ELLESMERE_P_67C4 		0x67C4   // Ellesmere GL XT
#define DEVICE_ID_VI_ELLESMERE_P_67C7 		0x67C7   // Ellesmere GL PRO
#define DEVICE_ID_VI_ELLESMERE_P_67DF 		0x67DF   // Ellesmere consumer, EllesmereM 
#define DEVICE_ID_VI_ELLESMERE_P_67D0 		0x67D0   // Ellesmere VF

//Ellesmere Kickers DID
#define DEVICE_ID_VI_ELLESMERE_P_67C8 		0x67C8   // EllesmereM GL XT Kicker
#define DEVICE_ID_VI_ELLESMERE_P_67C9 		0x67C9   // EllesmereM GL PRO  Kicker
#define DEVICE_ID_VI_ELLESMERE_P_67CA 		0x67CA   // Ellesmere Server XT Kicker
#define DEVICE_ID_VI_ELLESMERE_P_67CC 		0x67CC   // Ellesmere GL XT Kicker
#define DEVICE_ID_VI_ELLESMERE_P_67CF 		0x67CF   // Ellesmere GL PRO Kicker

// Ellesmere PCI Revision IDs 
#define PRID_VI_ELLESMERE_00                    0x00  // GL, Server
#define PRID_VI_ELLESMERE_C0                    0xC0  // Mobile XT, AIO XTA
#define PRID_VI_ELLESMERE_C4                    0xC4  // Chipdown XT, AIO PROA
#define PRID_VI_ELLESMERE_C5                    0xC5  // Chipdown PRO, AIO LEA
#define PRID_VI_ELLESMERE_C7                    0xC7  // Desktop XT
#define PRID_VI_ELLESMERE_CC                    0xCC  // Gemini XT
#define PRID_VI_ELLESMERE_CD                    0xCD  // Gemini PRO
#define PRID_VI_ELLESMERE_CF                    0xCF  // Desktop PRO
#define PRID_VI_ELLESMERE_FF                    0xFF  // LE

#define PRID_VI_ELLESMERE_04                    0x04  // Mobile Eng. To be deleted
#define PRID_VI_ELLESMERE_05                    0x05  // Mobile Eng. To be deleted

#define DEVICE_ID_VI_ELLESMERE_P_LITE_PALLADIUM 0x4B    // Palladium ID

// ELLESMERE ASIC internal revision number
#define INTERNAL_REV_VI_ELLESMERE_P_A0          0x00    // First spin of Ellesmere
#define INTERNAL_REV_VI_ELLESMERE_P_A1          0x01    // Second spin of Ellesmere

//
// Baffin device IDs (mainstream segment)
//
#define DEVICE_ID_VI_BAFFIN_M_67E0           0x67E0  // BaffinM GL XT
#define DEVICE_ID_VI_BAFFIN_M_67E3           0x67E3  // Baffin Desktop GL XT
#define DEVICE_ID_VI_BAFFIN_M_67E8           0x67E8  // BaffinM GL Pro
#define DEVICE_ID_VI_BAFFIN_M_67EB           0x67EB  // BaffinM Server
#define DEVICE_ID_VI_BAFFIN_M_67EF           0x67EF  // BaffinM, Baffin Desktop
#define DEVICE_ID_VI_BAFFIN_M_67FF           0x67FF  // BaffinM XPA

//Baffin Kickers DID
#define DEVICE_ID_VI_BAFFIN_M_67E1           0x67E1  // BaffinM GL XT Kicker
#define DEVICE_ID_VI_BAFFIN_M_67E7           0x67E7  // Baffin Desktop GL XT Kicker
#define DEVICE_ID_VI_BAFFIN_M_67E9           0x67E9  // BaffinM GL Pro Kicker

// Baffin PCI Revision IDs 
#define PRID_VI_BAFFIN_00                    	0x00  // GL, Mobile Server
#define PRID_VI_BAFFIN_C0                    	0xC0  // Mobile ULA, Mobile XPA, Mobile XLA
#define PRID_VI_BAFFIN_C1                    	0xC1  // Mobile XT, Mobile XPA
#define PRID_VI_BAFFIN_C3                    	0xC3  // Desktop XL
#define PRID_VI_BAFFIN_C5                    	0xC5  // Mobile PRO
#define PRID_VI_BAFFIN_C7                    	0xC7  // Mobile PROA
#define PRID_VI_BAFFIN_CF                    	0xCF  // Desktop PRO
#define PRID_VI_BAFFIN_EF                    	0xEF  // Mobile LEA

#define PRID_VI_BAFFIN_08                    	0x08  // Mobile Eng. To be deleted
#define PRID_VI_BAFFIN_FF                    	0xFF  // Mobile LEA. To be deleted

#define DEVICE_ID_VI_BAFFIN_M_LITE_PALLADIUM 0x4C    // Palladium ID


// BAFFIN ASIC internal revision number
#define INTERNAL_REV_VI_BAFFIN_M_A0          0x00    // First spin of Baffin
#define INTERNAL_REV_VI_BAFFIN_M_A1          0x01    // Second spin of Baffin

//
// Lexa device IDs (value segment)
//
#define DEVICE_ID_VI_LEXA_V_6980           0x6980  // Lexa DID

// Lexa PCI Revision IDs 
#define PRID_VI_LEXA_00                    0x00    // unfused

// LEXA ASIC internal revision number
#define INTERNAL_REV_VI_LEXA_V_A0          0x00    // First spin of Lexa

#endif  // _VI_ID_H
