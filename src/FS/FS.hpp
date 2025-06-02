#pragma once

#include <cstddef>
#include <raylib.h>

namespace FS
{
    struct Binaryfile
    {
        unsigned char *buffer;
        size_t size;
    };

    int Init();
    int AddDir(const char *dirPath);
    void Close();
    bool Exists(const char *fileName);
    Binaryfile LoadBinaryFile(const char *fileName);
    void FreeBinaryFile(Binaryfile &file);
    Image LoadImage(const char *fileName);
    Texture2D LoadTexture(const char *fileName);
}
