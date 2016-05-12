changequote(<<, >>)dnl
include(<<project.m4>>)dnl
dnl
dnl
dnl
__title (r<<>>__repoLCRev)
dnl
dnl
dnl
ifelse(__buildStability, stable,, <<dnl


    ******************************************
    ***                                    ***
    ***  NOTICE:                           ***
    ***                                    ***
    ***  THIS IS AN EXPERIMENTAL RELEASE.  ***
    ***  USE AT YOUR OWN RISK!             ***
    ***                                    ***
    ******************************************

    Please note that this release is considered experimental,
    meaning the stability of the release has not been tested
    in the wild, and documentation may be lacking. If you are
    not comfortable with using this kind of release, then you
    are advised to wait for a release classified as stable.
>>)dnl
dnl
dnl
dnl

For more information, details and changelog, please consult the accompanying
documentation found in the doc/ directory.

Please send all questions, comments, concerns, and bug reports to
the official forums at __website<<>> .
