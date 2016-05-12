changequote(<<, >>)dnl
changecom(<</*>>, <<*/>>)dnl
include(<<project.m4>>)dnl
dnl
dnl
dnl
#!/bin/sh
#

###############################################################################
##
## PACKAGE: __title (r<<>>__repoLCRev)
## AUTHOR:  Mr.Mxyzptlk
##
## Server control script for etded.x86 running on Linux or FreeBSD.
## You will need one (1) copy of this script for EACH server you run.
##
## Hierarchy expected is:
##     /usr/local/games/enemy-territory - full ET 2.60b Linux binaries
##
##     /home/etserver/server1/
##     /home/etserver/server1/etmain/
##     /home/etserver/server1/jaymod/
##     /home/etserver/server1/jaymod/qagame.mp.i386.so
##     /home/etserver/server1/jaymod/__pk3
##     /home/etserver/server1/jaymod/jaymod.cfg
##     /home/etserver/server1/jaymod/server.cfg
##     /home/etserver/server1/log/
##     /home/etserver/server1/serverctl
##
## Generally you'll want to create a new user account/ID that will
## run the server. This will help guarnatee this script cannot kill the
## wrong processes by mistake -- however unlikely.
##
## Next we use the name 'server1' to describe our server. This name is
## arbitrary and can be anything but keep it simple and without spaces.
##
## etmain/    is an empty dir but is a good place to put custom maps.
## jaymod/    is where all jaymod-specific stuff goes. So copy the qagame
##            module, PK3 and jaymod.cfg (both found in Jaymod's bundle).
##            Also create a server.cfg that at some point exec's jaymod.cfg .
## log/       is an empty dir where the server's output will be captured.
## serverctl  is THIS script. Make sure it has execute permissions.
##
## Edit the following values accordingly:
##
##    ET_INSTANCE - The abitrary simple name of your server. Think about it
##                  this way, you could be running more than 1 etded.x86 on
##                  the same host. So just give each copy of this script
##                  a unique name.
##
##    ET_IP       - your server's (public) IP as known to the Internet?
##
##    ET_PORT     - your server's (public) PORT as known to the Internet?
##
##    ET_ACCOUNT  - if not blank, enables a guard that prevents the any other
##                  user from running this script.
##
##    ET_SU_ENABLE - if set to 1, enables a special mode where if root
##                   invokes this script, it will use the command 'su'
##                   to switch to ET_ACCOUNT user, then run the script.
##                   This makes it possible to put this script in /etc/init.d
##                   and let the server's start automatically when the
##                   operating system boots up.
##
## The rest of the shell vars should be good for almost all installs.
##
###############################################################################

ET_BASE=/usr/local/games/enemy-territory
ET_INSTANCE=server1
ET_HOME=/home/etserver/$ET_INSTANCE
ET_IP=
ET_PORT=27960

#ET_ACCOUNT=etserver
#ET_SU_ENABLE=1

ET_ACCOUNT=
ET_SU_ENABLE=0

###############################################################################

ACTIVE_EUID=`id -u`
LOOPER_BASENAME=serverctl
SERVER_BASENAME=etded.x86

###############################################################################

fetch_pids()
{
    eval $1=`pgrep -u "$ACTIVE_EUID" -f "$LOOPER_BASENAME .*loop *$ET_INSTANCE"`
    eval $2=`pgrep -u "$ACTIVE_EUID" -f "$SERVER_BASENAME .*xx_instance *$ET_INSTANCE"`
}

###############################################################################

security()
{
    if [ -z "$ET_ACCOUNT" ]; then
        return
    fi

    ET_ACCOUNT_UID=`id -u "$ET_ACCOUNT"`

    if [ "$ACTIVE_EUID" -eq "$ET_ACCOUNT_UID" ]; then
        return
    fi

    if [ "$ET_SU_ENABLE" -eq 1 ]; then
        if [ "$ACTIVE_EUID" -ne 0 ]; then
            echo "ACCESS DENIED: $0 must be invoked by root or $ET_ACCOUNT"
            exit 1
        fi
        su -m "$ET_ACCOUNT" -c "$ET_HOME/$LOOPER_BASENAME $@" || exit 1
        exit 0
    fi

    echo "ACCESS DENIED: $ET_HOME/$LOOPER_BASENAME must be invoked by $ET_ACCOUNT"
    exit 1
}

###############################################################################

server_reload()
{
    fetch_pids pid_looper pid_server
    if [ -n "$pid_server" ]; then
        echo "server $ET_INSTANCE: ONLINE (PID=$pid_server)"
    else
        echo "server $ET_INSTANCE: OFFLINE"
    fi

    if [ -n "$pid_server" ]; then
        echo "Sending readconfing request to server @ PID=$pid_server..."
        kill -USR1 "$pid_server"
    fi
}

###############################################################################

server_start()
{
    fetch_pids pid_looper pid_server
    if [ -n "$pid_looper" ]; then
        echo "looper $ET_INSTANCE: ALREADY ONLINE (PID=$pid_looper)"
    fi

    if [ -n "$pid_server" ]; then
        echo "server $ET_INSTANCE: ALREADY ONLINE (PID=$pid_server)"
    fi

    if [ -n "$pid_looper" -o -n "$pid_server" ]; then
        exit 0
    fi

    echo "Starting LOOPER/SERVER $ET_INSTANCE..."
    trap "" SIGHUP
    $ET_HOME/$LOOPER_BASENAME loop $ET_INSTANCE
}

###############################################################################

server_stop()
{
    fetch_pids pid_looper pid_server
    if [ -n "$pid_looper" ]; then
        echo "looper $ET_INSTANCE: ONLINE (PID=$pid_looper)"
    else
        echo "looper $ET_INSTANCE: ALREADY OFFLINE"
    fi

    if [ -n "$pid_server" ]; then
        echo "server $ET_INSTANCE: ONLINE (PID=$pid_server)"
    else
        echo "server $ET_INSTANCE: ALREADY OFFLINE"
    fi

    if [ -n "$pid_looper" ]; then
        echo "Stopping looper @ PID=$pid_looper..."
        kill "$pid_looper"
    fi

    if [ -n "$pid_server" ]; then
        echo "Stopping server @ PID=$pid_server..."
        kill "$pid_server"
    fi
}

###############################################################################

server_status()
{
    fetch_pids pid_looper pid_server
    if [ -n "$pid_looper" ]; then
        echo "looper $ET_INSTANCE: ONLINE (PID=$pid_looper)"
    else
        echo "looper $ET_INSTANCE: OFFLINE"
    fi

    if [ -n "$pid_server" ]; then
        echo "server $ET_INSTANCE: ONLINE (PID=$pid_server)"
    else
        echo "server $ET_INSTANCE: OFFLINE"
    fi
}

###############################################################################

server_loop()
{
    ## Required for Omni-bot when using relative locations for omnibot_et.so .
    PATH=${PATH}:.

    ## Required for etded.x86 when running as effective-uid.
    export HOME=$ET_HOME

    while :
    do
        cd $ET_HOME
        LOGFILE=log/`date +%Y.%m.%d-%H:%M:%S`

        $ET_BASE/$SERVER_BASENAME \
            \
            +set com_hunkmegs   "128" \
            +set com_zonemegs   "48" \
            +set dedicated      "2" \
            +set fs_basepath    "$ET_BASE" \
            +set fs_game        "jaymod" \
            +set fs_homepath    "$ET_HOME" \
            +set net_ip         "$ET_IP" \
            +set net_port       "$ET_PORT" \
            +set omnibot_enable "1" \
            +set sv_punkbuster  "1" \
            +set ttycon         "0" \
            +set xx_instance    "$ET_INSTANCE" \
            \
            +exec "server.cfg" \
            > $LOGFILE 2>&1

        sleep 30
    done
}

###############################################################################

security $*

case $1 in
'reload')
    server_reload
    ;;

'start')
    server_start
    ;;

'stop')
    server_stop
    ;;

'status')
    server_status
    ;;

'restart')
    server_stop
    echo "Waiting 5 seconds..."
    sleep 5
    server_start
    ;;

'loop')
    server_loop &
    ;;

*)
    echo "usage: $0 { start | stop | status | restart | reload }"
    exit 1
    ;;
esac
