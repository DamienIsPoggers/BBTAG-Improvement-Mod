#include "hooks_bbtag.h"
#include "HookManager.h"
#include "hooks_steamApiWrapper.h"
#include "../SteamApiWrapper/SteamMatchmakingWrapper.h"
#include "../SteamApiWrapper/SteamNetworkingWrapper.h"
#include "../WindowManager/WindowManager.h"
#include "../Game/gamestates.h"
#include "../globals.h"
#include "../logger.h"

DWORD WindowMsgHandlerJmpBackAddr = 0;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void __declspec(naked)PassMsgToImGui()
{
	__asm
	{
		push ebp
		mov ebp, esp
		mov eax, [ebp + 8h]
	}
	LOG_ASM(7, "PassMsgToImGui\n");

	__asm
	{
		pushad
		movzx eax, WindowManager::Initialized
		cmp eax, 0
		je SKIP
	}
	__asm
	{
		push[ebp + 10h]; //lParam
		push[ebp + 0Ch]; //wParam
		push[ebp + 8h] //msg
		push g_gameProc.hWndGameWindow //hwnd
	}

	LOG_ASM(7, "PassMsgToImGui Inner\n");

	__asm
	{
		call ImGui_ImplWin32_WndProcHandler
		pop ebx //manually clearing stack...
		pop ebx
		pop ebx
		pop ebx
		cmp eax, 1
		je EXIT
SKIP:
		popad
		jmp[WindowMsgHandlerJmpBackAddr]
	}
EXIT:
	__asm
	{
		popad
		pop ebp
		retn 0Ch
	}
}

DWORD GetGameStateAndModeTitleScreenJmpBackAddr = 0;
void __declspec(naked)GetGameStateAndModeTitleScreen()
{
	LOG_ASM(2, "GetGameStateAndModeTitleScreen\n");

	__asm
	{
		pushad
		add edi, 110h
		lea ebx, g_gameVals.pGameMode
		mov[ebx], edi

		add edi, 4h
		lea ebx, g_gameVals.pGameState
		mov[ebx], edi
	}

	placeHooks_steamApiWrapper();

	WindowManager::Init(g_gameProc.hWndGameWindow, g_interfaces.pD3D9ExWrapper);

	__asm
	{
		popad
		mov dword ptr [edi + 114h], 4 
		jmp[GetGameStateAndModeTitleScreenJmpBackAddr]
	}
}

DWORD GetGameStateAndModeEntranceScreenJmpBackAddr = 0;
void __declspec(naked)GetGameStateAndModeEntranceScreen()
{
	LOG_ASM(2, "GetGameStateAndModeEntranceScreen\n");

	__asm
	{
		pushad
		add eax, 110h
		lea ebx, g_gameVals.pGameMode
		mov[ebx], eax

		add eax, 4h
		lea ebx, g_gameVals.pGameState
		mov[ebx], eax
	}

	placeHooks_steamApiWrapper();

	WindowManager::Init(g_gameProc.hWndGameWindow, g_interfaces.pD3D9ExWrapper);

	//ResetBackToMenu();

	__asm
	{
		popad
		mov dword ptr[eax + 114h], 17h //original bytes
		jmp[GetGameStateAndModeEntranceScreenJmpBackAddr]
	}
}

DWORD CpuUsageFixJmpBackAddr = 0;
void __declspec(naked)CpuUsageFix()
{
	//Pretty ghetto solution, but for potato comps it's still better than overheating
	__asm pushad
	Sleep(1);
	__asm popad
	__asm
	{
		push esi
		lea eax, [ebp-8h]
		mov esi, ecx
		jmp[CpuUsageFixJmpBackAddr]
	}
}

DWORD GetIsHUDHiddenJmpBackAddr = 0;
void __declspec(naked)GetIsHUDHidden()
{
	LOG_ASM(2, "GetIsHUDHidden\n");

	__asm
	{
		or dword ptr[eax + 3DD06Ch], 4
		push eax
		add eax, 3DD06Ch
		mov g_gameVals.pIsHUDHidden, eax
		pop eax
		jmp[GetIsHUDHiddenJmpBackAddr]
	}
}

DWORD GetCharObjectsJmpBackAddr = 0;
void __declspec(naked)GetCharObjects()
{
	static char* addr;
	LOG_ASM(2, "GetCharObjects\n");

	__asm
	{
		mov     dword ptr[ecx], 0

		pushad
		mov addr, ecx
	}

	g_interfaces.Player1.Char1().SetCharDataPtr(addr);
	addr += 0x4;
	g_interfaces.Player2.Char1().SetCharDataPtr(addr);
	addr += 0x4;
	g_interfaces.Player1.Char2().SetCharDataPtr(addr);
	addr += 0x4;
	g_interfaces.Player2.Char2().SetCharDataPtr(addr);

	__asm
	{
		popad
		jmp[GetCharObjectsJmpBackAddr]
	}
}

DWORD GetTimerJmpBackAddr = 0;
void __declspec(naked)GetTimer()
{
	LOG_ASM(2, "GetTimer\n");

	__asm
	{
		mov     dword ptr[ecx + 24h], 0

		push ecx
		add ecx, 24h
		mov g_gameVals.pGameTimer, ecx
	}

	__asm pushad
	WindowManager::OnMatchInit();
	__asm popad

	__asm
	{
		pop ecx

		jmp[GetTimerJmpBackAddr]
	}
}

DWORD GetPlayerMetersJmpBackAddr = 0;
void __declspec(naked)GetPlayerMeters()
{
	static char* addr;
	LOG_ASM(2, "GetPlayerMeters\n");

	__asm
	{
		mov addr, ebx
	}

	__asm pushad
	g_interfaces.Player1.SetMeterPtr(addr);
	g_interfaces.Player2.SetMeterPtr(addr + 0x4);
	__asm popad

	__asm
	{
		sub     eax, esi
		mov     dword ptr[ebx + esi * 4 + 20h], 0
		jmp[GetPlayerMetersJmpBackAddr]
	}
}

DWORD ForcePromptControlSchemeLayoutJmpBackAddr = 0;
void __declspec(naked)ForcePromptControlSchemeLayout()
{
	LOG_ASM(7, "ForcePromptControlSchemeLayout\n");

	__asm
	{
		push eax
		mov eax, [Settings::settingsIni.forcepromptbuttonlayout];
		mov[ecx + 177B40h], eax
		pop eax

		jmp[ForcePromptControlSchemeLayoutJmpBackAddr]
	}
}

DWORD RenderOrigGameHUDJmpBackAddr = 0;
void __declspec(naked)RenderOrigGameHUD()
{
	LOG_ASM(7, "RenderOrigGameHUD\n");

	__asm
	{
		test    byte ptr[eax + 3DD06Ch], 1
		jmp[RenderOrigGameHUDJmpBackAddr]
	}
}

DWORD ShowAllLobbies_ONE_JmpBackAddr = 0;
void __declspec(naked)ShowAllLobbies_ONE()
{
	__asm
	{
		mov     eax, [ebp - 80h]
		mov		[ebp - 11Ch], 0
		cmp     eax, [ebp - 11Ch]
		jmp[ShowAllLobbies_ONE_JmpBackAddr]
	}
}

DWORD ShowAllLobbies_TWO_JmpBackAddr = 0;
void __declspec(naked)ShowAllLobbies_TWO()
{
	__asm
	{
		mov     eax, [ebp - 70h]
		mov		[ebp - 0B4h], 0
		cmp     eax, [ebp - 0B4h]
		jmp[ShowAllLobbies_TWO_JmpBackAddr]
	}
}

DWORD GetP1CharsPaletteIndexesJmpBackAddr = 0;
void __declspec(naked)GetP1CharsPaletteIndexes()
{
	LOG_ASM(2, "GetP1CharsPaletteIndexes\n");

	static int* pPalIndex;

	__asm
	{
		mov[esi + 24D8h], eax

		push eax
		mov eax, esi
		add eax, 24D8h
		mov pPalIndex, eax
	}

	__asm pushad
	if (g_interfaces.Player1.Char1().PalHandle().IsNullPointerPalIndex())
		g_interfaces.Player1.Char1().PalHandle().SetPointerPalIndex(pPalIndex);
	else if (g_interfaces.Player1.Char2().PalHandle().IsNullPointerPalIndex())
		g_interfaces.Player1.Char2().PalHandle().SetPointerPalIndex(pPalIndex);
	__asm popad

	__asm
	{
		pop eax
		jmp[GetP1CharsPaletteIndexesJmpBackAddr]
	}
}

DWORD GetP2CharsPaletteIndexesJmpBackAddr = 0;
void __declspec(naked)GetP2CharsPaletteIndexes()
{
	LOG_ASM(2, "GetP2CharsPaletteIndexes\n");

	static int* pPalIndex;

	__asm
	{
		mov[esi + 24D8h], eax

		push eax
		mov eax, esi
		add eax, 24D8h
		mov pPalIndex, eax
	}

	__asm pushad
	if (g_interfaces.Player2.Char1().PalHandle().IsNullPointerPalIndex())
		g_interfaces.Player2.Char1().PalHandle().SetPointerPalIndex(pPalIndex);
	else if (g_interfaces.Player2.Char2().PalHandle().IsNullPointerPalIndex())
		g_interfaces.Player2.Char2().PalHandle().SetPointerPalIndex(pPalIndex);
	__asm popad

	__asm
	{
		pop eax
		jmp[GetP2CharsPaletteIndexesJmpBackAddr]
	}
}

DWORD GetPalBaseAddressesJmpBackAddr = 0;
void __declspec(naked)GetPalBaseAddresses()
{
	static int counter = 0;
	static char *palPointer = 0;

	LOG_ASM(2, "GetPalBaseAddresses\n");

	__asm
	{
		mov[ecx + 830h], eax

		add ecx, 830h
		mov palPointer, ecx
	}

	//////////////////////////////////////////////
	__asm pushad

	//counter 0 = p1char1
	//counter 1 = p2char1
	//coutner 2 = p1char2
	//counter 3 = p2char2
	switch (counter)
	{
	case 0:
		g_interfaces.Player1.Char1().PalHandle().SetPointerBasePal(palPointer);
		break;
	case 1:
		g_interfaces.Player2.Char1().PalHandle().SetPointerBasePal(palPointer);
		break;
	case 2:
		g_interfaces.Player1.Char2().PalHandle().SetPointerBasePal(palPointer);
		break;
	case 3:
		g_interfaces.Player2.Char2().PalHandle().SetPointerBasePal(palPointer);
		break;
	default:
		counter = -1; // -1 so it becomes 0 when we increment it in the following statement
	}

	counter++;

	__asm popad
	//////////////////////////////////////////////

	__asm
	{
		jmp[GetPalBaseAddressesJmpBackAddr]
	}
}

DWORD GetGameUpdatePauseJmpBackAddr = 0;
void __declspec(naked)GetGameUpdatePause()
{
	LOG_ASM(7, "GetGameUpdatePause\n");

	__asm
	{
		cmp g_gameVals.pGameMode, 0 //check for nullpointer
		je ORIG_CODE

		mov ecx, g_gameVals.pGameMode
		cmp [ecx], GameMode_Tutorial
		jne ORIG_CODE

		cmp g_gameVals.isPaletteModePaused, 0
		jnz EXIT

ORIG_CODE:
		cmp dword ptr[eax + 8], 0
		jnz EXIT

		jmp[GetGameUpdatePauseJmpBackAddr]
	}

	__asm
	{
EXIT:
		mov eax, 1
		retn
	}
}

int PassKeyboardInputToGame()
{
	int result = 0;

	if (GetForegroundWindow() != g_gameProc.hWndGameWindow)
		result = 0;
	else if (ImGui::GetIO().WantCaptureKeyboard)
		result = 0;
	else
		result = 1;

	return result;
}

DWORD DenyKeyboardInputFromGameJmpBackAddr = 0;
void __declspec(naked)DenyKeyboardInputFromGame()
{
	LOG_ASM(7, "DenyKeyboardInputFromGame\n");

	__asm
	{
		call PassKeyboardInputToGame
		test eax, eax
		jz EXIT

		lea     eax, [esi + 28h]
		push    eax // lpKeyState
		call    ds:GetKeyboardState
EXIT:
		jmp[DenyKeyboardInputFromGameJmpBackAddr]
	}
}

//runs in additional_hooks.cpp in the hook_steamnetworking and ID3D9EXWrapper_Device.cpp in constructor, since unlike in CF in this game this method runs after steam init
//These functions can be hooked after steam drm does its thing and d3d9device is up
bool placeHooks_bbtag()
{
	LOG(1, "placeHooks_bbtag\n");

	WindowMsgHandlerJmpBackAddr = HookManager::SetHook("WindowMsgHandler", "\x55\x8b\xec\x8b\x45\x08\x83\xe8\x02\x74\x00\x8b\x55\x0c",
		"xxxxxxxxxx?xxx", 6, PassMsgToImGui);

	DenyKeyboardInputFromGameJmpBackAddr = HookManager::SetHook("DenyKeyboardInputFromGame", "\x8d\x46\x28\x50\xff\x15", 
		"xxxxxx", 10, DenyKeyboardInputFromGame);
		
	if (Settings::settingsIni.cpuusagefix && !CpuUsageFixJmpBackAddr)
	{
		CpuUsageFixJmpBackAddr = HookManager::SetHook("CpuUsageFix", "\x56\x8d\x45\xf8\x8b\xf1\x50",
			"xxxxxxx", 6, CpuUsageFix);
	}

	if (Settings::settingsIni.forcepromptbuttonlayout != FORCE_PROMPT_LAYOUT_OFF)
	{
		ForcePromptControlSchemeLayoutJmpBackAddr = HookManager::SetHook("ForcePromptControlSchemeLayout", "\x89\x81\x40\x7b\x17\x00\xc3",
			"xxxxxxx", 6, ForcePromptControlSchemeLayout);
	}

	GetGameStateAndModeTitleScreenJmpBackAddr = HookManager::SetHook("GetGameStateAndModeTitleScreen", "\xc7\x87\x14\x01\x00\x00\x04\x00\x00\x00\xc7\x46\x58\x00\x00\x00\x00", 
		"xxxxxxxxxxxxxxxxx", 10, GetGameStateAndModeTitleScreen);

	GetGameStateAndModeEntranceScreenJmpBackAddr = HookManager::SetHook("GetGameStateAndModeEntranceScreen", "\xc7\x80\x14\x01\x00\x00\x17\x00\x00\x00\xe8", 
		"xxxxxxxxxxx", 10, GetGameStateAndModeEntranceScreen);

	GetIsHUDHiddenJmpBackAddr = HookManager::SetHook("GetIsHUDHidden", "\x83\x88\x6c\xd0\x3d\x00\x04\x8b\x06\xff\x50\x24", 
		"xxxxxxxxxxxx", 7, GetIsHUDHidden);

	GetCharObjectsJmpBackAddr = HookManager::SetHook("GetCharObjects", "\xc7\x01\x00\x00\x00\x00\xb8\x00\x00\x00\x00\xc7\x41\x04\x00\x00\x00\x00", 
		"xx????x????xxxxxxx", 6, GetCharObjects);
	
	GetTimerJmpBackAddr = HookManager::SetHook("GetTimer", "\xc7\x41\x24\x00\x00\x00\x00\xc7\x41\x2c\x83\x03\x00\x00",
		"xxxxxxxxxxxxxx", 7, GetTimer);

	GetPlayerMetersJmpBackAddr = HookManager::SetHook("GetPlayerMetersJmpBackAddr", "\x2b\xc6\xc7\x44\xb3\x20\x00\x00\x00\x00", 
		"xxxxxxxxxx", 10, GetPlayerMeters);

	GetP1CharsPaletteIndexesJmpBackAddr = HookManager::SetHook("GetP1CharsPaletteIndexes", "\x89\x86\xd8\x24\x00\x00\x8b\x87\x60\x06\x00\x00",
		"xxxxxxxxxxxx", 6, GetP1CharsPaletteIndexes);

	GetP2CharsPaletteIndexesJmpBackAddr = HookManager::SetHook("GetP2CharsPaletteIndexes", "\x89\x86\xd8\x24\x00\x00\x8b\x81\x60\x06\x00\x00",
		"xxxxxxxxxxxx", 6, GetP2CharsPaletteIndexes);

	GetPalBaseAddressesJmpBackAddr = HookManager::SetHook("GetPalBaseAddresses", "\x89\x81\x30\x08\x00\x00\x8b\xc8", 
		"xxxxxxxx", 6, GetPalBaseAddresses);

	//TODO: POSSIBLE HUD FIX AMONGST THE CALLS WHERE THE PATTERN IS FOUND
	GetGameUpdatePauseJmpBackAddr = HookManager::SetHook("GetGameUpdatePause", "\x83\x78\x08\x00\x75\x00\xe8",
		"xxxxx?x", 6, GetGameUpdatePause);

	///////////////// EXPERIMENTAL HOOKS BELOW ////////////////////////////

	//was unable to find an unique pattern with the overwrite-able opcode as base, so we start searching two opcodes above then modify the address
	//RenderOrigGameHUDJmpBackAddr = HookManager::SetHook("RenderOrigGameHUD", "\xc3\xe8\x00\x00\x00\x00\xf6\x80\x6c\xd0\x3d\x00\x01", 
	//	"xx????xxxxxxx", 7, RenderOrigGameHUD, false);
	//RenderOrigGameHUDJmpBackAddr += 6;
	//HookManager::SetJmpBackAddr("RenderOrigGameHUD", RenderOrigGameHUDJmpBackAddr);
	//HookManager::ActivateHook("RenderOrigGameHUD");
	//LOG(2, "New RenderOrigGameHUD hook address: 0x%p\n", HookManager::GetStartAddress("RenderOrigGameHUD"));
	//LOG(2, "New RenderOrigGameHUDJmpBackAddr: 0x%p\n", RenderOrigGameHUDJmpBackAddr);


	//attempt to force the game to show all lobbies, even the abandoned ones that hadnt been cleaned up yet
	//ShowAllLobbies_ONE_JmpBackAddr = HookManager::SetHook("ShowAllLobbies_ONE", "\x8b\x45\x80\x3b\x85\xe4\xfe\xff\xff",
	//	"xxxxxxxxx", 9, ShowAllLobbies_ONE);

	//ShowAllLobbies_TWO_JmpBackAddr = HookManager::SetHook("ShowAllLobbies_TWO", "\x8b\x45\x90\x3b\x85\x4c\xff\xff\xff",
	//	"xxxxxxxxx", 9, ShowAllLobbies_TWO);

	//////////////////////////////////////////////////////////////////////

	return true;
}