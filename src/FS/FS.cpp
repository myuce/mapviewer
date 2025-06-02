#include <algorithm>
#include <string>
#include <vector>
#include <dirent.h>
#include <physfs.h>
#include <string.h>
#include <stdio.h>
#include "FS.hpp"

int FS::Init()
{
    PHYSFS_init(NULL);

    if (!PHYSFS_isInit())
    {
        printf("Failed to initialize PhysFS\n");
        return -1;
    }

    return 0;
}

int FS::AddDir(const char *dirPath)
{
    PHYSFS_mount(dirPath, "/", 1);

    DIR *dir;
    struct dirent *ent;
    std::vector<std::string> zipFiles;

    if ((dir = opendir(dirPath)) != NULL)
    {
        // Collect all ZIP file names
        while ((ent = readdir(dir)) != NULL)
        {
            std::string fileName = ent->d_name;
            if (fileName.size() > 4 && fileName.substr(fileName.size() - 4) == ".pk3")
            {
                zipFiles.push_back(fileName);
            }
        }
        closedir(dir);

        // Sort the filenames alphabetically
        std::sort(zipFiles.begin(), zipFiles.end());

        // Mount them in order
        for (const std::string &zipFile : zipFiles)
        {
            std::string fullPath = std::string(dirPath) + "/" + zipFile;
            PHYSFS_mount(fullPath.c_str(), "/", 1);
        }
    }
    else
    {
        return -1; // Failed to open directory
    }

    return 0;
}

void FS::Close()
{
    PHYSFS_deinit();
}

bool FS::Exists(const char *fileName)
{
    return PHYSFS_exists(fileName);
}

FS::Binaryfile FS::LoadBinaryFile(const char *fileName)
{
    Binaryfile file = { .buffer = nullptr, .size = 0 };

    if (FS::Exists(fileName))
    {
        PHYSFS_File *physFile = PHYSFS_openRead(fileName);
        PHYSFS_sint64 length = PHYSFS_fileLength(physFile);
        file.size = length;
        file.buffer = new unsigned char[length];
        PHYSFS_readBytes(physFile, file.buffer, length);
        PHYSFS_close(physFile);
    }

    return file;
}

void FS::FreeBinaryFile(FS::Binaryfile &file)
{
    delete[] file.buffer;
    file.buffer = nullptr;
    file.size = 0;
}

Image FS::LoadImage(const char *fileName)
{
    Image image;

    if (FS::Exists(fileName))
    {
        Binaryfile file = FS::LoadBinaryFile(fileName);
        const char* fileType = GetFileExtension(fileName);
        image = LoadImageFromMemory(fileType, file.buffer, file.size);
        ImageMipmaps(&image);
        FS::FreeBinaryFile(file);
    }

    return image;
}

Texture2D FS::LoadTexture(const char *fileName)
{
    Texture2D texture = { 0 };

    if (FS::Exists(fileName))
    {
        Image image = FS::LoadImage(fileName);
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
    }

    return texture;
}
