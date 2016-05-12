#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IONetworkInterface.h>
#include <IOKit/network/IOEthernetController.h>

#include <base/public.h>
#include <bgame/q_shared.h>

static kern_return_t FindEthernetInterfaces(io_iterator_t *matchingServices);
static kern_return_t OSXGetMACAddress(io_iterator_t intfIterator, UInt8 *MACAddress, UInt8 bufferSize);

// These are helper functions from developer.apple.com
static kern_return_t FindEthernetInterfaces(io_iterator_t *matchingServices)
{
    kern_return_t		kernResult; 
    CFMutableDictionaryRef	matchingDict;
    CFMutableDictionaryRef	propertyMatchDict;
    
    matchingDict = IOServiceMatching(kIOEthernetInterfaceClass);

    if (NULL != matchingDict) {
        propertyMatchDict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    
        if (NULL != propertyMatchDict) {
            CFDictionarySetValue(propertyMatchDict, CFSTR(kIOPrimaryInterface), kCFBooleanTrue);
            CFDictionarySetValue(matchingDict, CFSTR(kIOPropertyMatchKey), propertyMatchDict);
            CFRelease(propertyMatchDict);
        }
    }

	kernResult = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict, matchingServices);    
    return kernResult;
}

// Given an iterator across a set of Ethernet interfaces, return the MAC address of the last one.
// If no interfaces are found the MAC address is set to an empty string.
// In this sample the iterator should contain just the primary interface.
static kern_return_t OSXGetMACAddress(io_iterator_t intfIterator, UInt8 *MACAddress, UInt8 bufferSize)
{
    io_object_t		intfService;
    io_object_t		controllerService;
    kern_return_t	kernResult = KERN_FAILURE;
    
    // Make sure the caller provided enough buffer space. Protect against buffer overflow problems.
	if (bufferSize < kIOEthernetAddressSize) {
		return kernResult;
	}
	
	// Initialize the returned address
    bzero(MACAddress, bufferSize);
    
    // IOIteratorNext retains the returned object, so release it when we're done with it.
    while ((intfService = IOIteratorNext(intfIterator)))
    {
        CFTypeRef	MACAddressAsCFData;        

        kernResult = IORegistryEntryGetParentEntry(intfService, kIOServicePlane, &controllerService);
		
        if (KERN_SUCCESS == kernResult) {
            // Retrieve the MAC address property from the I/O Registry in the form of a CFData
            MACAddressAsCFData = IORegistryEntryCreateCFProperty(controllerService, CFSTR(kIOMACAddress), kCFAllocatorDefault, 0);
            if (MACAddressAsCFData) {
                CFShow(MACAddressAsCFData); // for display purposes only; output goes to stderr
                
                // Get the raw bytes of the MAC address from the CFData
                CFDataGetBytes((CFDataRef)MACAddressAsCFData, CFRangeMake(0, kIOEthernetAddressSize), MACAddress);
                CFRelease(MACAddressAsCFData);
            }
                
            // Done with the parent Ethernet controller object so we release it.
            (void) IOObjectRelease(controllerService);
        }
        
        // Done with the Ethernet interface object so we release it.
        (void) IOObjectRelease(intfService);
    }
        
    return kernResult;
}

bool
GetMACAddress( string& mac ) {
    mac = "";

    kern_return_t	kernResult = KERN_SUCCESS;
    io_iterator_t	intfIterator;
    UInt8			MACAddress[kIOEthernetAddressSize];
 
    kernResult = FindEthernetInterfaces(&intfIterator);
    
    if (KERN_SUCCESS != kernResult) {
        IOObjectRelease(intfIterator);
		return false;
    }

	kernResult = OSXGetMACAddress(intfIterator, MACAddress, sizeof(MACAddress));
        
	if (KERN_SUCCESS != kernResult) {
		IOObjectRelease(intfIterator);
		return false;
	}

    ostringstream out;
    out << hex << nouppercase << setfill('0');
    out << setw(2) << (int)MACAddress[0] << ":"
        << setw(2) << (int)MACAddress[1] << ":"
        << setw(2) << (int)MACAddress[2] << ":"
        << setw(2) << (int)MACAddress[3] << ":"
        << setw(2) << (int)MACAddress[4] << ":"
        << setw(2) << (int)MACAddress[5];

    IOObjectRelease(intfIterator);

    mac = out.str();
	return true;
}
