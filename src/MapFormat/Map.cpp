#include <string.h>
#include "Parser.hpp"
#include "map.hpp"

std::string BaseName(const char *path)
{
    const char *lastSlash = strrchr(path, '/');
    if (lastSlash)
    {
        return std::string(lastSlash + 1);
    }
    return std::string(path);
}

bool Map::Load(const char *fileName, Map &map)
{
    FILE *file;
    file = fopen(fileName, "rb");

    if (!file)
    {
        perror("Failed to open file");
        return false;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(size + 1);
    if (fread(buffer, 1, size, file) != size)
    {
        perror("Failed to read file");
        fclose(file);
        free(buffer);
        return false;
    }
    buffer[size] = '\0';
    fclose(file);

    Lexer lexer(buffer);
    if (!parseMap(lexer, &map))
    {
        fprintf(stderr, "Parsing failed.\n");
        free(buffer);
        return false;
    }

    free(buffer);
    return true;
}

std::string Map::Stringify()
{
    std::string result;

    for (const Entity &e : entities)
    {
        result += "{\n";

        for (const auto &pair : e.properties)
        {
            result += "\"" + pair.first + "\" \"" + pair.second + "\"\n";
        }
        for (const Brush &b : e.brushes)
        {
            result += "{\n";
            for (const Face &f : b.faces)
            {
                result += "( " + std::to_string(f.p1.x) + " " + std::to_string(f.p1.y) + " " + std::to_string(f.p1.z) + " ) "
                                                                                                                        "( " +
                          std::to_string(f.p2.x) + " " + std::to_string(f.p2.y) + " " + std::to_string(f.p2.z) + " ) "
                                                                                                                 "( " +
                          std::to_string(f.p3.x) + " " + std::to_string(f.p3.y) + " " + std::to_string(f.p3.z) + " ) ";
                f.texture;

                switch (f.projectionType)
                {
                case TextureProjectionType::Standard:
                {
                    StandardUV sp = f.textureProjection.standard;
                    result += std::to_string(sp.xOffset) + " " + std::to_string(sp.yOffset) + " " + std::to_string(sp.rotation) + " " +
                              std::to_string(sp.xScale) + " " + std::to_string(sp.yScale) + "\n";
                    break;
                }
                case TextureProjectionType::Valve220:
                    Valve220 vp = f.textureProjection.valve220;
                    result += "[ " + std::to_string(vp.uAxis.x) + " " + std::to_string(vp.uAxis.y) + " " + std::to_string(vp.uAxis.z) + " " +
                              std::to_string(vp.xOffset) + " ] [ " +
                              std::to_string(vp.vAxis.x) + " " + std::to_string(vp.vAxis.y) + " " + std::to_string(vp.vAxis.z) + " " +
                              std::to_string(vp.yOffset) + " ] " +
                              std::to_string(vp.rotation) + " " +
                              std::to_string(vp.xScale) + " " +
                              std::to_string(vp.yScale) + "\n";
                    break;
                }

                result += std::to_string(f.flags[0]) + " " + std::to_string(f.flags[1]) + " " + std::to_string(f.flags[2]) + "\n";
            }

            result += "}\n";
        }

        for (const Patch &p : e.patches)
        {
            result += "{\npatchDef2\n{\n" + p.texture + "\n( " + std::to_string(p.height) + " " + std::to_string(p.width) + " " +
                      std::to_string(p.flags[0]) + " " + std::to_string(p.flags[1]) + " " + std::to_string(p.flags[2]) + " )\n(\n";

            for (const auto &row : p.controlPoints)
            {
                result += "( ";
                for (const PatchVert &v : row)
                {
                    result += "( " + std::to_string(v.position.x) + " " + std::to_string(v.position.y) + " " +
                              std::to_string(v.position.z) + " " +
                              std::to_string(v.uv.x) + " " +
                              std::to_string(v.uv.y) + " ) ";
                }
                result += ")\n";
            }
            result += ")\n}\n}\n";
        }

        result += "}\n";
    }

    return result;
}

void Map::Print()
{
    printf("%s", Stringify().c_str());
}
