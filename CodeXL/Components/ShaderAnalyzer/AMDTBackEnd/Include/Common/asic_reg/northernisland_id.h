/*****************************************************************************\
*
*  Module Name    northernisland_id.h
*  Project        NORTHERNISLANDS
*  Devices        NORTHERNISLANDS
*
*  Description    Defining Device IDs, ASIC revision IDs for NORTHERNISLANDS
*
*
*  (c) 2009 Advanced Micro Devices, Inc. (unpublished)
*
*  All rights reserved.  This notice is intended as a precaution against
*  inadvertent publication and does not imply publication or any waiver
*  of confidentiality.  The year included in the foregoing notice is the
*  year of creation of the work.
*
*  LOG OF CHANGES
*
\*****************************************************************************/

#ifndef _NORTHERNISLAND_ID_H
#define _NORTHERNISLAND_ID_H

enum
{
    NI_CAYMAN_P_A11           = 1,

    NI_BARTS_PM_A11           = 20,

    NI_TURKS_M_A11            = 40,
    NI_TURKS_LOMBOK_M_A11     = 50,

    NI_CAICOS_V_A11           = 60,
    NI_CAICOS_V_A12           = 61,

    KAUAI_A11                 = 80,    // not a production part

    NORTHERNISLAND_UNKNOWN    = 0xFF
};


#define ASICREV_IS_CAYMAN_P(eChipRev)       (eChipRev < NI_BARTS_PM_A11)
#define ASICREV_IS_BARTS_PM(eChipRev)       ((eChipRev >= NI_BARTS_PM_A11) && (eChipRev < NI_TURKS_M_A11))
#define ASICREV_IS_TURKS_M(eChipRev)        ((eChipRev >= NI_TURKS_M_A11) && (eChipRev < NI_CAICOS_V_A11))
#define ASICREV_IS_TURKS_LOMBOK_M(eChipRev) ((eChipRev >= NI_TURKS_LOMBOK_M_A11) && (eChipRev < NI_CAICOS_V_A11))  // this change is intentional.  ASICREV_IS_TURKS_LOMBOK_M is a subset of ASICREV_IS_TURKS_M
#define ASICREV_IS_CAICOS_V(eChipRev)       ((eChipRev >= NI_CAICOS_V_A11) && (eChipRev < KAUAI_A11))
#define ASICREV_IS_KAUAI(eChipRev)          (eChipRev >= KAUAI_A11)


//
// CAYMAN/GRANVILLE ISLAND device IDs
//
#define DEVICE_ID_NI_CAYMAN_P_6700          0x6700    //Cayman GL XT (Cliffhanger)
#define DEVICE_ID_NI_CAYMAN_P_6701          0x6701    //Cayman GL XT (Moonracer)
#define DEVICE_ID_NI_CAYMAN_P_6702          0x6702    //Cayman GL XT (Spellbreaker)
#define DEVICE_ID_NI_CAYMAN_P_6703          0x6703    //Cayman GL XT
#define DEVICE_ID_NI_CAYMAN_P_6704          0x6704    //Cayman GL PRO (Starsaber)
#define DEVICE_ID_NI_CAYMAN_P_6705          0x6705    //Cayman GL PRO
#define DEVICE_ID_NI_CAYMAN_P_6706          0x6706    //Cayman GL (Doubleheader)
#define DEVICE_ID_NI_CAYMAN_P_6707          0x6707    //Cayman GL LE (Jetstorm)
#define DEVICE_ID_NI_CAYMAN_P_6708          0x6708    //Cayman GL
#define DEVICE_ID_NI_CAYMAN_P_6709          0x6709    //Cayman GL
#define DEVICE_ID_NI_CAYMAN_P_6718          0x6718    //Cayman XT
#define DEVICE_ID_NI_CAYMAN_P_6719          0x6719    //Cayman Pro
#define DEVICE_ID_NI_CAYMAN_P_671C          0x671C    //Antilles Pro
#define DEVICE_ID_NI_CAYMAN_P_671D          0x671D    //Antilles XT
#define DEVICE_ID_NI_CAYMAN_P_671F          0x671F    //Cayman CE

// CAYMAN ASIC internal revision number
#define INTERNAL_REV_NI_CAYMAN_P_A11        0x00      // First spin of Cayman


#define DEVICE_ID_CAYMAN_PALLADIUM          0x0031    // Palladium ID
#define DEVICE_ID_CAYMAN_LITE_PALLADIUM     0x0032    // Palladium ID


//
// BARTS/BLACKCOMB device IDs
//
#define DEVICE_ID_NI_BARTS_PM_6722          0x6722    //Barts GL2
#define DEVICE_ID_NI_BARTS_PM_6723          0x6723    //Barts GL3
#define DEVICE_ID_NI_BARTS_PM_6726          0x6726    //Barts GL6
#define DEVICE_ID_NI_BARTS_PM_6727          0x6727    //Barts GL7
#define DEVICE_ID_NI_BARTS_PM_6728          0x6728    //Barts GL XT
#define DEVICE_ID_NI_BARTS_PM_6729          0x6729    //Barts GL PRO
#define DEVICE_ID_NI_BARTS_PM_6738          0x6738    //Barts XT
#define DEVICE_ID_NI_BARTS_PM_6739          0x6739    //Barts PRO
#define DEVICE_ID_NI_BARTS_PM_673E          0x673E    //Barts LE

#define DEVICE_ID_NI_BARTS_PM_6720          0x6720    //Blackcomb XT/PRO and GL XT/PRO
#define DEVICE_ID_NI_BARTS_PM_6721          0x6721    //Blackcomb LP and GL LP
#define DEVICE_ID_NI_BARTS_PM_6724          0x6724    //Blackcomb XT/PRO Gemini and XT GL/PRO Gemini
#define DEVICE_ID_NI_BARTS_PM_6725          0x6725    //Blackcomb LP Gemini and GL LP Gemini
#define DEVICE_ID_NI_BARTS_PM_6730          0x6730    //Victoria - Blackcomb for desktop all-in-one

// BARTS ASIC internal revision number
#define INTERNAL_REV_NI_BARTS_PM_A11        0x00      // First spin of Barts


//
// TURKS/WHISTLER device IDs
//
#define DEVICE_ID_NI_TURKS_M_6746           0x6746    // Turks GL6
#define DEVICE_ID_NI_TURKS_M_6747           0x6747    // Turks GL7
#define DEVICE_ID_NI_TURKS_M_6748           0x6748    // Turks GL8
#define DEVICE_ID_NI_TURKS_M_6749           0x6749    // Turks GL9
#define DEVICE_ID_NI_TURKS_M_674A           0x674A    // Turks GL
#define DEVICE_ID_NI_TURKS_M_6750           0x6750    // Onega
#define DEVICE_ID_NI_TURKS_M_6751           0x6751    // Onega2
#define DEVICE_ID_NI_TURKS_M_6758           0x6758    // Turks XT
#define DEVICE_ID_NI_TURKS_M_6759           0x6759    // Turks Pro
#define DEVICE_ID_NI_TURKS_M_675B           0x675B    // Turks XT2
#define DEVICE_ID_NI_TURKS_M_675D           0x675D    // Turks Pro2
#define DEVICE_ID_NI_TURKS_M_675F           0x675F    // Turks LE

#define DEVICE_ID_NI_TURKS_M_6740           0x6740    // Whistler XT/GL XT
#define DEVICE_ID_NI_TURKS_M_6741           0x6741    // Whistler Pro/LP/GL Pro/GL LP
#define DEVICE_ID_NI_TURKS_M_6742           0x6742    // Whistler LE
#define DEVICE_ID_NI_TURKS_M_6743           0x6743    // WhistlerCSP Pro
#define DEVICE_ID_NI_TURKS_M_6744           0x6744    // Whistler XT/PRO Gemini and GL XT/PRO Gemini
#define DEVICE_ID_NI_TURKS_M_6745           0x6745    // Whistler LP 256 G5
#define DEVICE_ID_NI_TURKS_M_6843           0x6843    // Whistler XTX

// TURKS ASIC internal revision number
#define INTERNAL_REV_NI_TURKS_M_A11         0x00      // First spin of Turks


//
// LOMBOK/THAMES MOBILE device IDs
//

#define DEVICE_ID_NI_LOMBOK_M_6849              0x6849    // Lombok XT GL
#define DEVICE_ID_NI_LOMBOK_M_6850              0x6850    // Lombok AIO
#define DEVICE_ID_NI_LOMBOK_M_6858              0x6858    // Lombok XT
#define DEVICE_ID_NI_LOMBOK_M_6859              0x6859    // Lombok Pro

#define DEVICE_ID_NI_LOMBOK_M_6840              0x6840    // Thames XT/GL
#define DEVICE_ID_NI_LOMBOK_M_6841              0x6841    // Thames Pro
#define DEVICE_ID_NI_LOMBOK_M_6842              0x6842    // Thames LE

// Lombok ASIC internal revision number
#define INTERNAL_REV_NI_LOMBOK_M_A11            0x00      // First spin of Lombok


//
// CAICOS/SEYMOUR device IDs
//
#define DEVICE_ID_NI_CAICOS_V_6762          0x6762    // Caicos GL2
#define DEVICE_ID_NI_CAICOS_V_6763          0x6763    // Caicos CSP512 G5
#define DEVICE_ID_NI_CAICOS_V_6766          0x6766    // Caicos GL6
#define DEVICE_ID_NI_CAICOS_V_6767          0x6767    // Caicos GL7
#define DEVICE_ID_NI_CAICOS_V_6768          0x6768    // Caicos GL PRO
#define DEVICE_ID_NI_CAICOS_V_6770          0x6770    // Caspian PRO (Caicos AiO)
#define DEVICE_ID_NI_CAICOS_V_6771          0x6771    // Caicos XTX
#define DEVICE_ID_NI_CAICOS_V_6772          0x6772    // Caspian PRO2 (Caicos AiO)
#define DEVICE_ID_NI_CAICOS_V_6778          0x6778    // Caicos XT
#define DEVICE_ID_NI_CAICOS_V_6779          0x6779    // Caicos PRO
#define DEVICE_ID_NI_CAICOS_V_677B          0x677B    // Caicos PRO2

#define DEVICE_ID_NI_CAICOS_V_6760          0x6760    // Seymour XT/PRO and XT/PRO GL
#define DEVICE_ID_NI_CAICOS_V_6761          0x6761    // Seymour LP and LP GL
#define DEVICE_ID_NI_CAICOS_V_6764          0x6764    // Seymour XT/PRO GL and GL XT/PRO Gemini
#define DEVICE_ID_NI_CAICOS_V_6765          0x6765    // Seymour LP and GL LP Gemini

// CAICOS ASIC internal revision number
#define INTERNAL_REV_NI_CAICOS_V_A11        0x00      // First spin of Caicos
#define INTERNAL_REV_NI_CAICOS_V_A12        0x01      // A12 spin of Caicos

#define DEVICE_ID_CAICOS_PALLADIUM          0x0030    // Palladium ID


//
// KAUAI device IDs - Kauai is not a production part.
//

#define DEVICE_ID_KAUAI_PALLADIUM           0x0018    // Palladium ID

// KAUAI ASIC internal revision number
#define INTERNAL_REV_KAUAI_A11              0x00      // First spin of KAUAI


// The follow definitions are defined for backward compatibility
// when the new naming convention is followed.
// Please use the new names instead of these
#define CAYMAN_A11                   NI_CAYMAN_P_A11
#define BARTS_A11                    NI_BARTS_PM_A11
#define TURKS_A11                    NI_TURKS_M_A11
#define CAICOS_A11                   NI_CAICOS_V_A11
#define ASICREV_IS_CAYMAN(eChipRev)  ASICREV_IS_CAYMAN_P(eChipRev)
#define ASICREV_IS_BARTS(eChipRev)   ASICREV_IS_BARTS_PM(eChipRev)
#define ASICREV_IS_TURKS(eChipRev)   ASICREV_IS_TURKS_M(eChipRev)
#define ASICREV_IS_CAICOS(eChipRev)  ASICREV_IS_CAICOS_V(eChipRev)
#define INTERNAL_REV_CAYMAN_A11      INTERNAL_REV_NI_CAYMAN_P_A11
#define INTERNAL_REV_BARTS_A11       INTERNAL_REV_NI_BARTS_PM_A11
#define INTERNAL_REV_TURKS_A11       INTERNAL_REV_NI_TURKS_M_A11
#define INTERNAL_REV_CAICOS_A11      INTERNAL_REV_NI_CAICOS_V_A11
#define DEVICE_ID_CAICOS_6768        DEVICE_ID_NI_CAICOS_V_6768
#define DEVICE_ID_CAICOS_6770        DEVICE_ID_NI_CAICOS_V_6770
#define DEVICE_ID_CAICOS_6779        DEVICE_ID_NI_CAICOS_V_6779
#define DEVICE_ID_CAICOS_6760        DEVICE_ID_NI_CAICOS_V_6760
#define DEVICE_ID_CAICOS_6761        DEVICE_ID_NI_CAICOS_V_6761



// the below definitions are obsolete and will be removed at appropriate time.
// Please don't use them.

#define IBIZA_A11                    NI_CAYMAN_P_A11
#define COZUMEL_A11                  NI_BARTS_PM_A11
#define ASICREV_IS_IBIZA(eChipRev)   (eChipRev < NI_BARTS_PM_A11)
#define ASICREV_IS_COZUMEL(eChipRev) ((eChipRev >= NI_BARTS_PM_A11) && (eChipRev < NI_TURKS_M_A11))


//
// IBIZA/IBIZA mobile device IDs
//

#define DEVICE_ID_IBIZA_PALLADIUM           0x0016    // Palladium ID
#define DEVICE_ID_IBIZA_LITE_PALLADIUM      0x0020    // Palladium ID

// IBIZA ASIC internal revision number
#define INTERNAL_REV_IBIZA_A11              0x00      // First spin of IBIZA


//
// COZUMEL/COZUMEL mobile device IDs
//

#define DEVICE_ID_COZUMEL_PALLADIUM         0x0017    // Palladium ID

// COZUMEL ASIC internal revision number
#define INTERNAL_REV_COZUMEL_A11            0x00      // First spin of COZUMEL


#endif  // _NORTHERNISLAND_ID_H
