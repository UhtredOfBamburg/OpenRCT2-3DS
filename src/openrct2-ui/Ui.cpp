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

#include <openrct2/audio/AudioContext.h>
#include <openrct2/Context.h>
#include <openrct2/OpenRCT2.h>
#include <openrct2/PlatformEnvironment.h>
#include <openrct2/ui/UiContext.h>
#include "audio/AudioContext.h"
#include "Ui.h"
#include "UiContext.h"

#include <openrct2/platform/platform.h>

using namespace OpenRCT2;
using namespace OpenRCT2::Audio;
using namespace OpenRCT2::Ui;
#ifdef __psp2__
int _newlib_heap_size_user   = 200 * 1024 * 1024;
unsigned int sceLibcHeapSize = 32 * 1024 * 1024;
#endif
#ifdef __3DS__
#include <3ds.h>
#include <SDL2/SDL.h>
#undef main
#include <cstdio>
#include <malloc.h>
extern "C" {
    extern char* fake_heap_start;
    extern char* fake_heap_end;
    u32 __ctru_heap = 0;
    u32 __ctru_linear_heap = 0;
    u32 __ctru_heap_size = 0;
    u32 __ctru_linear_heap_size = 0;
    u32 __stacksize__ = 2 * 1024 * 1024;

    void __system_allocateHeaps(void)
    {
        Result rc;

        Handle reslimit = 0;
        rc = svcGetResourceLimit(&reslimit, CUR_PROCESS_HANDLE);
        if (R_FAILED(rc))
            svcBreak(USERBREAK_PANIC);

        s64 maxCommit = 0, currentCommit = 0;
        ResourceLimitType reslimitType = RESLIMIT_COMMIT;
        svcGetResourceLimitLimitValues(&maxCommit, reslimit, &reslimitType, 1);
        svcGetResourceLimitCurrentValues(&currentCommit, reslimit, &reslimitType, 1);
        svcCloseHandle(reslimit);

        u32 remaining = (u32)(maxCommit - currentCommit) & ~0xFFF;

        // Reserve 4 MB for linear heap (framebuffers, audio DSP buffers)
        __ctru_linear_heap_size = 4 * 1024 * 1024;
        if (__ctru_linear_heap_size >= remaining)
        {
            __ctru_linear_heap_size = (remaining / 4) & ~0xFFF;
        }

        // Available for main malloc heap, safely capped below the 96MB virtual window limit (OS_HEAP_AREA_END - OS_HEAP_AREA_BEGIN)
        u32 maxHeapVirtual = (OS_HEAP_AREA_END - OS_HEAP_AREA_BEGIN) - (128 * 1024);
        u32 desiredHeap = remaining - __ctru_linear_heap_size;
        if (desiredHeap > maxHeapVirtual)
        {
            desiredHeap = maxHeapVirtual;
        }
        __ctru_heap_size = desiredHeap & ~0xFFF;

        // Try allocating app heap. If kernel runs low on physical memory, step down in 1MB increments
        while (__ctru_heap_size >= (16 * 1024 * 1024))
        {
            rc = svcControlMemory(&__ctru_heap, OS_HEAP_AREA_BEGIN, 0x0, __ctru_heap_size, MEMOP_ALLOC, (MemPerm)(MEMPERM_READ | MEMPERM_WRITE));
            if (R_SUCCEEDED(rc))
                break;
            __ctru_heap_size -= (1024 * 1024);
        }
        if (R_FAILED(rc))
            svcBreak(USERBREAK_PANIC);

        // Allocate linear heap
        while (__ctru_linear_heap_size >= (1024 * 1024))
        {
            rc = svcControlMemory(&__ctru_linear_heap, 0x0, 0x0, __ctru_linear_heap_size, MEMOP_ALLOC_LINEAR, (MemPerm)(MEMPERM_READ | MEMPERM_WRITE));
            if (R_SUCCEEDED(rc))
                break;
            __ctru_linear_heap_size -= (512 * 1024);
        }
        if (R_FAILED(rc))
            svcBreak(USERBREAK_PANIC);

        mappableInit(OS_MAP_AREA_BEGIN, OS_MAP_AREA_END);

        fake_heap_start = (char*)__ctru_heap;
        fake_heap_end = fake_heap_start + __ctru_heap_size;
    }

    void n3ds_draw_loading_progress(int current, int total)
    {
        if (!aptMainLoop())
            return;

        u16 topW = 0, topH = 0, botW = 0, botH = 0;
        u32 * topFb = (u32 *)gfxGetFramebuffer(GFX_TOP, GFX_LEFT, &topW, &topH);
        u32 * botFb = (u32 *)gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, &botW, &botH);

        if (topFb != nullptr)
        {
            for (int x = 0; x < 400; x++)
            {
                u32 * col = topFb + x * 240;
                for (int y = 0; y < 240; y++)
                {
                    bool border = (x < 6 || x >= 394 || y < 6 || y >= 234);
                    col[239 - y] = border ? 0xD4AF37FFu : 0x163828FFu;
                }
            }
            GSPGPU_FlushDataCache(topFb, 400 * 240 * 4);
            gfxScreenSwapBuffers(GFX_TOP, false);
        }

        if (botFb != nullptr)
        {
            int barLeft = 30;
            int barRight = 290;
            int barTop = 108;
            int barBottom = 132;
            int fillRight = barLeft;
            if (total > 0)
            {
                fillRight = barLeft + ((barRight - barLeft) * current) / total;
                if (fillRight > barRight) fillRight = barRight;
            }

            for (int x = 0; x < 320; x++)
            {
                u32 * col = botFb + x * 240;
                for (int y = 0; y < 240; y++)
                {
                    u32 pixel = 0x10281CFFu;
                    if (x >= barLeft - 2 && x <= barRight + 2 && y >= barTop - 2 && y <= barBottom + 2)
                    {
                        if (x < barLeft || x > barRight || y < barTop || y > barBottom)
                            pixel = 0xFFFFFFFFu;
                        else if (x <= fillRight)
                            pixel = 0xF4C430FFu;
                        else
                            pixel = 0x0A1810FFu;
                    }
                    col[239 - y] = pixel;
                }
            }
            GSPGPU_FlushDataCache(botFb, 320 * 240 * 4);
            gfxScreenSwapBuffers(GFX_BOTTOM, false);
        }
    }

    static u64 sBootStartTime = 0;

    void n3ds_boot_log(int percent, const char * step)
    {
        if (percent >= 0)
        {
            n3ds_draw_loading_progress(percent, 100);
        }
        u64 now = osGetTime();
        if (sBootStartTime == 0)
            sBootStartTime = now;
        u32 elapsedMs = (u32)(now - sBootStartTime);

        FILE * fp = fopen("sdmc:/3ds/openrct2/boot.log", percent <= 5 ? "w" : "a");
        if (fp != nullptr)
        {
            struct mallinfo mi = mallinfo();
            u32 heapTotal = envGetHeapSize();
            u32 linFree = linearSpaceFree();
            fprintf(fp, "[%3d%%] (+%3u.%03us) %s | heap_total=%uKB mall_used=%dKB lin_free=%uKB\n",
                    percent, elapsedMs / 1000, elapsedMs % 1000, step ? step : "",
                    (unsigned)(heapTotal / 1024),
                    (int)(mi.uordblks / 1024),
                    (unsigned)(linFree / 1024));
            fflush(fp);
            fclose(fp);
        }
    }
}
#endif
/**
 * Main entry point for non-Windows systems. Windows instead uses its own DLL proxy.
 */
#ifdef _MSC_VER
int NormalisedMain(int argc, const char * * argv)
#else
int main(int argc, const char * * argv)
#endif
{
#ifdef __3DS__
    osSetSpeedupEnable(true);
    romfsInit();
    gfxInit(GSP_RGBA8_OES, GSP_RGBA8_OES, false);
    gfxSetDoubleBuffering(GFX_TOP, true);
    gfxSetDoubleBuffering(GFX_BOTTOM, true);
    SDL_Init(SDL_INIT_TIMER);
    n3ds_boot_log(5, "main: gfxInit + SDL_Init complete");
#endif
    int runGame = cmdline_run(argv, argc);
    core_init();
    if (runGame == 1)
    {
        if (gOpenRCT2Headless)
        {
            // Run OpenRCT2 with a plain context
            auto context = CreateContext();
            context->RunOpenRCT2(argc, argv);
            delete context;
        }
        else
        {
            // Run OpenRCT2 with a UI context
            auto env = CreatePlatformEnvironment();
            auto audioContext = CreateAudioContext();
            auto uiContext = CreateUiContext(env);
            auto context = CreateContext(env, audioContext, uiContext);

            context->RunOpenRCT2(argc, argv);

            delete context;
            delete uiContext;
            delete audioContext;
        }
    }
#ifdef __3DS__
    gfxExit();
    romfsExit();
#endif
    return gExitCode;
}


#ifdef __ANDROID__
extern "C" {
int SDL_main(int argc, const char *argv[])
{
    return main(argc, argv);
}
}
#endif
