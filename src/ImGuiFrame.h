#pragma once
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_win32.h"
#include "../imgui/imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>

class ImGuiFrame
{
public:
	void Render();
	void EndFrame();
	void Initialize();
	void CreateFrame();

};