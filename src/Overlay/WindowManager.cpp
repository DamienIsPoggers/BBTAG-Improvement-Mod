#include "WindowManager.h"

#include "fonts.h"
#include "WindowDrawer.h"
#include "WindowContainer/WindowContainerImpl.h"
#include "Window/LogWindow.h"

#include "Core/interfaces.h"
#include "Core/logger.h"
#include "Core/Settings.h"
#include "Core/utils.h"
#include "Web/update_check.h"
#include <imgui.h>
#include <imgui_impl_dx9.h>

#include <sstream>
#include <time.h>

#define DEFAULT_ALPHA 0.87f

int keyToggleMainWindow;
int keyToggleHud;
int keyToggleCustomHud;

WindowManager* WindowManager::m_instance = nullptr;

WindowManager & WindowManager::GetInstance()
{
	if (m_instance == nullptr)
	{
		m_instance = new WindowManager();
	}
	return *m_instance;
}

bool WindowManager::Initialize(void *hwnd, IDirect3DDevice9 *device)
{
	if (m_initialized)
	{
		return true;
	}

	LOG(2, "WindowManager::Initialize\n");

	if (!hwnd)
	{
		LOG(2, "HWND not found!\n");
		return false;
	}
	if (!device)
	{
		LOG(2, "Direct3DDevice9 not found!\n");
		return false;
	}

	m_initialized = ImGui_ImplDX9_Init(hwnd, device);
	if (!m_initialized)
	{
		LOG(2, "ImGui_ImplDX9_Init failed!\n");
		return false;
	}

	m_pLogger = g_imGuiLogger;

	m_pLogger->Log("[system] Initialization starting...\n");

	m_windowContainer = new WindowContainerImpl();

	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowBorderSize = 1;
	style.FrameBorderSize = 1;
	style.ScrollbarSize = 14;
	style.Alpha = DEFAULT_ALPHA;

	if (Settings::settingsIni.menusize == 1)
	{
		ImFont* font = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(TinyFont_compressed_data_base85, 10);
		font->DisplayOffset.y += 1;
	}
	else if (Settings::settingsIni.menusize == 3)
	{
		ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(DroidSans_compressed_data, DroidSans_compressed_size, 20);
	}

	keyToggleMainWindow = Settings::getButtonValue(Settings::settingsIni.togglebutton);
	m_pLogger->Log("[system] Toggling key set to '%s'\n", Settings::settingsIni.togglebutton.c_str());

	keyToggleHud = Settings::getButtonValue(Settings::settingsIni.toggleHUDbutton);
	m_pLogger->Log("[system] HUD toggling key set to '%s'\n", Settings::settingsIni.toggleHUDbutton.c_str());

	keyToggleCustomHud = Settings::getButtonValue(Settings::settingsIni.togglecustomHUDbutton);
	m_pLogger->Log("[system] CustomHUD toggling key set to '%s'\n", Settings::settingsIni.togglecustomHUDbutton.c_str());

	g_interfaces.pPaletteManager->LoadAllPalettes();

	//Add the default font
	ImFontConfig cfg;
	cfg.SizePixels = 13;
	ImGui::GetIO().Fonts->AddFontDefault(&cfg);// ->DisplayOffset.y = SCALE;

	//Add our font
	ImFontConfig config;
	config.OversampleH = 3;
	config.OversampleV = 3;
	config.GlyphExtraSpacing.x = 1.0f;
	strcpy(config.Name, "CustomHUD");
	float hudScaleY = ((float)Settings::settingsIni.renderheight * Settings::settingsIni.customhudscale) / 1042.0f;
	float fontSize = 30.0f * hudScaleY;
	LOG(2, "CustomHUD fontsize: %f\n", fontSize);
	ImFont* font = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(TinyFont_compressed_data_base85, fontSize, &config);
	font->DisplayOffset.y += 1;

	/*Calling a frame to initialize beforehand to prevent a crash upon first call of
	/Update if the game window is not focused. Simply calling ImGui_ImplDX9_CreateDeviceObjects() might be enough too.*/
	ImGui_ImplDX9_NewFrame();
	ImGui::EndFrame();
	///////

	srand(time(NULL));

	StartAsyncUpdateCheck();

	m_pLogger->Log("[system] Finished initialization\n");
	m_pLogger->LogSeparator();
	LOG(2, "Initialize end\n");

	return true;
}

void WindowManager::Shutdown()
{
	if (!m_initialized)
	{
		return;
	}

	LOG(2, "WindowManager::Shutdown\n");

	SAFE_DELETE(m_windowContainer);
	delete m_instance;

	ImGui_ImplDX9_Shutdown();
}

void WindowManager::InvalidateDeviceObjects()
{
	if (!m_initialized)
	{
		return;
	}

	LOG(2, "WindowManager::InvalidateDeviceObjects\n");
	ImGui_ImplDX9_InvalidateDeviceObjects();
}

void WindowManager::CreateDeviceObjects()
{
	if (!m_initialized)
	{
		return;
	}

	LOG(2, "WindowManager::CreateDeviceObjects\n");
	ImGui_ImplDX9_CreateDeviceObjects();
}

void WindowManager::Render()
{
	if (!m_initialized)
	{
		return;
	}

	LOG(7, "WindowManager::Render\n");

	HandleButtons();

	ImGui_ImplDX9_NewFrame();

	bool isMainWindowOpen =
		m_windowContainer->GetWindow(WindowType_Main)->IsOpen();
	bool isUpdateNotifierWindowOpen =
		m_windowContainer->GetWindow(WindowType_UpdateNotifier)->IsOpen();
	bool isPaletteEditorWindowOpen =
		m_windowContainer->GetWindow(WindowType_PaletteEditor)->IsOpen();
	bool isLogWindowOpen =
		m_windowContainer->GetWindow(WindowType_Log)->IsOpen();
	bool isDebugWindowOpen =
		m_windowContainer->GetWindow(WindowType_Debug)->IsOpen();

	ImGui::GetIO().MouseDrawCursor = isMainWindowOpen | isLogWindowOpen | isPaletteEditorWindowOpen
		| isUpdateNotifierWindowOpen | isDebugWindowOpen;

	WindowDrawer::DrawAllWindows(m_windowContainer);

	ImGui::Render();
}

void WindowManager::HandleButtons()
{
	if (!m_initialized)
	{
		return;
	}

	if (ImGui::IsKeyPressed(keyToggleHud) && g_gameVals.pIsHUDHidden)
	{
		*g_gameVals.pIsHUDHidden ^= 1;
	}

	if (ImGui::IsKeyPressed(keyToggleCustomHud))
	{
		m_windowContainer->GetWindow(WindowType_CustomHud)->ToggleOpen();
	}

	if (ImGui::IsKeyPressed(keyToggleMainWindow))
	{
		m_windowContainer->GetWindow(WindowType_Main)->ToggleOpen();
	}
}
