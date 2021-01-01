#include "pch.h"
#include <iostream>
#include <windows.h>

/*
	HOOK WITH "PUSH RET" TECHNIQUE
*/

DWORD jmpBack;

bool Hook(void* toHook, void* ourFunct, int len) {

	if (len < 5) {
		return false;
	}

	DWORD curProtection;
	VirtualProtect(toHook, len, 0x40, &curProtection);

	memset(toHook, 0x90, len);

	DWORD absoluteAddress = (DWORD)ourFunct;

	*(BYTE*)toHook = 0x68;
	*(DWORD*)((DWORD)toHook + 1) = absoluteAddress;
	*(BYTE*)((DWORD)toHook + 5) = 0xC3;

	DWORD temp;
	VirtualProtect(toHook, len, curProtection, &temp);

	return true;
}

void __declspec(naked) ourFunct() {
	std::cout << "[HOOKED] ";
	__asm {
		add ecx, ecx
		mov edx, [ebp - 8]
		jmp[jmpBack]
	}
}

DWORD WINAPI MainThread(LPVOID param) {


	uintptr_t hModule = (uintptr_t)GetModuleHandle(L"HackMe.exe");
	if (hModule == NULL) {
		exit(1);
	}

	int hookLength = 6;

	DWORD textSegmentOffset = 0x11000;
	DWORD instructionOffset = 0x1768;
	DWORD moduleAddress = hModule;

	DWORD hookAddress = hModule + textSegmentOffset + instructionOffset;

	jmpBack = hookAddress + hookLength;

	Hook((void*)hookAddress, ourFunct, hookLength);

	while (true) {
		if (GetAsyncKeyState(VK_ESCAPE)) break;
		Sleep(50);
	}

	FreeLibraryAndExitThread((HMODULE)param, 0);

	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpReserved) {
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		CreateThread(0, 0, MainThread, hModule, 0, 0);
		break;
	}

	return TRUE;
}

