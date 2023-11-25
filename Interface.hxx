/*
 * Interface.hxx
 *
 * This file is part of the Chip8++ source code.
 * Copyright 2023 Patrick Melo <patrick@patrickmelo.com.br>
 */

#ifndef CHIP8_INTERFACE_H
#define CHIP8_INTERFACE_H

#include "Chip8.hxx"

#include <SDL2/SDL.h>

// Interface

class Interface : public Chip8::Interface {
    public:
        Interface(void);

        // Constants
        static constexpr charconst Tag    = "Interface";
        static constexpr uint      Width  = 720;
        static constexpr uint      Height = 360;

        // General
        bool Initialize(Chip8* chip8);
        void Finalize(void);
        void Update(void);

    private:
        // General
        bool isInitialized;

        // Chip8
        Chip8* chip8;

        // SDL
        SDL_Window*  sdlWindow;
        SDL_Surface* sdlWindowSurface;
        SDL_Surface* screenSurface;
};

#endif    // CHIP8_INTERFACE_H
