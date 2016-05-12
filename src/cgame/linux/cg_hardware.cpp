#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if.h>

#include <base/public.h>
#include <bgame/q_shared.h>

bool
GetMACAddress( string& mac ) {
    mac = "";

    int s = socket( AF_INET, SOCK_DGRAM, 0 );
    if (s == -1)
        return false;

    bool ok = false;
    struct ifreq ifr;
    struct ifreq *IFR;
    struct ifconf ifc;
    char buf[1024];

    ifc.ifc_len = sizeof( buf );
    ifc.ifc_buf = buf;
    ioctl( s, SIOCGIFCONF, &ifc );

    IFR = ifc.ifc_req;
    for ( int i = ifc.ifc_len / sizeof( struct ifreq ); --i >= 0; IFR++ ) {
        Q_strncpyz( ifr.ifr_name, IFR->ifr_name, sizeof( ifr.ifr_name ));
        if( ioctl( s, SIOCGIFFLAGS, &ifr ) == 0 ) {
            if(!( ifr.ifr_flags & IFF_LOOPBACK )) {
                if( ioctl( s, SIOCGIFHWADDR, &ifr ) == 0) {
                    ok = true;
                    break;
                }
            }
        }
    }

    close( s );

    if (!ok)
        return false;

    unsigned char addr[6];
    memcpy( addr, ifr.ifr_hwaddr.sa_data, sizeof(addr));

    ostringstream out;
    out << hex << nouppercase << setfill('0'); 
    out << setw(2) << (int)addr[0] << ":"
        << setw(2) << (int)addr[1] << ":"
        << setw(2) << (int)addr[2] << ":"
        << setw(2) << (int)addr[3] << ":"
        << setw(2) << (int)addr[4] << ":"
        << setw(2) << (int)addr[5];

    mac = out.str();
    return true;
}
