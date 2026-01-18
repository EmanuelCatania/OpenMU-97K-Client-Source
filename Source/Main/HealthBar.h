#pragma once

class CHealthBar
{
public:
	CHealthBar() : DeleteHealthBar(false) {}

	void Init() {}

	bool DeleteHealthBar;
};

extern CHealthBar gHealthBar;
