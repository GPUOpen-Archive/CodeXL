/*****************************************************************************\
*
*  Module Name    evergreen_id.h
*  Project        EVERGREEN
*  Devices        EVERGREEN
*
*  Description    Defining Device IDs, ASIC revision IDs for EVERGREEN
*
*
*  (c) 2008 Advanced Micro Devices, Inc. (unpublished)
*
*  All rights reserved.  This notice is intended as a precaution against
*  inadvertent publication and does not imply publication or any waiver
*  of confidentiality.  The year included in the foregoing notice is the
*  year of creation of the work.
*
*  LOG OF CHANGES
*
*  1.0    08/29/2008 [FYan] - initial revision
*
\*****************************************************************************/

#ifndef _EVERGREEN_ID_H
#define _EVERGREEN_ID_H

enum
{
    CYPRESS_A11          = 1,
    CYPRESS_A12          = 2,

    JUNIPER_A11          = 20,
    JUNIPER_A12          = 21,

    REDWOOD_A11          = 40,
    REDWOOD_A12          = 41,

    CEDAR_A11            = 60,
    CEDAR_A12            = 61,

    EVERGREEN_UNKNOWN    = 0xFF
};

#define ASICREV_IS_CYPRESS(eChipRev) (eChipRev < JUNIPER_A11)
#define ASICREV_IS_JUNIPER(eChipRev) ((eChipRev >= JUNIPER_A11) && (eChipRev < REDWOOD_A11))
#define ASICREV_IS_REDWOOD(eChipRev) ((eChipRev >= REDWOOD_A11) && (eChipRev < CEDAR_A11))
#define ASICREV_IS_CEDAR(eChipRev)   (eChipRev >= CEDAR_A11)


//
// CYPRESS/LEXINGTON device IDs
//
#define DEVICE_ID_CYPRESS_6888              0x6888    // CYPRESS GL XT
#define DEVICE_ID_CYPRESS_6889              0x6889    // CYPRESS GL Pro
#define DEVICE_ID_CYPRESS_688A              0x688A    // CYPRESS GL XT
#define DEVICE_ID_CYPRESS_688C              0x688C    // CYPRESS - FS (Osprey)
#define DEVICE_ID_CYPRESS_688D              0x688D    // CYPRESS - FS (Kestrel)
#define DEVICE_ID_CYPRESS_6898              0x6898    // CYPRESS XT
#define DEVICE_ID_CYPRESS_6899              0x6899    // CYPRESS PRO
#define DEVICE_ID_CYPRESS_689B              0x689B    // CYPRESS PRO (Rebranch)
#define DEVICE_ID_CYPRESS_689C              0x689C    // HEMLOCK XT      - CYPRESS Gemini
#define DEVICE_ID_CYPRESS_689D              0x689D    // HEMLOCK PRO     - CYPRESS Gemini
#define DEVICE_ID_CYPRESS_689E              0x689E    // CYPRESS LE

#define DEVICE_ID_CYPRESS_6880              0x6880    // LEXINGTON GL XT - CYPRESS Mobile
#define DEVICE_ID_CYPRESS_6890              0x6890    // LEXINGTON XT    - CYPRESS Mobile

#define DEVICE_ID_CYPRESS_PALLADIUM         0x0010    // Palladium ID
#define DEVICE_ID_CYPRESS_LITE_PALLADIUM    0x0012    // Palladium ID

// CYPRESS ASIC internal revision number
#define INTERNAL_REV_CYPRESS_A11            0x00      // First spin of CYPRESS ASIC
#define INTERNAL_REV_CYPRESS_A12            0x01      // Second spin of CYPRESS ASIC

//
// JUNIPER/BROADWAY device IDs
//
#define DEVICE_ID_JUNIPER_68A9              0x68A9    // JUNIPER GL Pro
#define DEVICE_ID_JUNIPER_68B8              0x68B8    // JUNIPER XT
#define DEVICE_ID_JUNIPER_68B9              0x68B9    // JUNIPER PRO
#define DEVICE_ID_JUNIPER_68BA              0x68BA    // JUNIPER XT (BD3D)
#define DEVICE_ID_JUNIPER_68BE              0x68BE    // JUNIPER LE
#define DEVICE_ID_JUNIPER_68BF              0x68BF    // JUNIPER LE (BD3D)

#define DEVICE_ID_JUNIPER_68A0              0x68A0    // BROADWAY XGL       - JUNIPER Mobile
#define DEVICE_ID_JUNIPER_68A1              0x68A1    // BROADWAY GL Pro/LP - JUNIPER Mobile
#define DEVICE_ID_JUNIPER_68A8              0x68A8    // BROADWAY PRO & LP  - JUNIPER Mobile
#define DEVICE_ID_JUNIPER_68B0              0x68B0    // BROADWAY XT        - JUNIPER Mobile
#define DEVICE_ID_JUNIPER_68B1              0x68B1    // BROADWAY PRO & LP  - JUNIPER Mobile

#define DEVICE_ID_JUNIPER_PALLADIUM         0x0011    // Palladium ID

// JUNIPER ASIC internal revision number
#define INTERNAL_REV_JUNIPER_A11            0x00      // First spin of JUNIPER ASIC
#define INTERNAL_REV_JUNIPER_A12            0x01      // Second spin of JUNIPER ASIC


//
// REDWOOD/PINEWOOD/MADISON device IDs
//
#define DEVICE_ID_REDWOOD_68C8              0x68C8    // Redwood XT GL
#define DEVICE_ID_REDWOOD_68C9              0x68C9    // Redwood PRO GL
#define DEVICE_ID_REDWOOD_68D8              0x68D8    // Redwood XT
#define DEVICE_ID_REDWOOD_68D9              0x68D9    // Redwood PRO
#define DEVICE_ID_REDWOOD_68DA              0x68DA    // Redwood PRO2
#define DEVICE_ID_REDWOOD_68DE              0x68DE    // Redwood LE
#define DEVICE_ID_REDWOOD_68C7              0x68C7    // Pinewood

#define DEVICE_ID_REDWOOD_68C0              0x68C0    // Madison XT GL
#define DEVICE_ID_REDWOOD_68C1              0x68C1    // Madison PRO & LP GL
#define DEVICE_ID_REDWOOD_68D0              0x68D0    // Madison XT
#define DEVICE_ID_REDWOOD_68D1              0x68D1    // Madison PRO & LP

#define DEVICE_ID_REDWOOD_PALLADIUM         0x0013    // Palladium ID

// REDWOOD ASIC internal revision number
#define INTERNAL_REV_REDWOOD_A11            0x00      // First spin of REDWOOD
#define INTERNAL_REV_REDWOOD_A12            0x01      // Second spin of REDWOOD


//
// CEDAR/PARK/ROBSON device IDs
//
#define DEVICE_ID_CEDAR_68E8                0x68E8    // Cedar XT GL
#define DEVICE_ID_CEDAR_68E9                0x68E9    // Cedar PRO GL
#define DEVICE_ID_CEDAR_68F2                0x68F2    // Cedar Prowl
#define DEVICE_ID_CEDAR_68F8                0x68F8    // Cedar XT
#define DEVICE_ID_CEDAR_68F9                0x68F9    // Cedar PRO
#define DEVICE_ID_CEDAR_68FA                0x68FA    // Cedar PRO2
#define DEVICE_ID_CEDAR_68FE                0x68FE    // Cedar LE

#define DEVICE_ID_CEDAR_68E0                0x68E0    // Park  PRO & XT GL
#define DEVICE_ID_CEDAR_68E1                0x68E1    // Park LP GL
#define DEVICE_ID_CEDAR_68E4                0x68E4    // Robson CE
#define DEVICE_ID_CEDAR_68E5                0x68E5    // Robson LE
#define DEVICE_ID_CEDAR_68F0                0x68F0    // Park PRO & XT
#define DEVICE_ID_CEDAR_68F1                0x68F1    // Park LP

#define DEVICE_ID_CEDAR_PALLADIUM           0x0014    // Palladium ID

// CEDAR ASIC internal revision number
#define INTERNAL_REV_CEDAR_A11              0x00      // First spin of CEDAR
#define INTERNAL_REV_CEDAR_A12              0x01      // Second spin of CEDAR


#endif  // _EVERGREEN_ID_H
