#include "raylib.h"
#include <vector>
#include <string>
#include <math.h>
#include <algorithm>
#include <memory>

// --- ADVANCED DEFINES ---
const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 800;
const Color GRID_CYAN = { 0, 255, 255, 255 };
const Color SARK_RED = { 255, 20, 20, 255 };
const Color DATA_WHITE = { 245, 250, 255, 255 };
const int MAX_PARTICLES = 1000;

// --- COMPONENT SYSTEM ---
enum EntityType { BUG, RECOGNIZER, LASER, ENERGY, DEBRIS, BULLET };
enum DroneState { SLEEP, PATROL, CRUSH, ASCEND };

struct Particle {
    Vector2 pos;
    Vector2 vel;
    float life;
    float maxLife;
    Color color;
};

struct GameObject {
    Rectangle rec;
    Vector2 vel;
    EntityType type;
    int state;
    float timer;
    float paramA; // General purpose logic float
    bool active;
    Color tint;
};

struct ParallaxLayer {
    std::vector<Rectangle> rects;
    float speed;
    Color color;
};

// --- THE ENGINE ---
class TronEngine {
private:
    // Player Physics
    Rectangle player = { 200, 100, 32, 44 };
    Vector2 playerVel = { 0, 0 };
    float energy = 100.0f;
    bool isGrounded = false;
    bool isCrouching = false;
    float jumpForce = -23.0f;
    float gravity = 1.35f;

    // World Data
    std::vector<Rectangle> collisionTiles;
    std::vector<GameObject> entities;
    std::vector<Particle> particlePool;
    std::vector<ParallaxLayer> bgLayers;
    
    // Camera & VFX
    Camera2D camera = { 0 };
    float shakeIntensity = 0.0f;
    float globalTime = 0.0f;

    // Dialogue Queue
    std::vector<std::string> quotes;
    std::string currentQuote = "";
    float quoteTimer = 0.0f;

public:
    TronEngine() {
        InitEngine();
        GenerateSectors(10000); // Massive world generation
    }

    void InitEngine() {
        camera.zoom = 1.0f;
        camera.offset = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
        
        // Setup Parallax Tiers (4 Layers of Depth)
        for (int i = 0; i < 4; i++) {
            ParallaxLayer layer;
            layer.speed = 0.1f * (i + 1);
            layer.color = { 0, (unsigned char)(20 + (i * 20)), (unsigned char)(40 + (i * 30)), 120 };
            for (int j = 0; j < 100; j++) {
                layer.rects.push_back({ (float)GetRandomValue(-2000, 100000), (float)GetRandomValue(50, 400), (float)GetRandomValue(40, 150), 800 });
            }
            bgLayers.push_back(layer);
        }

        quotes = { 
            "SARK: 'Bring in the logic probe!'", 
            "MCP: 'End of line.'", 
            "FLYNN: 'It's all in the wrists.'",
            "TRON: 'I fight for the Users!'" 
        };
    }

    void GenerateSectors(int totalWidth) {
        float floorY = 600;
        for (int i = 0; i < totalWidth; i++) {
            float x = i * 75;
            
            // 1. DYNAMIC FLOOR
            if (i % 50 == 0) floorY += GetRandomValue(-2, 2) * 40;
            collisionTiles.push_back({ x, floorY, 77, 1000 });

            // 2. HIGH-ALTITUDE DATA CACHES (Secret Paths)
            if (i % 30 == 0) {
                collisionTiles.push_back({ x, floorY - 240, 150, 20 });
                if (GetRandomValue(0, 10) > 7) {
                    GameObject e = { {x + 50, floorY - 290, 30, 45}, {0,0}, ENERGY, 0, 0, 0, true, DATA_WHITE };
                    entities.push_back(e);
                }
            }

            // 3. RECOGNIZER SQUADRONS (Complex Attack Patterns)
            if (i % 140 == 0) {
                for (int j = 0; j < 3; j++) {
                    GameObject d = { {x + (j * 130), floorY - 700, 110, 120}, {0,0}, RECOGNIZER, SLEEP, (float)j * 0.5f, floorY - 700, true, SARK_RED };
                    entities.push_back(d);
                }
            }

            // 4. GRID BUGS (Movie Red-Outline Crawlers)
            if (i % 25 == 0) {
                GameObject bug = { {x, floorY - 25, 45, 25}, {4.5f, 0}, BUG, 0, x, 250.0f, true, SARK_RED };
                entities.push_back(bug);
            }

            // 5. MOVING LOGIC GATES (Up/Down Obstacles)
            if (i % 65 == 0) {
                GameObject laser = { {x + 20, floorY - 300, 15, 140}, {0, 6.0f}, LASER, 0, floorY - 300, 220.0f, true, SARK_RED };
                entities.push_back(laser);
            }
        }
    }

    void TriggerDeRes(Vector2 pos, Color col) {
        for (int i = 0; i < 20; i++) {
            if (particlePool.size() < MAX_PARTICLES) {
                particlePool.push_back({ pos, {(float)GetRandomValue(-10, 10), (float)GetRandomValue(-10, 10)}, 1.0f, 1.0f, col });
            }
        }
    }

    void UpdatePhysics() {
        // --- CROUCH LOGIC (50% Height: 44 to 22) ---
        if (IsKeyDown(KEY_DOWN) && isGrounded) {
            if (!isCrouching) { player.height = 22; player.y += 22; isCrouching = true; }
        } else if (isCrouching) {
            player.height = 44; player.y -= 22; isCrouching = false;
        }

        // Horizontal
        float speed = isCrouching ? 4.0f : 8.0f;
        if (IsKeyDown(KEY_RIGHT)) playerVel.x = speed;
        else if (IsKeyDown(KEY_LEFT)) playerVel.x = -speed;
        else playerVel.x *= 0.85f;

        // Jump
        if (IsKeyPressed(KEY_SPACE) && isGrounded && !isCrouching) {
            playerVel.y = jumpForce;
            isGrounded = false;
        }

        playerVel.y += gravity;
        player.x += playerVel.x;
        player.y += playerVel.y;
        isGrounded = false;

        // Tile Collisions
        for (auto& tile : collisionTiles) {
            if (CheckCollisionRecs(player, tile)) {
                if (playerVel.y > 0 && player.y + player.height - playerVel.y <= tile.y) {
                    player.y = tile.y - player.height;
                    playerVel.y = 0;
                    isGrounded = true;
                } else if (playerVel.x != 0) {
                    // Side collision logic
                }
            }
        }
    }

    void UpdateEntities(float dt) {
        for (auto& e : entities) {
            if (!e.active) continue;

            switch (e.type) {
                case BUG:
                    e.rec.x += e.vel.x;
                    if (abs(e.rec.x - e.timer) > e.paramA) e.vel.x *= -1;
                    if (CheckCollisionRecs(player, e.rec)) { energy -= 1.0f; TriggerDeRes({e.rec.x, e.rec.y}, SARK_RED); }
                    break;

                case RECOGNIZER:
                    if (e.state == SLEEP) {
                        e.timer -= dt;
                        if (e.timer <= 0) e.state = PATROL;
                    } else if (e.state == PATROL) { // CRUSH ATTACK
                        e.rec.y += 20.0f;
                        if (e.rec.y > player.y) { 
                            e.state = ASCEND; 
                            if(abs(e.rec.x - player.x) < 500) shakeIntensity = 20.0f;
                        }
                    } else if (e.state == ASCEND) {
                        e.rec.y -= 5.0f;
                        if (e.rec.y <= e.paramA) { e.state = SLEEP; e.timer = (float)GetRandomValue(3, 8); }
                    }
                    if (CheckCollisionRecs(player, e.rec)) energy -= 5.0f;
                    break;

                case LASER:
                    e.rec.y += e.vel.y;
                    if (abs(e.rec.y - e.timer) > e.paramA) e.vel.y *= -1;
                    if (CheckCollisionRecs(player, e.rec)) energy -= 2.0f;
                    break;

                case ENERGY:
                    e.timer += dt * 5;
                    e.rec.y += sin(e.timer) * 0.5f;
                    if (CheckCollisionRecs(player, e.rec)) {
                        energy = fmin(100.0f, energy + 40.0f);
                        e.active = false;
                        TriggerDeRes({e.rec.x, e.rec.y}, GRID_CYAN);
                    }
                    break;
            }
        }
    }

    void Update(float dt) {
        if (energy <= 0) return;
        globalTime += dt;
        
        UpdatePhysics();
        UpdateEntities(dt);

        // Particle System Update
        for (int i = 0; i < (int)particlePool.size(); i++) {
            particlePool[i].pos.x += particlePool[i].vel.x;
            particlePool[i].pos.y += particlePool[i].vel.y;
            particlePool[i].life -= dt;
            if (particlePool[i].life <= 0) {
                particlePool.erase(particlePool.begin() + i);
                i--;
            }
        }

        // Camera Logic
        if (shakeIntensity > 0) shakeIntensity *= 0.9f;
        camera.target = { player.x, player.y };
        camera.offset = { SCREEN_WIDTH/2.0f + GetRandomValue(-shakeIntensity, shakeIntensity), 
                          SCREEN_HEIGHT/2.0f + GetRandomValue(-shakeIntensity, shakeIntensity) };

        // Dialogue System
        if (quoteTimer <= 0) {
            currentQuote = quotes[GetRandomValue(0, quotes.size()-1)];
            quoteTimer = 6.0f;
        } else quoteTimer -= dt;
    }

    void Draw() {
        BeginDrawing();
        ClearBackground({5, 5, 25, 255});
        
        BeginMode2D(camera);
            // Render Parallax
            for (auto& layer : bgLayers) {
                for (auto& r : layer.rects) {
                    float px = r.x + (camera.target.x * (1.0f - layer.speed));
                    DrawRectangle(px, r.y, r.width, r.height, layer.color);
                }
            }

            // Render Floor
            for (auto& tile : collisionTiles) {
                DrawRectangleRec(tile, BLACK);
                DrawRectangleLinesEx(tile, 2, GRID_CYAN);
            }

            // Render Entities
            for (auto& e : entities) {
                if (!e.active) continue;
                if (e.type == RECOGNIZER) { // U-SHAPE DRONE
                    DrawRectangleLinesEx(e.rec, 5, SARK_RED);
                    DrawRectangle(e.rec.x + 15, e.rec.y + 120, 25, 40, SARK_RED);
                    DrawRectangle(e.rec.x + 70, e.rec.y + 120, 25, 40, SARK_RED);
                    if (e.state == PATROL) DrawRectangle(e.rec.x + 45, e.rec.y + 120, 20, 1000, {255,0,0,80});
                } else if (e.type == BUG) {
                    DrawRectangleLinesEx(e.rec, 3, SARK_RED);
                } else if (e.type == ENERGY) {
                    DrawRectangleRec(e.rec, DATA_WHITE);
                    DrawRectangleLinesEx(e.rec, 3, GRID_CYAN);
                } else if (e.type == LASER) {
                    DrawRectangleRec(e.rec, SARK_RED);
                    DrawRectangleLinesEx(e.rec, 2, DATA_WHITE);
                }
            }

            // Particles
            for (auto& p : particlePool) {
                DrawRectangle(p.pos.x, p.pos.y, 4, 4, p.color);
            }

            // Player
            DrawRectangleRec(player, BLACK);
            DrawRectangleLinesEx(player, 3, isCrouching ? WHITE : GRID_CYAN);
        EndMode2D();

        // UI
        DrawRectangle(30, 30, energy * 4, 30, (energy > 30 ? GRID_CYAN : RED));
        DrawRectangleLinesEx({25, 25, 410, 40}, 2, WHITE);
        DrawText(currentQuote.c_str(), 450, 35, 20, DATA_WHITE);

        if (energy <= 0) DrawText("SYSTEM HALTED: END OF LINE.", 350, 400, 40, SARK_RED);
        EndDrawing();
    }
};

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "TRON: CORE_ENGINE_V20");
    SetTargetFPS(60);
    TronEngine* grid = new TronEngine();
    while (!WindowShouldClose()) {
        grid->Update(GetFrameTime());
        grid->Draw();
    }
    delete grid;
    CloseWindow();
    return 0;
}
