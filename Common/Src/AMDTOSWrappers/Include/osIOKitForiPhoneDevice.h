//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file
///
//=====================================================================

//------------------------------ osIOKitForiPhoneDevice.h ------------------------------

#ifndef __OSIOKITFORIPHONEDEVICE_H
#define __OSIOKITFORIPHONEDEVICE_H

// -----------------------------------------------------------------------------------------
// Yaki 12/1/2009
// IOKit header files are not available in the public iPhone OS SDK.
// Apple people told us that the IOKit header files should be identical to the desktop ones.
// Therefore, we copied the used IOKit and dependent types and functions definitions from the
// desktop header files and placed them here.
// -----------------------------------------------------------------------------------------

#define MACH_PORT_NULL 0
#define kIOReturnSuccess KERN_SUCCESS

typedef int kern_return_t;
typedef mach_port_t io_object_t;
typedef io_object_t io_iterator_t;
typedef io_object_t io_registry_entry_t;
typedef UInt32 IOOptionBits;

class IOService;
class IONotifier;

extern "C"
{
    typedef bool (*IOServiceMatchingNotificationHandler)(void* target, void* refCon, IOService* newService, IONotifier* notifier);
    kern_return_t IOMasterPort(mach_port_t bootstrapPort, mach_port_t* masterPort);
    CFMutableDictionaryRef IOServiceMatching(const char* name);
    kern_return_t IOServiceGetMatchingServices(mach_port_t masterPort, CFDictionaryRef matching, io_iterator_t* existing);
    io_object_t IOIteratorNext(io_iterator_t iterator);
    kern_return_t IOObjectRelease(io_object_t object);
    CFTypeRef IORegistryEntryCreateCFProperty(io_registry_entry_t entry, CFStringRef key, CFAllocatorRef allocator, IOOptionBits options);
};


#endif //__OSIOKITFORIPHONEDEVICE_H

