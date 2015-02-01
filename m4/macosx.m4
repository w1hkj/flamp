AC_DEFUN([AC_FL_MACOSX], [
case "$target_os" in
  darwin*)
      target_darwin="yes"
      ;;
  *)
      target_darwin="no"
      ;;
esac

AC_ARG_ENABLE([mac-universal], AC_HELP_STRING([--enable-mac-universal],
                                              [build a universal binary on Mac OS X @<:@no@:>@]),
              [case "${enableval}" in
                 yes|no) ac_cv_mac_universal="${enableval}" ;;
                 *)      AC_MSG_ERROR([bad value "${enableval}" for --enable-mac-universal]) ;;
	       esac],
              [ac_cv_mac_universal=no])

if test "x$target_darwin" = "xyes" && test "x$ac_cv_mac_universal" = "xyes"; then
    mac_minversion="-mmacosx-version-min=10.4"
    case "$target_os" in
      darwin8*)
        mac_arches="-arch i386 -arch ppc"
        mac_sysroot="-isysroot /Developer/SDKs/MacOSX10.4u.sdk"
        mac_frameworks="-framework CoreFoundation"
        ;;
      darwin9*)
        mac_arches="-arch i386 -arch ppc -arch x86_64 -arch ppc64"
        mac_sysroot="-isysroot /Developer/SDKs/MacOSX10.5.sdk"
        mac_frameworks="-framework CoreFoundation"
       ;;
      darwin10*)
        mac_arches="-arch i386 -arch x86_64"
        mac_sysroot="-isysroot /Developer/SDKs/MacOSX10.6.sdk"
        mac_frameworks="-framework CoreFoundation"
        ;;
      *)
        mac_arches=""
        mac_sysroot=""
        mac_frameworks="-framework CoreFoundation"
        ;;
    esac
    MAC_UNIVERSAL_CFLAGS="$mac_arches $mac_sysroot $mac_minversion  $mac_frameworks"
    MAC_UNIVERSAL_LDFLAGS="$mac_arches $mac_frameworks"
fi
AC_SUBST([MAC_UNIVERSAL_CFLAGS])
AC_SUBST([MAC_UNIVERSAL_LDFLAGS])

AM_CONDITIONAL([DARWIN], [test "x$target_darwin" = "xyes"])
])
