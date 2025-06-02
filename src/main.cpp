#include <cstdio>
#include <cstdlib>
#include <raylib.h>
#include "FS/FS.hpp"
#include "MapFormat/Map.hpp"

// helper to convert your vec3 -> raylib Vector3, with your 30-unit scale & axis swap
static Vector3 ToRay(const vec3 &v) {
    return { v.x/30.0f,  v.z/30.0f,  -v.y/30.0f };
}

#define Deg2Rad(degrees) degrees * (M_PI / 180.0f)

Mesh GenMeshPolyhedron(int vertexCount, Vector3 *vertices, Vector2 *uvs, Vector3 normal)
{
    Mesh mesh = { 0 };

    if (vertexCount < 3) return mesh;  // need at least a triangle

    int triCount = vertexCount - 2;
    mesh.vertexCount   = vertexCount;
    mesh.triangleCount = triCount;

    // allocate CPU arrays
    mesh.vertices  = (float *)RL_MALLOC(sizeof(float)*3*vertexCount);
    mesh.normals   = (float *)RL_MALLOC(sizeof(float)*3*vertexCount);
    mesh.texcoords = (float *)RL_MALLOC(sizeof(float)*2*vertexCount);
    mesh.indices   = (unsigned short *)RL_MALLOC(sizeof(unsigned short)*3*triCount);

    // fill vertex positions, normals & UVs
    for (int i = 0; i < vertexCount; i++)
    {
        // positions
        mesh.vertices[3*i + 0] = vertices[i].x;
        mesh.vertices[3*i + 1] = vertices[i].y;
        mesh.vertices[3*i + 2] = vertices[i].z;

        // all vertices share the same face normal
        mesh.normals[3*i + 0] = normal.x;
        mesh.normals[3*i + 1] = normal.y;
        mesh.normals[3*i + 2] = normal.z;

        // UVs
        mesh.texcoords[2*i + 0] = uvs[i].x;
        mesh.texcoords[2*i + 1] = uvs[i].y;
    }

    // build a simple triangle‐fan
    for (int i = 0; i < triCount; i++)
    {
        mesh.indices[3*i + 0] = 0;
        mesh.indices[3*i + 1] = i + 1;
        mesh.indices[3*i + 2] = i + 2;
    }

    // upload to GPU (static)
    UploadMesh(&mesh, false);

    return mesh;
}

Mesh GenMeshFromPatch(const std::vector<std::vector<PatchVert>>& patchVerts)
{
    Mesh mesh = { 0 };

    int rows = (int)patchVerts.size();
    if (rows < 2) return mesh;        // need at least one quad
    int cols = (int)patchVerts[0].size();
    if (cols < 2) return mesh;

    int vCount  = rows * cols;
    int tCount  = (rows - 1) * (cols - 1) * 2;

    mesh.vertexCount   = vCount;
    mesh.triangleCount = tCount;

    // allocate your arrays
    mesh.vertices  = (float *)RL_MALLOC(sizeof(float)*3*vCount);
    mesh.normals   = (float *)RL_MALLOC(sizeof(float)*3*vCount);
    mesh.texcoords = (float *)RL_MALLOC(sizeof(float)*2*vCount);
    mesh.indices   = (unsigned short *)RL_MALLOC(sizeof(unsigned short)*3*tCount);

    // fill vertex data
    int vid = 0;
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            const PatchVert &pv = patchVerts[i][j];

            // correct conversion & scale
            Vector3 p = ToRay(pv.position);
            vec3 _n = (pv.normal);
            Vector3 n = { _n.x, _n.z, -_n.y };

            mesh.vertices[3*vid + 0]  = p.x;
            mesh.vertices[3*vid + 1]  = p.y;
            mesh.vertices[3*vid + 2]  = p.z;

            mesh.normals[3*vid + 0]   = n.x;
            mesh.normals[3*vid + 1]   = n.y;
            mesh.normals[3*vid + 2]   = n.z;

            mesh.texcoords[2*vid + 0] = pv.uv.x;
            mesh.texcoords[2*vid + 1] = pv.uv.y;

            vid++;
        }
    }

    // build two tris per quad
    int idx = 0;
    for (int i = 0; i < rows - 1; i++)
    {
        for (int j = 0; j < cols - 1; j++)
        {
            int topLeft     = i*cols + j;
            int bottomLeft  = (i+1)*cols + j;
            int topRight    = topLeft + 1;
            int bottomRight = bottomLeft + 1;

            // tri A
            mesh.indices[3*idx + 0] = topLeft;
            mesh.indices[3*idx + 1] = bottomLeft;
            mesh.indices[3*idx + 2] = topRight;
            idx++;

            // tri B
            mesh.indices[3*idx + 0] = topRight;
            mesh.indices[3*idx + 1] = bottomLeft;
            mesh.indices[3*idx + 2] = bottomRight;
            idx++;
        }
    }

    // upload to GPU
    UploadMesh(&mesh, false);
    return mesh;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <mapfile>\n", argv[0]);
        return 1;
    }

    // initialize file system
    if (FS::Init() != 0) {
        fprintf(stderr, "Failed to initialize file system.\n");
        return 1;
    }

    // replace this line with your own path to the Quake 3 Arena baseq3 directory
    if (FS::AddDir("E:/Games/Steam/steamapps/common/Quake 3 Arena/baseq3") != 0)
    {
        fprintf(stderr, "Failed to add directory.\n");
        return 1;
    }

    Map map;
    if (!Map::Load(argv[1], map)) {
        fprintf(stderr, "Failed to load map.\n");
        return 1;
    }

    InitWindow(1800, 1000, "Map Viewer");
    SetTargetFPS(60);

    Camera camera = {
        .position = { 0.0f,  0.0f,   0.0f },
        .target   = { 0.0f,  0.0f,  10.0f },
        .up       = { 0.0f,  1.0f,   0.0f },
        .fovy     = 90.f,
        .projection = CAMERA_PERSPECTIVE
    };

    bool foundPlayerStart = false;
    for (auto &e : map.entities) {
        if (e.properties["classname"] == "info_player_deathmatch" && !foundPlayerStart)
        {
            if (GetRandomValue(0, 10) == 5) foundPlayerStart = true;


            vec3 pos = { 0.0f, 0.0f, 0.0f };
            if (e.properties.find("origin") != e.properties.end())
            {
                sscanf(e.properties["origin"].c_str(), "%f %f %f", &pos.x, &pos.y, &pos.z);
            }

            // set camera position
            camera.position = { pos.x/30.0f, pos.z/30.0f, -pos.y/30.0f };

            // get player start angle
            float angle = 0.0f;
            if (e.properties.find("angle") != e.properties.end())
            {
                angle = std::stof(e.properties["angle"]);
                angle = fmodf(angle, 360.0f);
            }

            // set camera target
            camera.target = { pos.x/30.0f + cosf(Deg2Rad(angle)), pos.z/30.0f, -pos.y/30.0f - sinf(Deg2Rad(angle)) };
        }

        for (auto &b : e.brushes)    b.CalculateGeometry();
        for (auto &p : e.patches)    p.CalculateGeometry();
    }

    Image defaultImage = GenImageChecked(1024, 1024, 1, 1, PURPLE, BLACK);
    Texture2D defaultTexture = LoadTextureFromImage(defaultImage);
    UnloadImage(defaultImage);

    // load textures
    std::unordered_map<std::string, Texture2D> textures;

    for (auto &tex: map.textureSizes) {
        // load texture
        std::string texName = "textures/" + tex.first;
        Texture2D texture;

        if (FS::Exists((texName + ".tga").c_str()))
        {
            texture = FS::LoadTexture((texName + ".tga").c_str());
        }
        else if (FS::Exists((texName + ".jpg").c_str()))
        {
            texture = FS::LoadTexture((texName + ".jpg").c_str());
        }
        else if (FS::Exists((texName + ".png").c_str()))
        {
            texture = FS::LoadTexture((texName + ".png").c_str());
        }
        else
        {
            texture = defaultTexture;
        }

        textures[tex.first] = texture;

        if (texture.width != 0 && texture.height != 0)
        {
            map.textureSizes[tex.first] = {(float)texture.width, (float)texture.height};
        }
    }

    std::vector<Model> models;

    for (auto &e : map.entities) {
        for (auto &b : e.brushes) {
            for (auto &f : b.faces)
            {
                // skip if texture name starts with common/
                if (f.texture.find("common/") == 0) continue;

                Vector3 verts[f.vertices.size()];
                Vector2 uvs[f.vertices.size()];
                vec3 n = f.GetNormal();
                Vector3 normal = { n.x, n.z, -n.y };
                f.textureSize = map.textureSizes[f.texture];
                
                for (size_t i = 0; i < f.vertices.size(); ++i)
                {
                    Vector3 vert = ToRay(f.parentBrush->vertices[f.vertices[i]]);
                    vec2 uv = f.GetUV(f.parentBrush->vertices[f.vertices[i]]);
                    verts[i] = { vert.x, vert.y, vert.z };
                    uvs[i]   = { uv.x, uv.y };
                }

                Mesh mesh = GenMeshPolyhedron(f.vertices.size(), verts, uvs, normal);
                Model model = LoadModelFromMesh(mesh);
                model.materialCount = 1;
                model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = textures[f.texture];
                models.push_back(model);
            }
        }
        for (auto &p : e.patches) {
            Mesh mesh = GenMeshFromPatch(p.vertices);
            Model model = LoadModelFromMesh(mesh);
            model.materialCount = 1;
            model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = textures[p.texture];
            models.push_back(model);
        }
    }

    bool disableCursor = false;
    std::vector<Color> FACE_COLORS = {
            LIGHTGRAY, GRAY, DARKGRAY, YELLOW, GOLD, ORANGE, PINK, RED, MAROON, GREEN, LIME,
            DARKGREEN, SKYBLUE, BLUE, DARKBLUE, PURPLE, VIOLET, DARKPURPLE, BEIGE, BROWN, DARKBROWN
    };

    while (!WindowShouldClose())
    {
        // toggle free-look
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) disableCursor = !disableCursor;

        if (disableCursor)
            HideCursor();
        else
            ShowCursor();

        if (IsCursorHidden())
            UpdateCamera(&camera, CAMERA_FREE);

        if (disableCursor)
            SetMousePosition(GetScreenWidth() / 2, GetScreenHeight() / 2);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);
            for (auto &model : models) {
                DrawModel(model, { 0.0f, 0.0f, 0.0f }, 1.0f, WHITE);
            }
        EndMode3D();

        EndDrawing();
    }

    FS::Close();
    for (auto &m : models)   UnloadModel(m);
    for (auto &kv: textures) UnloadTexture(kv.second);
    UnloadTexture(defaultTexture);

    CloseWindow();
    return 0;
}
