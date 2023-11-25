/*
 * Main.cxx
 *
 * This file is part of the Chip8++ source code.
 * Copyright 2023 Patrick Melo <patrick@patrickmelo.com.br>
 */

#include "Chip8.hxx"
#include "Core.hxx"
#include "Interface.hxx"

int main(int numberOfArguments, char** argumentsValues) {
    Chip8*     chip8 = new Chip8();
    Chip8::RAM chip8Memory;

    if (!Chip8::LoadProgram(argumentsValues[1], chip8Memory)) {
        delete chip8;
        return 1;
    }

    Interface* chip8Interface = new Interface();

    if (!chip8Interface->Initialize(chip8)) {
        delete chip8Interface;
        delete chip8;
        return 1;
    }

    chip8->SetRAM(&chip8Memory);
    chip8->Run();

    delete chip8;
    delete chip8Interface;

    return 0;
}
