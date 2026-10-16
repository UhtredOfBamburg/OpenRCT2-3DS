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

#ifdef __3DS__

#include <sstream>
#include <stdexcept>
#include <openrct2/common.h>
#include <openrct2/core/String.hpp>
#include <openrct2/ui/UiContext.h>
#include "UiContext.h"

#include <SDL.h>

namespace OpenRCT2 { namespace Ui
{
    class N3DSContext final : public IPlatformUiContext
    {
    public:
        N3DSContext()
        {
        }

        void SetWindowIcon(SDL_Window * window) override
        {
            UNUSED(window);
        }

        bool IsSteamOverlayAttached() override
        {
            return false;
        }

        void ShowMessageBox(SDL_Window * window, const std::string &message) override
        {
            UNUSED(window);
            log_verbose(message.c_str());
        }

        std::string ShowFileDialog(SDL_Window * window, const FileDialogDesc &desc) override
        {
            UNUSED(window);
            UNUSED(desc);
            return std::string();
        }

        std::string ShowDirectoryDialog(SDL_Window * window, const std::string &title) override
        {
            UNUSED(window);
            log_info(title.c_str());
            return "sdmc:/3ds/openrct2/rct2";
        }
    };

    IPlatformUiContext * CreatePlatformUiContext()
    {
        return new N3DSContext();
    }
} }

#endif // __3DS__
