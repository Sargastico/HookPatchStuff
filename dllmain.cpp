#include "pch.h"
#include <iostream>
#include <windows.h>


//Hook Function
bool Hook(void* toHook, void* ourFunct, int len) {
	if (len < 5) {
		return false;
	}

	DWORD curProtection; //Storage for Current Memory Protection 

	//Change Memory Protection for Patch (0x40 = PAGE_EXECUTE_READWRITE)
	VirtualProtect(toHook, len, PAGE_EXECUTE_READWRITE, &curProtection);

	//Set the 'NOP' instruction on stolen bytes to avoid unexpected behavior 
	memset(toHook, 0x90, len);

	//Calculates the address for Jump from the original Function (toHook) to our new Function (ourFunct)
	DWORD relativeAddress = ((DWORD)ourFunct - (DWORD)toHook) - 5;

	*(BYTE*)toHook = 0xE9; //OPCODE for 'JMP' (x86 - Jump near, relative, displacement relative to next instruction)
	*(DWORD*)((DWORD)toHook + 1) = relativeAddress; //Place address to jump to 1 bytes after 'JMP' instruction

	DWORD temp;

	//Restores the original memory protection 
	VirtualProtect(toHook, len, curProtection, &temp);

	return true;
}

DWORD relativeAddr;

//Function to execute our new code for Hooked Function + stolen bytes/instructions
void __declspec(naked) ourFunct() {
	std::cout << "[HOOKED] ";
	__asm {
		add ecx, ecx
		mov edx, [ebp - 8]
		jmp[relativeAddr]
	}
}


DWORD WINAPI MainThread(LPVOID param) {

	//Get module (HackMe.exe) base address
	uintptr_t hModule = (uintptr_t)GetModuleHandle(L"HackMe.exe");
	if (hModule == NULL) {
		exit(1);
	}

	//Size of stolen bytes (used to hook)
	int hookLength = 6;

	DWORD textSegmentOffset = 0x11000;
	DWORD instructionOffset = 0x1768;
	DWORD moduleAddress = hModule;

	//Calculate instruction address [ instruction = moduleBaseAddress + .textSegmentOffset + instructionOffset ] 
	DWORD hookAddress = hModule + textSegmentOffset + instructionOffset;

	//Relative Address to jump to ('JMP relativeAddr' - x86)
	relativeAddr = hookAddress + hookLength;

	//Hulk Smaaashhh
	Hook((void*)hookAddress, ourFunct, hookLength);

	while (true) {
		if (GetAsyncKeyState(VK_ESCAPE)) break;
		Sleep(50);
	}

	FreeLibraryAndExitThread((HMODULE)param, 0);

	return 0;
}


//dll stuff
BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpReserved) {
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		CreateThread(0, 0, MainThread, hModule, 0, 0);
		break;
	}

	return TRUE;
}

