/// SIMPLE WINDOWS HOTKEY GENERATOR NODE
// Rachel "Packetdancer" Blackman @ Rooibot Games
//
// This is intended to allow people to trigger external applications via global hotkeys, similar to how Unreal 
// itself handles Control-Alt-F11 for Live Coding rebuild. It *only* works on Windows, as a side note, but people 
// can certainly feel free to modify it to add macOS or Linux hotkey support as well. It is mostly useful for
// editor scripting to help with workflow.
//
// Original thread: https://forums.unrealengine.com/t/simulate-press-key/511935/5
//
// LICENSE:
// This code is free for anyone to use, modify, turn into origami, translate into Perl, or abuse in other manners.
// It was written solely to post in a thread on the Unreal Engine development forums, and thus is released into
// the world to see whence the digital winds scatter it. No attribution is necessary (though it'd be nice), use it
// in closed source projects, whatever. Enjoy!
//
// INSTRUCTIONS:
// To use this, just download the raw file as SimpleHotkeyLibrary.h and drop it into your project somewhere; the
// UnrealBuildTool system will automatically pick it up and include it on the next build. Then you can simply use
// the Generate Key Press Event node in your Blueprints, in order to send various keys.
// 
// The node functions by giving you a boolean toggle for "Press" vs "Release"; this is because various external
// hotkey systems will ignore keypresses that are too short, or need a delay between modifier keys and actual
// keys, or whatever. By giving you the ability to press and release specific keys wherever you need in a blueprint,
// you can work around those limitations.
//

#pragma once

// Make sure we're using the "lean and mean" versions of Windows header files.
#ifdef _MSC_VER 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN

// Since windows.h contains the same TEXT() macro that Unreal itself does, it
// would generate a warning about redefining the macro. Let's just turn that
// particular Visual C++ compiler warning off.
#pragma warning(disable:4005)

#include <windows.h>
#include <WinUser.h>

// Turn the redefinition warnings back on
#pragma warning(default:4005)
#endif // _MSC_VER

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SimpleHotkeyLibrary.generated.h"

UCLASS()
class USimpleHotkeyLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Given an Unreal key, generate a Windows input event."))
		static bool GenerateKeyPressEvent(const TArray<FKey>& InKeys, const bool bPressed)
	{
#ifdef _MSC_VER
		if (void* User32Dll = FPlatformProcess::GetDllHandle(TEXT("user32.dll")))
		{
			typedef UINT(WINAPI* SendInputFn)(UINT InputCount, LPINPUT Inputs, int InputStructSize);

			if (const SendInputFn SendWindowsInput = (SendInputFn)FPlatformProcess::GetDllExport(User32Dll, TEXT("SendInput")))
			{
				const auto Manager = FInputKeyManager::Get();
				const LPINPUT KeyArray = new INPUT[InKeys.Num()];

				for (int i = 0; i < InKeys.Num(); i++)
				{
					KeyArray[i] = {};

					const uint32* KeyCode = nullptr;
					const uint32* CharCode = nullptr;

					Manager.GetCodesFromKey(InKeys[i], KeyCode, CharCode);

					const uint32 KeyCodeVal = (KeyCode != nullptr) ? *KeyCode : -1;
					const uint32 CharCodeVal = (CharCode != nullptr) ? *CharCode : -1;

					KeyArray[i].type = INPUT_KEYBOARD;
					KeyArray[i].ki.dwFlags = (bPressed ? 0 : KEYEVENTF_KEYUP);
					KeyArray[i].ki.wVk = (KeyCodeVal != -1) ? KeyCodeVal : CharCodeVal;

				}

				const int16 Result = SendWindowsInput(InKeys.Num(), KeyArray, sizeof(INPUT));

				// Clean up
				delete[] KeyArray;

				// Make sure we sent the right number of keys.
				return Result == InKeys.Num();
			}
		}

		return false;

#else // _MSC_VER
		// If we're not running Windows, just return false; this function only works on Windows.
		return false;
#endif // _MSV_VER
	}

};