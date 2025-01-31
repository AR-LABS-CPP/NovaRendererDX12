#pragma once

void ShowErrorMessageBox(LPCWSTR lpErrorString);
void ShowCustomErrorMessageBox(LPCWSTR lpErrorString);

inline void ThrowIfFailed(HRESULT hr) {
	if (FAILED(hr)) {
		wchar_t err[256];
		char errA[256];
		size_t returnSize;
		
		memset(err, 0, 256);
		FormatMessageW(
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			hr,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			err,
			255,
			NULL
		);
		wcstombs_s(&returnSize, errA, 255, err, 255);
		
		OutputDebugStringA(errA);
#ifdef _DEBUG
		ShowErrorMessageBox(err);
#endif
		throw 1;
	}
}