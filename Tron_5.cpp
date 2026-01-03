#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <cmath>
#include <cstdlib>

#define GRID_SIZE 50
#define CELL_SIZE 1.0f
#define PLAYER_RADIUS 0.35f
#define PLAYER_SPEED 0.15f
#define DRONE_COUNT 6
#define FILE_COUNT 5
#define DISC_LIMIT 3

#define TRON_WHITE      (Color){230, 240, 255, 255}
#define TRON_BLUE_GRID  (Color){0, 120, 255, 255}
#define TRON_BLUE_GLOW  (Color){100, 200, 255, 255}
#define TRON_WALL_VOID  (Color){5, 5, 15, 255}
#define TRON_DRONE      (Color){255, 80, 80, 255}
#define TRON_FILE       (Color){0, 255, 100, 255}

int MAP[GRID_SIZE][GRID_SIZE] = { /* same 50x50 map from your code */ };

bool IsCellSolid(float x, float z) {
    int gx = (int)(x + 0.5f);
    int gz = (int)(z + 0.5f);
    if (gx < 0 || gx >= GRID_SIZE || gz < 0 || gz >= GRID_SIZE) return true;
    return (MAP[gz][gx] == 1);
}

struct Drone {
    Vector3 position;
    Vector3 origin;
    Vector3 direction;
    float speed;
    bool diving;
};

struct SystemFile {
    Vector3 position;
    bool collected;
};

struct Disc {
    Vector3 position;
    Vector3 direction;
    bool active;
};

struct Yori {
    Vector3 position;
    bool visible;
};

int main() {
    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "TRON_1982: I/O TOWER ACCESS");

    Camera3D camera = {0};
    camera.position = (Vector3){2.0f, 0.6f, 2.0f};
    camera.target = (Vector3){3.0f, 0.6f, 3.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 70.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Vector3 ioTower = (Vector3){47.0f, 0.0f, 47.0f};
    bool victory = false;

    std::vector<Drone> drones;
    for (int i = 0; i < DRONE_COUNT; i++) {
        Drone d;
        d.position = (Vector3){float(5 + rand()%40), 5.0f + float(i), float(5 + rand()%40)};
        d.origin = d.position;
        float angle = float(rand() % 360) * 0.0174533f;
        d.direction = (Vector3){cos(angle), 0.0f, sin(angle)};
        d.speed = 0.08f + float(rand()%5)/50.0f;
        d.diving = false;
        drones.push_back(d);
    }

    std::vector<SystemFile> files;
    for (int i = 0; i < FILE_COUNT; i++) {
        SystemFile f;
        f.position = (Vector3){float(2 + rand()%46), 0.5f, float(2 + rand()%46)};
        f.collected = false;
        files.push_back(f);
    }

    std::vector<Disc> discs;
    int discCount = DISC_LIMIT;

    Yori yori;
    yori.position = (Vector3){25.0f, 0.5f, 25.0f};
    yori.visible = true;

    DisableCursor();
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (!victory) {
            Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
            Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
            forward.y = 0; right.y = 0;

            Vector3 move = {0};
            if (IsKeyDown(KEY_W)) move = Vector3Add(move, Vector3Scale(forward, PLAYER_SPEED));
            if (IsKeyDown(KEY_S)) move = Vector3Add(move, Vector3Scale(forward, -PLAYER_SPEED));
            if (IsKeyDown(KEY_A)) move = Vector3Add(move, Vector3Scale(right, -PLAYER_SPEED));
            if (IsKeyDown(KEY_D)) move = Vector3Add(move, Vector3Scale(right, PLAYER_SPEED));

            Vector3 nextX = {camera.position.x + move.x, camera.position.y, camera.position.z};
            Vector3 nextZ = {camera.position.x, camera.position.y, camera.position.z + move.z};

            float bufferX = (move.x > 0) ? PLAYER_RADIUS : -PLAYER_RADIUS;
            if (!IsCellSolid(nextX.x + bufferX, camera.position.z)) {
                camera.position.x = nextX.x;
                camera.target.x += move.x;
            }

            float bufferZ = (move.z > 0) ? PLAYER_RADIUS : -PLAYER_RADIUS;
            if (!IsCellSolid(camera.position.x, nextZ.z + bufferZ)) {
                camera.position.z = nextZ.z;
                camera.target.z += move.z;
            }

            UpdateCameraPro(&camera, (Vector3){0,0,0}, (Vector3){GetMouseDelta().x*0.1f, GetMouseDelta().y*0.1f,0}, 0);

            for (auto &f : files) {
                if (!f.collected && Vector3Distance(camera.position, f.position) < 1.0f) f.collected = true;
            }

            bool allFilesCollected = true;
            for (auto &f : files) if (!f.collected) allFilesCollected = false;
            if (allFilesCollected && Vector3Distance(camera.position, ioTower) < 2.0f) victory = true;

            for (auto &d : drones) {
                Vector3 dirToPlayer = Vector3Subtract(camera.position, d.position);
                float dist = Vector3Length(dirToPlayer);
                if (dist < 4.0f && d.position.y > 1.0f) d.diving = true;

                if (d.diving) {
                    Vector3 norm = Vector3Normalize(dirToPlayer);
                    d.position = Vector3Add(d.position, Vector3Scale(norm, d.speed*2.0f));
                    if (dist < 0.5f) d.diving = false;
                } else {
                    d.position = Vector3Add(d.position, Vector3Scale(d.direction, d.speed));
                    if (Vector3Distance(d.position, d.origin) > 5.0f) d.direction = Vector3Scale(d.direction, -1);
                }
            }

            if (IsKeyPressed(KEY_SPACE) && discCount > 0) {
                Disc disc;
                disc.position = camera.position;
                disc.direction = forward;
                disc.active = true;
                discs.push_back(disc);
                discCount--;
            }

            for (auto &disc : discs) {
                if (disc.active) disc.position = Vector3Add(disc.position, Vector3Scale(disc.direction, 0.5f));
                for (auto &d : drones) {
                    if (disc.active && Vector3Distance(disc.position, d.position) < 0.7f) d.position.y = 20.0f, disc.active = false;
                }
            }
        }

        BeginDrawing();
            ClearBackground(TRON_WHITE);
            BeginMode3D(camera);

                DrawPlane((Vector3){25,0,25}, (Vector2){100,100}, TRON_BLUE_GRID);
                DrawGrid(100,1.0f);

                DrawCylinderEx(ioTower, (Vector3){47,300,47},0.8f,0.8f,24,WHITE);
                DrawCylinderWiresEx(ioTower, (Vector3){47,300,47},1.0f,1.0f,24,TRON_BLUE_GLOW);

                for (int z=0; z<GRID_SIZE; z++) {
                    for (int x=0; x<GRID_SIZE; x++) {
                        if (MAP[z][x]==1) {
                            Vector3 pos={(float)x,2.0f,(float)z};
                            DrawCube(pos,1.0f,4.0f,1.0f,TRON_WALL_VOID);
                            DrawCubeWires(pos,1.0f,4.0f,1.0f,TRON_BLUE_GLOW);
                        }
                    }
                }

                for (auto &f : files) {
                    if (!f.collected) DrawCube(f.position,0.6f,0.6f,0.6f,TRON_FILE);
                }

                for (auto &d : drones) DrawSphere(d.position,0.5f,TRON_DRONE);

                for (auto &disc : discs) if (disc.active) DrawSphere(disc.position,0.2f,TRON_BLUE_GLOW);

                if (yori.visible) DrawCube(yori.position,0.7f,1.7f,0.7f,(Color){255,180,255,255});

            EndMode3D();

            if (victory) {
                DrawRectangle(0,0,screenWidth,screenHeight,Fade(TRON_BLUE_GLOW,0.8f));
                DrawText("YOU ESCAPED TO REAL WORLD",screenWidth/2-280,screenHeight/2-30,40,BLACK);
            } else {
                DrawRectangleLines(10,10,screenWidth-20,screenHeight-20,TRON_BLUE_GRID);
                DrawText("MISSION: COLLECT ALL SYSTEM FILES",30,30,20,BLACK);
                DrawText("PRESS SPACE TO FIRE DISC (" + std::to_string(discCount) + " LEFT)",30,60,20,TRON_BLUE_GRID);
            }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}