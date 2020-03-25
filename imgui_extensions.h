#pragma once

#ifdef IMGUI_USER_CONFIG
#include IMGUI_USER_CONFIG
#endif
#if !defined(IMGUI_DISABLE_INCLUDE_IMCONFIG_H) || defined(IMGUI_INCLUDE_IMCONFIG_H)
#include "imconfig.h"
#endif

#ifndef IMGUI_DISABLE

namespace ImGui
{

float BezierValue(float dt01, float P[4]);
int Bezier(const char *label, float P[4]);
void ShowBezierDemo();

}

#endif // #ifndef IMGUI_DISABLE
