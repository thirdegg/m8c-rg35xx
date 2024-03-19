//
// Created by lexter on 14.03.24.
//

#ifndef M8C_SDL2_COMPAT_H
#define M8C_SDL2_COMPAT_H

#include "stdio.h"

static const int SDL_LOG_CATEGORY_APPLICATION = 1;
static const int SDL_LOG_CATEGORY_ERROR = 2;
static const int SDL_LOG_CATEGORY_SYSTEM = 3;
static const int SDL_LOG_PRIORITY_INFO = 4;
static const int SDL_LOG_CATEGORY_VIDEO = 4;
static const int SDL_LOG_CATEGORY_INPUT = 5;


void SDL_Log(const char * data, ...);

void SDL_LogDebug(int type, const char * data, ...);

void SDL_LogError(int type, const char * data, ...);

void SDL_LogCritical(int type, const char * data, ...);


#endif //M8C_SDL2_COMPAT_H
