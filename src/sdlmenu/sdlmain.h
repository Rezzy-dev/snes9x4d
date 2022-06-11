#ifndef __MAIN_h_
#define __MAIN_h_

#define APPHOMEDIR ".snes9x4d"
#define CFG_DIRECTORY "config"
#define SAV_DIRECTORY "saves"

extern uint16 sfc_key[256];
extern short SaveSlotNum;
extern bool8_32 Scale;
#ifdef BILINEAR_SCALE
extern bool8_32 Bilinear;
#endif
extern uint32 MaxAutoFrameSkip;

const char *GetHomeDirectory();
void OutOfMemory();
#ifdef SNESADVANCE_SPEEDHACKS
int S9xFindHacks(int game_crc32);
#endif

// Auxiliary functions from Snes9x
extern void S9xDisplayFrameRate(uint8 *, uint32);
extern void S9xDisplayString(const char *string, uint8 *, uint32, int);

#endif // __MAIN_h_
