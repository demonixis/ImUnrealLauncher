#include "ModernOrangeStyle.h"
#include "Styles.h"

namespace Styles
{
    void SetModernOrangeStyle()
    {
        auto& style = ImGui::GetStyle();
        auto& colors = ImGui::GetStyle().Colors;
    	auto& io = ImGui::GetIO();

    	// Fonts
        //main_font = io.Fonts->AddFontFromFileTTF("Assets/Roboto-Regular.ttf", 20);
        main_font = io.Fonts->AddFontDefault();
    	IM_ASSERT(main_font != nullptr);

	    // Colors
		
		// Window backgrounds
		colors[ImGuiCol_WindowBg] = ModernOrangeTheme::bg_medium;
		colors[ImGuiCol_ChildBg] = ModernOrangeTheme::bg_dark;
		colors[ImGuiCol_PopupBg] = ModernOrangeTheme::bg_popup;
		
		// Borders
		colors[ImGuiCol_Border] = ModernOrangeTheme::border;
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		
		// Title bars
		colors[ImGuiCol_TitleBg] = ModernOrangeTheme::bg_dark;
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ModernOrangeTheme::bg_dark;
		
		// Menu bars
		colors[ImGuiCol_MenuBarBg] = ModernOrangeTheme::bg_dark;
		
		// Text
		colors[ImGuiCol_Text] = ModernOrangeTheme::text_primary;
		colors[ImGuiCol_TextDisabled] = ModernOrangeTheme::text_disabled;
		colors[ImGuiCol_TextSelectedBg] = ImVec4(ModernOrangeTheme::orange_primary.x,
			ModernOrangeTheme::orange_primary.y,
			ModernOrangeTheme::orange_primary.z, 0.35f);
		
		// Frames
		colors[ImGuiCol_FrameBg] = ModernOrangeTheme::bg_dark;
		colors[ImGuiCol_FrameBgHovered] = ModernOrangeTheme::bg_light;
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		
		// Buttons
		colors[ImGuiCol_Button] = ImVec4(ModernOrangeTheme::orange_primary.x,
			ModernOrangeTheme::orange_primary.y,
			ModernOrangeTheme::orange_primary.z, 0.40f);
		colors[ImGuiCol_ButtonHovered] = ModernOrangeTheme::orange_hover;
		colors[ImGuiCol_ButtonActive] = ModernOrangeTheme::orange_active;
		
		// Headers
		colors[ImGuiCol_Header] = ImVec4(ModernOrangeTheme::orange_primary.x,
			ModernOrangeTheme::orange_primary.y,
			ModernOrangeTheme::orange_primary.z, 0.30f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(ModernOrangeTheme::orange_primary.x,
			ModernOrangeTheme::orange_primary.y,
			ModernOrangeTheme::orange_primary.z, 0.50f);
		colors[ImGuiCol_HeaderActive] = ImVec4(ModernOrangeTheme::orange_primary.x,
			ModernOrangeTheme::orange_primary.y,
			ModernOrangeTheme::orange_primary.z, 0.70f);
		
		// Separators
		colors[ImGuiCol_Separator] = ModernOrangeTheme::separator;
		colors[ImGuiCol_SeparatorHovered] = ModernOrangeTheme::orange_hover;
		colors[ImGuiCol_SeparatorActive] = ModernOrangeTheme::orange_active;
		
		// Resize grip
		colors[ImGuiCol_ResizeGrip] = ImVec4(ModernOrangeTheme::orange_primary.x,
			ModernOrangeTheme::orange_primary.y,
			ModernOrangeTheme::orange_primary.z, 0.25f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(ModernOrangeTheme::orange_primary.x,
			ModernOrangeTheme::orange_primary.y,
			ModernOrangeTheme::orange_primary.z, 0.50f);
		colors[ImGuiCol_ResizeGripActive] = ModernOrangeTheme::orange_primary;
		
		// Tabs
		colors[ImGuiCol_Tab] = ImVec4(ModernOrangeTheme::orange_primary.x,
			ModernOrangeTheme::orange_primary.y,
			ModernOrangeTheme::orange_primary.z, 0.20f);
		colors[ImGuiCol_TabHovered] = ModernOrangeTheme::orange_hover;
		colors[ImGuiCol_TabActive] = ImVec4(ModernOrangeTheme::orange_primary.x,
			ModernOrangeTheme::orange_primary.y,
			ModernOrangeTheme::orange_primary.z, 0.60f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		
		/*
		// Docking
		colors[ImGuiCol_DockingPreview] = ImVec4(ModernOrangeTheme::orange_primary.x,
			ModernOrangeTheme::orange_primary.y,
			ModernOrangeTheme::orange_primary.z, 0.10f);
		colors[ImGuiCol_DockingEmptyBg] = ModernOrangeTheme::bg_dark;*/
		
		// Checkboxes and radio buttons
		colors[ImGuiCol_CheckMark] = ModernOrangeTheme::orange_primary;
		
		// Sliders
		colors[ImGuiCol_SliderGrab] = ModernOrangeTheme::orange_primary;
		colors[ImGuiCol_SliderGrabActive] = ModernOrangeTheme::orange_active;
		
		// Scrollbars
		colors[ImGuiCol_ScrollbarBg] = ModernOrangeTheme::bg_dark;
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		
		// Plots
		colors[ImGuiCol_PlotLines] = ModernOrangeTheme::orange_primary;
		colors[ImGuiCol_PlotLinesHovered] = ModernOrangeTheme::orange_hover;
		colors[ImGuiCol_PlotHistogram] = ModernOrangeTheme::orange_primary;
		colors[ImGuiCol_PlotHistogramHovered] = ModernOrangeTheme::orange_hover;
		
		// Tables
		colors[ImGuiCol_TableHeaderBg] = ModernOrangeTheme::bg_light;
		colors[ImGuiCol_TableBorderStrong] = ModernOrangeTheme::border;
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.03f);
		
		// Drag and drop
		colors[ImGuiCol_DragDropTarget] = ModernOrangeTheme::orange_primary;
		
		// Navigation
		colors[ImGuiCol_NavHighlight] = ModernOrangeTheme::orange_primary;
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);

    	// Style settings
	
    	style.WindowPadding = ImVec2(12.0f, 12.0f);
    	style.FramePadding = ImVec2(8.0f, 4.0f);
    	style.ItemSpacing = ImVec2(8.0f, 6.0f);
    	style.ItemInnerSpacing = ImVec2(6.0f, 6.0f);
    	style.IndentSpacing = 20.0f;
    	style.ScrollbarSize = 14.0f;
    	style.GrabMinSize = 12.0f;
	
    	// Roundings
    	style.WindowRounding = 8.0f;
    	style.ChildRounding = 6.0f;
    	style.FrameRounding = 6.0f;
    	style.PopupRounding = 6.0f;
    	style.ScrollbarRounding = 12.0f;
    	style.GrabRounding = 6.0f;
    	style.TabRounding = 6.0f;
	
    	// Borders
    	style.WindowBorderSize = 1.0f;
    	style.ChildBorderSize = 1.0f;
    	style.PopupBorderSize = 1.0f;
    	style.FrameBorderSize = 0.0f;
    	style.TabBorderSize = 0.0f;
	
    	// Text alignment
    	style.WindowTitleAlign = ImVec2(0.50f, 0.50f);
    	style.ButtonTextAlign = ImVec2(0.50f, 0.50f);
	
    	// Anti-aliasing
    	style.AntiAliasedLines = true;
    	style.AntiAliasedLinesUseTex = true;
    	style.AntiAliasedFill = true;
    }
}
