//==============================================================================
// Copyright (c) 2012-2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools
/// \file
/// \brief  This class manages the dynamic loading of amdoclcl{32,64}.dll
//==============================================================================

#ifndef _ACL_MODULE_H_
#define _ACL_MODULE_H_

//#define WITH_VERSION_0_9 1

#define WITH_VERSION_0_8 1

/*
  Notes on discussion with Laurent about versioning of the ACL symbols on Linux:
    - the actual exported symbols on Linux will be appended with a version string:
      i.e. aclReadFromMem@@ACL_0.8
    - when moving to a new version of the library, any new entry-points will have a
      new version string (i.e.  aclSomeFunction@@ACL_0.9).
    - if an existing function changes semantically, then the library would actually
      export two versions: aclReadFromMem@@ACL_0.8 and aclReadFromMem@@ACL_0.9
    - apparently, dlsym knows how to deal with this, and by default will always
      return the address of the oldest version:
          i.e. using the above example of two versions of "aclReadFromMem", by default
          the call to dlsym(RTLD_NEXT, "aclReadFromMem") will return the address of
          aclReadFromMem@@ACL_0.8.  If there is only a single version, then dlsym will,
          of course, return just the address of that single symbol.
    - in order to override this default behavior (and thus bind to a newer version),
      you can use dlvsym in place of dlsym:
        dlvsym(RTLD_NEXT, "aclReadFromMem", "ACL_0.9");
    - it is expected that the presence of multiple versions of a single symbol will be
      quite rare
*/

#include "acl.h"
#include "DynamicLibraryModule.h"

typedef aclTargetInfo(ACL_API_ENTRY
                      *aclGetTargetInfoProc)(const char* arch,
                                             const char* device,
                                             acl_error* error_code);

typedef aclCompiler* (ACL_API_ENTRY
                      *aclCompilerInitProc)(aclCompilerOptions* opts, acl_error* error_code);

typedef acl_error(ACL_API_ENTRY
                  *aclCompilerFiniProc)(aclCompiler* cl);

typedef aclBinary* (ACL_API_ENTRY
                    *aclReadFromMemProc)(const void* mem,
                                         size_t size, acl_error* error_code);

typedef aclBinary* (ACL_API_ENTRY
                    *aclReadFromFileProc)(const char* str,
                                          acl_error* error_code);

typedef aclBinary* (ACL_API_ENTRY
                    *aclBinaryInitProc)(
                        size_t struct_version,
                        const aclTargetInfo* target,
                        const aclBinaryOptions* options,
                        acl_error* error_code);

typedef acl_error(ACL_API_ENTRY
                  *aclBinaryFiniProc)(aclBinary* bin);

typedef acl_error(ACL_API_ENTRY
                  *aclWriteToMemProc)(aclBinary* bin,
                                      void** mem, size_t* size);

typedef acl_error(ACL_API_ENTRY
                  *aclWriteToFileProc)(aclBinary* bin,
                                       const char* str);

typedef acl_error(ACL_API_ENTRY
                  *aclInsertSectionProc)(aclCompiler* cl,
                                         aclBinary* binary,
                                         const void* data,
                                         size_t data_size,
                                         aclSections id);

typedef const void* (ACL_API_ENTRY
                     *aclExtractSectionProc)(aclCompiler* cl,
                                             const aclBinary* binary,
                                             size_t* size,
                                             aclSections id,
                                             acl_error* error_code);

typedef const void* (ACL_API_ENTRY
                     *aclExtractSymbolProc)(aclCompiler* cl,
                                            const aclBinary* binary,
                                            size_t* size,
                                            aclSections id,
                                            const char* symbol,
                                            acl_error* error_code);

typedef acl_error(ACL_API_ENTRY
                  *aclRemoveSymbolProc)(aclCompiler* cl,
                                        aclBinary* binary,
                                        aclSections id,
                                        const char* symbol);

typedef acl_error(ACL_API_ENTRY
                  *aclInsertSymbolProc)(aclCompiler* cl,
                                        aclBinary* binary,
                                        const void* data,
                                        size_t data_size,
                                        aclSections id,
                                        const char* symbol);

typedef acl_error(ACL_API_ENTRY
                  *aclRemoveSectionProc)(aclCompiler* cl,
                                         aclBinary* binary,
                                         aclSections id);

#ifdef WITH_VERSION_0_9

typedef acl_error(ACL_API_ENTRY
                  *aclQueryInfoProc)(aclCompiler* cl,
                                     const aclBinary* binary,
                                     aclQueryType query,
                                     const char* kernel,
                                     void* data_ptr,
                                     size_t* ptr_size);
#else

typedef acl_error(ACL_API_ENTRY
                  *aclQueryInfoProc)(aclCompiler* cl,
                                     const aclBinary* binary,
                                     aclQueryType query,
                                     const char* kernel,
                                     void* data_ptr,
                                     size_t* ptr_size);

#endif

typedef acl_error(ACL_API_ENTRY
                  *aclDbgAddArgumentProc)(aclCompiler* cl,
                                          aclBinary* binary,
                                          const char* kernel,
                                          const char* name,
                                          bool byVal);

typedef acl_error(ACL_API_ENTRY
                  *aclCompileProc)(aclCompiler* cl,
                                   aclBinary* bin,
                                   const char* options,
                                   aclType from,
                                   aclType to,
                                   aclLogFunction compile_callback);

typedef acl_error(ACL_API_ENTRY
                  *aclDisassembleProc)(aclCompiler* cl,
                                       aclBinary* bin,
                                       const char* kernel,
                                       aclLogFunction disasm_callback);

#define ACL_API_TABLE \
    X(GetTargetInfo) \
    X(CompilerInit) \
    X(CompilerFini) \
    X(ReadFromMem) \
    X(ReadFromFile) \
    X(BinaryInit) \
    X(BinaryFini) \
    X(WriteToMem) \
    X(WriteToFile) \
    X(InsertSection) \
    X(ExtractSection) \
    X(RemoveSection) \
    X(QueryInfo) \
    X(DbgAddArgument) \
    X(ExtractSymbol) \
    X(InsertSymbol) \
    X(RemoveSymbol) \
    X(Compile) \
    X(Disassemble)

/// This class handles the dynamic loading of the compiler library from either 1) amdoclcl.dll/libamdoclcl.so,
/// 2) amdhsacl.dll/libamdhsacl.so or 3) amdocl.dll/libamdocl.so
class ACLModule
{
public:
    /// Constructor
    ACLModule(void);

    /// Destructor
    ~ACLModule(void);

    /// Default name to use for construction.
    /// This is usually amdocl12cl.dll or libamdocl12cl.so.
    static const char* s_DefaultModuleName;

    /// Pre-15.20 compiler library module. This is amdoclcl.dll or libamdoclcl.so.
    static const char* s_OldDefaultModuleName;

    /// HSA compiler library name (amdhsacl.dll)
    static const char* s_HSA_COMPILER_LIB_NAME;

    /// fallback compiler library name (amdocl.dll)
    static const char* s_TMP_MODULE_NAME;

    /// Load module.
    /// \param[in] name The module name.
    /// \return         true if successful, false otherwise
    bool LoadModule(const std::string& name = s_DefaultModuleName);

    /// Unload the compiler lib shared image.
    void UnloadModule();

    /// Have we successfully loaded the cal module?
    /// \returns enumeration value to answer query.
    bool IsLoaded() { return m_bModuleLoaded; }

#define X(SYM) acl##SYM##Proc SYM;
    ACL_API_TABLE;
#undef X

private:
    /// Initialize the internal data
    void Initialize();

    /// Have we loaded the compiler lib module?
    bool                 m_bModuleLoaded;

    /// Helper.
    DynamicLibraryModule m_DynamicLibraryHelper;
};

#endif //_ACL_MODULE_H_
