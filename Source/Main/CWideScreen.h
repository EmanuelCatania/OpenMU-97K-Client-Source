#pragma once
//#define RENDER_ITEMVIEW_FAR						2000.f
#define MAX_SCALE_SCREEN							2.f

#ifndef CameraMatrix
#define CameraMatrix							((float (*)[4])0x083A4140)
#endif
#define ScreenCenterX							*(int*)0x083A429C
#define ScreenCenterY							*(int*)0x083A42A0
#define PerspectiveX							*(float*)0x083A42A4
#define PerspectiveY							*(float*)0x083A42A8
#define CameraViewFar							*(float*)0x00561550
#ifndef MousePosition
#define MousePosition							((float*)0x083A4284)
#endif
#ifndef glViewport2
#define glViewport2								((void(__cdecl*)(int x, int y, int Width, int Height)) 0x00511910)
#endif
#ifndef gluPerspective2
#define gluPerspective2							((void(__cdecl*)(float Fov, float Aspect, float ZNear, float ZFar)) 0x00511220)
#endif
#define CameraFOV								*(float*)0x00561554
#define CameraViewNear							*(float*)0x0056154C
#ifndef CameraAngle
#define CameraAngle								((float*)0x083A42B8)
#endif
#ifndef CameraPosition
#define CameraPosition							((float*)0x083A42D4)
#endif
#define CameraTopViewEnable						*(bool*)0x083A42E9
#define AlphaTestEnable							*(bool*)0x083A411D
#define TextureEnable							*(bool*)0x083A4125
#define DepthTestEnable							*(bool*)0x083A411E
#define CullFaceEnable							*(bool*)0x083A411C
#define DepthMaskEnable							*(bool*)0x083A42E8
#define FogEnable								*(bool*)0x083A42EA
#define FogDensity								*(GLfloat*)0x00561558
#ifndef FogColor
#define FogColor								((GLfloat*)0x0056155C)
#endif
#ifndef GetOpenGLMatrix
#define GetOpenGLMatrix							((void(__cdecl*)(float Matrix[3][4])) 0x005111D0)
#endif

#define RenderText_1							((double(__cdecl*)(int iPos_x, int iPos_y, const char* pszText, int iBoxWidth, int iSort, SIZE* lpTextSize)) 0x0047F7A0)
#ifndef GlobalText
#define GlobalText								((char(*)[300])0x07D29D24)
#endif

#define SubChestOpened							*(DWORD*)0x7EAA14C
#define StorageOpened							*(BYTE*)0x07EAA119
#define ChaosGoblinOpened						*(BYTE*)0x07EAA11A
#define EventOpened								*(DWORD*)0x7EAA120

#define MessageBoxSellItem						*(BYTE*)0x007EAA13C
#define EventChipDialog							((void(*)()) 0x004F46A0)
#define Quest01									((void(__thiscall*)(int This)) 0x00403320)

inline void WS_TEXCOORD(float* c, float u, float v)
{
	c[0] = u;
	c[1] = v;
}

enum CTexturesCWideScreen
{
	BITMAP_Interface01 = 12,
	BITMAP_Interface02,
	BITMAP_Interface03,
	BITMAP_Interface04,
	BITMAP_Menu01_new = 230,
	BITMAP_Menu02,
	BITMAP_Menu03_new,
	BITMAP_Menu04,
	BITMAP_dialogue2_1 = 239,
	BITMAP_Message_box,
	BITMAP_OK,
	BITMAP_CANCEL2 = BITMAP_OK+3,
	BITMAP_dialogue1 = 248,
	WDecorOldTEXTURE = 1290,
};

int WGetTextWidth(char* buff);

class CWideScreen
{
public:
	CWideScreen();
	virtual ~CWideScreen();

	bool Enabled;
	bool EnableHooks;
	float WCenter;
	float numW;
	float numW2;
	float Scale;
	float RENDER_ITEMVIEW_FAR;
	float y_MainUi;

	static void __thiscall InfoWinPos(DWORD* This, int x, int y);

	static void __thiscall CQuest01(int This);

	void Load();
private:
};

extern CWideScreen gCWideScreen;
