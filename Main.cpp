#define _WIN32_WINNT 0x0501 
#define WINVER 0x0501 
#define NTDDI_VERSION 0x05010000
#define WIN32_LEAN_AND_MEAN
#define PSAPI_VERSION 1

#include <Windows.h>

#include <string>
#include <thread>

unsigned char* GameDll;

#define IsKeyPressed(CODE) ((GetAsyncKeyState(CODE) & 0x8000) > 0)

int goldpresstime = 0;



unsigned char* pW3XGlobalClass = 0;


void DisplayText(const std::string& szText, float fDuration)
{
	unsigned int dwDuration = *((unsigned int*)&fDuration);
	unsigned char* GAME_PrintToScreen = GameDll + 0x2F8E40;


	std::string outLineStr = (szText);
	const char* outLinePointer = outLineStr.c_str();

	if (!GameDll || !*(unsigned char**)pW3XGlobalClass)
		return;

	__asm
	{
		PUSH	0xFFFFFFFF;
		PUSH	dwDuration;
		PUSH	outLinePointer;
		PUSH	0x0;
		PUSH	0x0;
		MOV		ECX, [pW3XGlobalClass];
		MOV		ECX, [ECX];
		CALL	GAME_PrintToScreen;
	}
}

void* GetGlobalPlayerData()
{
	if (*(int*)(0xAB65F4 + GameDll) > 0)
	{
		return (void*)*(int*)(0xAB65F4 + GameDll);
	}
	else
		return nullptr;
}

int GetPlayerByNumber(int number)
{
	if (number == -1)
		return -1;

	void* arg1 = GetGlobalPlayerData();
	int result = -1;
	if (arg1 != nullptr && arg1)
	{
		result = (int)arg1 + (number * 4) + 0x58;
		result = *(int*)result;
	}
	return result;
}


int GetPlayerGold(int PlayerId)
{
	if (PlayerId > -1 && PlayerId < 12)
	{
		int addr = *(int*)(GameDll + 0xAB7788);
		if (addr > 0)
		{
			addr = *(int*)(addr + 0x4);
			if (addr > 0)
			{
				addr = *(int*)(addr + 0xC);
				if (addr > 0)
				{
					const int GOLD_OFFSETS[12] = {
						0xF8,
						0x1378,
						0x2608,
						0x3888,
						0x4B18,
						0x5D98,
						0x7028,
						0x82B8,
						0x9538,
						0xA7C8,
						0xBA48,
						0xCCD8
					};
					addr = *(int*)(addr + GOLD_OFFSETS[PlayerId]);
					if (addr > 0)
						return int(addr / 10);
				}
			}
		}
	}
	return 0;
}

int GetLocalPlayerNumber()
{
	void* gldata = GetGlobalPlayerData();
	if (gldata != nullptr && gldata)
	{
		int playerslotaddr = (int)gldata + 0x28;
		return (int)*(short*)(playerslotaddr);
	}
	else
		return -1;
}


int GetLocalPlayer()
{
	return GetPlayerByNumber(GetLocalPlayerNumber());
}

int GetPlayerTeam(int playeraddr)
{
	if (playeraddr <= 0)
		return 0;
	return *(int*)(playeraddr + 0x278);
}

struct Packet
{
	DWORD PacketClassPtr;	//+00, some unknown, but needed, Class Pointer
	BYTE* PacketData;		//+04
	DWORD _1;				//+08, zero
	DWORD _2;				//+0C, ??
	DWORD Size;				//+10, size of PacketData
	DWORD _3;				//+14, 0xFFFFFFFF
};

typedef void* (__fastcall* GAME_SendPacket_p) (Packet* packet, DWORD zero);
GAME_SendPacket_p GAME_SendPacket;

void SendPacket(BYTE* packetData, DWORD size)
{
	// @warning: this function thread-unsafe, do not use it in other thread.
	// note: this is very useful function, in fact this function
	// does wc3 ingame action, so you can use it for anything you want,
	// including unit commands and and gameplay commands,
	// i suppose its wc3 single action W3GS_INCOMING_ACTION (c) wc3noobpl.

	Packet packet;
	memset(&packet, 0, sizeof(Packet));

	packet.PacketClassPtr = (DWORD)(0x932D2C + GameDll); // Packet Class
	packet.PacketData = packetData;
	packet.Size = size;
	packet._2 = 0x5B4;
	packet._3 = 0xFFFFFFFF;
	GAME_SendPacket = (GAME_SendPacket_p)(GameDll + 0x54D970);
	GAME_SendPacket(&packet, 0);
}


void SetTlsForMe()
{
	UINT32 Data = *(UINT32*)(GameDll + 0xACEB4C);
	UINT32 TlsIndex = *(UINT32*)(GameDll + 0xAB7BF4);
	if (TlsIndex)
	{
		UINT32 v5 = **(UINT32**)(*(UINT32*)(*(UINT32*)(GameDll + 0xACEB5C) + 4 * Data) + 44);
		if (!v5 || !(*(LPVOID*)(v5 + 520)))
		{
			Sleep(1000);
			SetTlsForMe();
			return;
		}
		TlsSetValue(TlsIndex, *(LPVOID*)(v5 + 520));
	}
	else
	{
		Sleep(1000);
		SetTlsForMe();
		return;
	}
}

void WORK()
{

	if ((IsKeyPressed(VK_LCONTROL) || IsKeyPressed(VK_LSHIFT)) && (IsKeyPressed('1')
		|| IsKeyPressed('2')
		|| IsKeyPressed('3')
		|| IsKeyPressed('4')
		|| IsKeyPressed('5')
		|| IsKeyPressed('6')
		|| IsKeyPressed('7')
		|| IsKeyPressed('8')
		|| IsKeyPressed('9')
		|| IsKeyPressed('0')))
	{
		if (goldpresstime < 3)
		{
			DisplayText("Hold buttons to transfer gold " + std::to_string(3 - goldpresstime) + " seconds...", 7.5f);
			goldpresstime++;
		}
		else
		{
			goldpresstime = 2;
#pragma pack(1)
			struct PackGold
			{
				unsigned char bypass;
				unsigned char bypass1;
				unsigned char bypass2;
				unsigned char bypass3;
				unsigned char bypass31;
				unsigned char bypass32;
				unsigned char cmd;
				unsigned char pid;
				unsigned int gold;
				unsigned int wood;
				unsigned char bypass4;
				unsigned char bypass5;
				unsigned char bypass6;
			};



			PackGold tmpPackGold;
			tmpPackGold.bypass = rand() % 100 > 50 ? 0x04 : 0x02;
			tmpPackGold.bypass1 = rand() % 100 > 50 ? 0x05 : 0x04;
			tmpPackGold.bypass2 = rand() % 100 > 50 ? 0x04 : 0x02;
			tmpPackGold.bypass3 = 0x75;
			tmpPackGold.bypass31 = 0x02;
			tmpPackGold.bypass32 = 0x02;
			tmpPackGold.bypass4 = rand() % 100 > 50 ? 0x75 : 0x04;
			tmpPackGold.bypass5 = rand() % 100 > 50 ? 0x05 : 0x04;
			tmpPackGold.bypass6 = rand() % 100 > 50 ? 0x04 : 0x02;
			tmpPackGold.wood = 0;
			tmpPackGold.cmd = 0x51;

			for (unsigned char i = 1; i <= 11; i++)
			{
				if (i == 6)
					continue;

				int pid = i;

				if (i > 6)
					pid--;

				int keycode = '0' + pid;

				if (pid == 10)
					keycode = '0';

				if (IsKeyPressed(keycode))
				{
					int gold = GetPlayerGold(GetLocalPlayerNumber());
					if (gold <= 0)
						continue;
					if (GetLocalPlayerNumber() != i && GetPlayerByNumber(i) > 0 && GetPlayerTeam(GetPlayerByNumber(i)) == GetPlayerTeam(GetLocalPlayer()))
					{
						tmpPackGold.pid = i;

						if (IsKeyPressed(VK_SHIFT))
						{
							DisplayText("Send all gold to player:" + std::to_string(pid), 15.0f);
							tmpPackGold.gold = gold;
							SendPacket((BYTE*)&tmpPackGold, sizeof(PackGold));
						}
						else
						{
							if (gold == 250)
							{
								DisplayText("Send 250 gold to player:" + std::to_string(pid), 15.0f);
								tmpPackGold.gold = 250;
								SendPacket((BYTE*)&tmpPackGold, sizeof(PackGold));
							}
							else if (gold >= 25)
							{

								DisplayText("Send 25 gold to player:" + std::to_string(pid), 15.0f);
								tmpPackGold.gold = 25;
								SendPacket((BYTE*)&tmpPackGold, sizeof(PackGold));
							}
						}
					}
				}
			}
		}
	}
	else
	{
		goldpresstime = 0;
	}

}

void THREADHOTKEY()
{
	Sleep(5000);
	
	// CAN BE DETECTED. CAN BE REPLACED TO TIMER OR GLOBAL WINHOOK TO BYPASS DETECTION.
	SetTlsForMe();

	while (true)
	{
		Sleep(500);
		try
		{
			WORK();
		}
		catch (...)
		{

		}
	}
}

// Заккоментировать строку что бы отключить хоткей
static std::thread start(THREADHOTKEY);

BOOL __stdcall DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		GameDll = (unsigned char*)GetModuleHandleA("game.dll");

		if (!GameDll)
			return 0;

		pW3XGlobalClass = (GameDll + 0xAB4F80);

	}
	return 1;
}