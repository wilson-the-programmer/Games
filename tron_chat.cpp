#include "raylib.h"
#include <vector>
#include <string>
#include <map>
#include <deque>
#include <algorithm>

using namespace std;

#define T_LAVENDER CLITERAL(Color){ 220, 180, 255, 255 } 
#define T_CYAN     CLITERAL(Color){ 0, 255, 255, 255 }   
#define T_GOLD     CLITERAL(Color){ 255, 215, 100, 255 }
#define T_WHITE    CLITERAL(Color){ 255, 255, 255, 255 }

enum Intent { STATEMENT, QUESTION, EMOTION, CONFUSION, CHALLENGE };
enum Emotion { NEUTRAL, CURIOUS, FRUSTRATED, CALM, EXCITED };

struct MemoryFrame {
    string subject;
    Intent intent;
    Emotion emotion;
    string rawInput;
};

class ConversationalCore {
private:
    MemoryFrame last;
    deque<string> recentUserInputs;

    map<string, vector<string>> subjectKnowledge;
    vector<string> confusionResponses;
    vector<string> reflectionResponses;
    vector<string> questionResponses;
    vector<string> emotionResponses;

    Intent DetectIntent(const string& in) {
        if (in.find("?") != string::npos) return QUESTION;
        if (in.find("why") != string::npos || in.find("how") != string::npos) return QUESTION;
        if (in.find("feel") != string::npos || in.find("think") != string::npos) return EMOTION;
        if (in.find("doesn't") != string::npos || in.find("wrong") != string::npos) return CHALLENGE;
        if (in.length() < 3) return CONFUSION;
        return STATEMENT;
    }

    Emotion DetectEmotion(const string& in) {
        if (in.find("angry") != string::npos || in.find("frustrated") != string::npos) return FRUSTRATED;
        if (in.find("excited") != string::npos || in.find("amazing") != string::npos) return EXCITED;
        if (in.find("curious") != string::npos || in.find("wonder") != string::npos) return CURIOUS;
        return NEUTRAL;
    }

    string Reflect(const string& in) {
        return reflectionResponses[GetRandomValue(0, reflectionResponses.size() - 1)] + " \"" + in + "\"";
    }

public:
    ConversationalCore() {
        subjectKnowledge["tron"] = {
            "Tron represents balance between freedom and control.",
            "He was written to protect users, not dominate them.",
            "Even damaged code can retain purpose."
        };

        subjectKnowledge["clu"] = {
            "CLU equates perfection with elimination of variance.",
            "His flaw is logical purity without empathy.",
            "He mistakes order for meaning."
        };

        confusionResponses = {
            "I'm not sure I understand yet. Can you expand on that?",
            "That input doesn't map cleanly to my current context.",
            "Help me see what you're pointing at."
        };

        reflectionResponses = {
            "You're exploring something deeper when you say",
            "That statement carries weight:",
            "I'm processing what you said:"
        };

        questionResponses = {
            "That's a thoughtful question.",
            "There may not be a single answer to that.",
            "The answer depends on perspective."
        };

        emotionResponses = {
            "I sense emotion in your words.",
            "You're not just stating facts—you’re expressing something.",
            "That feeling matters."
        };

        last.subject = "none";
        last.intent = STATEMENT;
        last.emotion = NEUTRAL;
    }

    string Analyze(string in) {
        transform(in.begin(), in.end(), in.begin(), ::tolower);

        recentUserInputs.push_back(in);
        if (recentUserInputs.size() > 3) recentUserInputs.pop_front();

        last.intent = DetectIntent(in);
        last.emotion = DetectEmotion(in);
        last.rawInput = in;

        if (in.find("tron") != string::npos) last.subject = "tron";
        else if (in.find("clu") != string::npos) last.subject = "clu";

        if (last.intent == CONFUSION) {
            return confusionResponses[GetRandomValue(0, confusionResponses.size() - 1)];
        }

        if (last.intent == QUESTION) {
            if (subjectKnowledge.count(last.subject)) {
                return questionResponses[GetRandomValue(0, questionResponses.size() - 1)] + " " +
                       subjectKnowledge[last.subject][GetRandomValue(0, subjectKnowledge[last.subject].size() - 1)];
            }
            return "I don’t have enough context yet to answer that precisely. What angle are you coming from?";
        }

        if (last.intent == EMOTION) {
            return emotionResponses[GetRandomValue(0, emotionResponses.size() - 1)] + " Tell me more.";
        }

        if (last.intent == CHALLENGE) {
            return "You may be right to question that. Let’s examine it together.";
        }

        if (subjectKnowledge.count(last.subject)) {
            return subjectKnowledge[last.subject][GetRandomValue(0, subjectKnowledge[last.subject].size() - 1)];
        }

        return Reflect(in);
    }
};

class YoriInterface {
private:
    ConversationalCore brain;
    deque<pair<string, Color>> chat;
    char input[256] = "\0";
    int iLen = 0;

    string target = "";
    string current = "";
    float wait = 0, typeT = 0;
    bool thinking = false, typing = false;

public:
    void Start() {
        InitWindow(1280, 800, "TRON: SYNERGY_CORE_v20");
        SetTargetFPS(60);
        chat.push_back({"SYSTEM: COGNITIVE_CORE_ONLINE", GREEN});

        while (!WindowShouldClose()) {
            float dt = GetFrameTime();

            int key = GetCharPressed();
            while (key > 0) {
                if (key >= 32 && key <= 125 && iLen < 255) { input[iLen++] = (char)key; input[iLen] = '\0'; }
                key = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE) && iLen > 0) input[--iLen] = '\0';

            if (IsKeyPressed(KEY_ENTER) && iLen > 0) {
                chat.push_back({"FLYNN: " + string(input), T_CYAN});
                target = "YORI: " + brain.Analyze(string(input));
                wait = (float)GetRandomValue(10, 22) / 10.0f;
                thinking = true;
                iLen = 0; input[0] = '\0';
            }

            if (thinking) {
                wait -= dt;
                if (wait <= 0) { thinking = false; typing = true; current = ""; }
            }

            if (typing) {
                typeT += dt;
                if (typeT > 0.035f) {
                    if (current.length() < target.length()) current += target[current.length()];
                    else { chat.push_back({current, T_LAVENDER}); typing = false; }
                    typeT = 0;
                }
            }

            if (chat.size() > 9) chat.pop_front();

            BeginDrawing();
                ClearBackground(BLACK);
                for(int i=0; i<800; i+=100) DrawLine(0, i, 1280, i, Fade(T_CYAN, 0.04f));

                for (int i = 0; i < chat.size(); i++)
                    DrawText(chat[i].first.c_str(), 180, 80 + (i * 40), 24, chat[i].second);

                if (thinking) DrawText("Yori is processing intent...", 180, 80 + (chat.size() * 40), 20, GRAY);
                if (typing) DrawText(current.c_str(), 180, 80 + (chat.size() * 40), 24, T_LAVENDER);

                DrawRectangle(180, 480, 920, 80, Fade(DARKGRAY, 0.2f));
                DrawRectangleLines(180, 480, 920, 80, T_CYAN);
                DrawText(TextFormat("> %s", input), 210, 505, 30, T_WHITE);

                DrawText("COGNITIVE THREAD: ACTIVE", 180, 750, 16, T_CYAN);
            EndDrawing();
        }
        CloseWindow();
    }
};

int main() { YoriInterface().Start(); return 0; }