#include"guiRenderer.h"

void GuiRenderer::init(GLFWwindow* window)
{
	ImGui::CreateContext();
	ImGui::StyleColorsClassic();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();
}

void GuiRenderer::prepareFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void GuiRenderer::render()
{
	ImVec2 cursorPos = ImGui::GetIO().MousePos;
	bool showMenuOverride = (cursorPos.x >= 0 && cursorPos.y >= 0) && (cursorPos.y <= 25);
	if ((m_autoHideMenu && showMenuOverride) || !m_autoHideMenu || m_menuItemSelected)
	{
		if (ImGui::BeginMainMenuBar())
		{
			m_menuItemSelected = false;
			if (ImGui::BeginMenu("File"))
			{
				m_menuItemSelected = true;
				ImGui::MenuItem("Open...", nullptr, &m_openFileDialog);
				ImGui::MenuItem("Exit", nullptr, nullptr);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("System"))
			{
				m_menuItemSelected = true;
				bool pause = Config::GB.System.pause;
				ImGui::MenuItem("Pause emulation", nullptr, &pause);
				Config::GB.System.pause = pause;

				bool reset = false;
				ImGui::MenuItem("Reset", nullptr, &reset);
				if (reset)
					Config::GB.System.reset = true;

				bool bootrom = Config::GB.System.useBootRom;
				ImGui::MenuItem("Disable Boot Rom", nullptr, &bootrom);
				Config::GB.System.useBootRom = bootrom;

				bool dmgMode = Config::GB.System.DmgMode;
				ImGui::MenuItem("Prefer DMG Mode (Boot ROM must be disabled!)", nullptr, &dmgMode);
				Config::GB.System.DmgMode = dmgMode;

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Debug"))
			{
				m_menuItemSelected = true;
				bool serial = false;	//TODO: FIX
				ImGui::MenuItem("Serial Debug Output", nullptr, &serial);
				//Config::getInstance()->setValue<bool>("serialDebug", serial);

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("View"))
			{
				m_menuItemSelected = true;
				ImGui::MenuItem("Display Settings", nullptr, &m_showPPUDialog);
				ImGui::MenuItem("Auto hide Menubar", nullptr, &m_autoHideMenu);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Help"))
			{
				m_menuItemSelected = true;
				ImGui::MenuItem("About gbemu", nullptr, &m_showAboutDialog);
				ImGui::EndMenu();
			}
		}
		ImGui::EndMainMenuBar();
	}

	if (m_showAboutDialog)
	{
		ImGui::Begin("About", &m_showAboutDialog, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);
		ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 5) * 0.5f);	//center text hack
		ImGui::Text("GBEmu");
		ImGui::Text("NEA Project. By Henry Southall");
		ImGui::End();
	}

	if (m_showPPUDialog)
	{
		ImGui::Begin("PPU", &m_showPPUDialog, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);

		int lastScale = Config::GB.Display.displayScale;
		ImGui::SliderInt("Display Scale", &Config::GB.Display.displayScale, 1, 5);
		if (lastScale != Config::GB.Display.displayScale)
			Config::GB.Display.resize = true;
		ImGui::Checkbox("Color correction", &Config::GB.Display.colorCorrect);
		ImGui::Checkbox("Frame blending", &Config::GB.Display.frameBlend);

		ImGui::Separator();
		ImGui::TextColored(ImVec4(1, 0, 0, 1), "Debug settings");
		bool ppuOverride = Config::GB.PPU.debugOverride;
		ImGui::Checkbox("Override default behaviour", &ppuOverride);
		Config::GB.PPU.debugOverride = ppuOverride;

		if (ppuOverride)
		{
			bool showSprites = Config::GB.PPU.sprites;
			bool showBackground = Config::GB.PPU.background;
			bool showWindow = Config::GB.PPU.window;

			ImGui::Checkbox("Draw background layer", &showBackground);
			ImGui::Checkbox("Draw window layer", &showWindow);
			ImGui::Checkbox("Show sprites", &showSprites);

			Config::GB.PPU.sprites = showSprites;
			Config::GB.PPU.background = showBackground;
			Config::GB.PPU.window = showWindow;
		}
		ImGui::End();
	}

	if (m_openFileDialog)
	{
		OPENFILENAMEA ofn = {};
		CHAR szFile[255] = { 0 };

		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = NULL;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = "Game Boy ROM Files\0*.gb;*.gbc\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		if (GetOpenFileNameA(&ofn) == TRUE)
		{
			std::string filename = szFile;
			Config::GB.System.RomName = filename;
			Config::GB.System.reset = true;
		}

		m_openFileDialog = false;
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool GuiRenderer::m_showAboutDialog = false;
bool GuiRenderer::m_openFileDialog = false;
bool GuiRenderer::m_showPPUDialog = false;
bool GuiRenderer::m_autoHideMenu = false;
bool GuiRenderer::m_menuItemSelected = false;