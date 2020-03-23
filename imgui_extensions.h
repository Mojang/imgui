#pragma once

namespace ImGui
{

float BezierValue(float dt01, float P[4]);
int Bezier(const char *label, float P[4]);
void ShowBezierDemo();

}
