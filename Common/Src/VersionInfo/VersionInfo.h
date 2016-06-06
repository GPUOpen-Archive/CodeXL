//------------------------------ VersionInfo.h ------------------------------

#ifndef __VERSIONINFO_H
#define __VERSIONINFO_H

// Values read by the Windows OS and displayed as DLL/EXE file properties.
// Need also to update
//   \CodeXL\CodeXLVSPackage\CodeXLVSPackage\CodeXLVSPackageVSIX\Properties\AssemblyInfo.cs
//   \CodeXL\CodeXLVSPackage\CodeXLVSPackage\CodeXLVSPackageVS11VSIX\Properties\AssemblyInfo.cs
//   \CodeXL\CodeXLVSPackage\CodeXLVSPackage\CodeXLVSPackageVS12VSIX\Properties\AssemblyInfo.cs
//   \CodeXL\CodeXLVSPackage\CodeXLVSPackage\CodeXLVSPackageVS14VSIX\Properties\AssemblyInfo.cs
#define CODEXL_MAJOR_AND_MINOR_VERSION       2,1,0,0
#define CODEXL_MAJOR_AND_MINOR_VERSION_STR L"2,1,0,0\0"

#define FILEVER                     CODEXL_MAJOR_AND_MINOR_VERSION
#define PRODUCTVER                  CODEXL_MAJOR_AND_MINOR_VERSION
#define VS_PACKAGE_FILEVER          CODEXL_MAJOR_AND_MINOR_VERSION
#define VS_PACKAGE_PRODUCTVER       CODEXL_MAJOR_AND_MINOR_VERSION

#define STRFILEVER                  CODEXL_MAJOR_AND_MINOR_VERSION_STR
#define STRPRODUCTVER               CODEXL_MAJOR_AND_MINOR_VERSION_STR
#define VS_PACKAGE_STRFILEVER       CODEXL_MAJOR_AND_MINOR_VERSION_STR
#define VS_PACKAGE_STRPRODUCTVER    CODEXL_MAJOR_AND_MINOR_VERSION_STR
#define STRCOPYRIGHT                L"\0"
#define STRFILEDESCRIPTION          L"CodeXL - A comprehensive tool suite for the performance-aware developer\0"
#define STRPRODUCTNAME              L"CodeXL - A comprehensive tool suite for the performance-aware developer\0"
#define VS_PACKAGE_STRPRODUCTNAME   L"CodeXL - A comprehensive tool suite for the performance-aware developer\0"

#endif //__VERSIONINFO_H
