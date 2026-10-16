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
#include <openrct2/Context.h>
#include <openrct2/drawing/Drawing.h>
#include <openrct2/interface/Colour.h>
#include <openrct2/localisation/Localisation.h>
#include <openrct2/localisation/StringIds.h>
#include <openrct2/management/Finance.h>
#include <openrct2/OpenRCT2.h>
#include <openrct2/world/Footpath.h>
#include <openrct2/world/Park.h>
#include <openrct2-ui/interface/Viewport.h>
#include <openrct2-ui/interface/Widget.h>
#include <openrct2-ui/windows/Window.h>

#ifdef __3DS__
extern uint8 g3dsDualScreenMode;
#endif

static rct_widget window_main_widgets[] = {
    { WWT_VIEWPORT, 0, 0x0000, -1, 0x0000, -1, STR_VIEWPORT, 0xFFFF },
    { WIDGETS_END },
};

void window_main_paint(rct_window *w, rct_drawpixelinfo *dpi);

static rct_window_event_list window_main_events = {
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    window_main_paint,
    nullptr
};

/**
 * Creates the main window that holds the main viewport.
 *  rct2: 0x0066B3E8
 */
rct_window * window_main_open()
{
    window_main_widgets[0].right = context_get_width();
    window_main_widgets[0].bottom = context_get_height();
    rct_window * window = window_create(
        0, 0,
        window_main_widgets[0].right, window_main_widgets[0].bottom,
        &window_main_events,
        WC_MAIN_WINDOW,
        WF_STICK_TO_BACK
    );
    window->widgets = window_main_widgets;

#ifdef __3DS__
    bool isGameplay = !(gScreenFlags & SCREEN_FLAGS_TITLE_DEMO);
    viewport_create(
        window,
        isGameplay ? 40 : 0,
        isGameplay ? 240 : 0,
        isGameplay ? 320 : 400,
        240,
        0, 0x0FFF, 0x0FFF, 0, 0x1, -1
    );
#else
    viewport_create(window, window->x, window->y, window->width, window->height, 0,0x0FFF,0x0FFF, 0, 0x1, -1);
#endif
    window->viewport->flags |= VIEWPORT_FLAG_SOUND_ON;
    gCurrentRotation = 0;
    gShowGridLinesRefCount = 0;
    gShowLandRightsRefCount = 0;
    gShowConstuctionRightsRefCount = 0;
    gFootpathSelectedType = 0;

    return window;
}


/**
 *
 *  rct2: 0x66CCAE
 * This function immediately jumps to 0x00685BE1 this is the second function
 * decompiled.
 */
void window_main_paint(rct_window *w, rct_drawpixelinfo *dpi)
{
#ifdef __3DS__
    if (w->viewport == nullptr)
        return;

    bool isTitleDemo = (gScreenFlags & SCREEN_FLAGS_TITLE_DEMO) != 0;
    sint32 dpiRight = dpi->x + dpi->width;
    sint32 dpiBottom = dpi->y + dpi->height;

    if (isTitleDemo)
    {
        // Title Screen Demo: Top Screen (y < 240) = Full 400x240 Park Demo
        //                    Bottom Screen (y >= 240) = Touch Title Menu Background
        if (dpi->y < 240)
        {
            sint32 topClipBottom = std::min<sint32>(240, dpiBottom);
            viewport_render(dpi, w->viewport, dpi->x, dpi->y, dpiRight, topClipBottom);
        }

        if (dpiBottom > 240)
        {
            sint32 botTop = std::max<sint32>(240, dpi->y);
            gfx_fill_rect(dpi, 40, botTop, 359, dpiBottom - 1, ColourMapA[COLOUR_DARK_GREEN].darkest);
            gfx_fill_rect_inset(dpi, 40, 297, 359, 447, COLOUR_DARK_GREEN, INSET_RECT_FLAG_BORDER_INSET | INSET_RECT_FLAG_FILL_DONT_LIGHTEN);

            gCurrentFontSpriteBase = FONT_SPRITE_BASE_MEDIUM;
            const char * title1 = "ROLLERCOASTER TYCOON 1 & 2 - 3DS";
            gCurrentFontSpriteBase = FONT_SPRITE_BASE_SMALL;
            const char * title2 = "Toque nos icones abaixo com a Stylus para jogar";
            gfx_draw_string(dpi, (char *)title1, COLOUR_YELLOW, 68, 250);
            gfx_draw_string(dpi, (char *)title2, COLOUR_WHITE, 74, 268);
        }
    }
    else
    {
        // Gameplay Dual-Screen Architecture:
        //   Bottom Screen (y >= 240) = Always 1:1 Interactive Stylus Park Viewport (w->viewport at 40, 240, 320, 240)
        //   Top Screen (y < 240)     = Clean Widescreen Park View (400x240) with zero obstructing windows!
        //                              g3dsDualScreenMode == 1 (Default): 1:1 Widescreen Cinema View
        //                              g3dsDualScreenMode == 0 (SELECT):  Zoomed-out Overview + Yellow Focus Box
        if (dpi->y < 240)
        {
            sint32 topClipBottom = std::min<sint32>(240, dpiBottom);
            rct_viewport overviewVp = *w->viewport;
            overviewVp.x = 0;
            overviewVp.y = 0;
            overviewVp.width = 400;
            overviewVp.height = 240;
            uint8 zoomOffset = (g3dsDualScreenMode == 0) ? 1 : 0;
            overviewVp.zoom = std::min<uint8>(w->viewport->zoom + zoomOffset, 3);
            overviewVp.view_width = 400 << overviewVp.zoom;
            overviewVp.view_height = 240 << overviewVp.zoom;
            sint32 centerViewX = w->viewport->view_x + (w->viewport->view_width / 2);
            sint32 centerViewY = w->viewport->view_y + (w->viewport->view_height / 2);
            overviewVp.view_x = centerViewX - (overviewVp.view_width / 2);
            overviewVp.view_y = centerViewY - (overviewVp.view_height / 2);
            overviewVp.flags &= ~VIEWPORT_FLAG_SOUND_ON;

            viewport_render(dpi, &overviewVp, dpi->x, dpi->y, dpiRight, topClipBottom);

            if (g3dsDualScreenMode == 0)
            {
                // Draw Yellow Focus Box representing the Bottom Screen 1:1 Touch area
                sint32 boxW = w->viewport->view_width >> overviewVp.zoom;
                sint32 boxH = w->viewport->view_height >> overviewVp.zoom;
                sint32 boxL = 200 - (boxW / 2);
                sint32 boxT = 120 - (boxH / 2);
                sint32 boxR = boxL + boxW - 1;
                sint32 boxB = boxT + boxH - 1;
                uint8 yellowCol = ColourMapA[COLOUR_YELLOW].lighter;
                gfx_fill_rect(dpi, boxL, boxT, boxR, boxT + 1, yellowCol);
                gfx_fill_rect(dpi, boxL, boxB - 1, boxR, boxB, yellowCol);
                gfx_fill_rect(dpi, boxL, boxT, boxL + 1, boxB, yellowCol);
                gfx_fill_rect(dpi, boxR - 1, boxT, boxR, boxB, yellowCol);
            }

            // Draw Top Telemetry Header Bar
            if (dpi->y < 14)
            {
                gfx_filter_rect(dpi, 0, 0, 399, 13, PALETTE_DARKEN_2);
                gCurrentFontSpriteBase = FONT_SPRITE_BASE_SMALL;
                char headerBuf[128];
                snprintf(
                    headerBuf,
                    sizeof(headerBuf),
                    "[SELECT] Camera Topo: %s  |  Visitantes: %u  |  Nota: %d",
                    (g3dsDualScreenMode == 1) ? "1:1 Cinema" : "Panoramica",
                    (uint32)gNumGuestsInPark,
                    (sint32)gParkRating
                );
                gfx_draw_string(dpi, headerBuf, COLOUR_YELLOW, 8, 2);
            }
        }

        if (dpiBottom > 240)
        {
            sint32 botTop = std::max<sint32>(240, dpi->y);
            viewport_render(dpi, w->viewport, dpi->x, botTop, dpiRight, dpiBottom);
        }
    }
#else
    viewport_render(dpi, w->viewport, dpi->x, dpi->y, dpi->x + dpi->width, dpi->y + dpi->height);
#endif
}
