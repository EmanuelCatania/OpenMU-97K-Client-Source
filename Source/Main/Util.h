#pragma once

void SetByte(DWORD offset, BYTE value);

void SetWord(DWORD offset, WORD value);

void SetDword(DWORD offset, DWORD value);

void SetFloat(DWORD offset, float value);

void SetDouble(DWORD offset, double value);

void SetCompleteHook(BYTE head, DWORD offset, ...);

void MemoryCpy(DWORD offset, void* value, DWORD size);

void MemorySet(DWORD offset, DWORD value, DWORD size);

void VirtualizeOffset(DWORD offset, DWORD size);

void PacketArgumentEncrypt(char* out_buff, char* in_buff, int size);

char* ConvertModuleFileName(char* name);

SHORT WINAPI GetAsyncKeyStateCall(int key);

bool FileExists(char* name);

int GetTextWidth(char* buff);

int GetTextHeight(char* buff);

int CenterTextPosX(char* buff, int PosX);

int CenterTextPosY(const char* buff, int PosY);

int GetTextPosX(char* buff, int PosX);

int GetTextPosY(char* buff, int PosY);

float ImgCenterScreenPosX(float Size);

float ImgCenterScreenPosY(float Size);

bool IsWorkZone(int X, int Y, int Width, int Height);

void ConvertGold(double dGold, char* szText, int iDecimals = 0);
