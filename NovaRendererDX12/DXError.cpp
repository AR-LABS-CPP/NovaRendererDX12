#include "stdafx.h"
#include "DXError.h"

void ShowErrorMessageBox(LPCWSTR lpErrorString) {
	int msgBoxId = MessageBoxW(NULL, lpErrorString, L"Error", MB_OK);
}

void ShowCustomErrorMessageBox(LPCWSTR lpErrorString) {
	int msgBoxId = MessageBoxW(NULL, lpErrorString, L"Error", MB_OK | MB_TOPMOST);
}