#include <bgame/impl.h>
#include <windows.h>
#include <iphlpapi.h>
#include <base/public.h>
#include <bgame/q_shared.h>

bool
GetMACAddress( string& mac ) {
    mac = "";

    PIP_ADAPTER_INFO pAdapterInfo;

    pAdapterInfo = (IP_ADAPTER_INFO *) malloc( sizeof(IP_ADAPTER_INFO) );
    ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);

    // Make an initial call to GetAdaptersInfo to get
    // the necessary size into the ulOutBufLen variable
    if (GetAdaptersInfo( pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO *) malloc (ulOutBufLen); 
    }

	DWORD dwStatus = GetAdaptersInfo( pAdapterInfo, &ulOutBufLen);

	// Do not return anything if there is a problem
    if( dwStatus != ERROR_SUCCESS ) {
        free (pAdapterInfo);
		return false;
    }

    for ( IP_ADAPTER_INFO* p = pAdapterInfo; p; p = p->Next ) {
        if (p->Type != MIB_IF_TYPE_ETHERNET)
            continue;

        if (p->AddressLength != 6)
            continue;

        ostringstream out;
        out << hex << nouppercase << setfill('0'); 
        out << setw(2) << (int)pAdapterInfo->Address[0] << ":"
            << setw(2) << (int)pAdapterInfo->Address[1] << ":"
            << setw(2) << (int)pAdapterInfo->Address[2] << ":"
            << setw(2) << (int)pAdapterInfo->Address[3] << ":"
            << setw(2) << (int)pAdapterInfo->Address[4] << ":"
            << setw(2) << (int)pAdapterInfo->Address[5];

        mac = out.str();
        break;
    }

    free(pAdapterInfo);
	return true;
}
