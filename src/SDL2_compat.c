//
// Created by lexter on 14.03.24.
//

#include "stdio.h"
#include "stdarg.h"

void SDL_Log(const char * data, ...) {
    printf(data);
}

void SDL_LogDebug(int type, const char * data, va_list args) {
    printf(data, args);
}

void SDL_LogError(int type, const char * data, ...) {
    printf(data);
}

void SDL_LogCritical(int type, const char * data, ...) {
    printf(data);
}

char* SDL_GetPrefPath(const char *org, const char *app) {

}

char* SDL_GetBasePath() {

}

void SDL_LogSetAllPriority(int type) {

};
