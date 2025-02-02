#include "stdafx.h"
#include "Enums.h"
#include "DXDevice.h"
#include "DXSwapChain.h"
#include <array>
#include "DXWindow.h"

using namespace Nova;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

static const LPCWSTR WINDOW_NAME = L"Nova";
static DXWindow* windowInstance = nullptr;
static bool IsMinimized = false;

static RECT m_WindowRect;
static LONG borderedStyle = 0;
static LONG borderlessStyle = 0;
static UINT windowStyle = 0;

#if _DEBUG
static constexpr bool ENABLE_CPU_VALIDATION_DEFAULT = true;
static constexpr bool ENABLE_GPU_VALIDATION_DEFAULT = true;
#else
static constexpr bool ENABLE_CPU_VALIDATION_DEFAULT = false;
static constexpr bool ENABLE_GPU_VALIDATION_DEFAULT = false;
#endif

int RunFramework(
	HINSTANCE hInstance,
	LPSTR lpCmdLine,
	int nCmdShow,
	DXWindow* window
) {
	HWND hWnd;
	WNDCLASSEX windowClass;
	uint32_t width = 1920;
	uint32_t height = 1080;

	ZeroMemory(&windowClass, sizeof(WNDCLASSEX));
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.hInstance = hInstance;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = WINDOW_NAME;
	windowClass.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	RegisterClassEx(&windowClass);

	assert(window);
	if (!window) {
		return 1;
	}

	windowInstance = window;
	windowStyle = WS_OVERLAPPED;
	RECT windowRect = { 0, 0, (LONG)width, (LONG)height };

	window->ParseCommandLine(lpCmdLine, &width, &height);
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	AdjustWindowRect(&windowRect, windowStyle, FALSE);

	hWnd = CreateWindowEx(
		NULL,
		WINDOW_NAME,
		(LPCWSTR)window->GetName(),
		windowStyle,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	window->DeviceInit(hWnd);
	window->Create();

	ShowWindow(hWnd, nCmdShow);
	borderedStyle = GetWindowLong(hWnd, GWL_STYLE);
	borderlessStyle = borderedStyle & ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);

	MSG msg = { 0 };

	while (msg.message != WM_QUIT) {
		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else if (!IsMinimized) {
			window->Render();
		}
	}

	window->Destroy();
	window->DeviceShut();

	window = nullptr;
	delete window;

	return static_cast<char>(msg.wParam);
}

void SetFullscreen(HWND hWnd, bool fullscreen) {
	if (fullscreen) {
		GetWindowRect(hWnd, &m_WindowRect);
		SetWindowLong(hWnd, GWL_STYLE, windowStyle & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME));

		MONITORINFO monitorInfo;
		monitorInfo.cbSize = sizeof(monitorInfo);
		GetMonitorInfo(MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST), &monitorInfo);

		SetWindowPos(
			hWnd,
			HWND_NOTOPMOST,
			monitorInfo.rcMonitor.left,
			monitorInfo.rcMonitor.top,
			monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
			monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
			SWP_FRAMECHANGED | SWP_NOACTIVATE
		);
		ShowWindow(hWnd, SW_MAXIMIZE);
	}
	else {
		SetWindowLong(hWnd, GWL_STYLE, windowStyle);

		SetWindowPos(
			hWnd,
			HWND_NOTOPMOST,
			m_WindowRect.left,
			m_WindowRect.top,
			m_WindowRect.right - m_WindowRect.left,
			m_WindowRect.bottom - m_WindowRect.top,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);
		ShowWindow(hWnd, SW_NORMAL);
	}
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}

		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}

		case WM_KEYDOWN:
		{
			if (wParam == VK_ESCAPE) {
				PostQuitMessage(0);
			}

			break;
		}

		case WM_SYSKEYDOWN:
		{
			const bool bAltKeyDown = (lParam & (1 << 29));
		
			if ((wParam == VK_RETURN) && bAltKeyDown)
				windowInstance->ToggleFullScreen();
		
			break;
		}

		case WM_SIZE:
		{
			if (windowInstance) {
				RECT clientRect = {};
				GetClientRect(hWnd, &clientRect);
				windowInstance->Resize(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
				IsMinimized = (IsIconic(hWnd) == TRUE);
				
				return 0;
			}
			break;
		}

		/*	When window goes outof focus, use this event to fall back on SDR.
			If we don't gracefully fallback to SDR, the renderer will output HDR colours which will look extremely bright and washed out.
			However if you want to use breakpoints in HDR mode to inspect/debug values, you will have to comment this function call.
		*/
		case WM_ACTIVATE:
		{
			if (windowInstance) {
				windowInstance->Activate(wParam != WA_INACTIVE);
			}

			break;
		}

		case WM_MOVE:
		{
			if (windowInstance) {
				windowInstance->WindowMove();

				return 0;
			}
			break;
		}

		// Turn off MessageBeep sound on Alt+Enter
		case WM_MENUCHAR: return MNC_CLOSE << 16;
	}

	if (windowInstance) {
		MSG msg;
		msg.hwnd = hWnd;
		msg.message = message;
		msg.wParam = wParam;
		msg.lParam = lParam;
		
		windowInstance->Event(msg);
	}

	// Handle any messages the switch statement didn't
	return DefWindowProc(hWnd, message, wParam, lParam);
}

static std::string GetCPUNameString() {
	int nIds = 0;
	int nExIds = 0;
	char strCPUName[64] = {};

	std::array<int, 4> cpuInfo;
	std::vector<std::array<int, 4>> extData;

	__cpuid(cpuInfo.data(), 0);

	/*	Calling __cpuid with 0x80000000 as the function_id argument
		gets the number of the highest valid extended ID.
	*/ 
	__cpuid(cpuInfo.data(), 0x80000000);

	nExIds = cpuInfo[0];

	for (int idx = 0x80000000; idx <= nExIds; idx++) {
		__cpuidex(cpuInfo.data(), idx, 0);
		extData.push_back(cpuInfo);
	}

	// Interpret CPU strCPUName string if reported
	if (nExIds >= 0x80000004) {
		memcpy(strCPUName, extData[2].data(), sizeof(cpuInfo));
		memcpy(strCPUName + 16, extData[3].data(), sizeof(cpuInfo));
		memcpy(strCPUName + 32, extData[4].data(), sizeof(cpuInfo));
	}

	return strlen(strCPUName) != 0 ? strCPUName : "UNAVAILABLE";
}

namespace Nova {
	DXWindow::DXWindow(LPCSTR windowName) : m_Name(windowName), m_Width(0), m_Height(0),
		m_LastFrameTime(MillisecondsNow()), m_DeltaTime(0.0),
		m_WindowHandle(NULL), m_Device(), m_StablePowerState(false),
		m_IsCPUValidationLayerEnabled(ENABLE_CPU_VALIDATION_DEFAULT),
		m_IsGPUValidationLayerEnabled(ENABLE_GPU_VALIDATION_DEFAULT),
		m_SwapChain(), m_VsyncEnabled(false), m_Monitor(), m_DisableLocalDimming(false),
		m_SystemInfo(), m_FullscreenMode(PRESENTATIONMODE_WINDOWED), m_PreviousFullscreenMode(PRESENTATIONMODE_WINDOWED) {}

	void DXWindow::DeviceInit(HWND windowHandle) {
		const char* STR_NOVA = "Nova 0.0.0.0";

		m_WindowHandle = windowHandle;
		m_Device.Create(
			m_Name,
			STR_NOVA,
			m_IsCPUValidationLayerEnabled,
			m_IsGPUValidationLayerEnabled,
			m_WindowHandle
		);

		if (m_StablePowerState) {
			HRESULT hr = m_Device.GetDevice()->SetStablePowerState(TRUE);

			if (FAILED(hr)) {
				HRESULT reason = m_Device.GetDevice()->GetDeviceRemovedReason();
				
				wchar_t buffer[512];
				swprintf_s(buffer, 512, L"Warning: ID3D12Device::SetStablePowerState(TRUE) failed: Reason 0x%x (DXGI ERROR). Recreating device, setting m_StablePowerState = false.", reason);
				OutputDebugString(buffer);

				m_Device.Destroy();
				m_Device.Create(m_Name, STR_NOVA, m_IsGPUValidationLayerEnabled, m_IsGPUValidationLayerEnabled, m_WindowHandle);
				m_StablePowerState = false;
			}
		}

		m_Monitor = MonitorFromWindow(m_WindowHandle, MONITOR_DEFAULTTONEAREST);

		uint32_t dwNumberOfBackBuffers = 2;
		m_SwapChain.Create(m_Device, dwNumberOfBackBuffers, m_WindowHandle);
		
		if (m_PreviousFullscreenMode != m_FullscreenMode) {
			HandleFullScreen();
			m_PreviousFullscreenMode = m_FullscreenMode;
		}

		std::string dummyStr;
		m_Device.GetDeviceInfo(&m_SystemInfo.m_GPUName, &dummyStr);
		m_SystemInfo.m_CPUName = GetCPUNameString();
		m_SystemInfo.m_GFXAPI = "DirectX 12";
	}

	void DXWindow::DeviceShut() {
		if (m_FullscreenMode == PRESENTATIONMODE_EXCLUSIVE_FULLSCREEN) {
			m_SwapChain.SetFullScreen(false);
		}

		m_SwapChain.DestroyWindowSizeDependentResource();
		m_SwapChain.Destroy();
		m_Device.Destroy();
	}

	void DXWindow::ToggleFullScreen() {
		if (m_FullscreenMode == PRESENTATIONMODE_WINDOWED) {
			m_FullscreenMode = PRESENTATIONMODE_BORDERLESS_FULLSCREEN;
		}
		else {
			m_FullscreenMode = PRESENTATIONMODE_WINDOWED;
		}

		HandleFullScreen();
		m_PreviousFullscreenMode = m_FullscreenMode;
	}

	void DXWindow::HandleFullScreen() {
		m_Device.GPUFlush();
		bool resizeResources = false;

		switch (m_FullscreenMode) {
		case PRESENTATIONMODE_WINDOWED:
			if (m_PreviousFullscreenMode == PRESENTATIONMODE_EXCLUSIVE_FULLSCREEN) {
				m_SwapChain.SetFullScreen(false);
				resizeResources = true;
			}

			SetFullscreen(m_WindowHandle, false);
			break;

		case PRESENTATIONMODE_BORDERLESS_FULLSCREEN:
			if (m_PreviousFullscreenMode == PRESENTATIONMODE_WINDOWED) {
				SetFullscreen(m_WindowHandle, true);
			}
			else if (m_PreviousFullscreenMode == PRESENTATIONMODE_EXCLUSIVE_FULLSCREEN) {
				m_SwapChain.SetFullScreen(false);
				resizeResources = true;
			}

			break;

		case PRESENTATIONMODE_EXCLUSIVE_FULLSCREEN:
			if (m_PreviousFullscreenMode == PRESENTATIONMODE_WINDOWED) {
				SetFullscreen(m_WindowHandle, true);
			}

			m_SwapChain.SetFullScreen(true);
			resizeResources = true;

			break;
		}

		RECT clientRect = {};
		GetClientRect(m_WindowHandle, &clientRect);
		
		uint32_t nW = clientRect.right - clientRect.left;
		uint32_t nH = clientRect.bottom - clientRect.top;
		Resize(nW, nH);
		
		resizeResources = (resizeResources && nW == m_Width && nH == m_Height);
		if (resizeResources) {
			UpdateDisplay(m_DisableLocalDimming);
			Resize(true);
		}
	}

	void DXWindow::Activate(bool windowActive) {
		if (
			windowActive &&
			m_FullscreenMode == PRESENTATIONMODE_BORDERLESS_FULLSCREEN &&
			m_PreviousFullscreenMode == PRESENTATIONMODE_EXCLUSIVE_FULLSCREEN
		) {
			m_PreviousFullscreenMode = PRESENTATIONMODE_BORDERLESS_FULLSCREEN;
			HandleFullScreen();
			m_PreviousFullscreenMode = m_FullscreenMode;
		}
	}

	void DXWindow::WindowMove() {
		HMONITOR currentMonitor = MonitorFromWindow(m_WindowHandle, MONITOR_DEFAULTTONEAREST);

		if (m_Monitor != currentMonitor) {
			m_Monitor = currentMonitor;
			UpdateDisplay(m_DisableLocalDimming);
			Resize(true);
		}
	}

	void DXWindow::Resize(uint32_t width, uint32_t height) {
		bool forReal = (m_Width != width || m_Height != height);
		m_Width = width;
		m_Height = height;

		if (forReal) {
			UpdateDisplay(m_DisableLocalDimming);
			Resize(true);
		}
	}

	void DXWindow::UpdateDisplay(bool disableLocalDimming) {
		m_Device.GPUFlush();
		m_SwapChain.DestroyWindowSizeDependentResource();
		m_DisableLocalDimming = disableLocalDimming;
		m_SwapChain.CreateWindowSizeDependentResource(m_Width, m_Height, m_VsyncEnabled, m_DisableLocalDimming);
		
		UpdateDisplay();
	}

	void DXWindow::BeginFrame() {
		double timeNow = MillisecondsNow();
		m_DeltaTime = (float)(timeNow - m_LastFrameTime);
		m_LastFrameTime = timeNow;
	}

	void DXWindow::EndFrame() {
		m_SwapChain.PresentSwapChain();

		if (m_IsGPUValidationLayerEnabled) {
			m_Device.GPUFlush();
		}

		if (m_FullscreenMode == PRESENTATIONMODE_EXCLUSIVE_FULLSCREEN) {
			bool isFullscreen = m_SwapChain.GetFullScreen();

			if (!isFullscreen) {
				m_FullscreenMode = PRESENTATIONMODE_BORDERLESS_FULLSCREEN;
				HandleFullScreen();
			}
		}
	}
}