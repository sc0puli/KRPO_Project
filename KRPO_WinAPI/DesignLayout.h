#pragma once
#include <string>
#include <windows.h>

class DesignLayout
{
public:
    std::string shape;
    std::string layer;
    RECT rect;
    COLORREF color;
};
