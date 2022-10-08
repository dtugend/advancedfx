#pragma once

#include "jsonrpc/main.h"

#include <list>
#include <queue>

#include <d3d9.h>

namespace AfxHookSource {
namespace Gui {

//bool SetActive(bool value);

void DllProcessAttach(void);
void DllProcessDetach(void);

/// <returns>Whether the event was already handled or not.</returns>
bool WndProcHandler(HWND hwnd, UINT msg, WPARAM & wParam, LPARAM & lParam);
bool OnSetCursorPos(__in int X, __in int Y);
bool OnGetCursorPos(__out LPPOINT lpPoint);
bool OnSetCursor(__in_opt HCURSOR hCursor, HCURSOR & result);
bool OnSetCapture(HWND hWnd, HWND & result);
bool OnReleaseCapture();
bool OnGameFrameRenderEnd();

bool On_Direct3DDevice9_Init(void* hwnd, IDirect3DDevice9* device, UINT width, UINT height);
void On_Direct3DDevice9_Shutdown();

void On_Direct3DDevice9_EndScene();
void On_Direct3DDevice9_Present(bool deviceLost);
void On_Direct3DDevice9_Reset_Before();
void On_Direct3DDevice9_Reset_After();

void SetSharedTextureHandleThreadSafe(HANDLE handle);
void LockSharedTextueThreadSafe();
void UnlockSharedTextureThreadSafe();

void SetMouseCursorThreadSafe(advancedfx::afxhooksource::json::cursor cursor);

} // namespace Gui {
} // namespace AfxHookSource {

