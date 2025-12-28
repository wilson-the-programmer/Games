#include "raylib.h"
#include <string>
#include <vector>
#include <deque>
#include <cmath>
#include <cstdlib>
#include <sstream>
#include <algorithm>

using namespace std;

#define T_CYAN CLITERAL(Color){0,255,255,255}
#define T_LAVENDER CLITERAL(Color){220,180,255,255}
#define T_GOLD CLITERAL(Color){255,215,100,255}
#define T_WHITE CLITERAL(Color){255,255,255,255}

string Lower(string s){
    transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

bool ContainsMath(const string& s){
    for(char c: s)
        if(isdigit(c) || c=='+'||c=='-'||c=='*'||c=='/'||c=='('||c==')')
            return true;
    return false;
}

double Eval(const string& expr){
    string s = expr;
    replace(s.begin(), s.end(), '/', ' ');
    replace(s.begin(), s.end(), '*', ' ');
    replace(s.begin(), s.end(), '+', ' ');
    replace(s.begin(), s.end(), '-', ' ');

    stringstream ss(s);
    vector<double> nums;
    double v;
    while(ss >> v) nums.push_back(v);

    if(nums.size() < 2) return NAN;

    if(expr.find("/") != string::npos) return nums[0] / nums[1];
    if(expr.find("*") != string::npos) return nums[0] * nums[1];
    if(expr.find("+") != string::npos) return nums[0] + nums[1];
    if(expr.find("-") != string::npos) return nums[0] - nums[1];

    return NAN;
}

class Brain {
    vector<string> openers{
        "I think",
        "From my perspective",
        "It seems to me",
        "I understand that",
        "I am aware that"
    };

    vector<string> verbs{
        "you are exploring",
        "you are asking about",
        "you are considering",
        "you are testing",
        "you are questioning"
    };

    vector<string> endings{
        "something important.",
        "a deeper idea.",
        "the structure of things.",
        "how systems behave.",
        "your own reasoning."
    };

public:
    string Respond(string input){
        input = Lower(input);

        if(input.find("name") != string::npos)
            return "My name is Yori.";

        if(input.find("who are you") != string::npos)
            return "I am Yori. A program inside the Grid.";

        if(input.find("tron") != string::npos)
            return "Tron exists to protect users and restore balance.";

        if(ContainsMath(input)){
            double r = Eval(input);
            if(!isnan(r)){
                return "The result is " + to_string(r);
            }
            return "That expression does not resolve cleanly.";
        }

        string reply =
            openers[GetRandomValue(0, openers.size()-1)] + " " +
            verbs[GetRandomValue(0, verbs.size()-1)] + " " +
            endings[GetRandomValue(0, endings.size()-1)];

        return reply;
    }
};

class TronUI {
    Brain brain;
    deque<pair<string, Color>> chat;
    char input[256]{0};
    int len = 0;

    string target, current;
    float wait = 0, typeT = 0;
    bool thinking = false, typing = false;

public:
    void Run(){
        InitWindow(1280, 800, "TRON SYNERGY CORE");
        SetTargetFPS(60);

        chat.push_back({"SYSTEM ONLINE", T_GOLD});

        while(!WindowShouldClose()){
            float dt = GetFrameTime();

            int key = GetCharPressed();
            while(key > 0){
                if(key >= 32 && key <= 125 && len < 255){
                    input[len++] = (char)key;
                    input[len] = 0;
                }
                key = GetCharPressed();
            }

            if(IsKeyPressed(KEY_BACKSPACE) && len > 0)
                input[--len] = 0;

            if(IsKeyPressed(KEY_ENTER) && len > 0){
                chat.push_back({"USER: " + string(input), T_CYAN});
                target = "YORI: " + brain.Respond(string(input));
                current.clear();
                wait = (float)GetRandomValue(10, 40) / 10.0f;
                thinking = true;
                typing = false;
                len = 0;
                input[0] = 0;
            }

            if(thinking){
                wait -= dt;
                if(wait <= 0){
                    thinking = false;
                    typing = true;
                }
            }

            if(typing){
                typeT += dt;
                if(typeT > 0.03f){
                    if(current.size() < target.size())
                        current += target[current.size()];
                    else{
                        chat.push_back({current, T_LAVENDER});
                        typing = false;
                    }
                    typeT = 0;
                }
            }

            if(chat.size() > 10) chat.pop_front();

            BeginDrawing();
            ClearBackground(BLACK);

            for(int i=0;i<800;i+=100)
                DrawLine(0,i,1280,i,Fade(T_CYAN,0.05f));

            for(int i=0;i<chat.size();i++)
                DrawText(chat[i].first.c_str(),180,80+i*40,24,chat[i].second);

            if(thinking)
                DrawText("Yori is thinking...",180,80+chat.size()*40,20,GRAY);

            if(typing)
                DrawText(current.c_str(),180,80+chat.size()*40,24,T_LAVENDER);

            DrawRectangleLines(180,500,920,80,T_CYAN);
            DrawText(TextFormat("> %s", input),200,525,30,T_WHITE);

            EndDrawing();
        }
        CloseWindow();
    }
};

int main(){
    TronUI().Run();
    return 0;
}