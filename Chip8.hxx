/*
 * Chip8.hxx
 *
 * This file is part of the Chip8++ source code.
 * Copyright 2023 Patrick Melo <patrick@patrickmelo.com.br>
 */

#ifndef CHIP8_H
#define CHIP8_H

#include "Core.hxx"

// Chip8

class Chip8 {
    public:
        Chip8(void);

        // Types
        typedef uint8 RAM[4096];
        typedef uint8 VRAM[6144];
        typedef void (Chip8::*Operation)(void);

        class Interface {
            public:
                virtual ~Interface() {};

                // General
                virtual void Update(void) = 0;
        };

        // Constants
        static constexpr charconst Tag                 = "Chip8";
        static constexpr uint16    FontStartAddress    = 0x000;
        static constexpr uint16    ProgramStartAddress = 0x200;    // 512

        // Utilities
        static bool LoadProgram(const string filePath, RAM& programMemory);

        // CPU
        void Run(void);
        void Stop(void);
        void Reset(void);

        // Memory
        void SetRAM(RAM* mainMemory);
        void SetVRAM(VRAM* videoMemory);

        // Interface
        void SetInterface(Interface* newInterface);

    private:
        // CPU
        uint16  addressRegister;
        uint8   cpuRegisters[16];
        bool    cpuWait;
        timeval lastCpuTick;

        // Memory
        RAM*  mainMemory;
        VRAM* videoMemory;

        // Execution
        bool   isRunning;
        uint16 programCounter;
        uint8  stackPointer;
        uint16 callStack[16];
        uint16 opCode;

        void DebugOpCode(const string debugMessage, ...);
        void Halt(const string haltMessage, ...);

        // Timers
        uint8   delayTimer;
        uint8   soundTimer;
        timeval lastTick;

        void Tick(void);

        // Operations
        Operation operationsTable[16];

        void Op0x0(void);
        void Op0x1(void);
        void Op0x2(void);
        void Op0x3(void);
        void Op0x4(void);
        void Op0x5(void);
        void Op0x6(void);
        void Op0x7(void);
        void Op0x8(void);
        void Op0x9(void);
        void Op0xA(void);
        void Op0xB(void);
        void Op0xC(void);
        void Op0xD(void);
        void Op0xE(void);
        void Op0xF(void);

        // Interface
        Interface* currentInterface;
};

#endif    // CHIP8_H
