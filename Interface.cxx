/*
 * Interface.cxx
 *
 * This file is part of the Chip8++ source code.
 * Copyright 2023 Patrick Melo <patrick@patrickmelo.com.br>
 */

#include "Interface.hxx"

// Interface

Interface::Interface(void) :
    Chip8::Interface(),
    isInitialized(false),
    chip8(NULL),
    sdlWindow(NULL),
    sdlWindowSurface(NULL),
    screenSurface(NULL) {
    // Empty
}

// General

bool Interface::Initialize(Chip8* chip8) {
    if (this->isInitialized) {
        return false;
    }

    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO) < 0) {
        Error(Interface::Tag, "%s", SDL_GetError());
        return false;
    }

    this->chip8     = chip8;
    this->sdlWindow = SDL_CreateWindow("Chip8++", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Interface::Width, Interface::Height, SDL_WINDOW_SHOWN);

    if (!this->sdlWindow) {
        Error(Interface::Tag, "%s", SDL_GetError());
        return false;
    }

    this->sdlWindowSurface = SDL_GetWindowSurface(this->sdlWindow);
    this->screenSurface    = SDL_CreateRGBSurface(0, 64, 32, 24, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);

    if (!this->sdlWindowSurface || !this->screenSurface) {
        Error(Interface::Tag, "%s", SDL_GetError());
        SDL_FreeSurface(this->screenSurface);
        SDL_DestroyWindow(this->sdlWindow);
        return false;
    }

    memset(this->screenSurface->pixels, 0, 6144);

    this->chip8->SetVRAM(reinterpret_cast<Chip8::VRAM*>(this->screenSurface->pixels));
    this->chip8->SetInterface(this);

    Info(Interface::Tag, "Initialized.");
    return this->isInitialized = true;
}

void Interface::Finalize(void) {
    if (!this->isInitialized) {
        return;
    }

    SDL_FreeSurface(this->screenSurface);
    SDL_DestroyWindow(this->sdlWindow);
    SDL_Quit();

    this->isInitialized = false;
    printf("Interface finalized.\n");
}

// Chip8

void Interface::Update(void) {
    // Update the events

    SDL_Event sdlEvent;

    while (SDL_PollEvent(&sdlEvent)) {
        switch (sdlEvent.type) {
            case SDL_QUIT: {
                this->chip8->Stop();
                break;
            }
        }
    }

    // Update the screen

    SDL_BlitScaled(this->screenSurface, NULL, this->sdlWindowSurface, NULL);
    SDL_UpdateWindowSurface(this->sdlWindow);
}
