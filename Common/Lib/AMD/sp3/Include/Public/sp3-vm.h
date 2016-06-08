#ifndef SP3_VM_H
#define SP3_VM_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined (WIN_OS) && !defined(SP3_STATIC_LIB)
  #if defined(DLL_EXPORT_SP3)
    #define SP3_EXPORT __declspec(dllexport)
  #else
    #define SP3_EXPORT __declspec(dllimport)
  #endif
#else
  #define SP3_EXPORT
#endif

#ifdef _MSC_VER
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;

typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
#include <inttypes.h>
#endif

struct sp3_vma;

/// @file sp3-vm.h
/// @brief sp3 VM API
///
/// The VM API is used to manage virtual memory maps. Those maps are
/// used for binary storage for disassembly, as they can naturally
/// mirror the GPU's memory map (so no register translation is needed).

#define SP3_VM_PAGESIZE 64

/// @brief VM addresses are 64-bit and the address unit is 32 bits
///
typedef uint64_t sp3_vmaddr;

/// @brief Callback function that will fill a VMA on demand
///
/// The VMA to be filled will be specified through the request address.
/// The callback should fill the VMA using sp3_vm_write calls.
typedef void (* sp3_vmfill)(struct sp3_vma *vm, sp3_vmaddr addr, void *ctx);

/// @brief VM area
///
/// VMAs are kept in a sorted list
typedef struct sp3_vma {
    sp3_vmaddr base, len;
    sp3_vmfill fill;
    void *fill_ctx;
    uint32_t *data;
    struct sp3_vma *prev, *next;
} sp3_vma;

/// @brief Create a new VM that is empty.
///
SP3_EXPORT
sp3_vma *sp3_vm_new(void);

/// @brief Create a new VM that has a sp3_vmfill callback.
///
SP3_EXPORT
sp3_vma *sp3_vm_new_fill(sp3_vmfill fill, void *ctx);

/// @brief Create a new VM from an array of words.
/// @param base VM address to load array at.
/// @param len Number of 32-bit words in the array.
/// @param data Pointer to the array.
///
SP3_EXPORT
sp3_vma *sp3_vm_new_ptr(sp3_vmaddr base, sp3_vmaddr len, const uint32_t *data);

/// @brief Find a VMA, optionally adding it.
/// @param vm VM to search in.
/// @param addr Address to search for.
/// @param add Flag indicating whether a failure should result in adding a new VMA.
///
SP3_EXPORT
sp3_vma *sp3_vm_find(sp3_vma *vm, sp3_vmaddr addr, int add);

/// @brief Write a word to a VM.
///
SP3_EXPORT
void sp3_vm_write(sp3_vma *vm, sp3_vmaddr addr, uint32_t val);

/// @brief Read a word from a VM.
///
SP3_EXPORT
uint32_t sp3_vm_read(sp3_vma *vm, sp3_vmaddr addr);

/// @brief Probe VM for presence.
/// @return 1 if the specified address is backed in the VM, 0 otherwise.
///
SP3_EXPORT
int sp3_vm_present(sp3_vma *vm, sp3_vmaddr addr);

/// @brief Free a VM and all its storage.
///
SP3_EXPORT
void sp3_vm_free(sp3_vma *vm);

#ifdef __cplusplus
}
#endif

#endif
