/*
 * Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
 *
 * (c) Copyright 1996 - 2001 Gary Henderson (gary.henderson@ntlworld.com) and
 *                           Jerremy Koot (jkoot@snes9x.com)
 *
 * Super FX C emulator code
 * (c) Copyright 1997 - 1999 Ivar (ivar@snes9x.com) and
 *                           Gary Henderson.
 * Super FX assembler emulator code (c) Copyright 1998 zsKnight and _Demo_.
 *
 * DSP1 emulator code (c) Copyright 1998 Ivar, _Demo_ and Gary Henderson.
 * C4 asm and some C emulation code (c) Copyright 2000 zsKnight and _Demo_.
 * C4 C code (c) Copyright 2001 Gary Henderson (gary.henderson@ntlworld.com).
 *
 * DOS port code contains the works of other authors. See headers in
 * individual files.
 *
 * Snes9x homepage: http://www.snes9x.com
 *
 * Permission to use, copy, modify and distribute Snes9x in both binary and
 * source form, for non-commercial purposes, is hereby granted without fee,
 * providing that this license information and copyright notice appear with
 * all copies and any derived work.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event shall the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Snes9x is freeware for PERSONAL USE only. Commercial users should
 * seek permission of the copyright holders first. Commercial use includes
 * charging money for Snes9x or software derived from Snes9x.
 *
 * The copyright holders request that bug fixes and improvements to the code
 * should be forwarded to them so everyone can benefit from the modifications
 * in future versions.
 *
 * Super NES and Super Nintendo Entertainment System are trademarks of
 * Nintendo Co., Limited and its subsidiary companies.
 */
#include <string.h>
#include <ctype.h>

#ifdef __linux
#include <unistd.h>
#endif

#include "snes9x.h"
#include "memmap.h"
#include "cpuexec.h"
#include "ppu.h"
#include "display.h"
#include "apu.h"
#include "sa1.h"
#include "srtc.h"
#include "sdd1.h"

#ifdef CHEATS
#include "cheats.h"
#endif

#ifndef ZSNES_FX
#include "fxemu.h"
extern struct FxInit_s SuperFX;
#else
START_EXTERN_C
extern uint8 *SFXPlotTable;
END_EXTERN_C
#endif

static uint8 bytes0x2000[0x2000];

extern char *rom_filename;
extern bool8 LoadZip(const char *, int32 *, int32 *);

#ifdef SNESADVANCE_SPEEDHACKS
static const uint32 crc32Table[256] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832,
    0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856, 0x646ba8c0, 0xfd62f97a,
    0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3,
    0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab,
    0xb6662d3d, 0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01, 0x6b6b51f4,
    0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074,
    0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525,
    0x206f85b3, 0xb966d409, 0xce61e49f, 0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615,
    0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7, 0xfed41b76,
    0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c, 0x36034af6,
    0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7,
    0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7,
    0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278,
    0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9, 0xbdbdf21c, 0xcabac28a, 0x53b39330,
    0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d};

// CRC32 for char arrays
uint32 caCRC32(uint8 *array, uint32 size, register uint32 crc32)
{
	for (register uint32 i = 0; i < size; i++) {
		crc32 = ((crc32 >> 8) & 0x00FFFFFF) ^ crc32Table[(crc32 ^ array[i]) & 0xFF];
	}
	return ~crc32;
}
#endif

bool8_32 CMemory::AllASCII(uint8 *b, int size)
{
	for (int i = 0; i < size; i++) {
		if (b[i] < 32 || b[i] > 126)
			return (FALSE);
	}
	return (TRUE);
}

int CMemory::ScoreHiROM(bool8_32 skip_header)
{
	int score = 0;
	int o = skip_header ? 0xff00 + 0x200 : 0xff00;

	if ((Memory.ROM[o + 0xdc] + (Memory.ROM[o + 0xdd] << 8) + Memory.ROM[o + 0xde] + (Memory.ROM[o + 0xdf] << 8)) ==
	    0xffff)
		score += 2;

	if (Memory.ROM[o + 0xda] == 0x33)
		score += 2;
	if ((Memory.ROM[o + 0xd5] & 0xf) < 4)
		score += 2;
	if (!(Memory.ROM[o + 0xfd] & 0x80))
		score -= 4;
	if (CalculatedSize > 1024 * 1024 * 3)
		score += 4;
	if ((1 << (Memory.ROM[o + 0xd7] - 7)) > 48)
		score -= 1;
	if (!AllASCII(&Memory.ROM[o + 0xb0], 6))
		score -= 1;
	if (!AllASCII(&Memory.ROM[o + 0xc0], ROM_NAME_LEN - 1))
		score -= 1;

	return (score);
}

int CMemory::ScoreLoROM(bool8_32 skip_header)
{
	int score = 0;
	int o = skip_header ? 0x7f00 + 0x200 : 0x7f00;

	if ((Memory.ROM[o + 0xdc] + (Memory.ROM[o + 0xdd] << 8) + Memory.ROM[o + 0xde] + (Memory.ROM[o + 0xdf] << 8)) ==
	    0xffff)
		score += 2;

	if (Memory.ROM[o + 0xda] == 0x33)
		score += 2;
	if ((Memory.ROM[o + 0xd5] & 0xf) < 4)
		score += 2;
	if (CalculatedSize <= 1024 * 1024 * 16)
		score += 2;
	if (!(Memory.ROM[o + 0xfd] & 0x80))
		score -= 4;
	if ((1 << (Memory.ROM[o + 0xd7] - 7)) > 48)
		score -= 1;
	if (!AllASCII(&Memory.ROM[o + 0xb0], 6))
		score -= 1;
	if (!AllASCII(&Memory.ROM[o + 0xc0], ROM_NAME_LEN - 1))
		score -= 1;

	return (score);
}

char *CMemory::Safe(const char *s)
{
	static char *safe = NULL;
	static int safe_len = 0;

	int len = strlen(s);
	if (!safe || len + 1 > safe_len) {
		if (safe)
			free((char *)safe);
		safe = (char *)malloc(safe_len = len + 1);
	}

	for (int i = 0; i < len; i++) {
		if (s[i] >= 32 && s[i] < 127)
			safe[i] = s[i];
		else
			safe[i] = '?';
	}
	safe[len] = 0;
	return (safe);
}

/**********************************************************************************************/
/* Init() */
/* This function allocates all the memory needed by the emulator */
/**********************************************************************************************/
bool8_32 CMemory::Init()
{
	RAM = (uint8 *)malloc(0x20000);
	SRAM = (uint8 *)malloc(0x20000);
	VRAM = (uint8 *)malloc(0x10000);
	ROM = (uint8 *)malloc(MAX_ROM_SIZE + 0x200 + 0x8000);
	FillRAM = NULL;

	IPPU.TileCache[TILE_2BIT] = (uint8 *)malloc(MAX_2BIT_TILES * 128);
	IPPU.TileCache[TILE_4BIT] = (uint8 *)malloc(MAX_4BIT_TILES * 128);
	IPPU.TileCache[TILE_8BIT] = (uint8 *)malloc(MAX_8BIT_TILES * 128);

	IPPU.TileCached[TILE_2BIT] = (uint8 *)malloc(MAX_2BIT_TILES);
	IPPU.TileCached[TILE_4BIT] = (uint8 *)malloc(MAX_4BIT_TILES);
	IPPU.TileCached[TILE_8BIT] = (uint8 *)malloc(MAX_8BIT_TILES);

	if (!RAM || !SRAM || !VRAM || !ROM || !IPPU.TileCache[TILE_2BIT] || !IPPU.TileCache[TILE_4BIT] ||
	    !IPPU.TileCache[TILE_8BIT] || !IPPU.TileCached[TILE_2BIT] || !IPPU.TileCached[TILE_4BIT] ||
	    !IPPU.TileCached[TILE_8BIT]) {
		Deinit();
		return (FALSE);
	}

	// FillRAM uses first 32K of ROM image area, otherwise space just
	// wasted. Might be read by the SuperFX code.

	FillRAM = ROM;

	// Add 0x8000 to ROM image pointer to stop SuperFX code accessing
	// unallocated memory (can cause crash on some ports).
	ROM += 0x8000;

	C4RAM = ROM + 0x400000 + 8192 * 8;
	::ROM = ROM;
	::SRAM = SRAM;
	::RegRAM = FillRAM;

#ifdef ZSNES_FX
	SFXPlotTable = ROM + 0x400000;
#else
	SuperFX.pvRegisters = &Memory.FillRAM[0x3000];
	SuperFX.nRamBanks = 2; // Most only use 1.  1=64KB, 2=128KB=1024Mb
	SuperFX.pvRam = ::SRAM;
	SuperFX.nRomBanks = (2 * 1024 * 1024) / (32 * 1024);
	SuperFX.pvRom = (uint8 *)ROM;
#endif

	ZeroMemory(IPPU.TileCache[TILE_2BIT], MAX_2BIT_TILES * 128);
	ZeroMemory(IPPU.TileCache[TILE_4BIT], MAX_4BIT_TILES * 128);
	ZeroMemory(IPPU.TileCache[TILE_8BIT], MAX_8BIT_TILES * 128);

	ZeroMemory(IPPU.TileCached[TILE_2BIT], MAX_2BIT_TILES);
	ZeroMemory(IPPU.TileCached[TILE_4BIT], MAX_4BIT_TILES);
	ZeroMemory(IPPU.TileCached[TILE_8BIT], MAX_8BIT_TILES);

	SDD1Data = NULL;
	SDD1Index = NULL;

	return (TRUE);
}

void CMemory::Deinit()
{
	if (RAM) {
		free((char *)RAM);
		RAM = NULL;
	}
	if (SRAM) {
		free((char *)SRAM);
		SRAM = NULL;
	}
	if (VRAM) {
		free((char *)VRAM);
		VRAM = NULL;
	}
	if (ROM) {
		ROM -= 0x8000;
		free((char *)ROM);
		ROM = NULL;
	}

	if (IPPU.TileCache[TILE_2BIT]) {
		free((char *)IPPU.TileCache[TILE_2BIT]);
		IPPU.TileCache[TILE_2BIT] = NULL;
	}
	if (IPPU.TileCache[TILE_4BIT]) {
		free((char *)IPPU.TileCache[TILE_4BIT]);
		IPPU.TileCache[TILE_4BIT] = NULL;
	}
	if (IPPU.TileCache[TILE_8BIT]) {
		free((char *)IPPU.TileCache[TILE_8BIT]);
		IPPU.TileCache[TILE_8BIT] = NULL;
	}

	if (IPPU.TileCached[TILE_2BIT]) {
		free((char *)IPPU.TileCached[TILE_2BIT]);
		IPPU.TileCached[TILE_2BIT] = NULL;
	}
	if (IPPU.TileCached[TILE_4BIT]) {
		free((char *)IPPU.TileCached[TILE_4BIT]);
		IPPU.TileCached[TILE_4BIT] = NULL;
	}
	if (IPPU.TileCached[TILE_8BIT]) {
		free((char *)IPPU.TileCached[TILE_8BIT]);
		IPPU.TileCached[TILE_8BIT] = NULL;
	}

	FreeSDD1Data();
}

void CMemory::FreeSDD1Data()
{
	if (SDD1Index) {
		free((char *)SDD1Index);
		SDD1Index = NULL;
	}
	if (SDD1Data) {
		free((char *)SDD1Data);
		SDD1Data = NULL;
	}
}

/**********************************************************************************************/
/* LoadROM() */
/* This function loads a Snes-Backup image */
/**********************************************************************************************/
bool8_32 CMemory::LoadROM(const char *filename)
{
	unsigned long FileSize = 0;
	int retry_count = 0;
	STREAM ROMFile;
	bool8_32 Interleaved = FALSE;
	bool8_32 Tales = FALSE;
	char dir[_MAX_DIR + 1];
	char drive[_MAX_DRIVE + 1];
	char name[_MAX_FNAME + 1];
	char ext[_MAX_EXT + 1];
	char fname[_MAX_PATH + 1];
	int i;

	memset(&SNESGameFixes, 0, sizeof(SNESGameFixes));
	SNESGameFixes.SRAMInitialValue = 0x60;

	memset(bytes0x2000, 0, 0x2000);
	CPU.TriedInterleavedMode2 = FALSE;

	CalculatedSize = 0;
again:
	_splitpath(filename, drive, dir, name, ext);
	_makepath(fname, drive, dir, name, ext);

	int32 TotalFileSize = 0;

#ifdef UNZIP_SUPPORT
	if (strcasecmp(ext, "zip") == 0) {
		bool8 LoadZip(const char *, int32 *, int32 *);

		if (!LoadZip(fname, &TotalFileSize, &HeaderCount))
			return (FALSE);

		strcpy(ROMFilename, fname);
	} else
#endif
	{
		if ((ROMFile = OPEN_STREAM(fname, "rb")) == NULL)
			return (FALSE);

		strcpy(ROMFilename, fname);

		HeaderCount = 0;
		uint8 *ptr = ROM;
		bool8_32 more = FALSE;

		do {

			FileSize = READ_STREAM(ptr, MAX_ROM_SIZE + 0x200 - (ptr - ROM), ROMFile);
			CLOSE_STREAM(ROMFile);
			int calc_size = (FileSize / 0x2000) * 0x2000;

			if ((FileSize - calc_size == 512 && !Settings.ForceNoHeader) || Settings.ForceHeader) {
				memmove(ptr, ptr + 512, calc_size);
				HeaderCount++;
				FileSize -= 512;
			}
			ptr += FileSize;
			TotalFileSize += FileSize;

			int len;
			if (ptr - ROM < MAX_ROM_SIZE + 0x200 && (isdigit(ext[0]) && ext[1] == 0 && ext[0] < '9')) {
				more = TRUE;
				ext[0]++;
#ifdef __WIN32__
				memmove(&ext[1], &ext[0], 4);
				ext[0] = '.';
#endif
				_makepath(fname, drive, dir, name, ext);
			} else if (ptr - ROM < MAX_ROM_SIZE + 0x200 &&
				   (((len = strlen(name)) == 7 || len == 8) && strncasecmp(name, "sf", 2) == 0 &&
				    isdigit(name[2]) && isdigit(name[3]) && isdigit(name[4]) && isdigit(name[5]) &&
				    isalpha(name[len - 1]))) {
				more = TRUE;
				name[len - 1]++;
#ifdef __WIN32__
				memmove(&ext[1], &ext[0], 4);
				ext[0] = '.';
#endif
				_makepath(fname, drive, dir, name, ext);
			} else
				more = FALSE;
		} while (more && (ROMFile = OPEN_STREAM(fname, "rb")) != NULL);
	}

	if (HeaderCount == 0)
		S9xMessage(S9X_INFO, S9X_HEADERS_INFO, "No ROM file header found.");
	else {
		if (HeaderCount == 1)
			S9xMessage(S9X_INFO, S9X_HEADERS_INFO, "Found ROM file header (and ignored it).");
		else
			S9xMessage(S9X_INFO, S9X_HEADERS_INFO,
				   "Found multiple ROM file headers (and "
				   "ignored them).");
	}

	CheckForIPSPatch(filename, HeaderCount != 0, TotalFileSize);
	// int orig_hi_score, orig_lo_score;
	int hi_score, lo_score;

	/* orig_hi_score = */ hi_score = ScoreHiROM(FALSE);
	/* orig_lo_score = */ lo_score = ScoreLoROM(FALSE);

	if (HeaderCount == 0 && !Settings.ForceNoHeader &&
	    ((hi_score > lo_score && ScoreHiROM(TRUE) > hi_score) ||
	     (hi_score <= lo_score && ScoreLoROM(TRUE) > lo_score))) {
		memmove(Memory.ROM, Memory.ROM + 512, TotalFileSize - 512);
		TotalFileSize -= 512;
		S9xMessage(S9X_INFO, S9X_HEADER_WARNING,
			   "Try specifying the -nhd command line option if the "
			   "game doesn't work\n");
	}

	CalculatedSize = (TotalFileSize / 0x2000) * 0x2000;
	ZeroMemory(ROM + CalculatedSize, MAX_ROM_SIZE - CalculatedSize);

	// Check for cherryroms.com DAIKAIJYUMONOGATARI2

	if (CalculatedSize == 0x500000 && strncmp((const char *)&ROM[0x40ffc0], "DAIKAIJYUMONOGATARI2", 20) == 0 &&
	    strncmp((const char *)&ROM[0x40ffb0], "18AE6J", 6) == 0 && memcmp(&ROM[0x40ffb0], &ROM[0xffb0], 0x30)) {
		memmove(&ROM[0x100000], ROM, 0x500000);
		memmove(ROM, &ROM[0x500000], 0x100000);
	}

	Interleaved = Settings.ForceInterleaved || Settings.ForceInterleaved2;
	if (Settings.ForceLoROM || (!Settings.ForceHiROM && lo_score >= hi_score)) {
		LoROM = TRUE;
		HiROM = FALSE;

		// Ignore map type byte if not 0x2x or 0x3x
		if ((ROM[0x7fd5] & 0xf0) == 0x20 || (ROM[0x7fd5] & 0xf0) == 0x30) {
			switch (ROM[0x7fd5] & 0xf) {
			case 1:
				if (strncmp((char *)&ROM[0x7fc0], "TREASURE HUNTER G", 17) != 0)
					Interleaved = TRUE;
				break;
			case 2:
#if 0
				if (!Settings.ForceLoROM &&
				    strncmp((char *)&ROM[0x7fc0], "SUPER FORMATION SOCCE", 21) != 0 &&
				    strncmp((char *)&ROM[0x7fc0], "Star Ocean", 10) != 0) {
					LoROM = FALSE;
					HiROM = TRUE;
				}
#endif
				break;
			case 5:
				Interleaved = TRUE;
				Tales = TRUE;
				break;
			}
		}
	} else {
		if ((ROM[0xffd5] & 0xf0) == 0x20 || (ROM[0xffd5] & 0xf0) == 0x30) {
			switch (ROM[0xffd5] & 0xf) {
			case 0:
			case 3:
				Interleaved = TRUE;
				break;
			}
		}
		LoROM = FALSE;
		HiROM = TRUE;
	}

	// More
	if (!Settings.ForceHiROM && !Settings.ForceLoROM && !Settings.ForceInterleaved && !Settings.ForceInterleaved2 &&
	    !Settings.ForceNotInterleaved && !Settings.ForcePAL && !Settings.ForceSuperFX && !Settings.ForceDSP1 &&
	    !Settings.ForceSA1 && !Settings.ForceC4 && !Settings.ForceSDD1) {
		if (strncmp((char *)&ROM[0x7fc0], "YOSHI'S ISLAND", 14) == 0 && (*(uint16 *)&ROM[0x7FDE]) == 57611 &&
		    ROM[0x10002] == 0xA9) {
			Interleaved = true;
			Settings.ForceInterleaved2 = true;
		} else if (strncmp((char *)&ROM[0x7fc0], "YUYU NO QUIZ DE GO!GO!", 22) == 0) {
			LoROM = TRUE;
			HiROM = FALSE;
			Interleaved = FALSE;
		} else if (strncmp((char *)&ROM[0x7fc0], "SP MOMOTAROU DENTETSU2", 22) == 0) {
			LoROM = TRUE;
			HiROM = FALSE;
			Interleaved = FALSE;
		} else if (CalculatedSize == 0x100000 &&
			   strncmp((char *)&ROM[0xffc0], "WWF SUPER WRESTLEMANIA", 22) == 0) {
			int cvcount;

			memmove(&ROM[0x100000], ROM, 0x100000);
			for (cvcount = 0; cvcount < 16; cvcount++) {
				memmove(&ROM[0x8000 * cvcount], &ROM[0x10000 * cvcount + 0x100000 + 0x8000], 0x8000);
				memmove(&ROM[0x8000 * cvcount + 0x80000], &ROM[0x10000 * cvcount + 0x100000], 0x8000);
			}
			LoROM = TRUE;
			HiROM = FALSE;
			ZeroMemory(ROM + CalculatedSize, MAX_ROM_SIZE - CalculatedSize);
		}
	}

	if (!Settings.ForceNotInterleaved && Interleaved) {
		CPU.TriedInterleavedMode2 = TRUE;
		S9xMessage(S9X_INFO, S9X_ROM_INTERLEAVED_INFO, "ROM image is in interleaved format - converting...");

		int nblocks = CalculatedSize >> 16;
#if 0
		int step = 64;

		while (nblocks <= step)
		    step >>= 1;

		nblocks = step;
#endif
		uint8 blocks[256];

		if (Tales) {
			nblocks = 0x60;
			for (i = 0; i < 0x40; i += 2) {
				blocks[i + 0] = (i >> 1) + 0x20;
				blocks[i + 1] = (i >> 1) + 0x00;
			}
			for (i = 0; i < 0x80; i += 2) {
				blocks[i + 0x40] = (i >> 1) + 0x80;
				blocks[i + 0x41] = (i >> 1) + 0x40;
			}
			LoROM = FALSE;
			HiROM = TRUE;
		} else if (Settings.ForceInterleaved2) {
			for (i = 0; i < nblocks * 2; i++) {
				blocks[i] =
				    (i & ~0x1e) | ((i & 2) << 2) | ((i & 4) << 2) | ((i & 8) >> 2) | ((i & 16) >> 2);
			}
		} else {
			bool8_32 t = LoROM;

			LoROM = HiROM;
			HiROM = t;

			for (i = 0; i < nblocks; i++) {
				blocks[i * 2] = i + nblocks;
				blocks[i * 2 + 1] = i;
			}
		}

		uint8 *tmp = (uint8 *)malloc(0x8000);
		if (tmp) {
			for (i = 0; i < nblocks * 2; i++) {
				for (int j = i; j < nblocks * 2; j++) {
					if (blocks[j] == i) {
						memmove(tmp, &ROM[blocks[j] * 0x8000], 0x8000);
						memmove(&ROM[blocks[j] * 0x8000], &ROM[blocks[i] * 0x8000], 0x8000);
						memmove(&ROM[blocks[i] * 0x8000], tmp, 0x8000);
						uint8 b = blocks[j];
						blocks[j] = blocks[i];
						blocks[i] = b;
						break;
					}
				}
			}
			free((char *)tmp);
		}

		hi_score = ScoreHiROM(FALSE);
		lo_score = ScoreLoROM(FALSE);

		if ((HiROM && (lo_score >= hi_score || hi_score < 0)) ||
		    (LoROM && (hi_score > lo_score || lo_score < 0))) {
			if (retry_count == 0) {
				S9xMessage(S9X_INFO, S9X_ROM_CONFUSING_FORMAT_INFO,
					   "ROM lied about its type! Trying again.");
				Settings.ForceNotInterleaved = TRUE;
				Settings.ForceInterleaved = FALSE;
				retry_count++;
				goto again;
			}
		}
	}

	FreeSDD1Data();

	InitROM(Tales);

#ifdef CHEATS
	S9xLoadCheatFile(S9xGetFilename(".cht"));
	// S9xInitCheatData();
	// S9xApplyCheats();
#endif

	S9xReset();

	return (TRUE);
}

void S9xDeinterleaveMode2()
{
	S9xMessage(S9X_INFO, S9X_ROM_INTERLEAVED_INFO, "ROM image is in interleaved format - converting...");

	int nblocks = Memory.CalculatedSize >> 15;
	int step = 64;

	while (nblocks <= step)
		step >>= 1;

	nblocks = step;
	uint8 blocks[256];
	int i;

	for (i = 0; i < nblocks * 2; i++) {
		blocks[i] = (i & ~0x1e) | ((i & 2) << 2) | ((i & 4) << 2) | ((i & 8) >> 2) | ((i & 16) >> 2);
	}

	uint8 *tmp = (uint8 *)malloc(0x8000);

	if (tmp) {
		for (i = 0; i < nblocks * 2; i++) {
			for (int j = i; j < nblocks * 2; j++) {
				if (blocks[j] == i) {
					memmove(tmp, &Memory.ROM[blocks[j] * 0x8000], 0x8000);
					memmove(&Memory.ROM[blocks[j] * 0x8000], &Memory.ROM[blocks[i] * 0x8000],
						0x8000);
					memmove(&Memory.ROM[blocks[i] * 0x8000], tmp, 0x8000);
					uint8 b = blocks[j];
					blocks[j] = blocks[i];
					blocks[i] = b;
					break;
				}
			}
		}
		free((char *)tmp);
	}
	Memory.InitROM(FALSE);
	S9xReset();
}

void CMemory::InitROM(bool8_32 Interleaved)
{
#ifndef ZSNES_FX
	SuperFX.nRomBanks = CalculatedSize >> 15;
#endif
	Settings.MultiPlayer5Master = Settings.MultiPlayer5;
	Settings.MouseMaster = Settings.Mouse;
	Settings.SuperScopeMaster = Settings.SuperScope;
	Settings.DSP1Master = Settings.ForceDSP1;
	Settings.SuperFX = FALSE;
	Settings.SA1 = FALSE;
	Settings.C4 = FALSE;
	Settings.SDD1 = FALSE;
	Settings.SRTC = FALSE;
#ifdef SETA_DSP
	Settings.SETA = FALSE;
#endif

	ZeroMemory(BlockIsRAM, MEMMAP_NUM_BLOCKS);
	ZeroMemory(BlockIsROM, MEMMAP_NUM_BLOCKS);

	::SRAM = SRAM;
	memset(ROMId, 0, 5);
	memset(CompanyId, 0, 3);

	if (Memory.HiROM) {
		Memory.SRAMSize = ROM[0xffd8];
		strncpy(ROMName, (char *)&ROM[0xffc0], ROM_NAME_LEN - 1);
		ROMSpeed = ROM[0xffd5];
		ROMType = ROM[0xffd6];
		ROMSize = ROM[0xffd7];
		ROMChecksum = ROM[0xffde] + (ROM[0xffdf] << 8);
		ROMComplementChecksum = ROM[0xffdc] + (ROM[0xffdd] << 8);

		memmove(ROMId, &ROM[0xffb2], 4);
		memmove(CompanyId, &ROM[0xffb0], 2);

		// Try to auto-detect the DSP1 chip
		if (!Settings.ForceNoDSP1 && (ROMType & 0xf) >= 3 && (ROMType & 0xf0) == 0)
			Settings.DSP1Master = TRUE;

		Settings.SDD1 = Settings.ForceSDD1;
		if ((ROMType & 0xf0) == 0x40)
			Settings.SDD1 = !Settings.ForceNoSDD1;

		if (Settings.BS)
			BSHiROMMap();
#ifdef SPC7110_DSP
		else if (Settings.SPC7110)
			SPC7110HiROMMap();
#endif
		else if ((ROMSpeed & ~0x10) == 0x25)
			TalesROMMap(Interleaved);
		else if ((ROMSpeed & ~0x10) == 0x22 && strncmp(ROMName, "Super Street Fighter", 20) != 0) {
			AlphaROMMap();
		} else
			HiROMMap();
	} else {
		Memory.HiROM = FALSE;
		Memory.SRAMSize = ROM[0x7fd8];
		ROMSpeed = ROM[0x7fd5];
		ROMType = ROM[0x7fd6];
		ROMSize = ROM[0x7fd7];
		ROMChecksum = ROM[0x7fde] + (ROM[0x7fdf] << 8);
		ROMComplementChecksum = ROM[0x7fdc] + (ROM[0x7fdd] << 8);
		memmove(ROMId, &ROM[0x7fb2], 4);
		memmove(CompanyId, &ROM[0x7fb0], 2);

		strncpy(ROMName, (char *)&ROM[0x7fc0], ROM_NAME_LEN - 1);
		Settings.SuperFX = Settings.ForceSuperFX;

		if ((ROMType & 0xf0) == 0x10)
			Settings.SuperFX = !Settings.ForceNoSuperFX;

		// Try to auto-detect the DSP1 chip
		if (!Settings.ForceNoDSP1 && (ROMType & 0xf) >= 3 && (ROMType & 0xf0) == 0)
			Settings.DSP1Master = TRUE;

		Settings.SDD1 = Settings.ForceSDD1;
		if ((ROMType & 0xf0) == 0x40)
			Settings.SDD1 = !Settings.ForceNoSDD1;
		/* SiENcE: Not needed anymore with emulator code
			if (Settings.SDD1)
			    S9xLoadSDD1Data ();
		*/
#ifdef SETA_DSP
		if (((ROMType & 0xF0) == 0xF0) && ((ROMSpeed & 0x0F) != 5)) {
			SRAMSize = 2;
			SNESGameFixes.SRAMInitialValue = 0x00;
			if ((ROMType & 0x0F) == 6) {
				if (ROM[0x7FD7] == 0x09) {
					Settings.SETA = ST_011;
					SetSETA = &S9xSetST011;
					GetSETA = &S9xGetST011;
				} else {
					Settings.SETA = ST_010;
					SetSETA = &S9xSetST010;
					GetSETA = &S9xGetST010;
				}
			} else {
				Settings.SETA = ST_018;
			}
		}
#endif
		Settings.C4 = Settings.ForceC4;
		if ((ROMType & 0xf0) == 0xf0 &&
		    (strncmp(ROMName, "MEGAMAN X", 9) == 0 || strncmp(ROMName, "ROCKMAN X", 9) == 0)) {
			Settings.C4 = !Settings.ForceNoC4;
		}

#ifdef SETA_DSP
		if (Settings.SETA && Settings.SETA != ST_018)
			SetaDSPMap();
		else
#endif
		if (Settings.SuperFX) {
			//::SRAM = ROM + 1024 * 1024 * 4;
			SuperFXROMMap();
			Settings.MultiPlayer5Master = FALSE;
			// Settings.MouseMaster = FALSE;
			// Settings.SuperScopeMaster = FALSE;
			Settings.DSP1Master = FALSE;
			Settings.SA1 = FALSE;
			Settings.C4 = FALSE;
			Settings.SDD1 = FALSE;
		} else if (Settings.ForceSA1 || (!Settings.ForceNoSA1 && (ROMSpeed & ~0x10) == 0x23 &&
						 (ROMType & 0xf) > 3 && (ROMType & 0xf0) == 0x30)) {
			Settings.SA1 = TRUE;
			Settings.MultiPlayer5Master = FALSE;
			// Settings.MouseMaster = FALSE;
			// Settings.SuperScopeMaster = FALSE;
			Settings.DSP1Master = FALSE;
			Settings.C4 = FALSE;
			Settings.SDD1 = FALSE;
			SA1ROMMap();
		} else

		    if ((ROMSpeed & ~0x10) == 0x25)
			TalesROMMap(Interleaved);
		else if (strncmp((char *)&Memory.ROM[0x7fc0], "SOUND NOVEL-TCOOL", 17) == 0 ||
			 strncmp((char *)&Memory.ROM[0x7fc0], "DERBY STALLION 96", 17) == 0) {
			LoROM24MBSMap();
			Settings.DSP1Master = FALSE;
		} else if (strncmp((char *)&Memory.ROM[0x7fc0], "THOROUGHBRED BREEDER3", 21) == 0 ||
			   strncmp((char *)&Memory.ROM[0x7fc0], "RPG-TCOOL 2", 11) == 0) {
			SRAM512KLoROMMap();
			Settings.DSP1Master = FALSE;
		} else if (strncmp((char *)&Memory.ROM[0x7fc0], "DEZAEMON  ", 10) == 0) {
			Settings.DSP1Master = FALSE;
			SRAM1024KLoROMMap();
		} else if (strncmp((char *)&Memory.ROM[0x7fc0], "ADD-ON BASE CASSETE", 19) == 0) {
			Settings.MultiPlayer5Master = FALSE;
			Settings.MouseMaster = FALSE;
			Settings.SuperScopeMaster = FALSE;
			Settings.DSP1Master = FALSE;
			SufamiTurboLoROMMap();
			Memory.SRAMSize = 3;
		} else if ((ROMSpeed & ~0x10) == 0x22 && strncmp(ROMName, "Super Street Fighter", 20) != 0) {
			AlphaROMMap();
		} else
			LoROMMap();
	}

#ifdef SNESADVANCE_SPEEDHACKS
	ROMCRC32 = caCRC32(ROM, CalculatedSize, 0xFFFFFFFF);
#endif

	// SiENcE - detect ROM's with Highres
	//---------------------------------------------------------
	if (Settings.SupportHiRes == false) {
		// Secret of MANA
		// SeikenDensetsu 2
		// SD3/SOM2 (DEUTSCH)
		if ((strncmp(ROMName, "Secret of MANA", 14) == 0) ||
		    (strncmp(ROMName, "Vimm's Lair: vimm.net", 21) == 0) ||
		    (strncmp(ROMName, "SoM VMF by FuSoYa", 17) == 0) || (strncmp(ROMName, "Secret of Mana", 14) == 0) ||
		    (strncmp(ROMName, "SECRET OF MANA", 14) == 0) || (strncmp(ROMName, "SeikenDensetsu 2", 16) == 0) ||
		    (strncmp(ROMName, "SD3/SOM2 (DEUTSCH)", 18) == 0)) {
			Settings.SupportHiRes = true;
			printf("SupportHiRes = %s\n", "TRUE");
		}
		// SeikenDensetsu3
		// I Love Golden-3
		// M S YXM  S-S-3
		// NEVIN GINGER'S
		else if ((strncmp(ROMName, "SeikenDensetsu3", 15) == 0) ||
			 (strncmp(ROMName, "I Love Golden-3", 15) == 0) ||
			 (strncmp(ROMName, "M S YXM  S-S-3", 14) == 0) ||
			 (strncmp(ROMName, "NEVIN GINGER'S", 14) == 0)) {
			Settings.SupportHiRes = true;
			printf("SupportHiRes = %s\n", "TRUE");
		}
		// RUDORA NO HIHOU
		// RUDRA FR RC1 PAR TT
		// RUDRA AGTP V2.0
		// TREASURE OF THE RUDRAS
		// 1730 RUDORA NO HIHOU
		// RUDORA1995.06.26
		else if ((strncmp(ROMName, "RUDORA NO HIHOU", 15) == 0) ||
			 (strncmp(ROMName, "RUDRA FR RC1 PAR TT", 19) == 0) ||
			 (strncmp(ROMName, "RUDRA AGTP V2.0", 15) == 0) ||
			 (strncmp(ROMName, "TREASURE OF THE RUDRAS", 22) == 0) ||
			 (strncmp(ROMName, "1730 RUDORA NO HIHOU", 20) == 0) ||
			 (strncmp(ROMName, "RUDORA1995.06.26", 16) == 0)) {
			Settings.SupportHiRes = true;
			printf("SupportHiRes = %s\n", "TRUE");
		} else {
			printf("SupportHiRes = %s\n", "FALSE");
		}
	} else {
		printf("SupportHiRes = %s\n", "TRUE");
	}
	//---------------------------------------------------------

	int power2 = 0;
	int size = CalculatedSize;

	while (size >>= 1)
		power2++;

	size = 1 << power2;
	uint32 remainder = CalculatedSize - size;

	uint32 sum1 = 0;
	uint32 sum2 = 0;

	int i;

	for (i = 0; i < size; i++)
		sum1 += ROM[i];

	for (i = 0; i < (int)remainder; i++)
		sum2 += ROM[size + i];

	if (remainder) {
		// for Tengai makyou
		if (CalculatedSize == 0x500000 && Memory.HiROM && strncmp((const char *)&ROM[0xffb0], "18AZ", 4) == 0 &&
		    !memcmp(&ROM[0xffd5], "\x3a\xf9\x0d\x03\x00\x33\x00", 7))
			sum1 += sum2;
		else
			sum1 += sum2 * (size / remainder);
	}

	sum1 &= 0xffff;

	if (Settings.ForceNTSC)
		Settings.PAL = FALSE;
	else if (Settings.ForcePAL)
		Settings.PAL = TRUE;
	else if (Memory.HiROM)
		// Country code
		Settings.PAL = ROM[0xffd9] >= 2;
	else
		Settings.PAL = ROM[0x7fd9] >= 2;

	if (Settings.PAL) {
		Settings.FrameTime = Settings.FrameTimePAL;
		Memory.ROMFramesPerSecond = 50;
	} else {
		Settings.FrameTime = Settings.FrameTimeNTSC;
		Memory.ROMFramesPerSecond = 60;
	}

	ROMName[ROM_NAME_LEN - 1] = 0;
	if (strlen(ROMName)) {
		char *p = ROMName + strlen(ROMName) - 1;

		while (p > ROMName && *(p - 1) == ' ')
			p--;
		*p = 0;
	}

	// if (Settings.SuperFX)
	// {
	// 	SRAMMask = 0xffff;
	// 	Memory.SRAMSize = 16;
	// } else
	{
		SRAMMask = Memory.SRAMSize ? ((1 << (Memory.SRAMSize + 3)) * 128) - 1 : 0;
	}

	IAPU.OneCycle = ONE_APU_CYCLE;
	Settings.Shutdown = Settings.ShutdownMaster;

	ApplyROMFixes();
	sprintf(ROMName, "%s", Safe(ROMName));
	sprintf(ROMId, "%s", Safe(ROMId));
	sprintf(CompanyId, "%s", Safe(CompanyId));

	sprintf(String,
		"\"%s\" [%s] %s, %s, Type: %s, Mode: %s, TV: %s, S-RAM: %s, "
		"ROMId: %s Company: %2.2s\n",
		ROMName,
		(ROMChecksum + ROMComplementChecksum != 0xffff || ROMChecksum != sum1) ? "bad checksum" : "checksum ok",
		MapType(), Size(), KartContents(), MapMode(), TVStandard(), StaticRAMSize(), ROMId, CompanyId);

	S9xMessage(S9X_INFO, S9X_ROM_INFO, String);
}

bool8_32 CMemory::LoadSRAM(const char *filename)
{
	int size = Memory.SRAMSize ? (1 << (Memory.SRAMSize + 3)) * 128 : 0;

	memset(SRAM, SNESGameFixes.SRAMInitialValue, 0x20000);

	if (size > 0x20000)
		size = 0x20000;

	if (size) {
		FILE *file;
		if ((file = fopen(filename, "rb"))) {
			int len = fread((char *)::SRAM, 1, 0x20000, file);
			fclose(file);
			if (len - size == 512) {
				// S-RAM file has a header - remove it
				memmove(::SRAM, ::SRAM + 512, size);
			}
#ifdef S_RTC
			if (len == size + SRTC_SRAM_PAD) {
				S9xSRTCPostLoadState();
				S9xResetSRTC();
				rtc.index = -1;
				rtc.mode = MODE_READ;
			} else
				S9xHardResetSRTC();
#endif
			return (TRUE);
		}
#ifdef S_RTC
		S9xHardResetSRTC();
#endif
		return (FALSE);
	}
	return (TRUE);
}

bool8_32 CMemory::SaveSRAM(const char *filename)
{
	int size = Memory.SRAMSize ? (1 << (Memory.SRAMSize + 3)) * 128 : 0;
#ifdef S_RTC
	if (Settings.SRTC) {
		size += SRTC_SRAM_PAD;
		S9xSRTCPreSaveState();
	}
#endif
	if (size > 0x20000)
		size = 0x20000;

	if (size && *Memory.ROMFilename) {
		FILE *file;
		if ((file = fopen(filename, "wb"))) {
			fwrite((char *)::SRAM, size, 1, file);
			fclose(file);
#if defined(__linux)
			chown(filename, getuid(), getgid());
#endif
			return (TRUE);
		}
	}
	return (FALSE);
}

void CMemory::FixROMSpeed()
{
	int c;

	for (c = 0x800; c < 0x1000; c++) {
		if (BlockIsROM[c])
			MemorySpeed[c] = (uint8)CPU.FastROMSpeed;
	}
}

void CMemory::WriteProtectROM()
{
	memmove((void *)WriteMap, (void *)Map, sizeof(Map));
	for (int c = 0; c < 0x1000; c++) {
		if (BlockIsROM[c])
			WriteMap[c] = (uint8 *)MAP_NONE;
	}
}

void CMemory::MapRAM()
{
	int c;

	// Banks 7e->7f, RAM
	for (c = 0; c < 16; c++) {
		Map[c + 0x7e0] = RAM;
		Map[c + 0x7f0] = RAM + 0x10000;
		BlockIsRAM[c + 0x7e0] = TRUE;
		BlockIsRAM[c + 0x7f0] = TRUE;
		BlockIsROM[c + 0x7e0] = FALSE;
		BlockIsROM[c + 0x7f0] = FALSE;
	}

	// Banks 70->77, S-RAM
	for (c = 0; c < 0x80; c++) {
		Map[c + 0x700] = (uint8 *)MAP_LOROM_SRAM;
		BlockIsRAM[c + 0x700] = TRUE;
		BlockIsROM[c + 0x700] = FALSE;
	}
}

void CMemory::MapExtraRAM()
{
	int c;

	// Banks 7e->7f, RAM
	for (c = 0; c < 16; c++) {
		Map[c + 0x7e0] = RAM;
		Map[c + 0x7f0] = RAM + 0x10000;
		BlockIsRAM[c + 0x7e0] = TRUE;
		BlockIsRAM[c + 0x7f0] = TRUE;
		BlockIsROM[c + 0x7e0] = FALSE;
		BlockIsROM[c + 0x7f0] = FALSE;
	}

	// Banks 70->73, S-RAM
	for (c = 0; c < 16; c++) {
		Map[c + 0x700] = ::SRAM;
		Map[c + 0x710] = ::SRAM + 0x8000;
		Map[c + 0x720] = ::SRAM + 0x10000;
		Map[c + 0x730] = ::SRAM + 0x18000;

		BlockIsRAM[c + 0x700] = TRUE;
		BlockIsROM[c + 0x700] = FALSE;
		BlockIsRAM[c + 0x710] = TRUE;
		BlockIsROM[c + 0x710] = FALSE;
		BlockIsRAM[c + 0x720] = TRUE;
		BlockIsROM[c + 0x720] = FALSE;
		BlockIsRAM[c + 0x730] = TRUE;
		BlockIsROM[c + 0x730] = FALSE;
	}
}

void CMemory::LoROMMap()
{
	int c;
	int i;

	// Banks 00->3f and 80->bf
	for (c = 0; c < 0x400; c += 16) {
		Map[c + 0] = Map[c + 0x800] = RAM;
		Map[c + 1] = Map[c + 0x801] = RAM;
		BlockIsRAM[c + 0] = BlockIsRAM[c + 0x800] = TRUE;
		BlockIsRAM[c + 1] = BlockIsRAM[c + 0x801] = TRUE;

		Map[c + 2] = Map[c + 0x802] = (uint8 *)MAP_PPU;
#ifdef SETA_DSP
		if (Settings.SETA == ST_018)
			Map[c + 3] = Map[c + 0x803] = (uint8 *)MAP_SETA_RISC;
		else
#endif
			Map[c + 3] = Map[c + 0x803] = (uint8 *)MAP_PPU;
		Map[c + 4] = Map[c + 0x804] = (uint8 *)MAP_CPU;
		Map[c + 5] = Map[c + 0x805] = (uint8 *)MAP_CPU;
		if (Settings.DSP1Master) {
			Map[c + 6] = Map[c + 0x806] = (uint8 *)MAP_DSP;
			Map[c + 7] = Map[c + 0x807] = (uint8 *)MAP_DSP;
		} else if (Settings.C4) {
			Map[c + 6] = Map[c + 0x806] = (uint8 *)MAP_C4;
			Map[c + 7] = Map[c + 0x807] = (uint8 *)MAP_C4;
		} else if (Settings.OBC1) {
			Map[c + 6] = Map[c + 0x806] = (uint8 *)MAP_OBC_RAM;
			Map[c + 7] = Map[c + 0x807] = (uint8 *)MAP_OBC_RAM;
		} else {
			Map[c + 6] = Map[c + 0x806] = (uint8 *)bytes0x2000 - 0x6000;
			Map[c + 7] = Map[c + 0x807] = (uint8 *)bytes0x2000 - 0x6000;
		}

		for (i = c + 8; i < c + 16; i++) {
			Map[i] = Map[i + 0x800] = &ROM[(c << 11) % CalculatedSize] - 0x8000;
			BlockIsROM[i] = BlockIsROM[i + 0x800] = TRUE;
		}

		for (i = c; i < c + 16; i++) {
			int ppu = i & 15;

			MemorySpeed[i] = MemorySpeed[i + 0x800] = ppu >= 2 && ppu <= 3 ? ONE_CYCLE : SLOW_ONE_CYCLE;
		}
	}

	if (Settings.DSP1Master) {
		// Banks 30->3f and b0->bf
		for (c = 0x300; c < 0x400; c += 16) {
			for (i = c + 8; i < c + 16; i++) {
				Map[i] = Map[i + 0x800] = (uint8 *)MAP_DSP;
				BlockIsROM[i] = BlockIsROM[i + 0x800] = FALSE;
			}
		}
	}

	// Banks 40->7f and c0->ff
	for (c = 0; c < 0x400; c += 16) {
		for (i = c; i < c + 8; i++)
			Map[i + 0x400] = Map[i + 0xc00] = &ROM[(c << 11) % CalculatedSize];

		for (i = c + 8; i < c + 16; i++)
			Map[i + 0x400] = Map[i + 0xc00] = &ROM[((c << 11) + 0x200000) % CalculatedSize - 0x8000];

		for (i = c; i < c + 16; i++) {
			MemorySpeed[i + 0x400] = MemorySpeed[i + 0xc00] = SLOW_ONE_CYCLE;
			BlockIsROM[i + 0x400] = BlockIsROM[i + 0xc00] = TRUE;
		}
	}

	if (Settings.DSP1Master) {
		for (c = 0; c < 0x100; c++) {
			Map[c + 0xe00] = (uint8 *)MAP_DSP;
			MemorySpeed[c + 0xe00] = SLOW_ONE_CYCLE;
			BlockIsROM[c + 0xe00] = FALSE;
		}
	}
	MapRAM();
	WriteProtectROM();
}

#ifdef SETA_DSP
void CMemory::SetaDSPMap()
{
	int c;
	int i;
	int j;
	int mask[4];
	for (j = 0; j < 4; j++)
		mask[j] = 0x00ff;

	mask[0] = (CalculatedSize / 0x8000) - 1;

	int x;
	bool foundZeros;
	bool pastZeros;

	for (j = 0; j < 3; j++) {
		x = 1;
		foundZeros = false;
		pastZeros = false;

		mask[j + 1] = mask[j];

		while (x > 0x100 && !pastZeros) {
			if (mask[j] & x) {
				x <<= 1;
				if (foundZeros)
					pastZeros = true;
			} else {
				foundZeros = true;
				pastZeros = false;
				mask[j + 1] |= x;
				x <<= 1;
			}
		}
	}

	// Banks 00->3f and 80->bf
	for (c = 0; c < 0x400; c += 16) {
		Map[c + 0] = Map[c + 0x800] = RAM;
		Map[c + 1] = Map[c + 0x801] = RAM;
		BlockIsRAM[c + 0] = BlockIsRAM[c + 0x800] = TRUE;
		BlockIsRAM[c + 1] = BlockIsRAM[c + 0x801] = TRUE;

		Map[c + 2] = Map[c + 0x802] = (uint8 *)MAP_PPU;
		Map[c + 3] = Map[c + 0x803] = (uint8 *)MAP_PPU;
		Map[c + 4] = Map[c + 0x804] = (uint8 *)MAP_CPU;
		Map[c + 5] = Map[c + 0x805] = (uint8 *)MAP_CPU;
		Map[c + 6] = Map[c + 0x806] = (uint8 *)bytes0x2000 - 0x6000;
		Map[c + 7] = Map[c + 0x807] = (uint8 *)bytes0x2000 - 0x6000;

		for (i = c + 8; i < c + 16; i++) {
			int e = 3;
			int d = c >> 4;
			while (d > mask[0]) {
				d &= mask[e];
				e--;
			}
			Map[i] = Map[i + 0x800] = ROM + (((d)-1) * 0x8000);
			BlockIsROM[i] = BlockIsROM[i + 0x800] = TRUE;
		}
	}

	// Banks 40->7f and c0->ff
	for (c = 0; c < 0x400; c += 16) {
		for (i = c + 8; i < c + 16; i++) {
			int e = 3;
			int d = (c + 0x400) >> 4;
			while (d > mask[0]) {
				d &= mask[e];
				e--;
			}

			Map[i + 0x400] = Map[i + 0xc00] = ROM + (((d)-1) * 0x8000);
		}

		// only upper half is ROM
		for (i = c + 8; i < c + 16; i++) {
			BlockIsROM[i + 0x400] = BlockIsROM[i + 0xc00] = TRUE;
		}
	}

	memset(SRAM, 0, 0x1000);
	for (c = 0x600; c < 0x680; c += 0x10) {
		for (i = 0; i < 0x08; i++) {
			// where does the SETA chip access, anyway?
			// please confirm this?
			Map[c + 0x80 + i] = (uint8 *)MAP_SETA_DSP;
			BlockIsROM[c + 0x80 + i] = FALSE;
			BlockIsRAM[c + 0x80 + i] = TRUE;
		}

		for (i = 0; i < 0x04; i++) {
			// and this!
			Map[c + i] = (uint8 *)MAP_SETA_DSP;
			BlockIsROM[c + i] = FALSE;
		}
	}

	int sum = 0, k, l, bankcount;
	bankcount = 1 << (ROMSize - 7); // Mbits
	// safety for corrupt headers
	if (bankcount > 128)
		bankcount = (CalculatedSize / 0x8000) / 4;
	bankcount *= 4;     // to banks
	bankcount <<= 4;    // Map banks
	bankcount += 0x800; // normalize
	for (k = 0x800; k < (bankcount); k += 16) {
		uint8 *bank = 0x8000 + Map[k + 8];
		for (l = 0; l < 0x8000; l++)
			sum += bank[l];
	}
	CalculatedChecksum = sum & 0xFFFF;

	MapRAM();
	WriteProtectROM();
}
#endif

void CMemory::HiROMMap()
{
	int c;
	int i;

	// Banks 00->3f and 80->bf
	for (c = 0; c < 0x400; c += 16) {
		Map[c + 0] = Map[c + 0x800] = RAM;
		BlockIsRAM[c + 0] = BlockIsRAM[c + 0x800] = TRUE;
		Map[c + 1] = Map[c + 0x801] = RAM;
		BlockIsRAM[c + 1] = BlockIsRAM[c + 0x801] = TRUE;

		Map[c + 2] = Map[c + 0x802] = (uint8 *)MAP_PPU;
		Map[c + 3] = Map[c + 0x803] = (uint8 *)MAP_PPU;
		Map[c + 4] = Map[c + 0x804] = (uint8 *)MAP_CPU;
		Map[c + 5] = Map[c + 0x805] = (uint8 *)MAP_CPU;
		if (Settings.DSP1Master) {
			Map[c + 6] = Map[c + 0x806] = (uint8 *)MAP_DSP;
			Map[c + 7] = Map[c + 0x807] = (uint8 *)MAP_DSP;
		} else {
			Map[c + 6] = Map[c + 0x806] = (uint8 *)MAP_NONE;
			Map[c + 7] = Map[c + 0x807] = (uint8 *)MAP_NONE;
		}

		for (i = c + 8; i < c + 16; i++) {
			Map[i] = Map[i + 0x800] = &ROM[(c << 12) % CalculatedSize];
			BlockIsROM[i] = BlockIsROM[i + 0x800] = TRUE;
		}

		for (i = c; i < c + 16; i++) {
			int ppu = i & 15;

			MemorySpeed[i] = MemorySpeed[i + 0x800] = ppu >= 2 && ppu <= 3 ? ONE_CYCLE : SLOW_ONE_CYCLE;
		}
	}

	// Banks 30->3f and b0->bf, address ranges 6000->7fff is S-RAM.
	for (c = 0; c < 16; c++) {
		Map[0x306 + (c << 4)] = (uint8 *)MAP_HIROM_SRAM;
		Map[0x307 + (c << 4)] = (uint8 *)MAP_HIROM_SRAM;
		Map[0xb06 + (c << 4)] = (uint8 *)MAP_HIROM_SRAM;
		Map[0xb07 + (c << 4)] = (uint8 *)MAP_HIROM_SRAM;
		BlockIsRAM[0x306 + (c << 4)] = TRUE;
		BlockIsRAM[0x307 + (c << 4)] = TRUE;
		BlockIsRAM[0xb06 + (c << 4)] = TRUE;
		BlockIsRAM[0xb07 + (c << 4)] = TRUE;
	}

	// Banks 40->7f and c0->ff
	for (c = 0; c < 0x400; c += 16) {
		for (i = c; i < c + 16; i++) {
			Map[i + 0x400] = Map[i + 0xc00] = &ROM[(c << 12) % CalculatedSize];
			MemorySpeed[i + 0x400] = MemorySpeed[i + 0xc00] = SLOW_ONE_CYCLE;
			BlockIsROM[i + 0x400] = BlockIsROM[i + 0xc00] = TRUE;
		}
	}

	MapRAM();
	WriteProtectROM();
}

void CMemory::TalesROMMap(bool8_32 Interleaved)
{
	int c;
	int i;

	uint32 OFFSET0 = 0x400000;
	uint32 OFFSET1 = 0x400000;
	uint32 OFFSET2 = 0x000000;

	if (Interleaved) {
		OFFSET0 = 0x000000;
		OFFSET1 = 0x000000;
		OFFSET2 = 0x200000;
	}

	// Banks 00->3f and 80->bf
	for (c = 0; c < 0x400; c += 16) {
		Map[c + 0] = Map[c + 0x800] = RAM;
		Map[c + 1] = Map[c + 0x801] = RAM;
		BlockIsRAM[c + 0] = BlockIsRAM[c + 0x800] = TRUE;
		BlockIsRAM[c + 1] = BlockIsRAM[c + 0x801] = TRUE;

		Map[c + 2] = Map[c + 0x802] = (uint8 *)MAP_PPU;
		Map[c + 3] = Map[c + 0x803] = (uint8 *)MAP_PPU;
		Map[c + 4] = Map[c + 0x804] = (uint8 *)MAP_CPU;
		Map[c + 5] = Map[c + 0x805] = (uint8 *)MAP_CPU;
		Map[c + 6] = Map[c + 0x806] = (uint8 *)MAP_NONE;
		Map[c + 7] = Map[c + 0x807] = (uint8 *)MAP_NONE;
		for (i = c + 8; i < c + 16; i++) {
			Map[i] = &ROM[((c << 12) + OFFSET0) % CalculatedSize];
			Map[i + 0x800] = &ROM[((c << 12) + OFFSET0) % CalculatedSize];
			BlockIsROM[i] = TRUE;
			BlockIsROM[i + 0x800] = TRUE;
		}

		for (i = c; i < c + 16; i++) {
			int ppu = i & 15;

			MemorySpeed[i] = MemorySpeed[i + 0x800] = ppu >= 2 && ppu <= 3 ? ONE_CYCLE : SLOW_ONE_CYCLE;
		}
	}

	// Banks 30->3f and b0->bf, address ranges 6000->7ffff is S-RAM.
	for (c = 0; c < 16; c++) {
		Map[0x306 + (c << 4)] = (uint8 *)MAP_HIROM_SRAM;
		Map[0x307 + (c << 4)] = (uint8 *)MAP_HIROM_SRAM;
		Map[0xb06 + (c << 4)] = (uint8 *)MAP_HIROM_SRAM;
		Map[0xb07 + (c << 4)] = (uint8 *)MAP_HIROM_SRAM;
		BlockIsRAM[0x306 + (c << 4)] = TRUE;
		BlockIsRAM[0x307 + (c << 4)] = TRUE;
		BlockIsRAM[0xb06 + (c << 4)] = TRUE;
		BlockIsRAM[0xb07 + (c << 4)] = TRUE;
	}

	// Banks 40->7f and c0->ff
	for (c = 0; c < 0x400; c += 16) {
		for (i = c; i < c + 8; i++) {
			Map[i + 0x400] = &ROM[((c << 12) + OFFSET1) % CalculatedSize];
			Map[i + 0x408] = &ROM[((c << 12) + OFFSET1) % CalculatedSize];
			Map[i + 0xc00] = &ROM[((c << 12) + OFFSET2) % CalculatedSize];
			Map[i + 0xc08] = &ROM[((c << 12) + OFFSET2) % CalculatedSize];
			BlockIsROM[i + 0x400] = TRUE;
			BlockIsROM[i + 0x408] = TRUE;
			BlockIsROM[i + 0xc00] = TRUE;
			BlockIsROM[i + 0xc08] = TRUE;
			MemorySpeed[i + 0x400] = MemorySpeed[i + 0xc00] = SLOW_ONE_CYCLE;
			MemorySpeed[i + 0x408] = MemorySpeed[i + 0xc08] = SLOW_ONE_CYCLE;
		}
	}
	MapRAM();
	WriteProtectROM();
}

void CMemory::AlphaROMMap()
{
	int c;
	int i;

	// Banks 00->3f and 80->bf
	for (c = 0; c < 0x400; c += 16) {
		Map[c + 0] = Map[c + 0x800] = RAM;
		Map[c + 1] = Map[c + 0x801] = RAM;
		BlockIsRAM[c + 0] = BlockIsRAM[c + 0x800] = TRUE;
		BlockIsRAM[c + 1] = BlockIsRAM[c + 0x801] = TRUE;

		Map[c + 2] = Map[c + 0x802] = (uint8 *)MAP_PPU;
		Map[c + 3] = Map[c + 0x803] = (uint8 *)MAP_PPU;
		Map[c + 4] = Map[c + 0x804] = (uint8 *)MAP_CPU;
		Map[c + 5] = Map[c + 0x805] = (uint8 *)MAP_CPU;
		Map[c + 6] = Map[c + 0x806] = (uint8 *)MAP_DSP;
		Map[c + 7] = Map[c + 0x807] = (uint8 *)MAP_DSP;

		for (i = c + 8; i < c + 16; i++) {
			Map[i] = Map[i + 0x800] = &ROM[c << 11] - 0x8000;
			BlockIsROM[i] = TRUE;
		}

		for (i = c; i < c + 16; i++) {
			int ppu = i & 15;

			MemorySpeed[i] = MemorySpeed[i + 0x800] = ppu >= 2 && ppu <= 3 ? ONE_CYCLE : SLOW_ONE_CYCLE;
		}
	}

	// Banks 40->7f and c0->ff

	for (c = 0; c < 0x400; c += 16) {
		for (i = c; i < c + 16; i++) {
			Map[i + 0x400] = &ROM[(c << 12) % CalculatedSize];
			Map[i + 0xc00] = &ROM[(c << 12) % CalculatedSize];
			MemorySpeed[i + 0x400] = MemorySpeed[i + 0xc00] = SLOW_ONE_CYCLE;
			BlockIsROM[i + 0x400] = BlockIsROM[i + 0xc00] = TRUE;
		}
	}

	MapRAM();
	WriteProtectROM();
}

void DetectSuperFxRamSize()
{
	if (ROM[0x7FDA] == 0x33) {
		Memory.SRAMSize = ROM[0x7FBD];
	} else {
		if (strncmp(Memory.ROMName, "STAR FOX 2", 10) == 0) {
			Memory.SRAMSize = 6;
		} else
			Memory.SRAMSize = 5;
	}
}

void CMemory::SuperFXROMMap()
{
	int c;
	int i;

	DetectSuperFxRamSize();

	// Banks 00->3f and 80->bf
	for (c = 0; c < 0x400; c += 16) {
		Map[c + 0] = Map[c + 0x800] = RAM;
		Map[c + 1] = Map[c + 0x801] = RAM;
		BlockIsRAM[c + 0] = BlockIsRAM[c + 0x800] = TRUE;
		BlockIsRAM[c + 1] = BlockIsRAM[c + 0x801] = TRUE;

		Map[c + 2] = Map[c + 0x802] = (uint8 *)MAP_PPU;
		Map[c + 3] = Map[c + 0x803] = (uint8 *)MAP_PPU;
		Map[c + 4] = Map[c + 0x804] = (uint8 *)MAP_CPU;
		Map[c + 5] = Map[c + 0x805] = (uint8 *)MAP_CPU;
		Map[0x006 + c] = Map[0x806 + c] = (uint8 *)::SRAM - 0x6000;
		Map[0x007 + c] = Map[0x807 + c] = (uint8 *)::SRAM - 0x6000;
		BlockIsRAM[0x006 + c] = BlockIsRAM[0x007 + c] = BlockIsRAM[0x806 + c] = BlockIsRAM[0x807 + c] = TRUE;

		for (i = c + 8; i < c + 16; i++) {
			Map[i] = Map[i + 0x800] = &ROM[c << 11] - 0x8000;
			BlockIsROM[i] = BlockIsROM[i + 0x800] = TRUE;
		}
	}

	// Banks 40->7f and c0->ff
	for (c = 0; c < 0x400; c += 16) {
		for (i = c; i < c + 16; i++) {
			Map[i + 0x400] = Map[i + 0xc00] = &ROM[(c << 12) % CalculatedSize];

			BlockIsROM[i + 0x400] = BlockIsROM[i + 0xc00] = TRUE;
		}
	}

	// Banks 7e->7f, RAM
	for (c = 0; c < 16; c++) {
		Map[c + 0x7e0] = RAM;
		Map[c + 0x7f0] = RAM + 0x10000;
		BlockIsRAM[c + 0x7e0] = TRUE;
		BlockIsRAM[c + 0x7f0] = TRUE;
		BlockIsROM[c + 0x7e0] = FALSE;
		BlockIsROM[c + 0x7f0] = FALSE;
	}

	// Banks 70->71, S-RAM
	for (c = 0; c < 32; c++) {
		Map[c + 0x700] = ::SRAM + (((c >> 4) & 1) << 16);
		BlockIsRAM[c + 0x700] = TRUE;
		BlockIsROM[c + 0x700] = FALSE;
	}

	// Banks 00->3f and 80->bf address ranges 6000->7fff is RAM.
	// for (c = 0; c < 0x40; c++) {
	// 	Map[0x006 + (c << 4)] = (uint8 *)::SRAM - 0x6000;
	// 	Map[0x007 + (c << 4)] = (uint8 *)::SRAM - 0x6000;
	// 	Map[0x806 + (c << 4)] = (uint8 *)::SRAM - 0x6000;
	// 	Map[0x807 + (c << 4)] = (uint8 *)::SRAM - 0x6000;
	// 	BlockIsRAM[0x006 + (c << 4)] = TRUE;
	// 	BlockIsRAM[0x007 + (c << 4)] = TRUE;
	// 	BlockIsRAM[0x806 + (c << 4)] = TRUE;
	// 	BlockIsRAM[0x807 + (c << 4)] = TRUE;
	// }
	// Replicate the first 2Mb of the ROM at ROM + 2MB such that each 32K
	// block is repeated twice in each 64K block.
	for (c = 0; c < 64; c++) {
		memmove(&ROM[0x200000 + c * 0x10000], &ROM[c * 0x8000], 0x8000);
		memmove(&ROM[0x208000 + c * 0x10000], &ROM[c * 0x8000], 0x8000);
	}

	WriteProtectROM();
}

void CMemory::SA1ROMMap()
{
	int c;
	int i;

	// Banks 00->3f and 80->bf
	for (c = 0; c < 0x400; c += 16) {
		Map[c + 0] = Map[c + 0x800] = RAM;
		Map[c + 1] = Map[c + 0x801] = RAM;
		BlockIsRAM[c + 0] = BlockIsRAM[c + 0x800] = TRUE;
		BlockIsRAM[c + 1] = BlockIsRAM[c + 0x801] = TRUE;

		Map[c + 2] = Map[c + 0x802] = (uint8 *)MAP_PPU;
		Map[c + 3] = Map[c + 0x803] = (uint8 *)&Memory.FillRAM[0x3000] - 0x3000;
		Map[c + 4] = Map[c + 0x804] = (uint8 *)MAP_CPU;
		Map[c + 5] = Map[c + 0x805] = (uint8 *)MAP_CPU;
		Map[c + 6] = Map[c + 0x806] = (uint8 *)MAP_BWRAM;
		Map[c + 7] = Map[c + 0x807] = (uint8 *)MAP_BWRAM;
		for (i = c + 8; i < c + 16; i++) {
			Map[i] = Map[i + 0x800] = &ROM[c << 11] - 0x8000;
			BlockIsROM[i] = BlockIsROM[i + 0x800] = TRUE;
		}

		for (i = c; i < c + 16; i++) {
			int ppu = i & 15;

			MemorySpeed[i] = MemorySpeed[i + 0x800] = ppu >= 2 && ppu <= 3 ? ONE_CYCLE : SLOW_ONE_CYCLE;
		}
	}

	// Banks 40->7f
	for (c = 0; c < 0x400; c += 16) {
		for (i = c; i < c + 16; i++)
			Map[i + 0x400] = (uint8 *)&SRAM[(c << 12) & 0x1ffff];

		for (i = c; i < c + 16; i++) {
			MemorySpeed[i + 0x400] = SLOW_ONE_CYCLE;
			BlockIsROM[i + 0x400] = FALSE;
		}
	}

	// c0->ff
	for (c = 0; c < 0x400; c += 16) {
		for (i = c; i < c + 16; i++) {
			Map[i + 0xc00] = &ROM[(c << 12) % CalculatedSize];
			MemorySpeed[i + 0xc00] = SLOW_ONE_CYCLE;
			BlockIsROM[i + 0xc00] = TRUE;
		}
	}

	for (c = 0; c < 16; c++) {
		Map[c + 0x7e0] = RAM;
		Map[c + 0x7f0] = RAM + 0x10000;
		BlockIsRAM[c + 0x7e0] = TRUE;
		BlockIsRAM[c + 0x7f0] = TRUE;
		BlockIsROM[c + 0x7e0] = FALSE;
		BlockIsROM[c + 0x7f0] = FALSE;
	}
	WriteProtectROM();

	// Now copy the map and correct it for the SA1 CPU.
	memmove((void *)SA1.WriteMap, (void *)WriteMap, sizeof(WriteMap));
	memmove((void *)SA1.Map, (void *)Map, sizeof(Map));

	// Banks 00->3f and 80->bf
	for (c = 0; c < 0x400; c += 16) {
		SA1.Map[c + 0] = SA1.Map[c + 0x800] = &Memory.FillRAM[0x3000];
		SA1.Map[c + 1] = SA1.Map[c + 0x801] = (uint8 *)MAP_NONE;
		SA1.WriteMap[c + 0] = SA1.WriteMap[c + 0x800] = &Memory.FillRAM[0x3000];
		SA1.WriteMap[c + 1] = SA1.WriteMap[c + 0x801] = (uint8 *)MAP_NONE;
	}

	// Banks 60->6f
	for (c = 0; c < 0x100; c++)
		SA1.Map[c + 0x600] = SA1.WriteMap[c + 0x600] = (uint8 *)MAP_BWRAM_BITMAP;

	BWRAM = SRAM;
}

void CMemory::LoROM24MBSMap()
{
	int c;
	int i;

	// Banks 00->3f and 80->bf
	for (c = 0; c < 0x400; c += 16) {
		Map[c + 0] = Map[c + 0x800] = RAM;
		Map[c + 1] = Map[c + 0x801] = RAM;
		BlockIsRAM[c + 0] = BlockIsRAM[c + 0x800] = TRUE;
		BlockIsRAM[c + 1] = BlockIsRAM[c + 0x801] = TRUE;

		Map[c + 2] = Map[c + 0x802] = (uint8 *)MAP_PPU;
		Map[c + 3] = Map[c + 0x803] = (uint8 *)MAP_PPU;
		Map[c + 4] = Map[c + 0x804] = (uint8 *)MAP_CPU;
		Map[c + 5] = Map[c + 0x805] = (uint8 *)MAP_CPU;
		Map[c + 6] = Map[c + 0x806] = (uint8 *)MAP_NONE;
		Map[c + 7] = Map[c + 0x807] = (uint8 *)MAP_NONE;

		for (i = c + 8; i < c + 16; i++) {
			Map[i] = Map[i + 0x800] = &ROM[c << 11] - 0x8000;
			BlockIsROM[i] = BlockIsROM[i + 0x800] = TRUE;
		}

		for (i = c; i < c + 16; i++) {
			int ppu = i & 15;

			MemorySpeed[i] = MemorySpeed[i + 0x800] = ppu >= 2 && ppu <= 3 ? ONE_CYCLE : SLOW_ONE_CYCLE;
		}
	}

	// Banks 00->3f and 80->bf
	for (c = 0; c < 0x200; c += 16) {
		Map[c + 0x800] = RAM;
		Map[c + 0x801] = RAM;
		BlockIsRAM[c + 0x800] = TRUE;
		BlockIsRAM[c + 0x801] = TRUE;

		Map[c + 0x802] = (uint8 *)MAP_PPU;
		Map[c + 0x803] = (uint8 *)MAP_PPU;
		Map[c + 0x804] = (uint8 *)MAP_CPU;
		Map[c + 0x805] = (uint8 *)MAP_CPU;
		Map[c + 0x806] = (uint8 *)MAP_NONE;
		Map[c + 0x807] = (uint8 *)MAP_NONE;

		for (i = c + 8; i < c + 16; i++) {
			Map[i + 0x800] = &ROM[c << 11] - 0x8000 + 0x200000;
			BlockIsROM[i + 0x800] = TRUE;
		}

		for (i = c; i < c + 16; i++) {
			int ppu = i & 15;

			MemorySpeed[i + 0x800] = ppu >= 2 && ppu <= 3 ? ONE_CYCLE : SLOW_ONE_CYCLE;
		}
	}

	// Banks 40->7f and c0->ff
	for (c = 0; c < 0x400; c += 16) {
		for (i = c; i < c + 8; i++)
			Map[i + 0x400] = Map[i + 0xc00] = &ROM[(c << 11) + 0x200000];

		for (i = c + 8; i < c + 16; i++)
			Map[i + 0x400] = Map[i + 0xc00] = &ROM[(c << 11) + 0x200000 - 0x8000];

		for (i = c; i < c + 16; i++) {
			MemorySpeed[i + 0x400] = MemorySpeed[i + 0xc00] = SLOW_ONE_CYCLE;
			BlockIsROM[i + 0x400] = BlockIsROM[i + 0xc00] = TRUE;
		}
	}

	MapExtraRAM();
	WriteProtectROM();
}

void CMemory::SufamiTurboLoROMMap()
{
	int c;
	int i;

	// Banks 00->3f and 80->bf
	for (c = 0; c < 0x400; c += 16) {
		Map[c + 0] = Map[c + 0x800] = RAM;
		Map[c + 1] = Map[c + 0x801] = RAM;
		BlockIsRAM[c + 0] = BlockIsRAM[c + 0x800] = TRUE;
		BlockIsRAM[c + 1] = BlockIsRAM[c + 0x801] = TRUE;

		Map[c + 2] = Map[c + 0x802] = (uint8 *)MAP_PPU;
		Map[c + 3] = Map[c + 0x803] = (uint8 *)MAP_PPU;
		Map[c + 4] = Map[c + 0x804] = (uint8 *)MAP_CPU;
		Map[c + 5] = Map[c + 0x805] = (uint8 *)MAP_CPU;
		if (Settings.DSP1Master) {
			Map[c + 6] = Map[c + 0x806] = (uint8 *)MAP_DSP;
			Map[c + 7] = Map[c + 0x807] = (uint8 *)MAP_DSP;
		} else {
			Map[c + 6] = Map[c + 0x806] = (uint8 *)MAP_NONE;
			Map[c + 7] = Map[c + 0x807] = (uint8 *)MAP_NONE;
		}
		for (i = c + 8; i < c + 16; i++) {
			Map[i] = Map[i + 0x800] = &ROM[c << 11] - 0x8000;
			BlockIsROM[i] = BlockIsROM[i + 0x800] = TRUE;
		}

		for (i = c; i < c + 16; i++) {
			int ppu = i & 15;

			MemorySpeed[i] = MemorySpeed[i + 0x800] = ppu >= 2 && ppu <= 3 ? ONE_CYCLE : SLOW_ONE_CYCLE;
		}
	}

	if (Settings.DSP1Master) {
		// Banks 30->3f and b0->bf
		for (c = 0x300; c < 0x400; c += 16) {
			for (i = c + 8; i < c + 16; i++) {
				Map[i] = Map[i + 0x800] = (uint8 *)MAP_DSP;
				BlockIsROM[i] = BlockIsROM[i + 0x800] = FALSE;
			}
		}
	}

	// Banks 40->7f and c0->ff
	for (c = 0; c < 0x400; c += 16) {
		for (i = c; i < c + 8; i++)
			Map[i + 0x400] = Map[i + 0xc00] = &ROM[(c << 11) + 0x200000];

		for (i = c + 8; i < c + 16; i++)
			Map[i + 0x400] = Map[i + 0xc00] = &ROM[(c << 11) + 0x200000 - 0x8000];

		for (i = c; i < c + 16; i++) {
			MemorySpeed[i + 0x400] = MemorySpeed[i + 0xc00] = SLOW_ONE_CYCLE;
			BlockIsROM[i + 0x400] = BlockIsROM[i + 0xc00] = TRUE;
		}
	}

	if (Settings.DSP1Master) {
		for (c = 0; c < 0x100; c++) {
			Map[c + 0xe00] = (uint8 *)MAP_DSP;
			MemorySpeed[c + 0xe00] = SLOW_ONE_CYCLE;
			BlockIsROM[c + 0xe00] = FALSE;
		}
	}

	// Banks 7e->7f, RAM
	for (c = 0; c < 16; c++) {
		Map[c + 0x7e0] = RAM;
		Map[c + 0x7f0] = RAM + 0x10000;
		BlockIsRAM[c + 0x7e0] = TRUE;
		BlockIsRAM[c + 0x7f0] = TRUE;
		BlockIsROM[c + 0x7e0] = FALSE;
		BlockIsROM[c + 0x7f0] = FALSE;
	}

	// Banks 60->67, S-RAM
	for (c = 0; c < 0x80; c++) {
		Map[c + 0x600] = (uint8 *)MAP_LOROM_SRAM;
		BlockIsRAM[c + 0x600] = TRUE;
		BlockIsROM[c + 0x600] = FALSE;
	}

	WriteProtectROM();
}

void CMemory::SRAM512KLoROMMap()
{
	int c;
	int i;

	// Banks 00->3f and 80->bf
	for (c = 0; c < 0x400; c += 16) {
		Map[c + 0] = Map[c + 0x800] = RAM;
		Map[c + 1] = Map[c + 0x801] = RAM;
		BlockIsRAM[c + 0] = BlockIsRAM[c + 0x800] = TRUE;
		BlockIsRAM[c + 1] = BlockIsRAM[c + 0x801] = TRUE;

		Map[c + 2] = Map[c + 0x802] = (uint8 *)MAP_PPU;
		Map[c + 3] = Map[c + 0x803] = (uint8 *)MAP_PPU;
		Map[c + 4] = Map[c + 0x804] = (uint8 *)MAP_CPU;
		Map[c + 5] = Map[c + 0x805] = (uint8 *)MAP_CPU;
		Map[c + 6] = Map[c + 0x806] = (uint8 *)MAP_NONE;
		Map[c + 7] = Map[c + 0x807] = (uint8 *)MAP_NONE;

		for (i = c + 8; i < c + 16; i++) {
			Map[i] = Map[i + 0x800] = &ROM[c << 11] - 0x8000;
			BlockIsROM[i] = BlockIsROM[i + 0x800] = TRUE;
		}

		for (i = c; i < c + 16; i++) {
			int ppu = i & 15;

			MemorySpeed[i] = MemorySpeed[i + 0x800] = ppu >= 2 && ppu <= 3 ? ONE_CYCLE : SLOW_ONE_CYCLE;
		}
	}

	// Banks 40->7f and c0->ff
	for (c = 0; c < 0x400; c += 16) {
		for (i = c; i < c + 8; i++)
			Map[i + 0x400] = Map[i + 0xc00] = &ROM[(c << 11) + 0x200000];

		for (i = c + 8; i < c + 16; i++)
			Map[i + 0x400] = Map[i + 0xc00] = &ROM[(c << 11) + 0x200000 - 0x8000];

		for (i = c; i < c + 16; i++) {
			MemorySpeed[i + 0x400] = MemorySpeed[i + 0xc00] = SLOW_ONE_CYCLE;
			BlockIsROM[i + 0x400] = BlockIsROM[i + 0xc00] = TRUE;
		}
	}

	MapExtraRAM();
	WriteProtectROM();
}

void CMemory::SRAM1024KLoROMMap()
{
	int c;
	int i;

	// Banks 00->3f and 80->bf
	for (c = 0; c < 0x400; c += 16) {
		Map[c + 0] = Map[c + 0x800] = Map[c + 0x400] = Map[c + 0xc00] = RAM;
		Map[c + 1] = Map[c + 0x801] = Map[c + 0x401] = Map[c + 0xc01] = RAM;
		BlockIsRAM[c + 0] = BlockIsRAM[c + 0x800] = BlockIsRAM[c + 0x400] = BlockIsRAM[c + 0xc00] = TRUE;
		BlockIsRAM[c + 1] = BlockIsRAM[c + 0x801] = BlockIsRAM[c + 0x401] = BlockIsRAM[c + 0xc01] = TRUE;

		Map[c + 2] = Map[c + 0x802] = Map[c + 0x402] = Map[c + 0xc02] = (uint8 *)MAP_PPU;
		Map[c + 3] = Map[c + 0x803] = Map[c + 0x403] = Map[c + 0xc03] = (uint8 *)MAP_PPU;
		Map[c + 4] = Map[c + 0x804] = Map[c + 0x404] = Map[c + 0xc04] = (uint8 *)MAP_CPU;
		Map[c + 5] = Map[c + 0x805] = Map[c + 0x405] = Map[c + 0xc05] = (uint8 *)MAP_CPU;
		Map[c + 6] = Map[c + 0x806] = Map[c + 0x406] = Map[c + 0xc06] = (uint8 *)MAP_NONE;
		Map[c + 7] = Map[c + 0x807] = Map[c + 0x407] = Map[c + 0xc07] = (uint8 *)MAP_NONE;
		for (i = c + 8; i < c + 16; i++) {
			Map[i] = Map[i + 0x800] = Map[i + 0x400] = Map[i + 0xc00] = &ROM[c << 11] - 0x8000;
			BlockIsROM[i] = BlockIsROM[i + 0x800] = BlockIsROM[i + 0x400] = BlockIsROM[i + 0xc00] = TRUE;
		}

		for (i = c; i < c + 16; i++) {
			int ppu = i & 15;

			MemorySpeed[i] = MemorySpeed[i + 0x800] = MemorySpeed[i + 0x400] = MemorySpeed[i + 0xc00] =
			    ppu >= 2 && ppu <= 3 ? ONE_CYCLE : SLOW_ONE_CYCLE;
		}
	}

	MapExtraRAM();
	WriteProtectROM();
}

void CMemory::BSHiROMMap()
{
	int c;
	int i;

	// Banks 00->3f and 80->bf
	for (c = 0; c < 0x400; c += 16) {
		Map[c + 0] = Map[c + 0x800] = RAM;
		BlockIsRAM[c + 0] = BlockIsRAM[c + 0x800] = TRUE;
		Map[c + 1] = Map[c + 0x801] = RAM;
		BlockIsRAM[c + 1] = BlockIsRAM[c + 0x801] = TRUE;

		Map[c + 2] = Map[c + 0x802] = (uint8 *)MAP_PPU;
		Map[c + 3] = Map[c + 0x803] = (uint8 *)MAP_PPU;
		Map[c + 4] = Map[c + 0x804] = (uint8 *)MAP_CPU;
		// XXX: How large is SRAM??
		Map[c + 5] = Map[c + 0x805] = (uint8 *)SRAM;
		BlockIsRAM[c + 5] = BlockIsRAM[c + 0x805] = TRUE;
		Map[c + 6] = Map[c + 0x806] = (uint8 *)MAP_NONE;
		Map[c + 7] = Map[c + 0x807] = (uint8 *)MAP_NONE;

		for (i = c + 8; i < c + 16; i++) {
			Map[i] = Map[i + 0x800] = &ROM[(c << 12) % CalculatedSize];
			BlockIsROM[i] = BlockIsROM[i + 0x800] = TRUE;
		}

		for (i = c; i < c + 16; i++) {
			int ppu = i & 15;

			MemorySpeed[i] = MemorySpeed[i + 0x800] = ppu >= 2 && ppu <= 3 ? ONE_CYCLE : SLOW_ONE_CYCLE;
		}
	}

	// Banks 60->7d offset 0000->7fff & 60->7f offset 8000->ffff PSRAM
	// XXX: How large is PSRAM?
	for (c = 0x600; c < 0x7e0; c += 16) {
		for (i = c; i < c + 8; i++) {
			Map[i] = &ROM[0x400000 + (c << 11)];
			BlockIsRAM[i] = TRUE;
		}
		for (i = c + 8; i < c + 16; i++) {
			Map[i] = &ROM[0x400000 + (c << 11) - 0x8000];
			BlockIsRAM[i] = TRUE;
		}
	}

	// Banks 40->7f and c0->ff
	for (c = 0; c < 0x400; c += 16) {
		for (i = c; i < c + 16; i++) {
			Map[i + 0x400] = Map[i + 0xc00] = &ROM[(c << 12) % CalculatedSize];
			MemorySpeed[i + 0x400] = MemorySpeed[i + 0xc00] = SLOW_ONE_CYCLE;
			BlockIsROM[i + 0x400] = BlockIsROM[i + 0xc00] = TRUE;
		}
	}

	MapRAM();
	WriteProtectROM();
}

#ifdef SPC7110_DSP
void CMemory::SPC7110HiROMMap()
{
	int c;
	int i;

	// Banks 00->3f and 80->bf
	for (c = 0; c < 0x400; c += 16) {
		Map[c + 0] = Map[c + 0x800] = RAM;
		BlockIsRAM[c + 0] = BlockIsRAM[c + 0x800] = TRUE;
		Map[c + 1] = Map[c + 0x801] = RAM;
		BlockIsRAM[c + 1] = BlockIsRAM[c + 0x801] = TRUE;

		Map[c + 2] = Map[c + 0x802] = (uint8 *)MAP_PPU;
		Map[c + 3] = Map[c + 0x803] = (uint8 *)MAP_PPU;
		Map[c + 4] = Map[c + 0x804] = (uint8 *)MAP_CPU;
		Map[c + 5] = Map[c + 0x805] = (uint8 *)MAP_CPU;

		Map[c + 6] /*= Map [c + 0x806]*/ = (uint8 *)MAP_HIROM_SRAM;
		Map[c + 7] /*= Map [c + 0x807]*/ = (uint8 *)MAP_HIROM_SRAM;
		Map[c + 0x806] = Map[c + 0x807] = (uint8 *)MAP_NONE;

		for (i = c + 8; i < c + 16; i++) {
			Map[i] = Map[i + 0x800] = &ROM[(c << 12) % CalculatedSize];
			BlockIsROM[i] = BlockIsROM[i + 0x800] = TRUE;
		}
	}

	// Banks 30->3f and b0->bf, address ranges 6000->7fff is S-RAM.
	for (c = 0; c < 16; c++) {
		Map[0x306 + (c << 4)] = (uint8 *)MAP_HIROM_SRAM;
		Map[0x307 + (c << 4)] = (uint8 *)MAP_HIROM_SRAM;
		Map[0xb06 + (c << 4)] = (uint8 *)MAP_NONE;
		Map[0xb07 + (c << 4)] = (uint8 *)MAP_NONE;
		BlockIsRAM[0x306 + (c << 4)] = TRUE;
		BlockIsRAM[0x307 + (c << 4)] = TRUE;
		//	BlockIsRAM [0xb06 + (c << 4)] = TRUE;
		//	BlockIsRAM [0xb07 + (c << 4)] = TRUE;
	}

	// Banks 40->7f and c0->ff
	for (c = 0; c < 0x400; c += 16) {
		for (i = c; i < c + 16; i++) {
			Map[i + 0x400] = Map[i + 0xc00] = &ROM[(c << 12) % CalculatedSize];
			BlockIsROM[i + 0x400] = BlockIsROM[i + 0xc00] = TRUE;
		}
	}

	for (c = 0; c < 0x10; c++) {
		Map[0x500 + c] = (uint8 *)MAP_SPC7110_DRAM;
		BlockIsROM[0x500 + c] = TRUE;
	}

	for (c = 0; c < 0x100; c++) {
		Map[0xD00 + c] = (uint8 *)MAP_SPC7110_ROM;
		Map[0xE00 + c] = (uint8 *)MAP_SPC7110_ROM;
		Map[0xF00 + c] = (uint8 *)MAP_SPC7110_ROM;
		BlockIsROM[0xD00 + c] = BlockIsROM[0xE00 + c] = BlockIsROM[0xF00 + c] = TRUE;
	}
	S9xSpc7110Init();

	int sum = 0;
	for (i = 0; i < (int)CalculatedSize; i++) {
		sum += ROM[i];
	}

	if (CalculatedSize == 0x300000)
		sum <<= 1;
	CalculatedChecksum = sum & 0xFFFF;

	MapRAM();
	WriteProtectROM();
}

void CMemory::SPC7110Sram(uint8 newstate)
{
	if (newstate & 0x80) {
		Memory.Map[6] = (uint8 *)MAP_HIROM_SRAM;
		Memory.Map[7] = (uint8 *)MAP_HIROM_SRAM;
		Memory.Map[0x306] = (uint8 *)MAP_HIROM_SRAM;
		Memory.Map[0x307] = (uint8 *)MAP_HIROM_SRAM;

	} else {
		Memory.Map[6] = (uint8 *)MAP_RONLY_SRAM;
		Memory.Map[7] = (uint8 *)MAP_RONLY_SRAM;
		Memory.Map[0x306] = (uint8 *)MAP_RONLY_SRAM;
		Memory.Map[0x307] = (uint8 *)MAP_RONLY_SRAM;
	}
}
#endif

const char *CMemory::TVStandard() { return (Settings.PAL ? "PAL" : "NTSC"); }

const char *CMemory::Speed() { return (ROMSpeed & 0x10 ? "120ns" : "200ns"); }

const char *CMemory::MapType() { return (HiROM ? "HiROM" : "LoROM"); }

const char *CMemory::StaticRAMSize()
{
	static char tmp[20];

	if (Memory.SRAMSize > 16)
		return ("Corrupt");
	sprintf(tmp, "%dKb", (SRAMMask + 1) / 1024);
	return (tmp);
}

const char *CMemory::Size()
{
	static char tmp[20];

	if (ROMSize < 7 || ROMSize - 7 > 23)
		return ("Corrupt");
	sprintf(tmp, "%dMbits", 1 << (ROMSize - 7));
	return (tmp);
}

const char *CMemory::KartContents()
{
	static char tmp[30], result[30];
	static const char *CoPro[16] = {"DSP",	    "SuperFX",	"OBC1",	    "SA-1",	   "S-DD1",    "S-RTC",
					"CoPro#6",  "CoPro#7",	"CoPro#8",  "CoPro#9",	   "CoPro#10", "CoPro#11",
					"CoPro#12", "CoPro#13", "CoPro#14", "CoPro-Custom"};
	static const char *Contents[3] = {"ROM", "ROM+RAM", "ROM+RAM+BAT"};
	static const char *DSPSel[4] = {"DSP1", "DSP2", "DSP3", "DSP4"};
	if (ROMType == 0 && !Settings.BS)
		return ("ROM only");

	sprintf(tmp, "%s", Contents[(ROMType & 0xf) % 3]);
	// If there is not extra chip, just use ROMType
	memcpy(result, tmp, sizeof(tmp));

	if (Settings.BS)
		sprintf(result, "%s+%s", tmp, "BSX");
#if 0
	else if (Settings.SPC7110 && Settings.SPC7110RTC)
		sprintf(result, "%s+%s", tmp, "SPC7110+RTC");
	else if (Settings.SPC7110)
		sprintf(result, "%s+%s", tmp, "SPC7110");
#endif
	else if (Settings.C4)
		sprintf(result, "%s+%s", tmp, "C4");
#ifdef SETA_DSP
	else if (Settings.SETA != 0) {
		switch (Settings.SETA) {
		case ST_010:
			sprintf(result, "%s+%s", tmp, "ST-010");
			break;
		case ST_011:
			sprintf(result, "%s+%s", tmp, "ST-011");
			break;

		case ST_018:
			sprintf(result, "%s+%s", tmp, "ST-018");
			break;
		}
	}
#endif
	else if ((ROMType & 0xf) >= 3) {
		if (ROMType & 0xf0)
			sprintf(result, "%s+%s", tmp, CoPro[(ROMType & 0xf0) >> 4]);
		else
			sprintf(result, "%s+%s", tmp, DSPSel[DSP1.version]);
	}

	return (result);
}

const char *CMemory::MapMode()
{
	static char tmp[4];
	sprintf(tmp, "%02x", ROMSpeed & ~0x10);
	return (tmp);
}

const char *CMemory::ROMID() { return (ROMId); }

void CMemory::ApplyROMFixes()
{
	// Lantus: Dirty Hack for Star Fox 2
	if (strcmp(ROMName, "STAR FOX 2") == 0) {
		for (int c = 0; c < 0x400; c += 16) {
			for (int i = c; i < c + 16; i++) {
				int ppu = i & 15;
				MemorySpeed[i] = MemorySpeed[i + 0x800] =
				    ppu >= 2 && ppu <= 3 ? ONE_CYCLE : SLOW_ONE_CYCLE;
			}
		}
	}

	if (strncmp(ROMName, "WAR 2410", 8) == 0) {
		Map[0x005] = (uint8 *)RAM;
		BlockIsRAM[0x005] = TRUE;
		BlockIsROM[0x005] = FALSE;
	}

	if (strncmp(ROMName, "XBAND", 5) == 0) {
		for (int c = 0xE00; c < 0xE10; c++) {
			Map[c] = (uint8 *)MAP_LOROM_SRAM;
			BlockIsRAM[c] = TRUE;
			BlockIsROM[c] = FALSE;
		}
		WriteProtectROM();
	}

	// Ambiguous chip function pointer assignments
	DSP1.version = 0;

	// DSP switching:
	if (strncmp(ROMName, "DUNGEON MASTER", 14) == 0) {
		// Set DSP-2
		DSP1.version = 1;
		SetDSP = &DSP2SetByte;
		GetDSP = &DSP2GetByte;
	}

	if (strncmp(ROMName, "SD\x0b6\x0de\x0dd\x0c0\x0de\x0d1GX", 10) == 0) {
		// Set DSP-3
		DSP1.version = 2;
		strncpy(ROMName, "SD Gundam GX", 13);
		SetDSP = &DSP3SetByte;
		GetDSP = &DSP3GetByte;
		DSP3_Reset();
	}

	if (strncmp(ROMName, "TOP GEAR 3000", 13) == 0 || strncmp(ROMName, "PLANETS CHAMP TG3000", 20) == 0) {
		// Set DSP-4
		DSP1.version = 3;
		SetDSP = &DSP4SetByte;
		GetDSP = &DSP4GetByte;
	}

	// Enable S-RTC (Real Time Clock) emulation for Dai Kaijyu Monogatari 2
	Settings.SRTC = ((ROMType & 0xf0) >> 4) == 5;

	Settings.StrikeGunnerOffsetHack = strcmp(ROMName, "STRIKE GUNNER") == 0 ? 7 : 0;

	CPU.NMITriggerPoint = 4;
	if (strcmp(ROMName, "CACOMA KNIGHT") == 0)
		CPU.NMITriggerPoint = 25;

	// These games complain if the multi-player adaptor is 'connected'
	if (strcmp(ROMName, "TETRIS&Dr.MARIO") == 0 || strcmp(ROMName, "JIGSAW PARTY") == 0 ||
	    strcmp(ROMName, "SUPER PICROSS") == 0 || strcmp(ROMName, "KIRBY NO KIRA KIZZU") == 0 ||
	    strcmp(ROMName, "BLOCK") == 0 || strncmp(ROMName, "SUPER BOMBLISS", 14) == 0 ||
	    strcmp(ROMId, "ABOJ") == 0) {
		Settings.MultiPlayer5Master = FALSE;
		Settings.MouseMaster = FALSE;
		Settings.SuperScopeMaster = FALSE;
	}

	// Games which spool sound samples between the SNES and sound CPU using
	// H-DMA as the sample is playing.
	if (strcmp(ROMName, "EARTHWORM JIM 2") == 0 || strcmp(ROMName, "PRIMAL RAGE") == 0 ||
	    strcmp(ROMName, "CLAY FIGHTER") == 0 || strcmp(ROMName, "ClayFighter 2") == 0 ||
	    strncasecmp(ROMName, "MADDEN", 6) == 0 || strncmp(ROMName, "NHL", 3) == 0 ||
	    strcmp(ROMName, "WeaponLord") == 0 || strncmp(ROMName, "WAR 2410", 8) == 0) {
		Settings.Shutdown = FALSE;
	}

	// Stunt Racer FX
	if (strcmp(ROMId, "CQ  ") == 0 ||
	    // Illusion of Gaia
	    strncmp(ROMId, "JG", 2) == 0 || strcmp(ROMName, "GAIA GENSOUKI 1 JPN") == 0) {
		IAPU.OneCycle = 13;
	}

	// RENDERING RANGER R2
	if (strcmp(ROMId, "AVCJ") == 0 ||
	    // Mark Davis
	    strncmp(ROMName, "THE FISHING MASTER", 18) == 0 || // needs >= actual APU timing. (21 is .002 Mhz slower)
	    // Star Ocean
	    strncmp(ROMId, "ARF", 3) == 0 ||
	    // Tales of Phantasia
	    strncmp(ROMId, "ATV", 3) == 0 ||
	    // Act Raiser 1 & 2
	    strncasecmp(ROMName, "ActRaiser", 9) == 0 ||
	    // Soulblazer
	    strcmp(ROMName, "SOULBLAZER - 1 USA") == 0 || strncmp(ROMName, "SOULBLAZER 1", 12) == 0 ||
	    strcmp(ROMName, "SOULBLADER - 1") == 0 ||
	    // Terranigma
	    strncmp(ROMId, "AQT", 3) == 0 ||
	    // Robotrek
	    strncmp(ROMId, "E9 ", 3) == 0 || strcmp(ROMName, "SLAP STICK 1 JPN") == 0 ||
	    // ZENNIHON PURORESU2
	    strncmp(ROMId, "APR", 3) == 0 ||
	    // Bomberman 4
	    strncmp(ROMId, "A4B", 3) == 0 ||
	    // UFO KAMEN YAKISOBAN
	    strncmp(ROMId, "Y7 ", 3) == 0 || strncmp(ROMId, "Y9 ", 3) == 0 ||
	    // Panic Bomber World
	    strncmp(ROMId, "APB", 3) == 0 ||
	    ((strncmp(ROMName, "Parlor", 6) == 0 || strcmp(ROMName, "HEIWA Parlor!Mini8") == 0 ||
	      strncmp(ROMName, "SANKYO Fever! \xCC\xA8\xB0\xCA\xDE\xB0!", 21) == 0) && // SANKYO Fever! Fever!
	     strcmp(CompanyId, "A0") == 0) ||
	    strcmp(ROMName, "DARK KINGDOM") == 0 || strcmp(ROMName, "ZAN3 SFC") == 0 ||
	    strcmp(ROMName, "HIOUDEN") == 0 || strcmp(ROMName, "\xC3\xDD\xBC\xC9\xB3\xC0") == 0 || // Tenshi no Uta
	    strcmp(ROMName, "FORTUNE QUEST") == 0 || strcmp(ROMName, "FISHING TO BASSING") == 0 ||
	    strncmp(ROMName, "TokyoDome '95Battle 7", 21) == 0 || strcmp(ROMName, "OHMONO BLACKBASS") == 0 ||
	    strncmp(ROMName, "SWORD WORLD SFC", 15) == 0 || strcmp(ROMName, "MASTERS") == 0 || // Augusta 2 J
	    strcmp(ROMName, "SFC \xB6\xD2\xDD\xD7\xB2\xC0\xDE\xB0") == 0 ||		       // Kamen Rider
	    strncmp(ROMName, "LETs PACHINKO(", 14) == 0)				       // A set of BS games
	{
		IAPU.OneCycle = 15;
	}

	if (strcmp(ROMName, "BATMAN--REVENGE JOKER") == 0) {
		Memory.HiROM = FALSE;
		Memory.LoROM = TRUE;
		LoROMMap();
	}
	Settings.StarfoxHack = strcmp(ROMName, "STAR FOX") == 0 || strcmp(ROMName, "STAR WING") == 0;
	Settings.WinterGold =
	    strcmp(ROMName, "FX SKIING NINTENDO 96") == 0 || strcmp(ROMName, "DIRT RACER") == 0 || Settings.StarfoxHack;

	Settings.ChuckRock = strcmp(ROMName, "CHUCK ROCK") == 0;

	if (strcmp(ROMName, "RADICAL DREAMERS") == 0 || strcmp(ROMName, "TREASURE CONFLIX") == 0) {
		int c;

		for (c = 0; c < 0x80; c++) {
			Map[c + 0x700] = ROM + 0x200000 + 0x1000 * (c & 0xf0);
			BlockIsRAM[c + 0x700] = TRUE;
			BlockIsROM[c + 0x700] = FALSE;
		}
		for (c = 0; c < 0x400; c += 16) {
			Map[c + 5] = Map[c + 0x805] = ROM + 0x300000;
			BlockIsRAM[c + 5] = BlockIsRAM[c + 0x805] = TRUE;
		}
		WriteProtectROM();
	}

	// Totally wacky display...
	// seems to need a disproven behavior, so
	// we're definitely overlooking some other bug?
	if (strncmp(ROMName, "UNIRACERS", 9) == 0)
		SNESGameFixes.Uniracers = true;

	Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * Settings.CyclesPercentage) / 100;

	if (strcmp(ROMId, "ASRJ") == 0 && Settings.CyclesPercentage == 100)
		// Street Racer
		Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * 95) / 100;

	// Power Rangers Fight
	if (strncmp(ROMId, "A3R", 3) == 0 ||
	    // Clock Tower
	    strncmp(ROMId, "AJE", 3) == 0)
		Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * 103) / 100;

	if (strcmp(ROMId, "AWVP") == 0 || strcmp(ROMId, "AWVE") == 0 || strcmp(ROMId, "AWVJ") == 0) {
		// Wrestlemania Arcade
#if 0
	if (Settings.CyclesPercentage == 100)
	    Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * 140) / 100; // Fixes sound
#endif
		Settings.WrestlemaniaArcade = TRUE;
	}
	// Theme Park - disable offset-per-tile mode.
	if (strcmp(ROMId, "ATQP") == 0)
		Settings.WrestlemaniaArcade = TRUE;

	if (strncmp(ROMId, "A3M", 3) == 0 && Settings.CyclesPercentage == 100)
		// Mortal Kombat 3. Fixes cut off speech sample
		Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * 110) / 100;

	if (strcmp(ROMName, "\x0bd\x0da\x0b2\x0d4\x0b0\x0bd\x0de") == 0 && Settings.CyclesPercentage == 100)
		Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * 101) / 100;

#ifdef DETECT_NASTY_FX_INTERLEAVE
	// XXX: Test without these. Win32 port indicates they aren't needed?
	// Apparently are needed!
	if (strcmp(ROMName, "WILD TRAX") == 0 || strcmp(ROMName, "STAR FOX 2") == 0 ||
	    strcmp(ROMName, "YOSSY'S ISLAND") == 0 || strcmp(ROMName, "YOSHI'S ISLAND") == 0)
		CPU.TriedInterleavedMode2 = TRUE;
#endif

	// Start Trek: Deep Sleep 9
	if (strncmp(ROMId, "A9D", 3) == 0 && Settings.CyclesPercentage == 100)
		Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * 110) / 100;

	// SA-1 Speedup settings

	SA1.WaitAddress = NULL;
	SA1.WaitByteAddress1 = NULL;
	SA1.WaitByteAddress2 = NULL;

	/* Bass Fishing */
	if (strcmp(ROMId, "ZBPJ") == 0) {
		SA1.WaitAddress = SA1.Map[0x0093f1 >> MEMMAP_SHIFT] + 0x93f1;
		SA1.WaitByteAddress1 = FillRAM + 0x304a;
	}
	/* DAISENRYAKU EXPERTWW2 */
	if (strcmp(ROMId, "AEVJ") == 0) {
		SA1.WaitAddress = SA1.Map[0x0ed18d >> MEMMAP_SHIFT] + 0xd18d;
		SA1.WaitByteAddress1 = FillRAM + 0x3000;
	}
	/* debjk2 */
	if (strcmp(ROMId, "A2DJ") == 0) {
		SA1.WaitAddress = SA1.Map[0x008b62 >> MEMMAP_SHIFT] + 0x8b62;
	}
	/* Dragon Ballz HD */
	if (strcmp(ROMId, "AZIJ") == 0) {
		SA1.WaitAddress = SA1.Map[0x008083 >> MEMMAP_SHIFT] + 0x8083;
		SA1.WaitByteAddress1 = FillRAM + 0x3020;
	}
	/* SFC SDGUNDAMGNEXT */
	if (strcmp(ROMId, "ZX3J") == 0) {
		SA1.WaitAddress = SA1.Map[0x0087f2 >> MEMMAP_SHIFT] + 0x87f2;
		SA1.WaitByteAddress1 = FillRAM + 0x30c4;
	}
	/* ShougiNoHanamichi */
	if (strcmp(ROMId, "AARJ") == 0) {
		SA1.WaitAddress = SA1.Map[0xc1f85a >> MEMMAP_SHIFT] + 0xf85a;
		SA1.WaitByteAddress1 = SRAM + 0x0c64;
		SA1.WaitByteAddress2 = SRAM + 0x0c66;
	}
	/* KATO HIFUMI9DAN SYOGI */
	if (strcmp(ROMId, "A23J") == 0) {
		SA1.WaitAddress = SA1.Map[0xc25037 >> MEMMAP_SHIFT] + 0x5037;
		SA1.WaitByteAddress1 = SRAM + 0x0c06;
		SA1.WaitByteAddress2 = SRAM + 0x0c08;
	}
	/* idaten */
	if (strcmp(ROMId, "AIIJ") == 0) {
		SA1.WaitAddress = SA1.Map[0xc100be >> MEMMAP_SHIFT] + 0x00be;
		SA1.WaitByteAddress1 = SRAM + 0x1002;
		SA1.WaitByteAddress2 = SRAM + 0x1004;
	}
	/* igotais */
	if (strcmp(ROMId, "AITJ") == 0) {
		SA1.WaitAddress = SA1.Map[0x0080b7 >> MEMMAP_SHIFT] + 0x80b7;
	}
	/* J96 DREAM STADIUM */
	if (strcmp(ROMId, "AJ6J") == 0) {
		SA1.WaitAddress = SA1.Map[0xc0f74a >> MEMMAP_SHIFT] + 0xf74a;
	}
	/* JumpinDerby */
	if (strcmp(ROMId, "AJUJ") == 0) {
		SA1.WaitAddress = SA1.Map[0x00d926 >> MEMMAP_SHIFT] + 0xd926;
	}
	/* JKAKINOKI SHOUGI */
	if (strcmp(ROMId, "AKAJ") == 0) {
		SA1.WaitAddress = SA1.Map[0x00f070 >> MEMMAP_SHIFT] + 0xf070;
	}
	/* HOSHI NO KIRBY 3 & KIRBY'S DREAM LAND 3 JAP & US */
	if (strcmp(ROMId, "AFJJ") == 0 || strcmp(ROMId, "AFJE") == 0) {
		SA1.WaitAddress = SA1.Map[0x0082d4 >> MEMMAP_SHIFT] + 0x82d4;
		SA1.WaitByteAddress1 = SRAM + 0x72a4;
	}
	/* KIRBY SUPER DELUXE JAP */
	if (strcmp(ROMId, "AKFJ") == 0) {
		SA1.WaitAddress = SA1.Map[0x008c93 >> MEMMAP_SHIFT] + 0x8c93;
		SA1.WaitByteAddress1 = FillRAM + 0x300a;
		SA1.WaitByteAddress2 = FillRAM + 0x300e;
	}
	/* KIRBY SUPER DELUXE US */
	if (strcmp(ROMId, "AKFE") == 0) {
		SA1.WaitAddress = SA1.Map[0x008cb8 >> MEMMAP_SHIFT] + 0x8cb8;
		SA1.WaitByteAddress1 = FillRAM + 0x300a;
		SA1.WaitByteAddress2 = FillRAM + 0x300e;
	}
	/* SUPER MARIO RPG JAP & US */
	if (strcmp(ROMId, "ARWJ") == 0 || strcmp(ROMId, "ARWE") == 0) {
		SA1.WaitAddress = SA1.Map[0xc0816f >> MEMMAP_SHIFT] + 0x816f;
		SA1.WaitByteAddress1 = FillRAM + 0x3000;
	}
	/* marvelous.zip */
	if (strcmp(ROMId, "AVRJ") == 0) {
		SA1.WaitAddress = SA1.Map[0x0085f2 >> MEMMAP_SHIFT] + 0x85f2;
		SA1.WaitByteAddress1 = FillRAM + 0x3024;
	}
	/* AUGUSTA3 MASTERS NEW */
	if (strcmp(ROMId, "AO3J") == 0) {
		SA1.WaitAddress = SA1.Map[0x00dddb >> MEMMAP_SHIFT] + 0xdddb;
		SA1.WaitByteAddress1 = FillRAM + 0x37b4;
	}
	/* OSHABERI PARODIUS */
	if (strcmp(ROMId, "AJOJ") == 0) {
		SA1.WaitAddress = SA1.Map[0x8084e5 >> MEMMAP_SHIFT] + 0x84e5;
	}
	/* PANIC BOMBER WORLD */
	if (strcmp(ROMId, "APBJ") == 0) {
		SA1.WaitAddress = SA1.Map[0x00857a >> MEMMAP_SHIFT] + 0x857a;
	}
	/* PEBBLE BEACH NEW */
	if (strcmp(ROMId, "AONJ") == 0) {
		SA1.WaitAddress = SA1.Map[0x00df33 >> MEMMAP_SHIFT] + 0xdf33;
		SA1.WaitByteAddress1 = FillRAM + 0x37b4;
	}
	/* PGA EUROPEAN TOUR */
	if (strcmp(ROMId, "AEPE") == 0) {
		SA1.WaitAddress = SA1.Map[0x003700 >> MEMMAP_SHIFT] + 0x3700;
		SA1.WaitByteAddress1 = FillRAM + 0x3102;
	}
	/* PGA TOUR 96 */
	if (strcmp(ROMId, "A3GE") == 0) {
		SA1.WaitAddress = SA1.Map[0x003700 >> MEMMAP_SHIFT] + 0x3700;
		SA1.WaitByteAddress1 = FillRAM + 0x3102;
	}
	/* POWER RANGERS 4 */
	if (strcmp(ROMId, "A4RE") == 0) {
		SA1.WaitAddress = SA1.Map[0x009899 >> MEMMAP_SHIFT] + 0x9899;
		SA1.WaitByteAddress1 = FillRAM + 0x3000;
	}
	/* PACHISURO PALUSUPE */
	if (strcmp(ROMId, "AGFJ") == 0) {
		// Never seems to turn on the SA-1!
	}
	/* SD F1 GRAND PRIX */
	if (strcmp(ROMId, "AGFJ") == 0) {
		SA1.WaitAddress = SA1.Map[0x0181bc >> MEMMAP_SHIFT] + 0x81bc;
	}
	/* SHOUGI MARJONG */
	if (strcmp(ROMId, "ASYJ") == 0) {
		SA1.WaitAddress = SA1.Map[0x00f2cc >> MEMMAP_SHIFT] + 0xf2cc;
		SA1.WaitByteAddress1 = SRAM + 0x7ffe;
		SA1.WaitByteAddress2 = SRAM + 0x7ffc;
	}
	/* shogisai2 */
	if (strcmp(ROMId, "AX2J") == 0) {
		SA1.WaitAddress = SA1.Map[0x00d675 >> MEMMAP_SHIFT] + 0xd675;
	}
	/* SHINING SCORPION */
	if (strcmp(ROMId, "A4WJ") == 0) {
		SA1.WaitAddress = SA1.Map[0xc048be >> MEMMAP_SHIFT] + 0x48be;
	}
	/* SHIN SHOUGI CLUB */
	if (strcmp(ROMId, "AHJJ") == 0) {
		SA1.WaitAddress = SA1.Map[0xc1002a >> MEMMAP_SHIFT] + 0x002a;
		SA1.WaitByteAddress1 = SRAM + 0x0806;
		SA1.WaitByteAddress2 = SRAM + 0x0808;
	}

	// Other

	// Additional game fixes by sanmaiwashi ...
	if (strcmp(ROMName, "SFX "
			    "\xC5\xB2\xC4\xB6\xDE\xDD\xC0\xDE\xD1\xD3\xC9\xB6"
			    "\xDE\xC0\xD8 1") == 0) // Gundam Knight Story
	{
		bytes0x2000[0xb18] = 0x4c;
		bytes0x2000[0xb19] = 0x4b;
		bytes0x2000[0xb1a] = 0xea;
		SNESGameFixes.SRAMInitialValue = 0x6b;
	}

	// not MAD-1 compliant
	if (strcmp(ROMName, "WANDERERS FROM YS") == 0) {
		for (int c = 0; c < 0xE0; c++) {
			Map[c + 0x700] = (uint8 *)MAP_LOROM_SRAM;
			BlockIsROM[c + 0x700] = FALSE;
			BlockIsRAM[c + 0x700] = TRUE;
		}
		WriteProtectROM();
	}

	if (strcmp(ROMName, "GOGO ACKMAN3") == 0 || strcmp(ROMName, "HOME ALONE") == 0) {
		// Banks 00->3f and 80->bf
		for (int c = 0; c < 0x400; c += 16) {
			Map[c + 6] = Map[c + 0x806] = SRAM;
			Map[c + 7] = Map[c + 0x807] = SRAM;
			BlockIsROM[c + 6] = BlockIsROM[c + 0x806] = FALSE;
			BlockIsROM[c + 7] = BlockIsROM[c + 0x807] = FALSE;
			BlockIsRAM[c + 6] = BlockIsRAM[c + 0x806] = TRUE;
			BlockIsRAM[c + 7] = BlockIsRAM[c + 0x807] = TRUE;
		}
		WriteProtectROM();
	}

	if (strncmp(ROMName, "SWORD WORLD SFC", 15) == 0 || strcmp(ROMName, "SFC ¶ÒÝ×²ÀÞ°") == 0) {
		IAPU.OneCycle = 15;
		SNESGameFixes.NeedInit0x2137 = TRUE;
	}

	if (strncmp(ROMName, "SHIEN THE BLADE CHASE", 21) == 0)
		SNESGameFixes.Old_Read0x4200 = TRUE;

	if (strcmp(ROMName, "ºÞŒÞ× ¶²ŒÞ­³ÀÞ²¹¯ŸÝ") == 0)
		SNESGameFixes.NeedInit0x2137 = TRUE;

	if (strcmp(ROMName, "UMIHARAKAWASE") == 0)
		SNESGameFixes.umiharakawaseFix = TRUE;

	if (strcmp(ROMName, "ALIENS vs. PREDATOR") == 0)
		SNESGameFixes.alienVSpredetorFix = TRUE;

	if (strcmp(ROMName, "demon's blazon") == 0 || strcmp(ROMName, "demon's crest") == 0 ||
	    strcmp(ROMName, "ROCKMAN X") == 0 || strcmp(ROMName, "MEGAMAN X") == 0) {

		// CAPCOM's protect
		// Banks 0x808000, 0x408000 are mirroring.
		for (int c = 0; c < 8; c++)
			Map[0x408 + c] = ROM - 0x8000;
	}

	if (strcmp(ROMName, "œ°Êß°Ì§ÐœÀ") == 0 || strcmp(ROMName, "œ°Êß°Ì§ÐœÀ 2") == 0 ||
	    strcmp(ROMName, "ZENKI TENCHIMEIDOU") == 0 || strcmp(ROMName, "GANBA LEAGUE") == 0) {
		SNESGameFixes.APU_OutPorts_ReturnValueFix = TRUE;
	}

	// HITOMI3
	if (strcmp(ROMName, "HITOMI3") == 0) {
		Memory.SRAMSize = 1;
		SRAMMask = Memory.SRAMSize ? ((1 << (Memory.SRAMSize + 3)) * 128) - 1 : 0;
	}

	// sram value fixes
	if (strcmp(Memory.ROMName, "SUPER DRIFT OUT") == 0 || strcmp(Memory.ROMName, "SATAN IS OUR FATHER!") == 0 ||
	    strcmp(ROMName, "goemon 4") == 0)
		SNESGameFixes.SRAMInitialValue = 0x00;

	if (strcmp(ROMName, "PACHISLO ¹Ý·­³") == 0)
		SNESGameFixes._0x213E_ReturnValue = 1;

	if (strcmp(ROMName, "»Þ Ï°ŒÞ¬Ý Ä³Ê²ÃÞÝ") == 0)
		SNESGameFixes.TouhaidenControllerFix = TRUE;

	if (strcmp(ROMName, "DRAGON KNIGHT 4") == 0) {
		// Banks 70->7e, S-RAM
		for (int c = 0; c < 0xe0; c++) {
			Map[c + 0x700] = (uint8 *)MAP_LOROM_SRAM;
			BlockIsRAM[c + 0x700] = TRUE;
			BlockIsROM[c + 0x700] = FALSE;
		}
		WriteProtectROM();
	}

	if (strncmp(ROMName, "LETs PACHINKO(", 14) == 0) {
		IAPU.OneCycle = 15;
		if (!Settings.ForceNTSC && !Settings.ForcePAL) {
			Settings.PAL = FALSE;
			Settings.FrameTime = Settings.FrameTimeNTSC;
			Memory.ROMFramesPerSecond = 60;
		}
	}

	if (strcmp(ROMName, "FURAI NO SIREN") == 0)
		SNESGameFixes.SoundEnvelopeHeightReading2 = TRUE;
#if 0
    if(strcmp (ROMName, "XBAND JAPANESE MODEM") == 0)
    {
	for (c = 0x200; c < 0x400; c += 16)
	{
	    for (int i = c; i < c + 16; i++)
	    {
		Map [i + 0x400] = Map [i + 0xc00] = &ROM[c * 0x1000];
		MemorySpeed [i + 0x400] = MemorySpeed [i + 0xc00] = 8;
		BlockIsRAM [i + 0x400] = BlockIsRAM [i + 0xc00] = TRUE;
		BlockIsROM [i + 0x400] = BlockIsROM [i + 0xc00] = FALSE;
	    }
	}
	WriteProtectROM ();
    }
#endif

#define RomPatch(adr, ov, nv)                                                                                          \
	if (ROM[adr] == ov)                                                                                            \
	ROM[adr] = nv

	// Love Quest
	if (strcmp(ROMName, "LOVE QUEST") == 0) {
		RomPatch(0x1385ec, 0xd0, 0xea);
		RomPatch(0x1385ed, 0xb2, 0xea);
	}

	// Nangoku Syonen Papuwa Kun
	if (strcmp(ROMName, "NANGOKUSYONEN PAPUWA") == 0)
		RomPatch(0x1f0d1, 0xa0, 0x6b);

	// Tetsuwan Atom
	if (strcmp(ROMName, "Tetsuwan Atom") == 0) {
		RomPatch(0xe24c5, 0x90, 0xea);
		RomPatch(0xe24c6, 0xf3, 0xea);
	}

	// Oda Nobunaga
	if (strcmp(ROMName, "SFC ODA NOBUNAGA") == 0) {
		RomPatch(0x7497, 0x80, 0xea);
		RomPatch(0x7498, 0xd5, 0xea);
	}

	// Super Batter Up
	if (strcmp(ROMName, "Super Batter Up") == 0) {
		RomPatch(0x27ae0, 0xd0, 0xea);
		RomPatch(0x27ae1, 0xfa, 0xea);
	}

	// Super Professional Baseball 2
	if (strcmp(ROMName, "SUPER PRO. BASE BALL2") == 0) {
		RomPatch(0x1e4, 0x50, 0xea);
		RomPatch(0x1e5, 0xfb, 0xea);
	}

	if ((strncmp(ROMName, "FF MYSTIC QUEST", 15) == 0) || (strncmp(ROMName, "MYSTIC QUEST LEGEND", 19) == 0)) {
		if (Settings.CyclesPercentage == 100)
			Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * 120) / 100;
	}
}

// Read variable size MSB int from a file
static long ReadInt(FILE *f, unsigned nbytes)
{
	long v = 0;
	while (nbytes--) {
		int c = fgetc(f);
		if (c == EOF)
			return -1;
		v = (v << 8) | (c & 0xFF);
	}
	return (v);
}

#define IPS_EOF 0x00454F46l

void CMemory::CheckForIPSPatch(const char *rom_filename, bool8_32 header, int32 &rom_size)
{
	char dir[_MAX_DIR + 1];
	char drive[_MAX_DRIVE + 1];
	char name[_MAX_FNAME + 1];
	char ext[_MAX_EXT + 1];
	char fname[_MAX_PATH + 1];
	FILE *patch_file = NULL;
	long offset = header ? 512 : 0;

	_splitpath(rom_filename, drive, dir, name, ext);
	_makepath(fname, drive, dir, name, "ips");

	if (!(patch_file = fopen(fname, "rb"))) {
		if (!(patch_file = fopen(S9xGetFilename(".ips"), "rb")))
			return;
	}

	if (fread(fname, 1, 5, patch_file) != 5 || strncmp(fname, "PATCH", 5) != 0) {
		fclose(patch_file);
		return;
	}

	int32 ofs;

	for (;;) {
		long len;
		long rlen;
		int rchar;

		ofs = ReadInt(patch_file, 3);
		if (ofs == -1)
			goto err_eof;

		if (ofs == IPS_EOF)
			break;

		ofs -= offset;

		len = ReadInt(patch_file, 2);
		if (len == -1)
			goto err_eof;

		/* Apply patch block */
		if (len) {
			if (ofs + len > MAX_ROM_SIZE)
				goto err_eof;

			while (len--) {
				rchar = fgetc(patch_file);
				if (rchar == EOF)
					goto err_eof;
				ROM[ofs++] = (uint8)rchar;
			}
			if (ofs > rom_size)
				rom_size = ofs;
		} else {
			rlen = ReadInt(patch_file, 2);
			if (rlen == -1)
				goto err_eof;

			rchar = fgetc(patch_file);
			if (rchar == EOF)
				goto err_eof;

			if (ofs + rlen > MAX_ROM_SIZE)
				goto err_eof;

			while (rlen--)
				ROM[ofs++] = (uint8)rchar;

			if (ofs > rom_size)
				rom_size = ofs;
		}
	}

	// Check if ROM image needs to be truncated
	ofs = ReadInt(patch_file, 3);
	if (ofs != -1 && ofs - offset < rom_size) {
		// Need to truncate ROM image
		rom_size = ofs - offset;
	}
	fclose(patch_file);
	return;

err_eof:
	if (patch_file)
		fclose(patch_file);
}

int is_bsx(unsigned char *p)
{
	unsigned c;

	if (p[0x19] & 0x4f)
		goto notbsx;
	c = p[0x1a];
	if ((c != 0x33) && (c != 0xff)) // 0x33 = Manufacturer: Nintendo
		goto notbsx;
	c = (p[0x17] << 8) | p[0x16];
	if ((c != 0x0000) && (c != 0xffff)) {
		if ((c & 0x040f) != 0)
			goto notbsx;
		if ((c & 0xff) > 0xc0)
			goto notbsx;
	}
	c = p[0x18];
	if ((c & 0xce) || ((c & 0x30) == 0))
		goto notbsx;
	if ((p[0x15] & 0x03) != 0)
		goto notbsx;
	c = p[0x13];
	if ((c != 0x00) && (c != 0xff))
		goto notbsx;
	if (p[0x14] != 0x00)
		goto notbsx;
	int bs_name(unsigned char *);
	if (bs_name(p) != 0)
		goto notbsx;
	return 0; // It's a Satellaview ROM!
notbsx:
	return -1;
}
int bs_name(unsigned char *p)
{
	unsigned c;
	int lcount;
	int numv; // number of valid name characters seen so far
	numv = 0;
	for (lcount = 16; lcount > 0; lcount--) {
		int check_char(unsigned);
		if (check_char(c = *p++) != 0) {
			c = *p++;
			if (c < 0x20) {
				if ((numv != 0x0b) || (c != 0)) // Dr. Mario Hack
					goto notBsName;
			}

			numv++;
			lcount--;
			continue;
		} else {
			if (c == 0) {
				if (numv == 0)
					goto notBsName;
				continue;
			}

			if (c < 0x20)
				goto notBsName;
			if (c >= 0x80) {
				if ((c < 0xa0) || (c >= 0xf0))
					goto notBsName;
			}
			numv++;
		}
	}
	if (numv > 0)
		return 0;
notBsName:
	return -1;
}

int check_char(unsigned c)
{
	if ((c & 0x80) == 0)
		return 0;
	if ((c - 0x20) & 0x40)
		return 1;
	else
		return 0;
}

#undef INLINE
#define INLINE
#include "getset.h"
