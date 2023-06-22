#include "bios.h"
#include "util/apple/cf_helpers.h"

#include <IOKit/IOKitLib.h>

const char* ffDetectBios(FFBiosResult* bios)
{
    io_registry_entry_t registryEntry;

    #ifndef __aarch64__

    //https://github.com/osquery/osquery/blob/master/osquery/tables/system/darwin/smbios_tables.cpp
    //For Intel
    if((registryEntry = IORegistryEntryFromPath(MACH_PORT_NULL, "IODeviceTree:/rom")))
    {
        CFMutableDictionaryRef properties;
        if(IORegistryEntryCreateCFProperties(registryEntry, &properties, kCFAllocatorDefault, kNilOptions) != kIOReturnSuccess)
        {
            IOObjectRelease(registryEntry);
            return "IORegistryEntryCreateCFProperties(registryEntry) failed";
        }

        ffCfDictGetString(properties, CFSTR("vendor"), &bios->vendor);
        ffCfDictGetString(properties, CFSTR("version"), &bios->version);
        ffCfDictGetString(properties, CFSTR("release-date"), &bios->date);
        if(!ffStrbufContainC(&bios->date, '-'))
            ffStrbufAppendS(&bios->biosRelease, "Efi-");
        ffStrbufAppend(&bios->biosRelease, &bios->version);

        CFRelease(properties);
        IOObjectRelease(registryEntry);
        return NULL;
    }

    #else

    //For arm64
    if((registryEntry = IORegistryEntryFromPath(MACH_PORT_NULL, "IODeviceTree:/")))
    {
        CFMutableDictionaryRef properties;
        if(IORegistryEntryCreateCFProperties(registryEntry, &properties, kCFAllocatorDefault, kNilOptions) == kIOReturnSuccess)
        {
            ffCfDictGetString(properties, CFSTR("manufacturer"), &bios->vendor);
            CFRelease(properties);
        }
        IOObjectRelease(registryEntry);
        return NULL;
    }

    if((registryEntry = IORegistryEntryFromPath(MACH_PORT_NULL, "IODeviceTree:/chosen")))
    {
        CFMutableDictionaryRef properties;
        if(IORegistryEntryCreateCFProperties(registryEntry, &properties, kCFAllocatorDefault, kNilOptions) == kIOReturnSuccess)
        {
            ffCfDictGetString(properties, CFSTR("system-firmware-version"), &bios->biosRelease);
            ffStrbufAppend(&bios->version, &bios->biosRelease);
            ffStrbufSubstrAfterFirstC(&bios->version, '-');
            CFRelease(properties);
        }
        IOObjectRelease(registryEntry);
        return NULL;
    }

    #endif

    return "Failed to query bios info";
}
