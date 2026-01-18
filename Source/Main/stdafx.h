#pragma once

typedef unsigned __int64 QWORD;

#define WIN32_LEAN_AND_MEAN

#define _WIN32_WINNT _WIN32_WINNT_WINXP

// System Include
#include <windows.h>
#include <iostream>
#include <algorithm>
#include <list>
#include <map>
#include <math.h>
#include <set>
#include <stdlib.h>
#include <string>
#include <vector>
#include <winsock2.h>
#include <Mmsystem.h>
#include <gl\GL.h>
#include <gl\GLU.h>
#include <time.h>
#include "Console.h"
#include "Defines.h"
#include "Enums.h"
#include "Structs.h"
#include "MathLib.h"
#include "Offset.h"
#include "Util.h"
#include "OpenglUtil.h"

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Winmm.lib")
#pragma comment(lib,"Opengl32.lib")
#pragma comment(lib,"glu32.lib")
