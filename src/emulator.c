#include "emulator.h"

#define WINDOW_SCALE 12

#define MEM_SIZE     4096
#define STACK_SIZE   48
#define REGS_COUNT   16
#define PROGRAM_ADDR 0x200
#define FONT_ADDR    0
#define FONT_HEIGHT  5
#define ROM_SIZE     (MEM_SIZE - PROGRAM_ADDR)

#define HzToMs(HZ) (1.0 / (HZ) * 1000)

#define CYCLE_MS   HzToMs(660)
#define REFRESH_MS HzToMs(60)

uint8_t importedRom[ROM_SIZE];
size_t  importedRomSize;

static struct {
	bool     halt;
	uint8_t  mem[MEM_SIZE];
	uint16_t stack[STACK_SIZE];

	/* Registers */
	uint8_t  regs[REGS_COUNT], dt, st; /* delay timer, sound timer */
	uint16_t sp, pc, i; /* Stack pointer, program counter, memory index */

	/* Millisecond delay timers for consistent cycles and refresh rates */
	double cycleDelay, refreshDelay;
	bool   cycleRefreshed;

	/* Current key state while waiting for any key */
	uint8_t waitKey;
} emu;

static uint8_t font[FONT_HEIGHT * 16] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, /* 0 */
	0x20, 0x60, 0x20, 0x20, 0x70, /* 1 */
	0xF0, 0x10, 0xF0, 0x80, 0xF0, /* 2 */
	0xF0, 0x10, 0xF0, 0x10, 0xF0, /* 3 */
	0x90, 0x90, 0xF0, 0x10, 0x10, /* 4 */
	0xF0, 0x80, 0xF0, 0x10, 0xF0, /* 5 */
	0xF0, 0x80, 0xF0, 0x90, 0xF0, /* 6 */
	0xF0, 0x10, 0x20, 0x40, 0x40, /* 7 */
	0xF0, 0x90, 0xF0, 0x90, 0xF0, /* 8 */
	0xF0, 0x90, 0xF0, 0x10, 0xF0, /* 9 */
	0xF0, 0x90, 0xF0, 0x90, 0x90, /* A */
	0xE0, 0x90, 0xE0, 0x90, 0xE0, /* B */
	0xF0, 0x80, 0x80, 0x80, 0xF0, /* C */
	0xE0, 0x90, 0x90, 0x90, 0xE0, /* D */
	0xF0, 0x80, 0xF0, 0x80, 0xF0, /* E */
	0xF0, 0x80, 0xF0, 0x80, 0x80  /* F */
};

#define Advance() (emu.pc += 2)
#define Retreat() (emu.pc -= 2)

#define UnknownOpcode(OPCODE) Fatal("Unknown opcode 0x%04X", OPCODE)

static void RunOpcode(void) {
	Assert(emu.pc < MEM_SIZE, "Program counter overflow (pc = 0x%04X)", emu.pc);

	/*
		Technical reference - https://en.wikipedia.org/wiki/CHIP-8#Opcode_table
		Test suites         - https://github.com/Timendus/chip8-test-suite
		Screen quirks       - https://www.laurencescotford.net/2020/07/19/chip-8-on-the-cosmac-vip-drawing-sprites/
	*/

	uint16_t opcode = emu.mem[emu.pc] << 8 | emu.mem[emu.pc + 1],
	         nnn    =  opcode & 0x0FFF;
	uint8_t  nn     =  opcode & 0x00FF,
	         n      =  opcode & 0x000F,
	         x      = (opcode & 0x0F00) >> 8,
	         y      = (opcode & 0x00F0) >> 4;

	Advance();

	switch ((opcode & 0xF000) >> 12) {
	case 0x0:
		switch (nnn) {
		case 0x0E0: /* 00E0: Clear display */
			ClearVideo();
			break;

		case 0x0EE: /* 00EE: Return */
			Assert(emu.sp > 0, "Stack underflow (attempt to pop with sp = 0x%04X)", emu.sp);
			emu.pc = emu.stack[-- emu.sp];
			break;

		default: UnknownOpcode(opcode); /* 0NNN: Unsupported */
		}
		break;

	case 0x1: /* 1NNN: Jump to NNN */
		emu.pc = nnn;
		break;

	case 0x2: /* 2NNN: Call NNN */
		Assert(emu.sp < STACK_SIZE, "Stack overflow (attempt to push with sp = 0x%04X)", emu.sp);
		emu.stack[emu.sp ++] = emu.pc;
		emu.pc = nnn;
		break;

	case 0x3: /* 3XNN: If VX == NN */
		if (emu.regs[x] == nn)
			Advance();
		break;

	case 0x4: /* 4XNN: If VX != NN */
		if (emu.regs[x] != nn)
			Advance();
		break;

	case 0x5: /* 5XY0: If VX == VY */
		if (emu.regs[x] == emu.regs[y])
			Advance();
		break;

	case 0x6: /* 6XNN: VX = NN */
		emu.regs[x] = nn;
		break;

	case 0x7: /* 7XNN: VX = VX + NN */
		emu.regs[x] += nn;
		break;

	case 0x8:
		switch (n) {
		case 0x0: /* 8XY0: VX = VY */
			emu.regs[x] = emu.regs[y];
			break;

		case 0x1: /* 8XY1: VX = VX or VY */
			emu.regs[x]  |= emu.regs[y];
			emu.regs[0xF] = 0;
			break;

		case 0x2: /* 8XY2: VX = VX and VY */
			emu.regs[x]  &= emu.regs[y];
			emu.regs[0xF] = 0;
			break;

		case 0x3: /* 8XY3: VX = VX xor VY */
			emu.regs[x]  ^= emu.regs[y];
			emu.regs[0xF] = 0;
			break;

		case 0x4: /* 8XY4: VX = VX + VY */
			{
				uint8_t tmp   = emu.regs[x];
				emu.regs[x]  += emu.regs[y];
				emu.regs[0xF] = emu.regs[x] < tmp; /* 1 = Overflow, 0 = not */
			}
			break;

		case 0x5: /* 8XY5: VX = VX - VY */
			{
				uint8_t vf    = emu.regs[y] <= emu.regs[x]; /* 0 = Underflow, 1 = not */
				emu.regs[x]  -= emu.regs[y];
				emu.regs[0xF] = vf;
			}
			break;

		case 0x6: /* 8XY6: VX = VY >> 1 */
			{
				/* Uses the VY register, this breaks some roms. To fix, just replace y with x. */
				uint8_t vf    = emu.regs[y] & 0x01;
				emu.regs[x]   = emu.regs[y] >> 1;
				emu.regs[0xF] = vf;
			}
			break;

		case 0x7: /* 8XY7: VX = VY - VX */
			{
				uint8_t vf    = emu.regs[x] <= emu.regs[y]; /* 0 = Underflow, 1 = not */
				emu.regs[x]   = emu.regs[y] - emu.regs[x];
				emu.regs[0xF] = vf;
			}
			break;

		case 0xE: /* 8XYE: VX = VY << 1 */
			{
				/* Uses the VY register, this breaks some roms. To fix, just replace y with x. */
				uint8_t vf    = emu.regs[y] >> 7;
				emu.regs[x]   = emu.regs[y] << 1;
				emu.regs[0xF] = vf;
			}
			break;
		}
		break;

	case 0x9: /* 9XY0: If VX != VY */
		if (emu.regs[x] != emu.regs[y])
			Advance();
		break;

	case 0xA: /* ANNN: I = NNN */
		emu.i = nnn;
		break;

	case 0xB: /* BNNN: Jump to V0 + NNN */
		emu.pc = emu.regs[0x0] + nnn;
		break;

	case 0xC: /* CXNN: VX = random and NN */
		emu.regs[x] = (rand() % 256) & nn;
		break;

	case 0xD: /* DXYN: Draw sprite N at VX VY */
		/* Emulate waiting for the next screen refresh */
		if (emu.cycleRefreshed)
			emu.regs[0xF] = DrawSprite(emu.regs[x], emu.regs[y], emu.mem + emu.i, n);
		else
			Retreat();
		break;

	case 0xE:
		switch (nn) {
		case 0x9E: /* EX9E: If VX is pressed */
			if (KeyIsPressed(emu.regs[x]))
				Advance();
			break;

		case 0xA1: /* EXA1: If VX is not pressed */
			if (!KeyIsPressed(emu.regs[x]))
				Advance();
			break;

		default: UnknownOpcode(opcode);
		}
		break;

	case 0xF:
		switch (nn) {
		case 0x07: /* FX07: VX = DT */
			emu.regs[x] = emu.dt;
			break;

		case 0x0A: /* FX0A: VX = wait for a key press (wait until it is released too) */
			{
				if (emu.waitKey == KEYS_COUNT) { /* Key not pressed? */
					emu.waitKey = GetPressedKey();
					Retreat();
					break;
				}

				if (KeyIsPressed(emu.waitKey)) /* Wait for key release */
					Retreat();
				else {
					emu.regs[x] = emu.waitKey;
					emu.waitKey = KEYS_COUNT;
				}
			}
			break;

		case 0x15: /* FX15: DT = VX */
			emu.dt = emu.regs[x];
			break;

		case 0x18: /* FX18: ST = VX */
			emu.st = emu.regs[x];
			break;

		case 0x1E: /* FX1E: I = I + VX */
			emu.i += emu.regs[x];
			break;

		case 0x29: /* FX29: I = sprite VX */
			Assert(emu.regs[x] < 16, "Cannot get sprite for hex number %02x", emu.regs[x]);
			emu.i = emu.regs[x] * FONT_HEIGHT;
			break;

		case 0x33: /* FX33: Write 3 decimal digits of VX into memory at I (starts with hundreds) */
			emu.mem[emu.i]     = emu.regs[x] / 100 % 10;
			emu.mem[emu.i + 1] = emu.regs[x] / 10  % 10;
			emu.mem[emu.i + 2] = emu.regs[x]       % 10;
			break;

		case 0x55: /* FX55: Dump V0 to VX into memory starting at I */
			/* Wikipedia says 55 and 65 do not modify register I, but the test suites say they do.
			   I decided to follow the test suites. */
			for (int o = 0; o <= x; ++ o)
				emu.mem[emu.i ++] = emu.regs[o];
			break;

		case 0x65: /* FX65: Fill V0 to VX from memory starting at I */
			for (int o = 0; o <= x; ++ o)
				emu.regs[o] = emu.mem[emu.i ++];
			break;

		default: UnknownOpcode(opcode);
		}
		break;

	default: UnknownOpcode(opcode);
	}
}

static void UpdateDelays(void) {
	double ft = VideoFrameTime();

	emu.refreshDelay += ft;
	if (emu.refreshDelay   >= REFRESH_MS) {
		emu.refreshDelay   = 0;
		emu.cycleRefreshed = true;
	}

	emu.cycleDelay += ft;
	if (emu.cycleDelay >= CYCLE_MS)
		emu.cycleDelay = 0;
}

static void DumpInfo(void) {
	printf("\rPC: 0x%04X | SP: 0x%04X | DT: 0x%02X | ST: 0x%02X", emu.pc, emu.sp, emu.dt, emu.st);
	fflush(stdout);
}

static void Cycle(void) {
	RunOpcode();
	DumpInfo();

	if (emu.cycleRefreshed)
		emu.cycleRefreshed = false;
}

static void StartEmulator(const char *romPath);

static void CheckControlKeys(void) {
	if (CkeyIsPressed(CKEY_HALT))
		emu.halt = true;

	if (CkeyIsPressed(CKEY_RESET)) {
		StartEmulator(NULL);
		ClearVideo();
	}
}

static void Frame(void) {
	if (emu.refreshDelay == 0) {
		UpdateKeyboard();
		CheckControlKeys();
		DisplayVideo();
	}

	if (emu.cycleDelay == 0)
		Cycle();

	if (emu.refreshDelay == 0) {
		if (emu.dt > 0) -- emu.dt;
		if (emu.st > 0) -- emu.st;
		Beep(emu.st > 0);
	}

	UpdateDelays();
}

static void ImportRom(const char *path) {
	FILE *file = fopen(path, "rb");
	if (file == NULL)
		Fatal("Could not load ROM \"%s\": %s", path, strerror(errno));

	fseek(file, 0, SEEK_END);
	importedRomSize = ftell(file);
	rewind(file);

	Assert(importedRomSize < ROM_SIZE, "ROM \"%s\" is too big (%ull)",
	       path, (unsigned long long)importedRomSize);

	memset(importedRom, 0, ROM_SIZE);
	if (fread(importedRom, 1, importedRomSize, file) < importedRomSize)
		Fatal("Encountered an error while loading ROM \"%s\": %s", path, strerror(errno));

	fclose(file);
}

static void LoadFont(void) {
	for (int o = 0; o < (int)sizeof(font); ++ o)
		emu.mem[FONT_ADDR + o] = font[o];
}

static void LoadRom(void) {
	memcpy(emu.mem + PROGRAM_ADDR, importedRom, ROM_SIZE);
	emu.pc = PROGRAM_ADDR;
}

static void StartEmulator(const char *romPath) {
	srand(time(NULL));
	memset(&emu, 0, sizeof(emu));
	emu.waitKey = KEYS_COUNT;

	if (romPath != NULL)
		ImportRom(romPath);

	LoadFont();
	LoadRom();
}

void RunEmulator(const char *romPath) {
	StartEmulator(romPath);
	SetupSystem(WINDOW_SCALE);

	while (!emu.halt)
		Frame();

	printf("\n");
	CleanupSystem();
}
