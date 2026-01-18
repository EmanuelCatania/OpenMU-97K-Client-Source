#include "stdafx.h"
#include "LoadModels.h"
#include "Offset.h"

CLoadModels gLoadModels;

DWORD CLoadModels::GetModels()
{
	return Models;
}

BITMAP_t* CLoadModels::GetBitmap(int uiTextureNumber)
{
	if (uiTextureNumber < 0 || uiTextureNumber >= TOTAL_TEXTURE)
	{
		return nullptr;
	}

	return &Bitmaps[uiTextureNumber];
}
