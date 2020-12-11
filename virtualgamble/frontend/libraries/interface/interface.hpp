#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS

#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

#include "../../../globaloperands.h"

class QuarcInterface
{
public:
	QuarcInterface();
private:
	int _selectedfile = 0;
};