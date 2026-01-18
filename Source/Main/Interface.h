#pragma once

class Interface
{
public:
	void Init();

private:
	static void LoadImagesHook();
	static void UpdateWindowsMouseHook();
	static void RenderWindowsHook();
};

extern Interface gInterface;
