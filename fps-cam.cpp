#include <windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <tchar.h>
#include <vector>
#include <stdlib.h>
#include <string>
#include <sstream>

using namespace std;

HANDLE currentGame = GetCurrentProcess();
const float mouseSensitivity = 7.5;
//initialize variable which holds the current player position, then add or decrease it depending on direction and clip speed.
float newVel = 0.0;

//grab process Handle, just GetCurrentProcess() because DLL is in the process injected that we all want.


//grab Base Address of the Module, same as above, just GetModuleHandle() since we are already in the process thus have access to em all.
DWORD BaseAddress = (DWORD)GetModuleHandle("gamma.dll");

//take Base Address and add 0x840D0 to it to get an offset which gets our first memory address to construct our multi level pointer
// an asterisk (*) is the dereference operator- it dereferences an address, no need to use ReadProcessMemory(), makes it look cleaner.
auto address = *(DWORD*)(BaseAddress + 0x843E0);       // address   = ["gamma.dll" + 0x840D0]  -> Address1
auto thefirst = *(DWORD*)(address + 0x0);              // thefirst  = [Address1 + 4]           -> Address2
auto thesecond = *(DWORD*)(thefirst + 0xAC);            // thesecond = [Address2 + 8]           -> Address3
auto thethird = *(DWORD*)(thesecond + 0x51C);           // thethird  = [Address3 + 28]          -> Address4
auto thefourth = *(DWORD*)(thethird + 0x8);           // thefourth = [Address4 + 3C]          -> Address5
auto thefifth = *(DWORD*)(thefourth + 0x8);          // thefifth  = [Address5 + 59C]         -> Address6
auto thesixth = *(DWORD*)(thefifth + 0x70);            // thefifth  = [Address6 + 14]          -> Address7
//now the last one, no need to derefence it, just add 0x34 to the last derefenced pointer offset, now the correct memory offset should've been found.
auto velY = thesixth + 0x1C;                           // thefifth  = [Address7 + 34]          =  PosYAddress

// now the same but to get our player X position
auto address2 = *(DWORD*)(BaseAddress + 0x843E0);
auto thefirst2 = *(DWORD*)(address2 + 0x0);              // thefirst  = [Address1 + 4]           -> Address2
auto thesecond2 = *(DWORD*)(thefirst2 + 0xAC);            // thesecond = [Address2 + 8]           -> Address3
auto thethird2 = *(DWORD*)(thesecond2 + 0x51C);           // thethird  = [Address3 + 28]          -> Address4
auto thefourth2 = *(DWORD*)(thethird2 + 0x8);           // thefourth = [Address4 + 3C]          -> Address5
auto thefifth2 = *(DWORD*)(thefourth2 + 0x8);          // thefifth  = [Address5 + 59C]         -> Address6
auto thesixth2 = *(DWORD*)(thefifth2 + 0x6C);            // thefifth  = [Address6 + 14]          -> Address7
auto velX = thesixth2 + 0x24;                           // thefifth  = [Address7 + 30]          =  PosXAddress

bool useFPSCam = false;

DWORD WINAPI MainThread(LPVOID param)
{
	if (thefirst == 0)
	{
		MessageBoxA(NULL, "First Pointer Returned An Address of 0!", "ERROR!", MB_OK);
	}
	else
	{
		POINT previousPos{};
		GetCursorPos(&previousPos);
		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);
		int centerX = screenWidth / 2;
		int centerY = screenWidth / 2;

		SetCursorPos(centerX, centerY);

		while (true)
		{
			if (useFPSCam)
			{
				POINT currentPos{};
				GetCursorPos(&currentPos);

				int deltaX = currentPos.x - centerX;
				int deltaY = currentPos.y - centerY;

				if (deltaX != 0 || deltaY != 0)
				{
					deltaY = deltaY * mouseSensitivity * -1;
					deltaX = deltaX * mouseSensitivity * -1;
					float fDeltaY = static_cast<float>(deltaY);
					float fDeltaX = static_cast<float>(deltaX);
					WriteProcessMemory(currentGame, (LPVOID*)(velY), &fDeltaY, 4, 0);
					WriteProcessMemory(currentGame, (LPVOID*)(velX), &fDeltaX, 4, 0);
				}

				SetCursorPos(centerX, centerY);
				previousPos = currentPos;

				Sleep(10); // Sleep for 10 milliseconds

				if (GetAsyncKeyState('F') & 0x80000)
				{
					useFPSCam = !useFPSCam;
				}
				Sleep(50);
			}
			else
			{
				if (GetAsyncKeyState('F') & 0x80000)
				{
					useFPSCam = !useFPSCam;
				}
				Sleep(50);
			}
		}
		return 0;
	}
}

bool WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpReserverd)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		//MessageBoxA(NULL, "DLL Injected", "DLL Injected.", MB_OK);			//Freezes CE, thus fails injecting it if it takes to long to press OK.
		CreateThread(0, 0, MainThread, hModule, 0, 0);
	}
	return true;
}
