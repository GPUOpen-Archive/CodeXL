//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file Guids.h
///
//==================================================================================

// guids.h: definitions of GUIDs/IIDs/CLSIDs used in this VsPackage

/*
Do not use #pragma once, as this file needs to be included twice.  Once to declare the externs
for the GUIDs, and again right after including initguid.h to actually define the GUIDs.
*/



// package guid
// { ecdfbaee-ad99-452d-874c-99fce5a48b8e }
#define guidCodeXLVSPackagePkg { 0xECDFBAEE, 0xAD99, 0x452D, { 0x87, 0x4C, 0x99, 0xFC, 0xE5, 0xA4, 0x8B, 0x8E } }
#ifdef DEFINE_GUID
DEFINE_GUID(CLSID_CodeXLVSPackage,
            0xECDFBAEE, 0xAD99, 0x452D, 0x87, 0x4C, 0x99, 0xFC, 0xE5, 0xA4, 0x8B, 0x8E);
#endif

// Command set guid for our commands (used with IOleCommandTarget)
// { 1574ef7f-7885-467d-ae11-bacdd962ed31 }
#define guidCodeXLVSPackageCmdSet { 0x1574EF7F, 0x7885, 0x467D, { 0xAE, 0x11, 0xBA, 0xCD, 0xD9, 0x62, 0xED, 0x31 } }
#ifdef DEFINE_GUID
DEFINE_GUID(CLSID_CodeXLVSPackageCmdSet,
            0x1574EF7F, 0x7885, 0x467D, 0xAE, 0x11, 0xBA, 0xCD, 0xD9, 0x62, 0xED, 0x31);
#endif

#ifdef DEFINE_GUID
// GUID of the Calls History view persistence slot.
// {FDFA7DAA-6EC0-43E7-9C5A-64D55E82EB70}
DEFINE_GUID(CLSID_VSPCallsHistoryPersistanceId,
            0xfdfa7daa, 0x6ec0, 0x43e7, 0x9c, 0x5a, 0x64, 0xd5, 0x5e, 0x82, 0xeb, 0x70);

// GUID of the Properties view persistence slot.
// {E55CEC72-3946-4998-90C0-4023ED31C795}
DEFINE_GUID(CLSID_VSPPropertiesPersistanceId,
            0xe55cec72, 0x3946, 0x4998, 0x90, 0xc0, 0x40, 0x23, 0xed, 0x31, 0xc7, 0x95);

// GUID of the CodeXL Explorer persistence slot.
// {822CCAA6-124E-4B99-BF9F-CEDF92DB67AF}
DEFINE_GUID(CLSID_VSPObjectsExplorerPersistanceId,
            0x822ccaa6, 0x124e, 0x4b99, 0xbf, 0x9f, 0xce, 0xdf, 0x92, 0xdb, 0x67, 0xaf);

// GUID of the Statistics view persistence slot.
// {5BB901CC-4789-46F6-886C-5726A1FB13BB}
DEFINE_GUID(CLSID_VSPStatisticsPersistanceId,
            0x5bb901cc, 0x4789, 0x46f6, 0x88, 0x6c, 0x57, 0x26, 0xa1, 0xfb, 0x13, 0xbb);

// GUID of the memory view persistence slot:
// {B7E624E2-B99B-48C5-90D3-54B241156627}
DEFINE_GUID(CLSID_VSPMemoryPersistanceId,
            0xb7e624e2, 0xb99b, 0x48c5, 0x90, 0xd3, 0x54, 0xb2, 0x41, 0x15, 0x66, 0x27);

// GUID of the state variables view persistence slot:
//  {8B863DFA-D08D-4E1C-B054-49DD78BD5030}
DEFINE_GUID(CLSID_VSPStateVariablesPersistanceId,
            0x8b863dfa, 0xd08d, 0x4e1c, 0xb0, 0x54, 0x49, 0xdd, 0x78, 0xbd, 0x50, 0x30);

// GUID of the command queues view persistence slot:
// {E5BBA4D8-BA73-4D53-BBEC-79B879A94B81}
// DEFINE_GUID(CLSID_VSPCommandQueuesPersistanceId,
//     0xe5bba4d8, 0xba73, 0x4d53, 0xbb, 0xec, 0x79, 0xb8, 0x79, 0xa9, 0x4b, 0x81);

// GUID of the MultiWatch 1 persistence slot:
// {4B6B71CF-EE4C-4C68-90A3-DBB00851DAF6}
DEFINE_GUID(CLSID_VSPMultiwatch1PersistanceId,
            0x4b6b71cf, 0xee4c, 0x4c68, 0x90, 0xa3, 0xdb, 0xb0, 0x8, 0x51, 0xda, 0xf6);

// GUID of the MultiWatch 2 persistence slot:
// {945BD993-7B39-455C-98AD-66EF276C4F4C}
DEFINE_GUID(CLSID_VSPMultiwatch2PersistanceId,
            0x945bd993, 0x7b39, 0x455c, 0x98, 0xad, 0x66, 0xef, 0x27, 0x6c, 0x4f, 0x4c);

// GUID of the MultiWatch 3 persistence slot:
// {D9287201-B55F-46D9-9EDF-1337FF0FFF0C}
DEFINE_GUID(CLSID_VSPMultiwatch3PersistanceId,
            0xd9287201, 0xb55f, 0x46d9, 0x9e, 0xdf, 0x13, 0x37, 0xff, 0xf, 0xff, 0xc);

// GUID of session view persistence slot:
// {B1833AB4-CAA1-48EB-828B-F1E15BA751CC}
DEFINE_GUID(CLSID_VSPSessionPersistanceId,
            0xb1833ab4, 0xcaa1, 0x48eb, 0x82, 0x8b, 0xf1, 0xe1, 0x5b, 0xa7, 0x51, 0xcc);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// Add more view GUIDs here (Tools > Create GUID > DEFINE_GUID(...)
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#endif // def DEFINE_GUID



// Guid for the image viewer icon list referenced in the VSCT file
// {D4FB3B5D-513A-48DE-B76E-FFDBE3398FB2}
#define guidImagesEditorToolbarIcons { 0xD4FB3B5D, 0x513A, 0x48dE, { 0xB7, 0x6E, 0xFF, 0xDB, 0xE3, 0x39, 0x8F, 0xB2 } }
#ifdef DEFINE_GUID
DEFINE_GUID(CLSID_ImagesEditorToolbarIcons,
            0xd4fb3b5d, 0x513a, 0x48de, 0xb7, 0x6e, 0xff, 0xdb, 0xe3, 0x39, 0x8f, 0xb2);
#endif

// Guid for the menu commands image list in the VSCT file
// {934A5738-BD5C-4A5B-8252-87BBACD508AD}
#define guidMenuCommandIcons { 0x934A5738, 0xBD5C, 0x4A5B, { 0x82, 0x52, 0x87, 0xBB, 0xAC, 0xD5, 0x08, 0xAD } }
#ifdef DEFINE_GUID
DEFINE_GUID(CLSID_MenuCommandIcons,
            0x934a5738, 0xbd5c, 0x4a5b, 0x82, 0x52, 0x87, 0xbb, 0xac, 0xd5, 0x8, 0xad);
#endif

// Guid for the Editor Factory
// {43946143-BAB8-41D6-968C-BBA70DB6E6D1}
#define guidCodeXLVSPackageEditorFactory { 0x43946143, 0xBAB8, 0x41D6, { 0x96, 0x8C, 0xBB, 0xA7, 0x0D, 0xB6, 0xE6, 0xD1 } }
#ifdef DEFINE_GUID
DEFINE_GUID(CLSID_CodeXLVSPackageEditorFactory,
            0x43946143, 0xbab8, 0x41d6, 0x96, 0x8c, 0xbb, 0xa7, 0xd, 0xb6, 0xe6, 0xd1);
#endif

// Guid for the Editor Document (the document is the actual editor)
// {CF90A284-AB86-44CE-82AD-7F8971249635}
#define guidCodeXLVSPackageEditorDocument { 0xCF90A284, 0xAB86, 0x44CE, { 0x82, 0xAD, 0x7F, 0x89, 0x71, 0x24, 0x96, 0x35 } }
#ifdef DEFINE_GUID
DEFINE_GUID(CLSID_CodeXLVSPackageEditorDocument,
            0xcf90a284, 0xab86, 0x44ce, 0x82, 0xad, 0x7f, 0x89, 0x71, 0x24, 0x96, 0x35);
#endif

// Guid for the Profile Session Editor Factory
// {7DD989D1-F681-462C-9D06-B352F39F7068}
#define guidCodeXLVSPackageProfileSessionEditorFactory { 0x7DD989D1, 0xF681, 0x462C, { 0x9D, 0x06, 0xB3, 0x52, 0xF3, 0x9F, 0x70, 0x68 } }
#ifdef DEFINE_GUID
DEFINE_GUID(CLSID_CodeXLVSPackageProfileSessionEditorFactory,
            0x7dd989d1, 0xf681, 0x462c, 0x9d, 0x6, 0xb3, 0x52, 0xf3, 0x9f, 0x70, 0x68);
#endif

// Guid for the Profile Session Editor Document (the document is the actual editor)
// {46C9ED2E-CC57-4FFE-A166-96DD37AAC20A}
#define guidCodeXLVSPackageProfileSessionEditorDocument { 0x46C9ED2E, 0xCC57, 0x4FFE, { 0xA1, 0x66, 0x96, 0xDD, 0x37, 0xAA, 0xC2, 0x0A } }
#ifdef DEFINE_GUID
DEFINE_GUID(CLSID_CodeXLVSPackageProfileSessionEditorDocument,
            0x46c9ed2e, 0xcc57, 0x4ffe, 0xa1, 0x66, 0x96, 0xdd, 0x37, 0xaa, 0xc2, 0x0a);
#endif

// Guid for the Kernel Analyzer Editor Factory
// {34918D20-6DE3-4DFB-B888-D5BDA6ABD775}
#ifdef DEFINE_GUID
DEFINE_GUID(CLSID_CodeXLVSPackageKernelAnalyzerEditorFactory,
            0x34918d20, 0x6de3, 0x4dfb, 0xb8, 0x88, 0xd5, 0xbd, 0xa6, 0xab, 0xd7, 0x75);
#endif

// Guid for the Kernel Analyzer Editor Document (the document is the actual editor)
// {4B0F6625-1540-4943-B1C6-21C05E037275}
#ifdef DEFINE_GUID
DEFINE_GUID(CLSID_CodeXLVSPackageKernelAnalyzerEditorDocument,
            0x4b0f6625, 0x1540, 0x4943, 0xb1, 0xc6, 0x21, 0xc0, 0x5e, 0x3, 0x72, 0x75);
#endif

// Guid for the Power Profiling Editor Document (the document is the actual editor)
// {849B34BA-DAA6-4510-AE67-0C5F9DB3DDFE}
#ifdef DEFINE_GUID
DEFINE_GUID(CLSID_CodeXLVSPackagePowerProfilingEditorDocument,
            0x849b34ba, 0xdaa6, 0x4510, 0xae, 0x67, 0xc, 0x5f, 0x9d, 0xb3, 0xdd, 0xfe);
#endif
