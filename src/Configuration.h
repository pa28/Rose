//
// Created by richard on 2021-03-06.
//

#ifndef ROSE_CONFIGURATION_H
#define ROSE_CONFIGURATION_H

#include <SDL.h>

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
static constexpr uint32_t rmask = 0xff000000;
static constexpr uint32_t rshift = 24U;
static constexpr uint32_t gmask = 0x00ff0000;
static constexpr uint32_t gshift = 16U;
static constexpr uint32_t bmask = 0x0000ff00;
static constexpr uint32_t bshift = 8U;
static constexpr uint32_t amask = 0x000000ff;
static constexpr uint32_t ashift = 0U;
static constexpr uint32_t cmask = rmask | gmask | bmask;
#else
static constexpr uint32_t rmask = 0x000000ffu;
static constexpr uint32_t rshift = 0U;
static constexpr uint32_t gmask = 0x0000ff00u;
static constexpr uint32_t gshift = 8U;
static constexpr uint32_t bmask = 0x00ff0000u;
static constexpr uint32_t bshift = 16U;
static constexpr uint32_t amask = 0xff000000u;
static constexpr uint32_t ashift = 24U;
static constexpr uint32_t cmask = rmask | gmask | bmask;
#endif


#endif //ROSE_CONFIGURATION_H
