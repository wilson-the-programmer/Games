#include "raylib.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#define TRON_CYAN  Color{ 0, 255, 255, 255 }
#define TRON_PINK  Color{ 255, 0, 255, 255 }
#define TRON_YELLOW Color{ 255, 255, 0, 255 }
#define SHAPE_CYAN Color{ 0, 100, 150, 80 }

struct Monolith {
    Vector2 pos;
    float width, height, speed;
};

int main() {
    const int screenWidth = 850;
    const int screenHeight = 600; 
    InitWindow(screenWidth, screenHeight, "YORI_SHELL_V3_4_SHORTCUTS");
    SetConfigFlags(FLAG_WINDOW_TOPMOST);
    SetTargetFPS(60);

    std::vector<Monolith> towers;
    for(int i = 0; i < 6; i++) {
        towers.push_back({ {(float)GetRandomValue(0, 850), (float)GetRandomValue(0, 600)}, 
                           (float)GetRandomValue(40, 90), (float)GetRandomValue(100, 250), 
                           (float)GetRandomValue(2, 5) * 0.1f });
    }

    std::vector<std::pair<std::string, bool>> outputLines;
    outputLines.push_back({"YORI_OS Shell [Version 3.4]", false});
    outputLines.push_back({"Shortcuts: CTRL+A, CTRL+E, CTRL+U: ACTIVE", false});
    
    std::string currentInput = "";
    int inputCursor = 0;
    std::vector<std::string> history;
    int historyIndex = -1, frames = 0;

    while (!WindowShouldClose()) {
        frames++;

        // --- BACKGROUND ---
        for(auto &t : towers) {
            t.pos.x -= t.speed;
            if (t.pos.x + t.width < 0) { t.pos.x = screenWidth + 20; t.pos.y = GetRandomValue(0, 400); }
        }

        // --- SHORTCUTS (CTRL) ---
        bool ctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
        if (ctrl) {
            if (IsKeyPressed(KEY_A)) inputCursor = 0;
            if (IsKeyPressed(KEY_E)) inputCursor = (int)currentInput.length();
            if (IsKeyPressed(KEY_U)) { currentInput = ""; inputCursor = 0; }
        }

        // --- STANDARD INPUT ---
        int key = GetCharPressed();
        while (key > 0) {
            // Only add text if Ctrl isn't being held (to avoid 'a' or 'e' appearing when using shortcuts)
            if (!ctrl && key >= 32 && key <= 125) {
                currentInput.insert(inputCursor, 1, (char)key);
                inputCursor++;
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_LEFT) && inputCursor > 0) inputCursor--;
        if (IsKeyPressed(KEY_RIGHT) && inputCursor < (int)currentInput.length()) inputCursor++;

        if (IsKeyPressed(KEY_BACKSPACE) && inputCursor > 0) {
            currentInput.erase(inputCursor - 1, 1);
            inputCursor--;
        }

        // --- EXECUTION ---
        if (IsKeyPressed(KEY_ENTER)) {
            outputLines.push_back({"$ " + currentInput, true});
            if (!currentInput.empty()) {
                history.push_back(currentInput);
                historyIndex = -1;
                if (currentInput == "clear") outputLines.clear();
                else {
                    system((currentInput + " > cmd_out.txt 2>&1").c_str());
                    std::ifstream file("cmd_out.txt");
                    std::string line;
                    while (std::getline(file, line)) outputLines.push_back({line, false});
                }
            }
            currentInput = "";
            inputCursor = 0;
        }

        // --- HISTORY ---
        if (IsKeyPressed(KEY_UP) && !history.empty()) {
            if (historyIndex == -1) historyIndex = (int)history.size() - 1;
            else if (historyIndex > 0) historyIndex--;
            currentInput = history[historyIndex];
            inputCursor = currentInput.length();
        }

        BeginDrawing();
            ClearBackground({ 1, 2, 8, 255 });

            for(auto &t : towers) {
                DrawRectangleLinesEx({t.pos.x, t.pos.y, t.width, t.height}, 1, SHAPE_CYAN);
                DrawLineV({t.pos.x, t.pos.y}, {t.pos.x + 20, t.pos.y - 20}, SHAPE_CYAN);
                DrawLine(t.pos.x + 20, t.pos.y - 20, t.pos.x + t.width + 20, t.pos.y - 20, SHAPE_CYAN);
            }

            int startY = 60, lineSpacing = 24, maxVisibleLines = 18; 
            int startIdx = (outputLines.size() >= maxVisibleLines) ? (int)outputLines.size() - maxVisibleLines + 1 : 0;
            for (int i = startIdx; i < (int)outputLines.size(); i++) {
                Color textColor = outputLines[i].second ? TRON_YELLOW : Color{ 160, 255, 255, 255 };
                DrawText(outputLines[i].first.c_str(), 25, startY + ((i - startIdx) * lineSpacing), 20, textColor);
            }

            int currentLinePos = (outputLines.size() < maxVisibleLines) ? (int)outputLines.size() : maxVisibleLines - 1;
            int promptY = startY + (currentLinePos * lineSpacing);
            int promptWidth = MeasureText("$ ", 20);

            DrawText("$ ", 25, promptY, 20, TRON_CYAN);
            DrawText(currentInput.c_str(), 25 + promptWidth, promptY, 20, TRON_YELLOW);

            if ((frames / 30) % 2 == 0) {
                std::string visibleToCursor = currentInput.substr(0, inputCursor);
                int cursorXOffset = MeasureText(visibleToCursor.c_str(), 20);
                DrawRectangle(25 + promptWidth + cursorXOffset, promptY, 2, 20, TRON_PINK);
            }

            DrawRectangleLinesEx({10, 40, 830, 540}, 2, TRON_CYAN);
            DrawText("SYSTEM_USER_SHORTCUTS_ON", 20, 15, 15, TRON_CYAN);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}