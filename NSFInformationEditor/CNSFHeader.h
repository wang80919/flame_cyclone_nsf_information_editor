#pragma once

#include <cstdint>

#pragma pack(push, 1)
// https://www.nesdev.org/wiki/NSF
typedef struct _NSF_HEADER
{
	uint8_t		format_file[5];		// 'N','E','S','M',$1A (denotes an NES sound format file)
	uint8_t		version_number;		// Version number $01 (or $02 for NSF2)
	uint8_t		total_songs;		// Total songs   (1=1 song, 2=2 songs, etc)
	uint8_t		start_song;		// Starting song (1=1st song, 2=2nd song, etc)
	uint16_t	load_address;		// (lo, hi) load address of data ($8000-FFFF)
	uint16_t	init_address;		// (lo, hi) init address of data ($8000-FFFF)
	uint16_t	play_address;		// (lo, hi) play address of data ($8000-FFFF)
	char		name[32];			// The name of the song, null terminated
	char		artist[32];			// The artist, if known, null terminated
	char		copyright[32];		// The copyright holder, null terminated
	uint16_t	play_speed_ntsc;	// (lo, hi) Play speed, in 1/1000000th sec ticks, NTSC (see text)
	uint8_t		bankswitch[8];		// Bankswitch init values (see text, and FDS section)
	uint16_t	play_speed_pal;		// (lo, hi) Play speed, in 1/1000000th sec ticks, PAL (see text)
	uint8_t		standard;			// PAL/NTSC bits
									// bit 0: if clear, this is an NTSC tune
									// bit 0: if set, this is a PAL tune
									// bit 1: if set, this is a dual PAL/NTSC tune
									// bits 2-7: reserved, must be 0
	uint8_t		extra_sound;		// Extra Sound Chip Support
									// bit 0: if set, this song uses VRC6 audio
									// bit 1: if set, this song uses VRC7 audio
									// bit 2: if set, this song uses FDS audio
									// bit 3: if set, this song uses MMC5 audio
									// bit 4: if set, this song uses Namco 163 audio
									// bit 5: if set, this song uses Sunsoft 5B audio
									// bit 6: if set, this song uses VT02+ audio
									// bit 7: reserved, must be zero
	uint8_t		reserved_for_nsf2;	// Reserved for NSF2
	uint8_t		program_data[3];	// 24-bit length of contained program data.
									// If 0, all data until end of file is part of the program.
									// If used, can be used to provide NSF2 metadata
									// in a backward compatible way.
}NSF_HEADER;
#pragma pack(pop)