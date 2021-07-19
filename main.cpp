#include <iostream>
#include <map>
#include <string>
#include "raylib.h"
#include "mapFiles.h"

typedef struct Tri {
    Vector3 p1, p2, p3;
    Color color;
} Tri;

int main() {
    std::vector<Entity> mapEnts;
    std::vector<Tri> tris;
    Color colors[21] = {LIGHTGRAY, GRAY, DARKGRAY, YELLOW, GOLD, ORANGE, PINK, RED, MAROON, GREEN, LIME, DARKGREEN, SKYBLUE, BLUE, DARKBLUE, PURPLE, VIOLET, DARKPURPLE, BEIGE, BROWN, DARKBROWN };
    int c = 0;
    if(parseMap(mapEnts, "test.map"))
    {
        for (Entity ent: mapEnts)
        {
            if (ent.brushCount == 0)
                continue;
            
            for (Brush brush: ent.brushes)
            {
                brush.getIntersectionPoints();
                for (Side side: brush.sides)
                {
                    if (side.pointCount < 3)
                    {
                        continue;
                    }
                    side.sortVertices();
                    tris.push_back({
                        {side.points[0].x / 30.f, side.points[0].z / 30.f, side.points[0].y / -30.f},
                        {side.points[side.pointCount - 1].x / 30.f, side.points[side.pointCount - 1].z / 30.f, side.points[side.pointCount - 1].y / -30.f},
                        {side.points[1].x / 30.f, side.points[1].z / 30.f, side.points[1].y / -30.f},
                        colors[c++ % 20]

                        });
                    for (int i = 1; i < (side.pointCount / 2); i++) {
                        tris.push_back({
                            {side.points[i].x / 30.f, side.points[i].z / 30.f, side.points[i].y / -30.f},
                            {side.points[side.pointCount - i].x / 30.f, side.points[side.pointCount - i].z / 30.f, side.points[side.pointCount - i].y / -30.f},
                            {side.points[i + 1].x / 30.f, side.points[i + 1].z / 30.f, side.points[i + 1].y / -30.f},
                            colors[c++ % 20]
                            });

                        tris.push_back({
                            {side.points[side.pointCount - i].x / 30.f, side.points[side.pointCount - i].z / 30.f, side.points[side.pointCount - i].y / -30.f},
                            {side.points[side.pointCount - i - 1].x / 30.f, side.points[side.pointCount - i - 1].z / 30.f, side.points[side.pointCount - i - 1].y / -30.f},
                            {side.points[i + 1].x / 30.f, side.points[i + 1].z / 30.f, side.points[i + 1].y / -30.f},
                            colors[c++ % 20]
                        });
                    }
                }
            }
        }
    }

    const int screenWidth = 1360;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "map viewer");

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = { 4.0f, 2.0f, 4.0f };
    camera.target = { 0.0f, -1.8f, 0.0f };
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.type = CAMERA_PERSPECTIVE;
    SetCameraMode(camera, CAMERA_FREE);
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        UpdateCamera(&camera);          // Update camera

        if (IsKeyDown('Z')) camera.target = { 0.0f, 0.0f, 0.0f };

        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                for (Tri tri: tris)
                {
                    DrawTriangle3D(tri.p1, tri.p2, tri.p3, tri.color);
                }

                DrawGrid(10, 1.0f);

            EndMode3D();

            DrawFPS(10, 10);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}