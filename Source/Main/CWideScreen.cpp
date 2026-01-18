#include "stdafx.h"
#include "CWideScreen.h"
#include "Enums.h"

#include "ServerList.h"
#include "OptionsMenu.h"

/*kayito 97k*/
//Cada modificacion tiene un "/*WideScreen*/ o //WideScreen"

/*Se modifico FullMap.cpp (falta posicionar la ventana)*/
/*Se modifico GuildLogo.cpp*/
/*Se modifico Interface.cpp*/
/*Se modifico OptionsMenu.cpp ( Se anula la linea "gOptionsMenu.Init()" de Main.cpp )*/
/*Se modifico Patchs.cpp*/
/*Se modifico PrintPlayer.cpp*/
/*Se modifico ServerList.cpp ( Se anula la linea "this->Init()" de ServerList.cpp )*/
/*Se modifico Window.cpp*/
/**/

//---------------------------------------------------------------------------------------------
CWideScreen gCWideScreen;
CWideScreen::CWideScreen() 
{ 
	char buffer[256];
	this->Enabled = GetPrivateProfileIntA("WideScreen", "Enabled", 1, ".\\Config.ini") != 0;
	this->EnableHooks = GetPrivateProfileIntA("WideScreen", "EnableHooks", 1, ".\\Config.ini") != 0;
	GetPrivateProfileStringA("Window", "Scale", "1.0", buffer, sizeof(buffer), ".\\Config.ini");
	this->Scale = (float)atof(buffer);
}

CWideScreen::~CWideScreen() 
{
	char Text[33];
	sprintf_s(Text, sizeof(Text), "%.1f", gCWideScreen.Scale);
	WritePrivateProfileString("Window", "Scale", Text, ".\\Config.ini");
}
//---------------------------------------------------------------------------------------------

float WConvertX(float x)
{
	return x * (float)WindowWidth / gCWideScreen.numW;
}

float WConvertY(float y)
{
	return y * (float)WindowHeight / gCWideScreen.numW2;
}

void WRenderBitmap(int Texture, float x, float y, float Width, float Height, float u, float v, float uWidth, float vHeight, bool Scale, bool StartScale)
{
	/*ErrorMessageX*/
	//Falta terminar SubChestOpened
	if(!SubChestOpened)
	{	
		if (x == 213.f || Texture >= BITMAP_OK && Texture <= BITMAP_CANCEL2 ||
			(Texture == BITMAP_Message_box && x == 260.f))
		{
			x = x + gCWideScreen.WCenter;
		}
	}
	if(m_Resolution >= R1280x720)
	{
		if (Texture >= BITMAP_Menu01_new && Texture <= BITMAP_Menu03_new)
		{
			y = gCWideScreen.y_MainUi;
		}
		else if(Texture >= BITMAP_dialogue1 && Texture <= BITMAP_dialogue1 + 1)
		{
			y = gCWideScreen.y_MainUi - 17.f;
		}
		else if(Texture == BITMAP_dialogue2_1)
		{
			y = gCWideScreen.y_MainUi - 12.f;
		}
		else if(Texture == BITMAP_Menu04)
		{
			x = gCWideScreen.numW + 200.f;
		}
		else if(Texture >= 30 && Texture <= 32)
		{
			if(SceneFlag == LOADING_SCENE)
			{
				y = (gCWideScreen.numW2 - 256.f) / 2.f;
			}
		}
	}
	//Cuando se presiona "ESC", se cierran las ventanas para que no intervenga la ventana de "Options"
	if (GuildCreatorOpened || SubChestOpened)
	{
		if (KeyState[VK_ESCAPE])
		{
			SubChestOpened = 0;
			GuildCreatorOpened = 0;
			ErrorMessage = 0;
		}
	}	
	/**/

	if (StartScale)
	{
		x = WConvertX(x);
		y = WConvertY(y);
	}
	if (Scale)
	{
		Width = WConvertX(Width);
		Height = WConvertY(Height);
	}

	BindTexture(Texture);

	float p[4][2];

	y = WindowHeight - y;

	p[0][0] = x; p[0][1] = y;
	p[1][0] = x; p[1][1] = y - Height;
	p[2][0] = x + Width; p[2][1] = y - Height;
	p[3][0] = x + Width; p[3][1] = y;

	float c[4][2];
	WS_TEXCOORD(c[0], u, v);
	WS_TEXCOORD(c[3], u + uWidth, v);
	WS_TEXCOORD(c[2], u + uWidth, v + vHeight);
	WS_TEXCOORD(c[1], u, v + vHeight);

	glBegin(GL_TRIANGLE_FAN);
	for (int i = 0; i < 4; ++i)
	{
		glTexCoord2f(c[i][0], c[i][1]);
		glVertex2f(p[i][0], p[i][1]);
	}
	glEnd();
}

void WCreateScreenVector(int sx, int sy, vec3_t Target)
{
	sx = sx * WindowWidth / /*640*/(int)gCWideScreen.numW;
	sy = sy * WindowHeight / (int)gCWideScreen.numW2;
	vec3_t p1, p2;
	p1[0] = (float)(sx - ScreenCenterX) * PerspectiveX * gCWideScreen.RENDER_ITEMVIEW_FAR;
	p1[1] = -(float)(sy - ScreenCenterY) * PerspectiveY * gCWideScreen.RENDER_ITEMVIEW_FAR;
	p1[2] = -gCWideScreen.RENDER_ITEMVIEW_FAR;
	p2[0] = -CameraMatrix[0][3];
	p2[1] = -CameraMatrix[1][3];
	p2[2] = -CameraMatrix[2][3];
	VectorIRotate(p2, CameraMatrix, MousePosition);
	VectorIRotate(p1, CameraMatrix, p2);
	VectorAdd(MousePosition, p2, Target);
}

void WProjection(vec3_t Position, int* sx, int* sy)
{
	vec3_t TrasformPosition;
	VectorTransform(Position, CameraMatrix, TrasformPosition);
	*sx = -(int)(TrasformPosition[0] / PerspectiveX / TrasformPosition[2]) + ScreenCenterX;
	*sy = (int)(TrasformPosition[1] / PerspectiveY / TrasformPosition[2]) + ScreenCenterY;
	*sx = *sx * /*640*/(int)gCWideScreen.numW / (int)WindowWidth;
	*sy = *sy * (int)gCWideScreen.numW2 / (int)WindowHeight;
}

bool InvTradeShopEventCheck = false;
void CInvTradeShopEventCheck(float x, float y, int check)
{
	((void(__cdecl*)(float x, float y, int check)) 0x004ECBA0)(x, y, check);//Window_RenderFrame

	InvTradeShopEventCheck = true;
}

float wScreenWidth()
{
	if (SceneFlag == MAIN_SCENE && m_Resolution >= R1280x720 && gCWideScreen.Scale >= 1.05f)
	{
		if (InventoryOpened || CharacterOpened || PartyOpened || GuildOpened || GuildCreatorOpened || (GoldenArcherOpenType != 0) || ChaosGoblinOpened || StorageOpened || (InvTradeShopEventCheck == true))
		{
			if(CharacterOpened && InventoryOpened || ChaosGoblinOpened && InventoryOpened || StorageOpened && InventoryOpened || InventoryOpened && (InvTradeShopEventCheck == true))
			{
				return -225.f;
			}
			if (!InventoryOpened)
				InvTradeShopEventCheck = false;
			return -110.f;
		}
	}
	return 0.f;
}

void WBeginOpengl(int x, int y, int Width, int Height)
{
	if (Width == 640)
		Width = (int)gCWideScreen.numW;
	else if (Height == 480)
		Height = (int)gCWideScreen.numW2;

	x = x * WindowWidth / (int)gCWideScreen.numW;
	y = y * WindowHeight / (int)gCWideScreen.numW2;
	Width = Width * WindowWidth / (int)gCWideScreen.numW;
	Height = Height * WindowHeight / (int)gCWideScreen.numW2;

	float wFov;
	float wangle;
	if (m_Resolution >= R1280x720)
	{
		wFov = (SceneFlag == MAIN_SCENE ? CameraFOV + 4.f : CameraFOV);
		wangle = (SceneFlag == MAIN_SCENE ? CameraAngle[0] - -2.f : CameraAngle[0]);
	}
	else
	{
		wFov = CameraFOV;
		wangle = CameraAngle[0];
	}
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glViewport2(x, y, Width, Height);
	gluPerspective2(wFov, (float)Width / (float)Height, CameraViewNear, CameraViewFar * 1.4f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glRotatef(CameraAngle[1], 0.f, 1.f, 0.f);
	if (CameraTopViewEnable == false)
		glRotatef(wangle, 1.f, 0.f, 0.f);
	glRotatef(CameraAngle[2], 0.f, 0.f, 1.f);
	glTranslatef(-CameraPosition[0] + wScreenWidth(), -CameraPosition[1] + wScreenWidth(), -CameraPosition[2]);
	glDisable(GL_ALPHA_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDepthMask(true);
	AlphaTestEnable = false;
	TextureEnable = true;
	DepthTestEnable = true;
	CullFaceEnable = true;
	DepthMaskEnable = true;
	glDepthFunc(GL_LEQUAL);
	glAlphaFunc(GL_GREATER, 0.25f);
	if (FogEnable)
	{
		glEnable(GL_FOG);
		glFogi(GL_FOG_MODE, GL_LINEAR);
		glFogf(GL_FOG_DENSITY, FogDensity);
		glFogfv(GL_FOG_COLOR, FogColor);
	}
	else
	{
		glDisable(GL_FOG);
	}
	GetOpenGLMatrix(CameraMatrix);
}

int WGetTextWidth(char* buff)
{
	SIZE sz;

	GetTextExtentPoint(m_hFontDC, buff, strlen(buff), &sz);

	return ((int)gCWideScreen.numW * sz.cx / WindowWidth);
}

void WRenderText(int iPos_x, int iPos_y, const char* pszText, int iBoxWidth, int iSort, SIZE* lpTextSize)
{
	iBoxWidth = m_Resolution >= R1280x720 ? ((int)(iBoxWidth * 0.75 / gCWideScreen.Scale)) : iBoxWidth;
	RenderText_1(iPos_x, iPos_y, pszText, iBoxWidth, ((BYTE)iSort != 0) + 1, lpTextSize);
}

void WRenderText_2(int iPos_x, int iPos_y, char * pszText)
{
	SIZE sz;
	GetTextExtentPoint(g_hDC, pszText, strlen(pszText), &sz);
	RenderText(iPos_x - (((int)gCWideScreen.numW * sz.cx / WindowWidth) >> 1), iPos_y, pszText, 0, 0, 0);
}

int WGetScreenWidth()
{
	if (m_Resolution >= R1280x720 && gCWideScreen.Scale >= 1.05f)
		return (int)gCWideScreen.numW;
	else
		return GetScreenWidth();
}

void Center_RenderBitmap(int Texture, float x, float y, float Width, float Height, float u, float v, float uWidth, float vHeight, bool Scale, bool StartScale)
{
	x = x + gCWideScreen.WCenter;
	RenderBitmap(Texture, x, y, Width, Height, u, v, uWidth, vHeight, Scale, StartScale);
	if (m_Resolution >= R1280x720)
	{
		if (Texture == BITMAP_Menu03_new)
		{
			y = gCWideScreen.y_MainUi;
			EnableAlphaTest(true);
			RenderBitmap(WDecorOldTEXTURE, x - 445.f, y - 21.f, 144.f, 69.f, 0.f, 0.f, 1.f, 1.f, true, true);
			RenderBitmap(WDecorOldTEXTURE, x + 172.f, y - 21.f, 144.f, 69.f, 1.f, 0.f, -1.f, 1.f, true, true);
			DisableAlphaBlend();
		}
	}
}

void Center_RenderText(int iPos_x, int iPos_y, const char* pszText, int iBoxWidth, int iSort, SIZE* lpTextSize)
{
	SIZE sz;
	int x = 320;
	GetTextExtentPoint(m_hFontDC, pszText, strlen(pszText), &sz);
	if(ErrorMessage == 151 || ErrorMessage == 153)
	{
		x = (MessageBoxSellItem == 1 ? 350 : 320);
	}
	RenderText((x - (((int)gCWideScreen.numW * sz.cx / WindowWidth) >> 1)) + (int)gCWideScreen.WCenter, iPos_y, (char*)pszText, iBoxWidth, iSort, lpTextSize);
}

void Center_RenderTextNotice(int iPos_x, int iPos_y, const char* pszText, int iBoxWidth, int iSort, SIZE* lpTextSize)
{
	SIZE sz;
	GetTextExtentPoint(m_hFontDC, pszText, strlen(pszText), &sz);
	RenderText_1((320 - (((int)gCWideScreen.numW * sz.cx / WindowWidth) >> 1)) + (int)gCWideScreen.WCenter, iPos_y, (char*)pszText, iBoxWidth, iSort, lpTextSize);
}

void Center_RenderColor(float x, float y, float width, float height)
{
	RenderColor(x + gCWideScreen.WCenter, y, width, height);
}

void Center_RenderTipText(int sx, int sy, char* Text)
{
	RenderTipText(sx + (int)gCWideScreen.WCenter, (int)gCWideScreen.y_MainUi - 15, Text);
}

void Center_RenderNumber2D(float x, float y, int Num, float Width, float Height)
{
	RenderNumber2D(x + gCWideScreen.WCenter, y, Num, Width, Height);
}

void OKCANCELGuildCreaton_RenderBitmap(int Texture, float x, float y, float Width, float Height, float u, float v, float uWidth, float vHeight, bool Scale, bool StartScale)
{
	RenderBitmap(Texture, x - gCWideScreen.WCenter, y, Width, Height, u, v, uWidth, vHeight, Scale, StartScale);
}

void GlobalText454_RenderText(int iPos_x, int iPos_y, const char* pszText, int iBoxWidth, int iSort, SIZE* lpTextSize)
{
	SIZE Size;
	char Text[100];

	strcpy(Text, GlobalText[454]);
	GetTextExtentPointA(g_hDC, Text, lstrlen(Text), &Size);
	RenderText((335 + (int)gCWideScreen.WCenter) - Size.cx * (int)gCWideScreen.numW / WindowWidth, ((int)gCWideScreen.numW2 - 1) - (int)gCWideScreen.numW * Size.cy / WindowWidth, Text, iBoxWidth, iSort, lpTextSize);

	strcpy(Text, GlobalText[455]);
	GetTextExtentPointA(g_hDC, Text, lstrlen(Text), &Size);
	RenderText(335 + (int)gCWideScreen.WCenter, ((int)gCWideScreen.numW2 - 1) - (int)gCWideScreen.numW * Size.cy / WindowWidth, Text, iBoxWidth, iSort, lpTextSize);

	sprintf(Text, GlobalText[456], m_ExeVersion);
	GetTextExtentPointA(g_hDC, Text, lstrlen(Text), &Size);
	RenderText(0, ((int)gCWideScreen.numW2 - 1) - (int)gCWideScreen.numW * Size.cy / WindowWidth, Text, iBoxWidth, iSort, lpTextSize);
}

void WRenderLogInScene()
{
	SetCompleteHook(0xE8, 0x0052174D, &Center_RenderBitmap);
	SetCompleteHook(0xE8, 0x005217FD, &Center_RenderBitmap);
	SetCompleteHook(0xE8, 0x005218AB, &Center_RenderBitmap);
	SetDword(0x0051FC8C + 6, (DWORD)gCWideScreen.numW2);
	SetDword(0x00520FF7 + 1, (DWORD)gCWideScreen.numW2 - 216);

	SetDword(0x00521773 + 1, 295 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x0052179B + 1, 295 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x00521761 + 1, 227 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x0052178C + 1, 227 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x00521838 + 1, 357 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x005218DA + 1, 319 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x005217B0 + 1, 323 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x005217BB + 1, 393 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x005202DB + 2, 323 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x005202E3 + 2, 393 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x0052185E + 1, 285 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x00521869 + 1, 355 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x00520F2C + 2, 285 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x00520F38 + 2, 355 + (DWORD)gCWideScreen.WCenter);

	SetDword(0x00520260 + 2, 293 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x00520268 + 2, 417 + (DWORD)gCWideScreen.WCenter);

	SetCompleteHook(0xE8, 0x005219AD, &GlobalText454_RenderText);	
	SetCompleteHook(0xE8, 0x00521A20, &GlobalText454_RenderText);
	SetCompleteHook(0xE8, 0x00521A80, &GlobalText454_RenderText);

	SetCompleteHook(0xE8, 0x00521B31, &Center_RenderBitmap);
	SetCompleteHook(0xE8, 0x00521BC0, &Center_RenderBitmap);

	SetDword(0x00521BD2 + 1, 285 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x00521BD9 + 1, 355 + (DWORD)gCWideScreen.WCenter);

	SetDword(0x00521CAC + 1, 320 + (DWORD)gCWideScreen.WCenter);

	SetDword(0x00520190 + 2, 285 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x0052019C + 2, 355 + (DWORD)gCWideScreen.WCenter);

	//ServerListY
	SetDword(0x0051F259 + 1, ((DWORD)gCWideScreen.numW2 / 2) - 20);//Interface
	SetDword(0x0051E85C + 1, ((DWORD)gCWideScreen.numW2 / 2) - 20);//MouseY
}

float EM_ChaosGoblinMouseX1 = 0.f; float EM_ChaosGoblinMouseX2 = 0.f;
void ErrorMessage_ChaosGoblin()
{
	SetCompleteHook(0xE8, 0x0051B6DF, &Center_RenderBitmap);
	SetCompleteHook(0xE8, 0x0051B727, &Center_RenderBitmap);
	SetCompleteHook(0xE8, 0x0051B76A, &Center_RenderBitmap);
	SetCompleteHook(0xE8, 0x0051B819, &Center_RenderBitmap);

	SetCompleteHook(0xE8, 0x0051B8E0, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051B906, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051B920, &Center_RenderText);

	EM_ChaosGoblinMouseX1 = 245.f + gCWideScreen.WCenter; EM_ChaosGoblinMouseX2 = 395.f + gCWideScreen.WCenter;
	SetDword(0x0051B7AA + 2, (DWORD)&EM_ChaosGoblinMouseX1);
	SetDword(0x0051B7B9 + 2, (DWORD)&EM_ChaosGoblinMouseX2);

	SetDword(0x00515A81 + 2, (DWORD)&EM_ChaosGoblinMouseX1);
	SetDword(0x00515A90 + 2, (DWORD)&EM_ChaosGoblinMouseX2);

	SetCompleteHook(0xE8, 0x0051B894, &Center_RenderText);
}

__declspec(naked) void PosOKButton_MessageBoxSellItem()
{
	static DWORD CReturn = 0x0051E3C4;
	_asm
	{
		MOV DWORD PTR SS : [ESP + 0x10] , 1
		MOV DWORD PTR SS : [ESP + 0x14] , EAX
		MOV DWORD PTR SS : [ESP + 0x18] , EDX
		MOV DWORD PTR SS : [ESP + 0x20] , 21
		JMP CReturn
	}
}
__declspec(naked) void PosCancelButton_MessageBoxSellItem()
{
	static DWORD CReturn = 0x0051E3F0;
	_asm
	{
		MOV DWORD PTR SS : [ESP + 0x2C] , EDX
		MOV DWORD PTR SS : [ESP + 0x34] , 21
		MOV EAX, DWORD PTR SS : [ESP + 0x78]
		JMP CReturn
	}
}

float XButtonOKCancel_CreateOkMessageBox = 0.f;
void ErrorMessageX()
{
	ErrorMessage_ChaosGoblin();

	SetCompleteHook(0xE8, 0x0051B11C, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051B176, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051B1C2, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051B211, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051B29A, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051B2E6, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051B335, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051B386, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051B4F3, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051B57A, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051B601, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051B64C, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051C24D, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051C29A, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051C2F2, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051C3DE, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051C467, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051C616, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051C9B2, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051CA88, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051CBA8, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051CC74, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051CCFD, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051CDB8, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051CE61, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051D105, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051D200, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051D32A, &Center_RenderText);
	
	//Blood Castle
	SetCompleteHook(0xE8, 0x0051DB7E, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051DC07, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051DC53, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051DCF3, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051DD6C, &Center_RenderText);
	SetCompleteHook(0xE8, 0x0051DDDC, &Center_RenderText);

	SetDword(0x0051BBDA + 1, 323 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x0051BBE7 + 1, 363 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x0051BC62 + 1, 373 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x0051BC69 + 1, 413 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x0051BAA9 + 1, 234 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x0051BAB0 + 1, 304 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x0051BB2B + 1, 334 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x0051BB32 + 1, 404 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x0051BA17 + 1, 284 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x0051BA1E + 1, 354 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x0051B037 + 1, 260 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x0051B03E + 1, 380 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x0051B412 + 1, 260 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x0051B419 + 1, 380 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x0051C037 + 2, 249 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x0051C044 + 1, 202 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x0051C0E8 + 2, 377 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x0051C0F5 + 1, 202 + (DWORD)gCWideScreen.WCenter);

	SetDword(0x00517CEB + 1, 323 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x00517CDF + 1, 234 + (DWORD)gCWideScreen.WCenter);

	SetDword(0x005199B0 + 1, 373 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x005199A4 + 1, 334 + (DWORD)gCWideScreen.WCenter);

	SetDword(0x0051448D + 2, 260 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x00514499 + 2, 380 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x00515970 + 2, 260 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x0051597C + 2, 380 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x00515B89 + 1, 284 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x00515B90 + 1, 354 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x0051AC2B + 1, 249 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x0051AC32 + 2, 202 + (DWORD)gCWideScreen.WCenter);

	SetDword(0x0051B9AD + 1, 278 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x0051BBBE + 1, 243 + (DWORD)gCWideScreen.WCenter);

	SetDword(0x0051D732 + 4, 71 + (DWORD)gCWideScreen.WCenter);//CreateOkMessageBox(0x0051D6F0)
	SetDword(0x0051D7E8 + 4, 71 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x0051DA28 + 4, 71 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x0051DABE + 4, 71 + (DWORD)gCWideScreen.WCenter);
	XButtonOKCancel_CreateOkMessageBox = 213.f - gCWideScreen.WCenter;
	SetDword(0x0051BFC9 + 2, (DWORD)&XButtonOKCancel_CreateOkMessageBox);

	//MessageBoxSellItem
	SetCompleteHook(0xE8, 0x0051D437, &Center_RenderText);
	SetFloat(0x0051E1D8 + 1, 245.f + gCWideScreen.WCenter);//RenderItem3D
	SetDword(0x0051BE18 + 2, (DWORD)&XButtonOKCancel_CreateOkMessageBox);
	SetDword(0x0051E390 + 1, 21 + (DWORD)gCWideScreen.WCenter);//OK
	SetDword(0x0051E3DC + 4, 120 + (DWORD)gCWideScreen.WCenter);//Cancel
	SetCompleteHook(0xE9, 0x0051E3B0, &PosOKButton_MessageBoxSellItem);
	SetCompleteHook(0xE9, 0x0051E3E4, &PosCancelButton_MessageBoxSellItem);
	SetFloat(0x00552CAC, 245.f + gCWideScreen.WCenter);
	SetFloat(0x00552C24, 285.f + gCWideScreen.WCenter);
	SetDword(0x004F67D6 + 1, 265 + (DWORD)gCWideScreen.WCenter);

	SetFloat(0x0051C0A2 + 1, 253.f + gCWideScreen.WCenter);
	SetFloat(0x0051C153 + 1, 383.f + gCWideScreen.WCenter);

	SetDword(0x0051AD70 + 1, 319 + (DWORD)gCWideScreen.WCenter);
}

void WRenderInterfaceEdge()
{
	EnableAlphaTest(true);
	glColor3f(1.f, 1.f, 1.f);

	RenderBitmap(BITMAP_Interface01, 64.f, 0.f, 256.f + gCWideScreen.WCenter, 86.f, 1.f, 0.f, -1.f, 0.671875f, true, true);
	RenderBitmap(BITMAP_Interface02, 0.f, 0.f, 64.0, 256.f, 1.0, 0.f, -1.f, 1.f, true, true);
	RenderBitmap(BITMAP_Interface03, 0.f, 256.f, 64.f, gCWideScreen.numW2 - 256.f, 1.f, 0.f, -1.f, 0.875f, true, true);
	RenderBitmap(BITMAP_Interface04, 64.f, gCWideScreen.numW2 - 32.f, 256.f, 32.f, 1.f, 0.f, -1.f, 1.f, true, true);

	RenderBitmap(BITMAP_Interface01, 320.f + gCWideScreen.WCenter, 0.f, 256.f + gCWideScreen.WCenter, 86.f, 0.f, 0.f, 1.f, 0.671875f, true, true);
	RenderBitmap(BITMAP_Interface02, gCWideScreen.numW - 64.f, 0.f, 64.f, 256.f, 0.f, 0.f, 1.f, 1.f, true, true);
	RenderBitmap(BITMAP_Interface03, gCWideScreen.numW - 64.f, 256.f, 64.f, gCWideScreen.numW2 - 256.f, 0.f, 0.f, 1.f, 0.875f, true, true);
	RenderBitmap(BITMAP_Interface04, gCWideScreen.numW - 256.f, gCWideScreen.numW2 - 32.f, 256.f, 32.f, 0.f, 0.f, 1.f, 1.f, true, true);
}

void WRenderCharacterScene()
{
	SetCompleteHook(0xE9, 0x005239A0, &WRenderInterfaceEdge);

	SetCompleteHook(0xE8, 0x005248E8, &Center_RenderBitmap);
	SetCompleteHook(0xE8, 0x00524949, &Center_RenderBitmap);
	SetDword(0x005248F9 + 2, 286 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x00524905 + 2, 356 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x0052243E + 2, 286 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x0052244D + 2, 356 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x00524A35 + 1, (DWORD)gCWideScreen.numW - 204);
	SetDword(0x0052276E + 1, (DWORD)gCWideScreen.numW - 200);

	SetCompleteHook(0xE8, 0x005242ED, &Center_RenderBitmap);
	SetCompleteHook(0xE8, 0x0052432A, &Center_RenderBitmap);
	SetCompleteHook(0xE8, 0x00524397, &Center_RenderBitmap);
	SetCompleteHook(0xE8, 0x0052445F, &Center_RenderBitmap);
	SetCompleteHook(0xE8, 0x00524528, &Center_RenderBitmap);
	SetCompleteHook(0xE8, 0x00524595, &Center_RenderBitmap);
	SetCompleteHook(0xE8, 0x005245F8, &Center_RenderBitmap);
	SetCompleteHook(0xE8, 0x00524653, &Center_RenderBitmap);

	SetCompleteHook(0xE8, 0x005243D8, &Center_RenderColor);
	SetCompleteHook(0xE8, 0x0052449B, &Center_RenderColor);

	SetCompleteHook(0xE8, 0x005246C6, &Center_RenderText);

	SetDword(0x0052468C + 1, 285 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x00524796 + 1, 285 + (DWORD)gCWideScreen.WCenter);

	SetDword(0x00524409 + 1, 384 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x00524414 + 1, 404 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x0052433F + 1, 363 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x0052434A + 1, 383 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x005244D0 + 1, 363 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x005244DB + 1, 383 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x00524543 + 1, 384 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x0052454E + 1, 404 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x005245AE + 1, 235 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x005245BB + 1, 307 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x0052460B + 1, 335 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x00524616 + 1, 407 + (DWORD)gCWideScreen.WCenter);

	SetDword(0x005229C1 + 2, 363 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x005229CF + 2, 383 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x00522A12 + 2, 384 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x00522A1A + 2, 404 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x00522A53 + 2, 235 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x00522A60 + 2, 307 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x00522AEC + 2, 335 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x00522AF4 + 2, 407 + (DWORD)gCWideScreen.WCenter);

	SetDword(0x00522846 + 2, 363 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x00522858 + 2, 383 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x005228E5 + 2, 384 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x005228F1 + 2, 404 + (DWORD)gCWideScreen.WCenter);
}

void WUIMainFrameWindow_RenderButtons(BYTE a1)
{
	if (IsWorkZone(348 + (int)gCWideScreen.WCenter, (int)gCWideScreen.y_MainUi+20, 24, 24))
	{
		RenderTipText(348 + (int)gCWideScreen.WCenter, (int)gCWideScreen.y_MainUi+5, GlobalText[361]);
	}
	if (PartyOpened)
		RenderBitmap(236, 348.f + gCWideScreen.WCenter, gCWideScreen.y_MainUi + 20.f, 24.f, 24.f, 0.f, 0.f, 0.75f, 0.75f, true, true);

	if (IsWorkZone(379 + (int)gCWideScreen.WCenter, (int)gCWideScreen.y_MainUi + 20, 24, 24))
	{
		RenderTipText(379 + (int)gCWideScreen.WCenter, (int)gCWideScreen.y_MainUi+5, GlobalText[362]);
	}
	if (CharacterOpened)
		RenderBitmap(237, 379.f + gCWideScreen.WCenter, gCWideScreen.y_MainUi + 20.f, 24.f, 24.f, 0.f, 0.f, 0.75f, 0.75f, true, true);

	if (IsWorkZone(410 + (int)gCWideScreen.WCenter, (int)gCWideScreen.y_MainUi + 20, 24, 24))
	{
		RenderTipText(410 + (int)gCWideScreen.WCenter, (int)gCWideScreen.y_MainUi+5, GlobalText[363]);
	}
	if (InventoryOpened)
		RenderBitmap(238, 410.f + gCWideScreen.WCenter, gCWideScreen.y_MainUi + 20.f, 24.f, 24.f, 0.f, 0.f, 0.75f, 0.75f, true, true);

	if (IsWorkZone(582 + (int)gCWideScreen.WCenter, (int)gCWideScreen.y_MainUi+27, 24, 24))
	{
		RenderTipText(582 + (int)gCWideScreen.WCenter, (int)gCWideScreen.y_MainUi+12, GlobalText[364]);
	}
	if (GuildOpened)
		RenderBitmap(250, 582.f + gCWideScreen.WCenter, gCWideScreen.y_MainUi+27.f, 52.f, 18.f, 0.f, 0.f, 0.8125f, 0.5625f, true, true);

	EnableAlphaTest(true);
}

float HPMouseX1 = 0.f;float HPMouseX2 = 0.f;
float MPMouseX1 = 0.f;float MPMouseX2 = 0.f;
float BPMouseX1 = 0.f;float BPMouseX2 = 0.f;

float HPMouseY1 = 0.f; float HPMouseY2 = 0.f;

float SkillBarX = 0.f;
void WUIMainFrameWindow()
{
	//SetCompleteHook(0xE8, 0x004BD300, &Center_RenderBitmap);//&this->RenderFullMap(kayito 97k)
	//SetCompleteHook(0xE8, 0x004BD332, &Center_RenderBitmap);//&this->RenderZoom(kayito 97k)
	SetCompleteHook(0xE8, 0x004BD364, &Center_RenderBitmap);
	SetCompleteHook(0xE8, 0x004BD602, &Center_RenderBitmap);
	SetCompleteHook(0xE8, 0x004BD632, &Center_RenderBitmap);

	SetCompleteHook(0xE8, 0x004BD9F8, &Center_RenderTipText);
	SetCompleteHook(0xE8, 0x004BDAC2, &Center_RenderTipText);
	SetCompleteHook(0xE8, 0x004BD069, &Center_RenderTipText);
	HPMouseX1 = 97.f + gCWideScreen.WCenter;	HPMouseX2 = 150.f + gCWideScreen.WCenter;
	MPMouseX1 = 489.f + gCWideScreen.WCenter;MPMouseX2 = 542.f + gCWideScreen.WCenter;
	BPMouseX1 = 551.f + gCWideScreen.WCenter;BPMouseX2 = 566.f + gCWideScreen.WCenter;
	SetDword(0x004BD99D + 2, (DWORD)&HPMouseX1);SetDword(0x004BD9AA + 2, (DWORD)&HPMouseX2);
	SetDword(0x004BDA64 + 2, (DWORD)&MPMouseX1);SetDword(0x004BDA71 + 2, (DWORD)&MPMouseX2);
	SetDword(0x004BD01C + 2, (DWORD)&BPMouseX1);SetDword(0x004BD029 + 2, (DWORD)&BPMouseX2);

	HPMouseY1 = gCWideScreen.y_MainUi;  HPMouseY2 = gCWideScreen.y_MainUi + 48.f;
	SetDword(0x004BD9BD + 2, (DWORD)&HPMouseY1); SetDword(0x004BD9CA + 2, (DWORD)&HPMouseY2);
	SetDword(0x004BDA84 + 2, (DWORD)&HPMouseY1); SetDword(0x004BDA91 + 2, (DWORD)&HPMouseY2);
	SetDword(0x004BD03B + 1, (int)gCWideScreen.y_MainUi+5); SetDword(0x004BD042 + 1, (int)gCWideScreen.y_MainUi+41);

	/*Pociones*/
	SetDword(0x004BFED0 + 1, 208 + (DWORD)gCWideScreen.WCenter);
	SetFloat(0x004BFF14 + 1, gCWideScreen.y_MainUi + 22.f);
	SetDword(0x004B5EC8 + 1, 208 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x004B5ED8 + 2, (int)gCWideScreen.y_MainUi+20);
	SetDword(0x004B5EE0 + 2, (int)gCWideScreen.y_MainUi+40);

	SetDword(0x004BFF29 + 2, 301 + (DWORD)gCWideScreen.WCenter);
	SetCompleteHook(0xE8, 0x004BE4C2, &Center_RenderNumber2D);
	SetFloat(0x004BE4AD + 1, gCWideScreen.y_MainUi + 15.f);
	/**/

	SetCompleteHook(0xE8, 0x004BE0F9, &Center_RenderNumber2D);//coord
	SetFloat(0x004BE0EF + 1, gCWideScreen.y_MainUi + 30.f);
	SetCompleteHook(0xE8, 0x004BE1CC, &Center_RenderNumber2D);//coord
	SetFloat(0x004BE1C2 + 1, gCWideScreen.y_MainUi + 30.f);

	/*Skill*/
	SetDword(0x004B3D57 + 1, 304 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x004B3D62 + 1, 336 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x004B3D6E + 1, (DWORD)gCWideScreen.y_MainUi+12);
	SetDword(0x004B3D75 + 1, (DWORD)gCWideScreen.y_MainUi+48);

	SetDword(0x004B3D8A + 6, 320 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x004B3DA0 + 6, (DWORD)gCWideScreen.y_MainUi + 12);

	SetDword(0x004B3E23 + 1, 320 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x004B3E41 + 2, (DWORD)gCWideScreen.y_MainUi - 62);
	SetDword(0x004B3E4D + 2, (DWORD)gCWideScreen.y_MainUi - 20);
	SetDword(0x004B3E6E + 6, (DWORD)gCWideScreen.y_MainUi - 62);

	SetDword(0x004BDE24 + 1, 304 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x004BDE1F + 1, (DWORD)gCWideScreen.y_MainUi + 12);
	SkillBarX = 320.f + gCWideScreen.WCenter;
	SetDword(0x004BDE85 + 2, (DWORD)&SkillBarX);

	SetDword(0x004BDF11 + 1, (DWORD)gCWideScreen.y_MainUi - 62);
	/**/

	SetCompleteHook(0xE8, 0x004BE012, &WUIMainFrameWindow_RenderButtons);
	MemorySet(0x004BDACC, 0x90, 0x25E); // Remove Mouse,RenderBitmap,RenderTipText Button
	//-- Button Main::Party
	SetDword(0x004B22D2 + 2, 348 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x004B22DE + 2, 372 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x004B22EA + 1, (DWORD)gCWideScreen.y_MainUi + 20);
	SetDword(0x004B22F5 + 1, (DWORD)gCWideScreen.y_MainUi + 44);
	//-- Button Main::Character
	SetDword(0x004B2C3C + 2, 379 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x004B2C48 + 2, 403 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x004B2C54 + 1, (DWORD)gCWideScreen.y_MainUi + 20);
	SetDword(0x004B2C5F + 1, (DWORD)gCWideScreen.y_MainUi + 44);
	//-- Button Main::Inventory
	SetDword(0x004B3500 + 2, 410 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x004B350C + 2, 434 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x004B3518 + 1, (DWORD)gCWideScreen.y_MainUi + 20);
	SetDword(0x004B3523 + 1, (DWORD)gCWideScreen.y_MainUi + 44);
	//-- Button Main::Guild
	SetDword(0x004B1967 + 2, 582 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x004B1973 + 2, 634 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x004B1984 + 1, (DWORD)gCWideScreen.y_MainUi + 27);
	SetDword(0x004B198F + 1, (DWORD)gCWideScreen.y_MainUi + 45);

	/*Chat*/
	SetCompleteHook(0xE8, 0x004BE562, &Center_RenderBitmap);
	SetCompleteHook(0xE8, 0x004BE592, &Center_RenderBitmap);
	SetCompleteHook(0xE8, 0x004BE701, &Center_RenderBitmap);
	SetDword(0x004BE5DA + 1, 191 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x004BE5D5 + 1,(DWORD)gCWideScreen.y_MainUi - 10);

	SetDword(0x004BE5F5 + 1, 376 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x004BE5ED + 1, (DWORD)gCWideScreen.y_MainUi - 10);

	SetDword(0x004BE667 + 1, 376 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x004BE660 + 2, (DWORD)gCWideScreen.y_MainUi - 10);

	SetDword(0x004BB801 + 1, 186 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x004BB808 + 1, 376 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x004BB80F + 2, (DWORD)gCWideScreen.y_MainUi - 17);
	SetDword(0x004BB817 + 2, (DWORD)gCWideScreen.y_MainUi - 10);

	SetDword(0x004BB85D + 1, 376 + (DWORD)gCWideScreen.WCenter);//mm
	SetDword(0x004BB8C3 + 1, 434 + (DWORD)gCWideScreen.WCenter);//mm
	SetDword(0x004BB8D1 + 2, (DWORD)gCWideScreen.y_MainUi - 17);
	SetDword(0x004BB8D9 + 2, (DWORD)gCWideScreen.y_MainUi - 10);

	SetDword(0x004BB864 + 1, 434 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x004BB8CA + 1, 454 + (DWORD)gCWideScreen.WCenter);
	SetDword(0x004BB86B + 2, (DWORD)gCWideScreen.y_MainUi - 17);
	SetDword(0x004BB873 + 2, (DWORD)gCWideScreen.y_MainUi - 10);

	SetFloat(0x005590B0 + 0, 295.0f + gCWideScreen.WCenter); //-- chat buttons
	SetFloat(0x005590B4, gCWideScreen.y_MainUi - 15.f); //-- chat buttons

	SetDword(0x0040E288 + 1, 160 + (DWORD)gCWideScreen.WCenter); //-- InputX
	SetDword(0x0040E283 + 1, (DWORD)gCWideScreen.y_MainUi + 4); //-- InputY

	SetDword(0x0040E9FD + 1, 460 + (DWORD)gCWideScreen.WCenter); //-InputX
	//SetDword(0x0040E9F2 + 1, (DWORD)gCWideScreen.y_MainUi - 45); //-- InputY
	/**/
}

void CWideScreen::InfoWinPos(DWORD* This, int x, int y)
{
	if (x == 186)//g_bUseChatListBox
		x = x + (DWORD)gCWideScreen.WCenter;
	else if(y == 420)
		y = (DWORD)gCWideScreen.y_MainUi - 12;
	/*Este HDP no actualiza cuando se cambia de resolucion(flashback del S6)*/
	else if (x >= 460)//GuildInfoWindowPos
		x = (DWORD)gCWideScreen.numW - 180;
	/**/
	This[11] = x;
	This[12] = y;
}

__declspec(naked) void InfoWinPos_UseChatListBox()
{
	static DWORD CReturn = 0x005258DB;
	static DWORD CGetScreenWidth = 0x004CB520;

	_asm
	{

		PUSH 420
		CALL CGetScreenWidth
		SUB EAX, 259
		PUSH EAX
		JMP CReturn
	}
}

void RenderLogInScene_BeginOpengl(int x, int y, int Width, int Height)
{
	WBeginOpengl(x, (int)(y / g_fScreenRate_y * ((float)WindowHeight / 480)), Width, (int)(Height / g_fScreenRate_y * ((float)WindowHeight / 480)));
}

void glViewport2MOD(int x, int y)
{
	x = x * WindowWidth / (int)gCWideScreen.numW;
	y = y * WindowHeight / (int)gCWideScreen.numW2;
	int Width = (int)gCWideScreen.numW * WindowWidth / (int)gCWideScreen.numW;
	int Height = (int)gCWideScreen.numW2 * WindowHeight / (int)gCWideScreen.numW2;

	glViewport2(x, y, Width, Height);
}

void glViewport2MOD_RenderBitmap(int Texture, float x, float y, float Width, float Height, float u, float v, float uWidth, float vHeight, bool Scale, bool StartScale)
{
	RenderBitmap(Texture, x - (gCWideScreen.numW - 640.f), y, Width, Height, u, v, uWidth, vHeight, Scale, StartScale);
}
void glViewport2MOD_RenderText(int iPos_x, int iPos_y, const char* pszText, int iBoxWidth, int iSort, SIZE* lpTextSize)
{
	RenderText(iPos_x - ((int)gCWideScreen.numW - 640), iPos_y, (char*)pszText, iBoxWidth, iSort, lpTextSize);
}
void glViewport2MOD_RenderTipText(int sx, int sy, char* Text)
{
	RenderTipText(sx - ((int)gCWideScreen.numW - 640), sy, Text);
}

void WEventChipDialogV2()
{
	glViewport2MOD((int)gCWideScreen.numW - 640, 0);

	EventChipDialog();

	glViewport2MOD(0, 0);
}
float Text_x1 = 0.f; float GoldenArcher_MouseX01 = 0.f; float GoldenArcher_MouseX02 = 0.f;
void CGoldenArcher()
{
	Text_x1 = 95.f - (gCWideScreen.numW - 640.f);
	SetDword(0x004F4FCB + 2, (DWORD)&Text_x1);

	SetCompleteHook(0xE8, 0x004F5CC1, &WEventChipDialogV2);
	SetCompleteHook(0xE8, 0x004F4759, &glViewport2MOD_RenderText);
	SetCompleteHook(0xE8, 0x004F4EF6, &glViewport2MOD_RenderBitmap);
	SetCompleteHook(0xE8, 0x004F4F8D, &glViewport2MOD_RenderTipText);

	SetDword(0x004F46DB + 6, (DWORD)gCWideScreen.numW - 190);
	SetDword(0x004E7AFA + 1, (DWORD)gCWideScreen.numW - 190);//MouseX Ventana
	SetDword(0x004E7B07 + 2, (DWORD)gCWideScreen.numW);//MouseX Ventana

	GoldenArcher_MouseX01 = gCWideScreen.numW - 155.f; GoldenArcher_MouseX02 = gCWideScreen.numW - 120.f;
	SetDword(0x004F48C1 + 2, (DWORD)&GoldenArcher_MouseX01); SetDword(0x004F48CE + 2, (DWORD)&GoldenArcher_MouseX02);

	SetDword(0x004E7B2B + 2, (DWORD)gCWideScreen.numW - 155);
	SetDword(0x004E7B37 + 2, (DWORD)gCWideScreen.numW - 120);

	SetDword(0x004E7DCA + 1, (DWORD)gCWideScreen.numW - 155);
	SetDword(0x004E7DEB + 2, (DWORD)gCWideScreen.numW - 120);

	SetFloat(0x004F553E + 1, gCWideScreen.numW - 100.f);
	SetFloat(0x004F5558 + 1, gCWideScreen.numW - 100.f);
}

void CWideScreen::CQuest01(int This)
{
	glViewport2MOD((int)gCWideScreen.numW - 640, 0);
	Quest01(This);
	glViewport2MOD(0, 0);
}
float CQuest01_MouseX01 = 0.f; float CQuest01_MouseX02 = 0.f;
float CQuest01_MouseX03 = 0.f; float CQuest01_MouseX04 = 0.f;
void CQuestMU()
{
	CQuest01_MouseX01 = gCWideScreen.numW - 155.f; CQuest01_MouseX02 = gCWideScreen.numW - 35.f;
	SetDword(0x004033E5 + 2, (DWORD)&CQuest01_MouseX01); SetDword(0x004033F2 + 2, (DWORD)&CQuest01_MouseX02);

	CQuest01_MouseX03 = gCWideScreen.numW - 165.f; CQuest01_MouseX04 = gCWideScreen.numW - 141.f;
	SetDword(0x00403592 + 2, (DWORD)&CQuest01_MouseX03); SetDword(0x0040359F + 2, (DWORD)&CQuest01_MouseX04);

	SetDword(0x00402F46 + 1, (DWORD)gCWideScreen.numW - 190);//MouseX Ventana
	SetDword(0x00402F4F + 1, (DWORD)gCWideScreen.numW);//MouseX Ventana

	SetDword(0x00402873 + 1, (DWORD)gCWideScreen.numW - 190);
	SetDword(0x0040287E + 1, (DWORD)gCWideScreen.numW);

	SetDword(0x004028A9 + 2, (DWORD)gCWideScreen.numW - 155);
	SetDword(0x004028B5 + 2, (DWORD)gCWideScreen.numW - 35);

	SetDword(0x00402ECF + 2, (DWORD)gCWideScreen.numW - 165);
	SetDword(0x00402ED7 + 2, (DWORD)gCWideScreen.numW - 141);

	SetDword(0x00402F79 + 1, (DWORD)gCWideScreen.numW - 165);
	SetDword(0x00402F80 + 1, (DWORD)gCWideScreen.numW - 141);

	SetDword(0x004030C9 + 1, (DWORD)gCWideScreen.numW - 84);//Fucking MouseX rendertext

	SetDword(0x00401B7D + 1, (DWORD)gCWideScreen.numW - 74);

	SetCompleteHook(0xE8, 0x00403F46, &CWideScreen::CQuest01);
}

float MBoxBloodCastle_MouseX01 = 0.f; float MBoxBloodCastle_MouseX02 = 0.f; float WidthMax = 0.f;
void CWideScreen::Load()
{	
	if (GetPrivateProfileIntA("WideScreen", "InitServerList", 1, ".\\Config.ini") != 0)
	{
		gServerList.Init();
	}
	if (GetPrivateProfileIntA("WideScreen", "InitOptionsMenu", 1, ".\\Config.ini") != 0)
	{
		gOptionsMenu.Init();
	}
	if (!this->Enabled || !this->EnableHooks)
	{
		return;
	}
	if (GetPrivateProfileIntA("WideScreen", "ApplyPatches", 1, ".\\Config.ini") == 0)
	{
		return;
	}

	if (GetPrivateProfileIntA("WideScreen", "PatchCoreHooks", 1, ".\\Config.ini") != 0)
	{
		SetCompleteHook(0xE9, 0x005125A0, &WRenderBitmap);
		SetCompleteHook(0xE9, 0x00511950, &WConvertX);
		SetCompleteHook(0xE9, 0x00511980, &WConvertY);
		SetCompleteHook(0xE9, 0x005112F0, &WCreateScreenVector);
		SetCompleteHook(0xE9, 0x005113F0, &WProjection);
		SetCompleteHook(0xE9, 0x005119B0, &WBeginOpengl);
		SetCompleteHook(0xE9, 0x0047F650, &WRenderText);
		SetCompleteHook(0xE9, 0x00514270, &WRenderText_2);
		SetCompleteHook(0xE8, 0x00525B6F, &WGetScreenWidth);
	}

	if (GetPrivateProfileIntA("WideScreen", "PatchSceneLimits", 1, ".\\Config.ini") != 0)
	{
		SetDword(0x0041981E + 2, (DWORD)gCWideScreen.numW);//Limite del MouseX
		SetDword(0x00419841 + 2, (DWORD)gCWideScreen.numW2);//Limite del MouseY
		SetDword(0x00525B67 + 2, (DWORD)(m_Resolution >= R1280x720 ? gCWideScreen.numW2 : 432));//Limite Zona negra(432)
		SetDword(0x00525488 + 6, (DWORD)(m_Resolution >= R1280x720 ? gCWideScreen.y_MainUi : 432));//Limite Zona negraMouseY(432)
	}

	//SetDword(0x00521665 + 1, 320 / (DWORD)gCWideScreen.Scale);//BeginOpenglY_RenderLogInScene

	if (GetPrivateProfileIntA("WideScreen", "PatchSceneHooks", 1, ".\\Config.ini") != 0)
	{
		SetCompleteHook(0xE8, 0x00521673, &RenderLogInScene_BeginOpengl);
		SetDword(0x00523B70 + 1, (DWORD)gCWideScreen.numW2);//BeginOpenglY_RenderCharacterScene
		
		SetCompleteHook(0xE8, 0x0047FDBE, &Center_RenderTextNotice);
		SetDword(0x0047FD25 + 1, (DWORD)gCWideScreen.y_MainUi - 132);
		
		WRenderLogInScene();
		WRenderCharacterScene();
		ErrorMessageX();
		WUIMainFrameWindow();
	}

	if (GetPrivateProfileIntA("WideScreen", "PatchLoadingScene", 1, ".\\Config.ini") != 0)
	{
		SetCompleteHook(0xE8, 0x00524BE9, &Center_RenderBitmap);//LoadingScene
		SetCompleteHook(0xE8, 0x00524C16, &Center_RenderBitmap);//LoadingScene
		SetCompleteHook(0xE8, 0x00524C46, &Center_RenderBitmap);//LoadingScene
	}

	/*RenderSands*/
	if (GetPrivateProfileIntA("WideScreen", "PatchRenderSands", 1, ".\\Config.ini") != 0)
	{
		SetFloat(0x004BC036 + 1, gCWideScreen.numW);//Width
		SetFloat(0x004BC082 + 1, gCWideScreen.numW);//Width
		SetFloat(0x004BC031 + 1, gCWideScreen.numW2);//Height
		SetFloat(0x004BC07D + 1, gCWideScreen.numW2);//Height

		WidthMax = gCWideScreen.numW;;
		SetDword(0x004BBE1D + 2, (DWORD)&WidthMax); SetDword(0x004BBE34 + 2, (DWORD)&WidthMax);//RenderBar
	}
	
	if (GetPrivateProfileIntA("WideScreen", "PatchEventChecks", 1, ".\\Config.ini") != 0)
	{
		SetCompleteHook(0xE8, 0x004F214B, &CInvTradeShopEventCheck);//Shop
		SetCompleteHook(0xE8, 0x004F1362, &CInvTradeShopEventCheck);//Trade
		SetCompleteHook(0xE8, 0x004F3F59, &CInvTradeShopEventCheck);//Blood Castle
		SetCompleteHook(0xE8, 0x004F3C8D, &CInvTradeShopEventCheck);//Devil Square
	}

	//float WRight = (m_Resolution >= R1280x720 ? 213.f : 0.f);
	/*Ventanas (450 = 190 , 260 = 380)*/
	if (GetPrivateProfileIntA("WideScreen", "PatchWindowCore", 1, ".\\Config.ini") != 0)
	{
		SetDword(0x004CB686 + 1, (DWORD)gCWideScreen.numW - 380);//GetScreenWidth(260)
		SetDword(0x004CB6E0 + 1, (DWORD)gCWideScreen.numW - 190);//GetScreenWidth(450)
		SetDword(0x004CB6D9 + 1, (DWORD)gCWideScreen.numW);//GetScreenWidth(640)

		SetCompleteHook(0xE9, 0x0040C690, &InfoWinPos);
		SetCompleteHook(0xE9, 0x005258D4, &InfoWinPos_UseChatListBox);
		SetDword(0x005258DD + 1, (DWORD)gCWideScreen.y_MainUi - 12);//UseChatListBox
	}
	
	//Inventory
	if (GetPrivateProfileIntA("WideScreen", "PatchWindowRight", 1, ".\\Config.ini") != 0)
	{
		SetDword(0x004F0A72 + 1, (DWORD)gCWideScreen.numW - 190);
		SetDword(0x004F0A79 + 1, (DWORD)gCWideScreen.numW - 380);
	//Trade
		SetDword(0x004F134D + 1, (DWORD)gCWideScreen.numW - 380);
		SetDword(0x004F1352 + 6, (DWORD)gCWideScreen.numW - 380);
	//Stats
		SetDword(0x004F5B9B + 1, (DWORD)gCWideScreen.numW - 190);
	//Storage(Falta mover subventana Lock)
		SetDword(0x004F318B + 1, (DWORD)gCWideScreen.numW - 380);
		SetDword(0x004F3190 + 6, (DWORD)gCWideScreen.numW - 380);
	//Shops
		SetDword(0x004F2132 + 1, (DWORD)gCWideScreen.numW - 380);
		SetDword(0x004F2137 + 6, (DWORD)gCWideScreen.numW - 380);
	//Guild
		SetDword(0x004F5A75 + 1, (DWORD)gCWideScreen.numW - 190);
		SetDword(0x004F585B + 1, (DWORD)gCWideScreen.numW - 190);//GuildCreator
		SetCompleteHook(0xE8, 0x004F5962, &OKCANCELGuildCreaton_RenderBitmap);
		SetCompleteHook(0xE8, 0x004F5A53, &OKCANCELGuildCreaton_RenderBitmap);
	//Party
		SetDword(0x004F5B7F + 1, (DWORD)gCWideScreen.numW - 190);
	}
	//Party Bar
	if (GetPrivateProfileIntA("WideScreen", "PatchPartyBar", 1, ".\\Config.ini") != 0)
	{
		SetDword(0x004EFD0F + 4, (DWORD)gCWideScreen.numW);
		SetDword(0x004EFD1B + 4, (DWORD)gCWideScreen.numW - 190);
		SetDword(0x004EFCBB + 4, (DWORD)gCWideScreen.numW - 380);
	/*Party Bar_MouseX Buff*/
		SetDword(0x004E5A6E + 4, (DWORD)gCWideScreen.numW);
		SetDword(0x004E5A7A + 4, (DWORD)gCWideScreen.numW - 190);
		SetDword(0x004E5A1A + 4, (DWORD)gCWideScreen.numW - 380);
	}
	//Chaos Goblin
	if (GetPrivateProfileIntA("WideScreen", "PatchEventWindows", 1, ".\\Config.ini") != 0)
	{
		SetDword(0x004F280A + 1, (DWORD)gCWideScreen.numW - 380);
		SetDword(0x004F280F + 6, (DWORD)gCWideScreen.numW - 380);
	//
		MBoxBloodCastle_MouseX01 = (gCWideScreen.numW - 380.f) + 25.f; MBoxBloodCastle_MouseX02 = (gCWideScreen.numW - 380.f) + 165.f;
		SetDword(0x004E6CAE + 2, (DWORD)&MBoxBloodCastle_MouseX01); SetDword(0x004E6CC3 + 2, (DWORD)&MBoxBloodCastle_MouseX02);
		SetDword(0x004E76BA + 2, (DWORD)&MBoxBloodCastle_MouseX01); SetDword(0x004E76C9 + 2, (DWORD)&MBoxBloodCastle_MouseX02);
	//Blood Castle
		SetDword(0x004F3F44 + 1, (DWORD)gCWideScreen.numW - 380);
		SetDword(0x004F3F49 + 6, (DWORD)gCWideScreen.numW - 380);
	//Devil Square
		SetDword(0x004F3C78 + 1, (DWORD)gCWideScreen.numW - 380);
		SetDword(0x004F3C7D + 6, (DWORD)gCWideScreen.numW - 380);
	}
	//Quest
	if (GetPrivateProfileIntA("WideScreen", "PatchQuest", 1, ".\\Config.ini") != 0)
	{
		CQuestMU();
	}
	//Golden Archer
	if (GetPrivateProfileIntA("WideScreen", "PatchGoldenArcher", 1, ".\\Config.ini") != 0)
	{
		CGoldenArcher();
	}
	/**/
}
