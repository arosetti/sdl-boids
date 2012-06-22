AC_PREREQ(2.61)
AC_INIT([boids_sdl], [0.1.0])
AM_PROG_CC_C_O

AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wall foreign])

AC_CONFIG_HEADERS([config.h])

AC_CHECK_HEADERS([semaphore.h stdint.h stdbool.h stdlib.h unistd.h math.h])
AC_CHECK_FUNCS()

AC_CHECK_LIB(m, sqrt)
AC_CHECK_LIB(pthread, pthread_create)

AC_CHECK_LIB(SDL, main, LIBS="$LIBS -lSDL", 
             AC_MSG_ERROR([*** SDL library not found!]))

AC_CHECK_LIB(SDL_image, main, LIBS="$LIBS -lSDL_image", 
             AC_MSG_ERROR([*** SDL_image library not found!]))

AC_CHECK_LIB(SDL_ttf, main, LIBS="$LIBS -lSDL_ttf",
             AC_MSG_ERROR([*** SDL_ttf library not found!]))

PKG_CHECK_MODULES(SDL, sdl)
AC_SUBST(SDL_LIBS)
AC_SUBST(SDL_CFLAGS)

#debug
AC_ARG_ENABLE(
debug,
[--enable-debug=ARG enable support for debugging (default=no)],
[enable_debug=$enableval],
[enable_debug=no]
)
if test "$enable_debug" = "yes"; then
CFLAGS="-O0 -g -Wall -Werror -pedantic"
AC_DEFINE(ENABLE_DEBUG,1,[debug option])
else
AC_DEFINE(ENABLE_DEBUG,0,[debug option])
fi

#release
AC_ARG_ENABLE(
release,
[--enable-release=ARG enable release version (default=no)],
[enable_release=$enableval],
[enable_release=no]
)
if test "$enable_release" = "yes"; then
CFLAGS="-O3 -mtune=native -march=native -pipe -fno-builtin"
AC_DEFINE(ENABLE_RELEASE,1,[release option])
else
AC_DEFINE(ENABLE_RELEASE,0,[release option])
fi

CFLAGS="$CFLAGS -std=c99"

DISTCLEANFILES="Makefile.in"
AC_SUBST(DISTCLEANFILES)

#AC_SUBST(artdir, "${bcdatadir}/art/", "Art")
#AC_SUBST(fontsdir, "${bcdatadir}/fonts/", "Fonts")

AC_CONFIG_FILES([Makefile]) # art/Makefile])
AC_OUTPUT
