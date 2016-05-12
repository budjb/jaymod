changequote(<<, >>)dnl
changecom(<</*>>, <<*/>>)dnl
include(<<project.m4>>)dnl
dnl
dnl
dnl
#!/bin/env perl
#

## NOTE:
## This script requires the str2time() function available from either of
## the following perl modules. Please enable exactly 1 of them.

#use Date::Parse;
use HTTP::Date;

###############################################################################
##
## PACKAGE: __title (__repoLCDate)
## AUTHOR:  Mr.Mxyzptlk
##
## Jaymod 2.0 -> Jaymod 2.1 shrubbot.cfg conversion utility.
##
###############################################################################

%OPTIONS = (
    shrubbot => "shrubbot.cfg",
    level    => "level.db",
    user     => "user.db",
);

###############################################################################

%LEVELDB = ();
%USERDB  = ();

###############################################################################

sub processRecord;
sub read_shrub;
sub write_level;
sub write_user;
sub usage;

###############################################################################

for ($_ = $ARGV[0]; $i <= $#ARGV; $_ = $ARGV[++$i]) {
    if (/^(.+)\s*=\s*(.+)/) {
        $OPTIONS{$1} = $2;
        next;
    }

    printf("ERROR: unrecognized option: %s\n", $_);
    usage();
    exit( 1 );
}

read_shrub();
write_level();
write_user();

###############################################################################

sub read_shrub
{
    printf("Reading: %s\n", $OPTIONS{shrubbot});
    if (!open(IN, '<', $OPTIONS{shrubbot})) {
        printf("unable to open file: %s\n", $OPTIONS{shrubbot});
        exit(1);
    }

    my %record = ();
    while (<IN>) {
        chomp();
        if (/^\s*\[(.+)\]$/) {
            processRecord(\%record);
            %record = ();
            $record{TYPE} = $1;
            next;
        }

        if (/^\s*([^=\s]+)\s*=\s*(.+)$/) {
            $record{$1} = $2;
        }
    }

    close(IN);
}

###############################################################################

sub processRecord
{
    my %record = %{$_[0]};

    if (!exists( $record{TYPE})) {
        return;
    }

    if ($record{TYPE} eq "level") {
        %{$LEVELDB{$record{level}}} = %record;
        return;
    }

    if ($record{TYPE} eq "admin") {
        %{$USERDB{$record{guid}}} = %record;
        return;
    }

    if ($record{TYPE} eq "ban") {
        return if ($record{expires} != 0);

        if (!exists( $USERDB{$record{guid}} )) {
            ${$USERDB{$record{guid}}}{guid} = $record{guid};
            ${$USERDB{$record{guid}}}{name} = $record{name};
        }

        ${$USERDB{$record{guid}}}{ip} = $record{ip};
        ${$USERDB{$record{guid}}}{mac} = $record{mac};
        ${$USERDB{$record{guid}}}{banned} = 1;
        ${$USERDB{$record{guid}}}{banTime} = str2time($record{made});
        ${$USERDB{$record{guid}}}{banExpiry} = 0;
        ${$USERDB{$record{guid}}}{banReason} = $record{reason};
        ${$USERDB{$record{guid}}}{banAuthority} = $record{banner};
        ${$USERDB{$record{guid}}}{banAuthorityx} = $record{banner};

        return;
    }

    printf("WARNING: unknown record type: %s\n", $record{TYPE});
}

###############################################################################

sub usage
{
    printf("\n");
    printf("usage: convert_shrub [NAME=VALUE]\n");
    printf("\n");
    printf("where:\n");
    for (sort(keys(%OPTIONS))) {
        printf("    %s=%s\n", $_, $OPTIONS{$_});
    }
}

###############################################################################

sub write_level
{
    printf("Writing %d levels: %s\n", scalar(keys(%LEVELDB)), $OPTIONS{level});
    if (!open(OUT, '>', $OPTIONS{level})) {
        printf("unable to open file: %s\n", $OPTIONS{level});
        exit(1);
    }

    for (sort(keys(%LEVELDB))) {
        %record = %{$LEVELDB{$_}};

        next if !exists($record{level}) || $record{level} < 0;

        printf(OUT "\n");
        printf(OUT "level = %d\n", $record{level});
        printf(OUT "flags = %s\n", $record{flags});
        printf(OUT "name = %s\n", $record{name});

        if (exists($record{greeting})) {
            printf(OUT "greetingtext = %s\n", $record{greeting});
        }
    }

    close(OUT);
}

###############################################################################

sub write_user
{
    printf("Writing %d users: %s\n", scalar(keys(%USERDB)), $OPTIONS{user});
    if (!open(OUT, '>', $OPTIONS{user})) {
        printf("unable to open file: %s\n", $OPTIONS{user});
        exit(1);
    }

    for (sort(keys(%USERDB))) {
        %record = %{$USERDB{$_}};

        next if !exists($record{guid});

        printf(OUT "\n");
        printf(OUT "guid = %s\n", lc($record{guid}));
        printf(OUT "name = %s\n", $record{name});
        printf(OUT "namex = %s\n", $record{name});
        printf(OUT "authLevel = %d\n", $record{level});

        if (exists($record{lastseen})) {
            printf(OUT "timestamp = %s\n", str2time($record{lastseen}));
        }

        if (exists($record{flags})) {
            printf(OUT "flags = %s\n", $record{flags});
        }

        if (exists($record{greeting})) {
            printf(OUT "greetingtext = %s\n", $record{greeting});
        }

        if (exists($record{ip})) {
            printf(OUT "ip = %s\n", $record{ip});
        }

        if (exists($record{mac})) {
            printf(OUT "mac = %s\n", $record{mac});
        }

        if (exists($record{banned}) && $record{banned} == 1) {
            printf(OUT "banned = 1\n");
            printf(OUT "banTime = %d\n", $record{banTime});
            printf(OUT "banExpiry = %d\n", $record{banExpiry});
            printf(OUT "banReason = %s\n", $record{banReason});
            printf(OUT "banAuthority = %s\n", $record{banAuthority});
            printf(OUT "banAuthorityx = %s\n", $record{banAuthorityx});
        }
    }

    close(OUT);
}
