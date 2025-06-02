#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <glm/glm.hpp>

using vec3 = glm::vec3;
using vec2 = glm::vec2;

class Brush;
class Entity;
class Map;

struct StandardUV
{
    float xScale, yScale, xOffset, yOffset, rotation;
};

struct Valve220
{
    float xScale, yScale, xOffset, yOffset, rotation;
    vec3 uAxis, vAxis;
};

union TextureProjection
{
    StandardUV standard;
    Valve220 valve220;
};

enum class TextureProjectionType
{
    Standard,
    Valve220,
    BrushPrimitive
};

class Face
{
public:
    Brush *parentBrush;

    vec3 p1, p2, p3;
    std::string texture;
    vec2 textureSize;
    std::vector<int> vertices;
    std::vector<vec2> uvs;

    TextureProjectionType projectionType;
    TextureProjection textureProjection;

    int flags[3] = {0, 0, 0};
    int flagCount = 0;

    Face(Brush *parent): parentBrush(parent) {}

    vec3 GetNormal();
    float GetDistance();
    vec3 GetCenter();
    std::vector<vec3> GetVertices();
    void SortVertices();
    vec2 GetUV(vec3 &vert);

private:
    vec3 normal = vec3(0.0f);
    vec3 center = vec3(0.0f);
    float distance = 0.0f;
};

struct AABB
{
    vec3 min, max;
};

class Brush
{
public:
    int id;
    Entity *parentEntity;
    std::vector<Face> faces;

    Brush(int brushID, Entity *entity)
        : id(brushID), parentEntity(entity) {}

    vec3 GetCenter();
    AABB GetAABB();
    void CalculateGeometry();

    vec3 center = vec3(0.0f);
    AABB aabb = {vec3(0.0f), vec3(0.0f)};
    std::vector<vec3> vertices;
};

struct PatchVert
{
    vec3 position;
    vec2 uv;
    vec3 normal;
};

class Patch
{
public:
    int id;
    Entity *parentEntity;
    std::string texture;
    vec2 textureSize;
    int width, height;
    int flags[3];

    std::vector<std::vector<PatchVert>> controlPoints;
    std::vector<std::vector<PatchVert>> vertices;

    Patch(int patchID, Entity *entity)
        : id(patchID), parentEntity(entity) {}

    void CalculateGeometry();
};

class Entity
{
public:
    int id;
    Map *parentMap;
    std::unordered_map<std::string, std::string> properties;
    std::vector<Brush> brushes;
    std::vector<Patch> patches;

    Entity(int entityID, Map *map) : id(entityID), parentMap(map) {}
};

class Map
{
public:
    std::string name;
    std::vector<Entity> entities;
    std::unordered_map<std::string, vec2> textureSizes;
    std::unordered_set<std::string> models;

    bool failed;

    Map() = default;
    static bool Load(const char *filename, Map &map);
    std::string Stringify();
    void Print();
};
