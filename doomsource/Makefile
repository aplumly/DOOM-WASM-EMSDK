################################################################
#
# $Id:$
#
# $Log:$
#
CC=  emcc  # Use emcc for Emscripten compilation

# Compilation flags (WITHOUT LINKER FLAGS)
CFLAGS=-g -Wall -DNORMALUNIX -DLINUX -s USE_SDL=2 -Wno-unused-variable -Wno-unused-but-set-variable -Wno-enum-compare

# Linking flags (ONLY USED IN FINAL LINKING STEP)
LDFLAGS=-s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS=1 -s EXIT_RUNTIME=1 -s USE_SDL=2 --preload-file wad@/wad -s EXPORTED_FUNCTIONS="['_main']" -s EXPORTED_RUNTIME_METHODS="['ccall', 'cwrap']" -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1

# Libraries to link
LIBS=-lm

# Subdirectory for objects
O=linux


# Object files to compile
OBJS= \
		$(O)/doomdef.o \
		$(O)/doomstat.o \
		$(O)/dstrings.o \
		$(O)/i_system.o \
		$(O)/i_sound.o \
		$(O)/i_video.o \
		$(O)/i_net.o \
		$(O)/tables.o \
		$(O)/f_finale.o \
		$(O)/f_wipe.o \
		$(O)/d_main.o \
		$(O)/d_net.o \
		$(O)/d_items.o \
		$(O)/g_game.o \
		$(O)/m_menu.o \
		$(O)/m_misc.o \
		$(O)/m_argv.o \
		$(O)/m_bbox.o \
		$(O)/m_fixed.o \
		$(O)/m_swap.o \
		$(O)/m_cheat.o \
		$(O)/m_random.o \
		$(O)/am_map.o \
		$(O)/p_ceilng.o \
		$(O)/p_doors.o \
		$(O)/p_enemy.o \
		$(O)/p_floor.o \
		$(O)/p_inter.o \
		$(O)/p_lights.o \
		$(O)/p_map.o \
		$(O)/p_maputl.o \
		$(O)/p_plats.o \
		$(O)/p_pspr.o \
		$(O)/p_setup.o \
		$(O)/p_sight.o \
		$(O)/p_spec.o \
		$(O)/p_switch.o \
		$(O)/p_mobj.o \
		$(O)/p_telept.o \
		$(O)/p_tick.o \
		$(O)/p_saveg.o \
		$(O)/p_user.o \
		$(O)/r_bsp.o \
		$(O)/r_data.o \
		$(O)/r_draw.o \
		$(O)/r_main.o \
		$(O)/r_plane.o \
		$(O)/r_segs.o \
		$(O)/r_sky.o \
		$(O)/r_things.o \
		$(O)/w_wad.o \
		$(O)/wi_stuff.o \
		$(O)/v_video.o \
		$(O)/st_lib.o \
		$(O)/st_stuff.o \
		$(O)/hu_stuff.o \
		$(O)/hu_lib.o \
		$(O)/s_sound.o \
		$(O)/z_zone.o \
		$(O)/info.o \
		$(O)/sounds.o

all: $(O)/doom.html

clean:
	rm -f *.o *~ *.flc
	rm -rf linux/*

$(O)/doom.html:	$(OBJS) $(O)/i_main.o
	$(CC) $(OBJS) $(O)/i_main.o -o $(O)/doom.html $(LDFLAGS) $(LIBS)

$(O)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

#############################################################
#
#############################################################