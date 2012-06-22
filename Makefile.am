AUTOMAKE_OPTIONS = subdir-objects foreign
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}
bin_PROGRAMS = boids

AM_CPPFLAGS = $(SDL_CFLAGS)
boids_LDADD = $(SDL_LIBS)

boids_SOURCES =  src/boids.c src/boids.h\
                 src/boids_engine.c src/boids_engine.h\
                 src/sdl_utils.c src/sdl_utils.h\
                 src/sdl_mirror.c src/sdl_mirror.h\
                 src/sdl_text.c src/sdl_text.h\
                 src/shared.c src/shared.h\
                 src/vector.c src/vector.h

dist_noinst_SCRIPTS = autogen.sh clean.sh

#SUBDIRS = art fonts
