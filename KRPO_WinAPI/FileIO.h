#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <windows.h>
#include "ColorLib.h"
#include "DesignLayout.h"

extern int design_width;
extern int design_height;
extern float scale;
extern std::vector<DesignLayout> layout;

void OpenFile(HWND hWnd, std::vector<DesignLayout>& layout);
void SaveFile(HWND hWnd, std::vector<DesignLayout>& layout);
