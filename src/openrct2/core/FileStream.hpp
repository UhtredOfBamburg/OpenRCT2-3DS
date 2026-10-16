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

#pragma once

#include "../common.h"
#include "IStream.hpp"
#include "Math.hpp"
#include "String.hpp"

#include "../localisation/Language.h"

#ifdef __psp2__
#include <psp2/io/fcntl.h>
#endif

enum
{
    FILE_MODE_OPEN,
    FILE_MODE_WRITE,
    FILE_MODE_APPEND,
};

/**
 * A stream for reading and writing to files.
 */
class FileStream final : public IStream
{
private:
#ifdef __psp2__
    SceUID      _file           = 0;
#else
    FILE *      _file           = nullptr;
#endif
    bool        _ownsFilePtr    = false;
    bool        _canRead        = false;
    bool        _canWrite       = false;
    bool        _disposed       = false;
    uint64      _fileSize       = 0;
    uint64      _pos            = 0;

public:
    FileStream(const std::string &path, sint32 fileMode) :
        FileStream(path.c_str(), fileMode)
    {
    }
#ifdef __psp2__
    FileStream(const utf8 * path, sint32 fileMode)
    {
        SceMode mode;
        switch (fileMode) {
        case FILE_MODE_OPEN:
            mode = SCE_O_RDONLY;
            _canRead = true;
            _canWrite = false;
            break;
        case FILE_MODE_WRITE:
            mode = SCE_O_RDWR | SCE_O_CREAT;
            _canRead = true;
            _canWrite = true;
            break;
        case FILE_MODE_APPEND:
            mode = SCE_O_APPEND | SCE_O_CREAT;
            _canRead = false;
            _canWrite = true;
            break;
        default:
            throw;
        }


        _file = sceIoOpen(path, mode, 0777);
        if (_file == 0)
        {
            throw IOException(String::StdFormat("Unable to open '%s'", path));
        }

        Seek(0, STREAM_SEEK_END);
        _fileSize = GetPosition();
        Seek(0, STREAM_SEEK_BEGIN);

        _ownsFilePtr = true;
    }
#else
    FileStream(const utf8 * path, sint32 fileMode)
    {
        const char * mode;
        switch (fileMode) {
        case FILE_MODE_OPEN:
            mode = "rb";
            _canRead = true;
            _canWrite = false;
            break;
        case FILE_MODE_WRITE:
            mode = "w+b";
            _canRead = true;
            _canWrite = true;
            break;
        case FILE_MODE_APPEND:
            mode = "a";
            _canRead = false;
            _canWrite = true;
            break;
        default:
            throw;
        }

#ifdef _WIN32
        wchar_t * pathW = utf8_to_widechar(path);
        wchar_t * modeW = utf8_to_widechar(mode);
        _file = _wfopen(pathW, modeW);
        free(pathW);
        free(modeW);
#else

        _file = fopen(path, mode);
#endif
        if (_file == nullptr)
        {
            throw IOException(String::StdFormat("Unable to open '%s'", path));
        }

#ifdef __3DS__
        setvbuf(_file, nullptr, _IOFBF, 64 * 1024);
#endif

        Seek(0, STREAM_SEEK_END);
        _fileSize = GetPosition();
        Seek(0, STREAM_SEEK_BEGIN);
        

        _ownsFilePtr = true;
    }
#endif
#ifdef __psp2__
    ~FileStream() override
    {
        if (!_disposed)
        {
            _disposed = true;
            if (_ownsFilePtr)
            {
                sceIoClose(_file);
            }
        }
    }
#else
    ~FileStream() override
    {
        if (!_disposed)
        {
            _disposed = true;
            if (_ownsFilePtr)
            {
                fclose(_file);
            }
        }
    }
#endif
    bool CanRead()  const override { return _canRead;  }
    bool CanWrite() const override { return _canWrite; }

    uint64 GetLength()   const override { return _fileSize; }
    uint64 GetPosition() const override
    {
#if defined(_MSC_VER)
        return _ftelli64(_file);
#elif (defined(__APPLE__) && defined(__MACH__)) || defined(__ANDROID__) || defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__3DS__)
        return ftello(_file);
#elif  defined(__psp2__)
        return _pos;
#else
        return ftello64(_file);
#endif
    }

    void SetPosition(uint64 position) override
    {
        Seek(position, STREAM_SEEK_BEGIN);
    }

    void Seek(sint64 offset, sint32 origin) override
    {
#if defined(_MSC_VER)
        switch (origin) {
        case STREAM_SEEK_BEGIN:
            _fseeki64(_file, offset, SEEK_SET);
            break;
        case STREAM_SEEK_CURRENT:
            _fseeki64(_file, offset, SEEK_CUR);
            break;
        case STREAM_SEEK_END:
            _fseeki64(_file, offset, SEEK_END);
            break;
        }
#elif (defined(__APPLE__) && defined(__MACH__)) || defined(__ANDROID__) || defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__3DS__)
        switch (origin) {
        case STREAM_SEEK_BEGIN:
            fseeko(_file, offset, SEEK_SET);
            break;
        case STREAM_SEEK_CURRENT:
            fseeko(_file, offset, SEEK_CUR);
            break;
        case STREAM_SEEK_END:
            fseeko(_file, offset, SEEK_END);
            break;
        }
#elif  defined(__psp2__)
        switch (origin) {
        case STREAM_SEEK_BEGIN:
            // fseeko(_file, offset, SEEK_SET);
            _pos = sceIoLseek(_file, offset, SCE_SEEK_SET);
            break;
        case STREAM_SEEK_CURRENT:
            // fseeko(_file, offset, SEEK_CUR);
            _pos = sceIoLseek(_file, offset, SCE_SEEK_CUR);
            break;
        case STREAM_SEEK_END:
            _pos = sceIoLseek(_file, offset, SCE_SEEK_END);
            break;
        }
#else
        switch (origin) {
        case STREAM_SEEK_BEGIN:
            fseeko64(_file, offset, SEEK_SET);
            break;
        case STREAM_SEEK_CURRENT:
            fseeko64(_file, offset, SEEK_CUR);
            break;
        case STREAM_SEEK_END:
            fseeko64(_file, offset, SEEK_END);
            break;
    }
#endif
    }
#ifdef __psp2__
    void Read(void * buffer, uint64 length) override
    {
        uint64 remainingBytes = GetLength() - GetPosition();
        if (length <= remainingBytes)
        {
            _pos += sceIoRead(_file, buffer, length);
            return;
        }
        throw IOException("Attempted to read past end of file.");
    }

    void Write(const void * buffer, uint64 length) override
    {
        _pos += sceIoWrite(_file, buffer, length);


        // uint64 position = GetPosition();
        _fileSize = Math::Max(_fileSize, _pos);
    }

    uint64 TryRead(void * buffer, uint64 length) override
    {
        size_t readBytes = sceIoRead(_file, buffer, length);
        _pos += readBytes;
        return readBytes;
    }
#else
    void Read(void * buffer, uint64 length) override
    {
        uint64 remainingBytes = GetLength() - GetPosition();
        if (length <= remainingBytes)
        {
            if (fread(buffer, (size_t)length, 1, _file) == 1)
            {
                return;
            }
        }
        throw IOException("Attempted to read past end of file.");
    }

    void Write(const void * buffer, uint64 length) override
    {
        if (fwrite(buffer, (size_t)length, 1, _file) != 1)
        {
            throw IOException("Unable to write to file.");
        }

        uint64 position = GetPosition();
        _fileSize = Math::Max(_fileSize, position);
    }

    uint64 TryRead(void * buffer, uint64 length) override
    {
        size_t readBytes = fread(buffer, 1, (size_t)length, _file);
        return readBytes;
    }
#endif
};
