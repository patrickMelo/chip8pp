/*
 * Chip8.cxx
 *
 * This file is part of the Chip8++ source code.
 * Copyright 2023 Patrick Melo <patrick@patrickmelo.com.br>
 */

#include "Chip8.hxx"

// Contants

const uint8 DefaultFontData[80] =
    {
        0xF0, 0x90, 0x90, 0x90, 0xF0,
        0x20, 0x60, 0x20, 0x20, 0x70,
        0xF0, 0x10, 0xF0, 0x80, 0xF0,
        0xF0, 0x10, 0xF0, 0x80, 0xF0,
        0xF0, 0x10, 0xF0, 0x80, 0xF0,
        0xF0, 0x10, 0xF0, 0x80, 0xF0,
        0xF0, 0x10, 0xF0, 0x80, 0xF0,
        0xF0, 0x10, 0xF0, 0x80, 0xF0,
        0xF0, 0x10, 0xF0, 0x80, 0xF0,
        0xF0, 0x10, 0xF0, 0x80, 0xF0,
        0xF0, 0x10, 0xF0, 0x80, 0xF0,
        0xF0, 0x10, 0xF0, 0x80, 0xF0,
        0xF0, 0x10, 0xF0, 0x80, 0xF0,
        0xF0, 0x10, 0xF0, 0x80, 0xF0,
        0xF0, 0x10, 0xF0, 0x80, 0xF0,
        0xF0, 0x10, 0xF0, 0x80, 0xF0};

// Chip8

Chip8::Chip8(void) :
    addressRegister(0),
    cpuWait(false),
    mainMemory(NULL),
    videoMemory(NULL),
    isRunning(false),
    programCounter(0),
    stackPointer(0),
    opCode(0),
    delayTimer(0),
    soundTimer(0),
    currentInterface(NULL) {
    memset(this->cpuRegisters, 0, sizeof(this->cpuRegisters));
    memset(this->callStack, 0, sizeof(this->callStack));

    this->operationsTable[0x0] = &Chip8::Op0x0;
    this->operationsTable[0x1] = &Chip8::Op0x1;
    this->operationsTable[0x2] = &Chip8::Op0x2;
    this->operationsTable[0x3] = &Chip8::Op0x3;
    this->operationsTable[0x4] = &Chip8::Op0x4;
    this->operationsTable[0x5] = &Chip8::Op0x5;
    this->operationsTable[0x6] = &Chip8::Op0x6;
    this->operationsTable[0x7] = &Chip8::Op0x7;
    this->operationsTable[0x8] = &Chip8::Op0x8;
    this->operationsTable[0x9] = &Chip8::Op0x9;
    this->operationsTable[0xA] = &Chip8::Op0xA;
    this->operationsTable[0xB] = &Chip8::Op0xB;
    this->operationsTable[0xC] = &Chip8::Op0xC;
    this->operationsTable[0xD] = &Chip8::Op0xD;
    this->operationsTable[0xE] = &Chip8::Op0xE;
    this->operationsTable[0xF] = &Chip8::Op0xF;
}

// Utilities

bool Chip8::LoadProgram(const string filePath, RAM& programMemory) {
    FILE* programFile = fopen(filePath.c_str(), "rb");

    if (!programFile) {
        Error(Chip8::Tag, "Could not open the program file.");
        return false;
    }

    fseeko(programFile, 0, SEEK_END);
    uint fileSize = ftello(programFile);
    fseeko(programFile, 0, SEEK_SET);

    if (fread(&programMemory[Chip8::ProgramStartAddress], fileSize, 1, programFile) != 1) {
        Error(Chip8::Tag, "Could not read the program file.");
        fclose(programFile);
        return false;
    }

    memcpy(&programMemory[Chip8::FontStartAddress], DefaultFontData, sizeof(DefaultFontData));

    fclose(programFile);
    Info(Chip8::Tag, "Program loaded from %s", filePath.c_str());
    return true;
}

// CPU

void Chip8::Run(void) {
    if (!this->currentInterface) {
        Error(Chip8::Tag, "Cannot run without an interface.");
        return;
    }

    if ((!this->mainMemory) || (!this->videoMemory)) {
        Error(Chip8::Tag, "Cannot run without RAM and VRAM.");
        return;
    }

    this->Reset();
    this->isRunning = true;

    while (this->isRunning) {
        this->Tick();

        if (this->cpuWait) {
            continue;
        }

        // Execute the next opCode

        this->opCode = ((*this->mainMemory)[this->programCounter] << 8) | ((*this->mainMemory)[this->programCounter + 1]);
        (this->*operationsTable[this->opCode >> 12])();
        this->cpuWait = true;
    }
}

void Chip8::Stop(void) {
    this->isRunning = false;
}

void Chip8::Reset(void) {
    this->addressRegister = 0;
    this->programCounter  = Chip8::ProgramStartAddress;
    this->stackPointer    = 0;
    this->opCode          = 0;
    this->delayTimer      = 0;
    this->soundTimer      = 0;
    this->cpuWait         = false;

    gettimeofday(&(this)->lastTick, NULL);
    gettimeofday(&(this)->lastCpuTick, NULL);

    memset(this->cpuRegisters, 0, sizeof(this->cpuRegisters));
    memset(this->callStack, 0, sizeof(this->callStack));
    memset(*this->videoMemory, 0, sizeof(*this->videoMemory));
}

// Memory

void Chip8::SetRAM(RAM* mainMemory) {
    this->mainMemory = mainMemory;
}

void Chip8::SetVRAM(VRAM* videoMemory) {
    this->videoMemory = videoMemory;
}

// Interface

void Chip8::SetInterface(Interface* newInterface) {
    this->currentInterface = newInterface;
}

// Execution

#ifdef CHIP8_DEBUG

inline void Chip8::DebugOpCode(const string debugMessage, ...) {
    if (this->delayTimer > 0) {
        return;
    }

    static char    messageBuffer[4097];
    static va_list messageArguments;

    va_start(messageArguments, debugMessage);
    vsprintf(messageBuffer, debugMessage.c_str(), messageArguments);
    va_end(messageArguments);

    Debug(Chip8::Tag, "$%03x (%02x %02x): %s", this->programCounter, this->opCode >> 8, this->opCode & 0xFF, messageBuffer);
}

#else

inline void Chip8::DebugOpCode(const string debugMessage, ...) {
    // Empty
}

#endif    // CHIP8_DEBUG

void Chip8::Halt(const string haltMessage, ...) {
    static char    messageBuffer[4097];
    static va_list messageArguments;

    va_start(messageArguments, haltMessage);
    vsprintf(messageBuffer, haltMessage.c_str(), messageArguments);
    va_end(messageArguments);

    Error(Chip8::Tag, ">>> HALTED AT $%03x >>> %s", this->programCounter, messageBuffer);
    this->isRunning = false;
}

// Timers

void Chip8::Tick(void) {
    static timeval currentTime;
    static uint    elapsedTime;

    gettimeofday(&currentTime, NULL);
    elapsedTime = ((currentTime.tv_sec * 1000000) + currentTime.tv_usec) - ((this->lastTick.tv_sec * 1000000) + this->lastTick.tv_usec);

    if (elapsedTime >= 16000) {
        this->currentInterface->Update();

        if (this->soundTimer > 0) {
            this->soundTimer--;
        }

        if (this->delayTimer > 0) {
            this->delayTimer--;
        }

        gettimeofday(&(this)->lastTick, NULL);
    }

    elapsedTime = ((currentTime.tv_sec * 1000000) + currentTime.tv_usec) - ((this->lastCpuTick.tv_sec * 1000000) + this->lastCpuTick.tv_usec);

    if (elapsedTime >= 2000) {
        this->cpuWait = false;
        gettimeofday(&(this)->lastCpuTick, NULL);
    }
}

// Operations

void Chip8::Op0x0(void) {
    switch (this->opCode & 0xFF) {
        case 0xE0: {
            this->DebugOpCode("CLS");
            memset(*this->videoMemory, 0, sizeof(*this->videoMemory));
            break;
        }

        case 0xEE: {
            if (this->stackPointer == 0) {
                this->Halt("No subroutine to return from.");
                return;
            }

            this->DebugOpCode("RTS");
            this->programCounter = this->callStack[--this->stackPointer];
            break;
        }

        default: {
            this->Halt("Unknown opCode: %02x %02x.", this->opCode >> 8, this->opCode & 0xFF);
            return;
        }
    }

    this->programCounter += 2;
}

void Chip8::Op0x1(void) {
    static uint16 jumpAddress;

    jumpAddress = this->opCode & 0xFFF;

    this->DebugOpCode("JMP $%03x", jumpAddress);
    this->programCounter = jumpAddress;
}

void Chip8::Op0x2(void) {
    if (this->stackPointer == 15) {
        this->Halt("Stack overflow.");
        return;
    }

    static uint16 jumpAddress;

    jumpAddress = this->opCode & 0xFFF;

    this->DebugOpCode("JSR $%03x", jumpAddress);
    this->callStack[this->stackPointer++] = this->programCounter + 2;
    this->programCounter                  = jumpAddress;
}

void Chip8::Op0x3(void) {
    static uint8 registerX, testValue;

    registerX = (this->opCode >> 8) & 0xF;
    testValue = this->opCode & 0xFF;

    this->DebugOpCode("SKEQ V%X, $%02x", registerX, testValue);
    this->programCounter += ((this->cpuRegisters[registerX] == testValue) * 2) + 2;
}

void Chip8::Op0x4(void) {
    static uint8 registerX, testValue;

    registerX = (this->opCode >> 8) & 0xF;
    testValue = this->opCode & 0xFF;

    this->DebugOpCode("SKNE V%X, $%02x", registerX, testValue);
    this->programCounter += ((this->cpuRegisters[registerX] != testValue) * 2) + 2;
}

void Chip8::Op0x5(void) {
    static uint8 registerX, registerY;

    registerX = (this->opCode >> 8) & 0xF;
    registerY = (this->opCode >> 4) & 0xF;

    this->DebugOpCode("SKEQ V%X, V%X", registerX, registerY);
    this->programCounter += ((this->cpuRegisters[registerX] == this->cpuRegisters[registerY]) * 2) + 2;
}

void Chip8::Op0x6(void) {
    static uint8 registerX, newValue;

    registerX = (this->opCode >> 8) & 0xF;
    newValue  = this->opCode & 0xFF;

    this->DebugOpCode("MOV V%X, $%02x", registerX, newValue);
    this->cpuRegisters[registerX] = newValue;
    this->programCounter += 2;
}

void Chip8::Op0x7(void) {
    static uint8 registerX, addValue;

    registerX = (this->opCode >> 8) & 0xF;
    addValue  = this->opCode & 0xFF;

    this->DebugOpCode("ADD V%X, $%02x", registerX, addValue);
    this->cpuRegisters[registerX] += addValue;
    this->programCounter += 2;
}

void Chip8::Op0x8(void) {
    static uint8 registerX, registerY;

    registerX = (this->opCode >> 8) & 0xF;
    registerY = (this->opCode >> 4) & 0xF;

    switch (this->opCode & 0xF) {
        case 0x0: {
            this->DebugOpCode("MOV V%X, V%X", registerX, registerY);
            this->cpuRegisters[registerX] = this->cpuRegisters[registerY];
            break;
        }

        case 0x1: {
            this->DebugOpCode("OR V%X, V%X", registerX, registerY);
            this->cpuRegisters[registerX] |= this->cpuRegisters[registerY];
            break;
        }

        case 0x2: {
            this->DebugOpCode("AND V%X, V%X", registerX, registerY);
            this->cpuRegisters[registerX] &= this->cpuRegisters[registerY];
            break;
        }

        case 0x3: {
            this->DebugOpCode("XOR V%X, V%X", registerX, registerY);
            this->cpuRegisters[registerX] ^= this->cpuRegisters[registerY];
            break;
        }

        case 0x4: {
            this->DebugOpCode("ADD V%X, V%X", registerX, registerY);

            this->cpuRegisters[0xF] = UINT16(this->cpuRegisters[registerX] + this->cpuRegisters[registerY]) > 255;
            this->cpuRegisters[registerX] += this->cpuRegisters[registerY];

            break;
        }

        case 0x5: {
            this->DebugOpCode("SUB V%X, V%X", registerX, registerY);

            this->cpuRegisters[0xF] = !(this->cpuRegisters[registerX] < this->cpuRegisters[registerY]);
            this->cpuRegisters[registerX] -= this->cpuRegisters[registerY];

            break;
        }

        case 0x6: {
            this->DebugOpCode("SHR V%X", registerX);

            this->cpuRegisters[0xF] = this->cpuRegisters[registerX] & 0x1;
            this->cpuRegisters[registerX] >>= 1;

            break;
        }

        case 0x7: {
            this->DebugOpCode("RSB V%X, V%X", registerX, registerY);

            this->cpuRegisters[0xF]       = !(this->cpuRegisters[registerY] < this->cpuRegisters[registerX]);
            this->cpuRegisters[registerX] = this->cpuRegisters[registerY] - this->cpuRegisters[registerX];
            break;
        }

        case 0xE: {
            this->DebugOpCode("SHL V%X", registerX);

            this->cpuRegisters[0xF] = this->cpuRegisters[registerX] >> 7;
            this->cpuRegisters[registerX] <<= 1;
            break;
        }

        default: {
            this->Halt("Unknown opCode: %02x %02x.", this->opCode >> 8, this->opCode & 0xFF);
            return;
        }
    }

    this->programCounter += 2;
}

void Chip8::Op0x9(void) {
    static uint8 registerX, registerY;

    registerX = (this->opCode >> 8) & 0xF;
    registerY = (this->opCode >> 4) & 0xF;

    this->DebugOpCode("SKNE V%X, V%X", registerX, registerY);
    this->programCounter += ((this->cpuRegisters[registerX] != this->cpuRegisters[registerY]) * 2) + 2;
}

void Chip8::Op0xA(void) {
    static uint16 newAddress;

    newAddress = this->opCode & 0xFFF;

    this->DebugOpCode("MVI $%03x", newAddress);
    this->addressRegister = newAddress;
    this->programCounter += 2;
}

void Chip8::Op0xB(void) {
    static uint16 jumpAddress;

    jumpAddress = this->opCode & 0xFFF;

    this->DebugOpCode("JMI $%03x", jumpAddress);
    this->programCounter = jumpAddress + this->cpuRegisters[0];
}

void Chip8::Op0xC(void) {
    static uint8 registerX, maskValue;

    registerX = (this->opCode >> 8) & 0xF;
    maskValue = this->opCode & 0xFF;

    this->DebugOpCode("RAND V%X, $%02x", registerX, maskValue);
    this->cpuRegisters[registerX] = (rand() % 256) & maskValue;
    this->programCounter += 2;
}

void Chip8::Op0xD(void) {
    static uint8 registerX, registerY, spriteHeight;

    registerX    = (this->opCode >> 8) & 0xF;
    registerY    = (this->opCode >> 4) & 0xF;
    spriteHeight = this->opCode & 0xF;
    ;

    this->DebugOpCode("SPRITE V%X, V%X, $%x", registerX, registerY, spriteHeight);

    uint16 lineAddress = this->addressRegister;
    uint8  currentLine = (*this->mainMemory)[lineAddress++];
    uint   startingX   = this->cpuRegisters[registerX];
    uint   startingY   = this->cpuRegisters[registerY];
    uint   xPosition   = startingX;
    uint   yPosition   = startingY;
    uint8  pixelValue;
    uint8  screenValue;

    this->cpuRegisters[0xF] = 0;

    for (uint8 yPixels = 0; yPixels < spriteHeight; ++yPixels) {
        if (yPosition > 32) {
            yPosition -= 32;
        }

        xPosition = startingX;

        for (uint8 xPixels = 0; xPixels < 8; ++xPixels) {
            if (xPosition > 64) {
                xPosition -= 64;
            }

            pixelValue  = (currentLine >> (7 - xPixels)) & 0x1;
            screenValue = (*this->videoMemory)[((yPosition * 64) + xPosition) * 3] / 255;

            if ((pixelValue == screenValue) && (screenValue == 0x1)) {
                this->cpuRegisters[0xF] = 1;
            }

            memset(&(*this->videoMemory)[((yPosition * 64) + xPosition) * 3], (screenValue ^ pixelValue) * 255, 3);
            xPosition++;
        }

        currentLine = (*this->mainMemory)[lineAddress++];
        yPosition++;
    }

    this->programCounter += 2;
}

void Chip8::Op0xE(void) {
    static uint8 registerX;

    registerX = (this->opCode >> 8) & 0xF;

    switch (this->opCode & 0xFF) {
        case 0x9E: {
            this->DebugOpCode("SKPR V%X", registerX);
            // this->currentInterface->ReadKeys( this->keyStates );
            // this->programCounter += this->keyStates[ this->cpuRegisters[ registerIndex ] ] ? 4 : 2;

            // TODO
            this->programCounter += 2;
            break;
        }

        case 0xA1: {
            this->DebugOpCode("SKUP V%X", registerX);
            // this->currentInterface->ReadKeys( this->keyStates );
            // this->programCounter += this->keyStates[ this->cpuRegisters[ registerIndex ] ] ? 2 : 4;

            // TODO
            this->programCounter += 4;
            break;
        }

        default: {
            this->Halt("Unknown opCode: %02x %02x.", this->opCode >> 8, this->opCode & 0xFF);
            return;
        }
    }
}

void Chip8::Op0xF(void) {
    static uint8 registerX;

    registerX = (this->opCode >> 8) & 0xF;

    switch (this->opCode & 0xFF) {
        case 0x07: {
            this->DebugOpCode("GDELAY V%X", registerX);
            this->cpuRegisters[registerX] = this->delayTimer;
            break;
        }

        case 0x0A: {
            // this->waitingForKey = true;
            // this->keyRegister = registerIndex;

            // TODO
            break;
        }

        case 0x15: {
            this->DebugOpCode("SDELAY V%X", registerX);
            this->delayTimer = this->cpuRegisters[registerX];
            break;
        }

        case 0x18: {
            this->DebugOpCode("SSOUND V%X", registerX);
            this->soundTimer = this->cpuRegisters[registerX];
            break;
        }

        case 0x1E: {
            this->DebugOpCode("ADI V%X", registerX);
            this->addressRegister += this->cpuRegisters[registerX];
            break;
        }

        case 0x29: {
            this->DebugOpCode("FONT V%X", registerX);
            this->addressRegister = Chip8::FontStartAddress + (this->cpuRegisters[registerX] * 5);
            break;
        }

        case 0x33: {
            this->DebugOpCode("BCD V%X", registerX);

            (*this->mainMemory)[this->addressRegister]     = this->cpuRegisters[registerX] / 100;
            (*this->mainMemory)[this->addressRegister + 1] = (this->cpuRegisters[registerX] % 100) / 10;
            (*this->mainMemory)[this->addressRegister + 2] = (this->cpuRegisters[registerX] % 100) % 10;
            break;
        }

        case 0x55: {
            this->DebugOpCode("STR V%X", registerX);
            memcpy(&(*this->mainMemory)[Chip8::ProgramStartAddress + this->addressRegister], this->cpuRegisters, registerX + 1);
            break;
        }

        case 0x65: {
            this->DebugOpCode("LDR V%X", registerX);
            memcpy(this->cpuRegisters, &(*this->mainMemory)[Chip8::ProgramStartAddress + this->addressRegister], registerX + 1);
            break;
        }

        default: {
            this->Halt("Unknown opCode: %02x %02x.", this->opCode >> 8, this->opCode & 0xFF);
            return;
        }
    }

    this->programCounter += 2;
}
