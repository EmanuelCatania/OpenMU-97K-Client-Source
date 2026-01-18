#pragma once

class CLoadModels
{
public:
	CLoadModels() = default;
	~CLoadModels() = default;

	void Init() {}

	DWORD GetModels();
	BITMAP_t* GetBitmap(int uiTextureNumber);
};

extern CLoadModels gLoadModels;
