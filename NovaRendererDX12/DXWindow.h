#pragma once

namespace Nova {
	class DXWindow {
	public:
		DXWindow(LPCSTR windowName);
		virtual ~DXWindow() {}

		virtual void ParseCommandLine(LPSTR lpCmdLine, uint32_t* width, uint32_t* height) = 0;
		virtual void Create() = 0;
		virtual void Destroy() = 0;
		virtual void Render() = 0;
		virtual void Event(MSG msg) = 0;
		virtual void Resize(bool resizeRender) = 0;
		virtual void UpdateDisplay() = 0;

		void DeviceInit(HWND windowHandle);
		void DeviceShut();
		void BeginFrame();
		void EndFrame();

		void ToggleFullScreen();
		void HandleFullScreen();
		void Activate(bool windowActive);
		void WindowMove();
		void UpdateDisplay(bool disableLocalDimming);
		void Resize(uint32_t width, uint32_t height);

		inline LPCSTR GetName() const {
			return m_Name;
		}
	private:
		LPCSTR m_Name;
		int m_Width;
		int m_Height;

		double m_LastFrameTime;
		double m_DeltaTime;

		HWND m_WindowHandle;
		DXDevice m_Device;
		DXSwapChain m_SwapChain;
		HMONITOR m_Monitor;

		bool m_StablePowerState;
		bool m_IsCPUValidationLayerEnabled;
		bool m_IsGPUValidationLayerEnabled;
		bool m_VsyncEnabled;
		bool m_DisableLocalDimming;

		PresentationMode m_FullscreenMode;
		PresentationMode m_PreviousFullscreenMode;

		struct SystemInfo {
			std::string m_CPUName = "N/A";
			std::string m_GPUName = "N/A";
			std::string m_GFXAPI = "N/A";
		};

		SystemInfo m_SystemInfo;
	};
}

int RunFramework(
	HINSTANCE hInstance,
	LPSTR lpCmdLine,
	int nCmdShow,
	Nova::DXWindow* window
);
void SetFullscreen(HWND hWnd, bool fullscreen);