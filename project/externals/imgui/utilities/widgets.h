#pragma once
#ifdef _DEBUG
#include "imgui.h"
#include "drawing.h"
#endif // _DEBUG

#ifdef _DEBUG
namespace ax {
	namespace Widgets {

		using Drawing::IconType;

		void Icon(const ImVec2& size, IconType type, bool filled, const ImVec4& color = ImVec4(1, 1, 1, 1), const ImVec4& innerColor = ImVec4(0, 0, 0, 0));

	} // namespace Widgets
} // namespace ax
#endif // _DEBUG