#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <wrl/client.h>
#include <KnownFolders.h>
#include <shlobj.h>

#include <malloc.h>
#include <tchar.h>
#include <cassert>
#include <cstdlib>

#include <d3d12.h>
#include <d3dx12.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
using namespace DirectX;

#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <vector>
#include <mutex>
#include <limits>
#include <algorithm>

#include "Time.h"
#include "Misc.h"