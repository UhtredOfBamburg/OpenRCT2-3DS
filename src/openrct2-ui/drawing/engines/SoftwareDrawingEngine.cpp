#pragma region Copyright (c) 2014-2017 OpenRCT2 Developers
/*****************************************************************************
 * OpenRCT2, an open source clone of Roller Coaster Tycoon 2.
 *
 * OpenRCT2 is the work of many authors, a full list can be found in contributors.md
 * For more information, visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * A full copy of the GNU General Public License can be found in licence.txt
 *****************************************************************************/
#pragma endregion

#include <algorithm>
#include <openrct2/common.h>
#include <SDL2/SDL.h>
#ifdef __psp2__
#include <SDL2/SDL_image.h>
#endif
#ifdef __3DS__
#include <3ds.h>
#endif
#include <openrct2/config/Config.h>
#include <openrct2/core/Guard.hpp>
#include <openrct2/drawing/IDrawingEngine.h>
#include <openrct2/drawing/X8DrawingEngine.h>
#include <openrct2/Game.h>
#include <openrct2/ui/UiContext.h>
#include "DrawingEngines.h"

using namespace OpenRCT2;
using namespace OpenRCT2::Drawing;
using namespace OpenRCT2::Ui;

class SoftwareDrawingEngine final : public X8DrawingEngine
{
private:
    IUiContext * const  _uiContext;
    SDL_Window *        _window         = nullptr;
    SDL_Surface *       _surface        = nullptr;
    SDL_Surface *       _cursor         = nullptr;
    SDL_Surface *       _RGBASurface    = nullptr;
    SDL_Palette *       _palette        = nullptr;
#ifdef __3DS__
    uint32              _paletteRGBA8[256] = { 0 };
#endif

public:
    explicit SoftwareDrawingEngine(IUiContext * uiContext)
        : X8DrawingEngine(uiContext),
          _uiContext(uiContext)
    {
        _window = (SDL_Window *)_uiContext->GetWindow();
#ifdef __psp2__
        _cursor = IMG_Load("app0:cursor.png");
#elif !defined(__3DS__)
        _cursor = SDL_LoadBMP("./cursor.bmp");
#endif
    }

    ~SoftwareDrawingEngine() override
    {
        SDL_FreeSurface(_surface);
        SDL_FreeSurface(_cursor);
        SDL_FreeSurface(_RGBASurface);
        SDL_FreePalette(_palette);
    }

    void Initialise() override
    {
    }

    void Resize(uint32 width, uint32 height) override
    {
#ifdef __3DS__
        ConfigureBits(width, height, width);
#else
        SDL_FreeSurface(_surface);
        SDL_FreeSurface(_RGBASurface);
        SDL_FreePalette(_palette);

        _surface = SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0);
        _RGBASurface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
        SDL_SetSurfaceBlendMode(_RGBASurface, SDL_BLENDMODE_NONE);
        _palette = SDL_AllocPalette(256);

        if (_surface == nullptr ||
            _palette == nullptr ||
            _RGBASurface == nullptr)
        {
            log_fatal("%p || %p || %p == nullptr %s", _surface, _palette, _RGBASurface, SDL_GetError());
            exit(-1);
        }

        if (SDL_SetSurfacePalette(_surface, _palette))
        {
            log_fatal("SDL_SetSurfacePalette failed %s", SDL_GetError());
            exit(-1);
        }

        ConfigureBits(width, height, _surface->pitch);
#endif
    }

    void SetPalette(const rct_palette_entry * palette) override
    {
#ifdef __3DS__
        for (sint32 i = 0; i < 256; i++)
        {
            uint32 r = palette[i].red;
            uint32 g = palette[i].green;
            uint32 b = palette[i].blue;
            _paletteRGBA8[i] = (r << 24) | (g << 16) | (b << 8) | 0xFFu;
        }
#else
        SDL_Surface * windowSurface = SDL_GetWindowSurface(_window);
        if (windowSurface != nullptr && _palette != nullptr)
        {
            SDL_Colour colours[256];
            for (sint32 i = 0; i < 256; i++) {
                colours[i].r = palette[i].red;
                colours[i].g = palette[i].green;
                colours[i].b = palette[i].blue;
                colours[i].a = palette[i].alpha;
            }
            SDL_SetPaletteColors(_palette, colours, 0, 256);
        }
#endif
    }

    void EndDraw() override
    {
        Display();
    }

private:
#ifdef __3DS__
    static void DrawCursor3DS(uint32 * fb, sint32 fbWidth, sint32 fbHeight, sint32 cx, sint32 cy)
    {
        // Classic 10x14 pointer bitmap (1 = black border, 2 = white fill)
        static const uint8 cursorShape[14][10] = {
            { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
            { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
            { 1, 2, 1, 0, 0, 0, 0, 0, 0, 0 },
            { 1, 2, 2, 1, 0, 0, 0, 0, 0, 0 },
            { 1, 2, 2, 2, 1, 0, 0, 0, 0, 0 },
            { 1, 2, 2, 2, 2, 1, 0, 0, 0, 0 },
            { 1, 2, 2, 2, 2, 2, 1, 0, 0, 0 },
            { 1, 2, 2, 2, 2, 2, 2, 1, 0, 0 },
            { 1, 2, 2, 2, 2, 2, 2, 2, 1, 0 },
            { 1, 2, 2, 2, 2, 1, 1, 1, 1, 0 },
            { 1, 2, 2, 1, 2, 2, 1, 0, 0, 0 },
            { 1, 2, 1, 0, 1, 2, 2, 1, 0, 0 },
            { 1, 1, 0, 0, 1, 2, 2, 1, 0, 0 },
            { 0, 0, 0, 0, 0, 1, 1, 0, 0, 0 },
        };

        for (sint32 dy = 0; dy < 14; dy++)
        {
            sint32 py = cy + dy;
            if (py < 0 || py >= fbHeight)
                continue;
            for (sint32 dx = 0; dx < 10; dx++)
            {
                uint8 v = cursorShape[dy][dx];
                if (v == 0)
                    continue;
                sint32 px = cx + dx;
                if (px < 0 || px >= fbWidth)
                    continue;
                fb[px * 240 + (239 - py)] = (v == 1) ? 0x000000FFu : 0xFFFFFFFFu;
            }
        }
    }
#endif

    void Display()
    {
#ifdef __3DS__
        if (_bits == nullptr)
        {
            return;
        }

        u16 topFbW = 0, topFbH = 0;
        u16 botFbW = 0, botFbH = 0;
        uint32 * topFb = (uint32 *)gfxGetFramebuffer(GFX_TOP, GFX_LEFT, &topFbW, &topFbH);
        uint32 * botFb = (uint32 *)gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, &botFbW, &botFbH);

        const uint32 pitch = _pitch;
        const uint32 maxW = std::min<uint32>(_width, 400);
        const uint32 maxH = std::min<uint32>(_height, 480);

        if (topFb != nullptr)
        {
            const uint32 topH = std::min<uint32>(maxH, 240);
            for (uint32 x = 0; x < maxW; x++)
            {
                uint32 * dstCol = topFb + x * 240;
                const uint8 * srcCol = _bits + x;
                for (uint32 y = 0; y < topH; y++)
                {
                    dstCol[239 - y] = _paletteRGBA8[srcCol[y * pitch]];
                }
            }
        }

        if (botFb != nullptr && maxH > 240)
        {
            const uint32 botH = std::min<uint32>(maxH - 240, 240);
            for (uint32 bx = 0; bx < 320; bx++)
            {
                uint32 * dstCol = botFb + bx * 240;
                uint32 srcX = bx + 40;
                if (srcX < maxW)
                {
                    const uint8 * srcCol = _bits + 240 * pitch + srcX;
                    for (uint32 by = 0; by < botH; by++)
                    {
                        dstCol[239 - by] = _paletteRGBA8[srcCol[by * pitch]];
                    }
                }
            }
        }

        // Native 3DS touch interface: no artificial PC mouse pointer drawn

        if (topFb != nullptr)
        {
            GSPGPU_FlushDataCache(topFb, 400 * 240 * 4);
            gfxScreenSwapBuffers(GFX_TOP, false);
        }
        if (botFb != nullptr)
        {
            GSPGPU_FlushDataCache(botFb, 320 * 240 * 4);
            gfxScreenSwapBuffers(GFX_BOTTOM, false);
        }
        gspWaitForVBlank();
#else
        // Lock the surface before setting its pixels
        if (SDL_MUSTLOCK(_surface))
        {
            if (SDL_LockSurface(_surface) < 0)
            {
                log_error("locking failed %s", SDL_GetError());
                return;
            }
        }

        // Copy pixels from the virtual screen buffer to the surface
        std::copy_n(_bits, _surface->pitch * _surface->h, (uint8 *)_surface->pixels);

        // Unlock the surface
        if (SDL_MUSTLOCK(_surface))
        {
            SDL_UnlockSurface(_surface);
        }

        // Copy the surface to the window
        if (gConfigGeneral.window_scale == 1 || gConfigGeneral.window_scale <= 0)
        {
            SDL_Surface * windowSurface = SDL_GetWindowSurface(_window);
           
            if (SDL_BlitSurface(_surface, nullptr, windowSurface, nullptr))
            {
                log_fatal("SDL_BlitSurface %s", SDL_GetError());
                exit(1);
            }
            SDL_Rect r;
            // printf("%d\n",  _uiContext->GetCursorState()->x);
            r.x = _uiContext->GetCursorState()->abs_x;
            r.y = _uiContext->GetCursorState()->abs_y;
            SDL_BlitSurface(_cursor, nullptr, windowSurface, &r);
        }
        else
        {
            // first blit to rgba surface to change the pixel format
            if (SDL_BlitSurface(_surface, nullptr, _RGBASurface, nullptr))
            {
                log_fatal("SDL_BlitSurface %s", SDL_GetError());
                exit(1);
            }

            // then scale to window size. Without changing to RGBA first, SDL complains
            // about blit configurations being incompatible.
            if (SDL_BlitScaled(_surface, nullptr, SDL_GetWindowSurface(_window), nullptr))
            {
                log_fatal("SDL_BlitScaled %s", SDL_GetError());
                exit(1);
            }
        }
        if (SDL_UpdateWindowSurface(_window))
        {
            log_fatal("SDL_UpdateWindowSurface %s", SDL_GetError());
            exit(1);
        }
#endif
    }
};

IDrawingEngine * OpenRCT2::Ui::CreateSoftwareDrawingEngine(IUiContext * uiContext)
{
    return new SoftwareDrawingEngine(uiContext);
}
