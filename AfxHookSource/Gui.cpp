#include "stdafx.h"

// TODO:
// - Lots.
// - Maybe handle RawInput mouse buttons in drag mode (so the game doesn't see them, but currently it doesn't care about them anyway.).


#include "Gui.h"

#ifndef AFX_ENABLE_GUI
#define AFX_ENABLE_GUI 1
#endif

#if AFX_ENABLE_GUI

#include <SourceInterfaces.h>
#include "WrpVEngineClient.h"
#include "addresses.h"
#include "hlaeFolder.h"

#include <shared/StringTools.h>

#include <list>
#include <string>
#include <vector>
#include <mutex>

#include <Windows.h>
#include <Windowsx.h>

#include "d3d9Hooks.h"

//#include "Model/Schedule.h"

extern SourceSdkVer g_SourceSdkVer;
extern WrpVEngineClient * g_VEngineClient;

namespace AfxHookSource {
namespace Gui {

bool m_Active = true;
bool m_KeyDownEaten[512] = {};
bool m_PassThrough = false;
POINT m_GameCursorPos = { 0, 0 };
POINT m_OldCursorPos = { 0, 0 };
HCURSOR m_OwnCursor = 0;
HCURSOR m_GameCursor = 0;
bool m_CursorSet = false;
std::string m_IniFilename;
std::string m_LogFileName;
bool m_FirstEndScene = true;
bool m_GameCaptured = false;
bool m_InMouseLook = false;
bool m_HadSetCursorMouseLook = false;
bool m_MouseButtonDown[3]={ false,false,false };
bool m_WantMouseCapture = false;

int m_Width = 0;
int m_Height = 0;
std::mutex m_SharedTextureMutex;
IDirect3DTexture9* m_SharedTexture = nullptr;
HANDLE m_SharedTextureHandle = INVALID_HANDLE_VALUE;

// Data
static HWND                     g_hWnd = 0;
static INT64                    g_Time = 0;
static INT64                    g_TicksPerSecond = 0;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;

static class Cursors_s
{
private:
	HCURSOR m_None = 0;
	HCURSOR m_AppStarting = LoadCursor(NULL, IDC_APPSTARTING);
	HCURSOR m_Arrow = LoadCursor(NULL, IDC_ARROW);
	HCURSOR m_Cross = LoadCursor(NULL, IDC_CROSS);
	HCURSOR m_Hand = LoadCursor(NULL, IDC_HAND);
	HCURSOR m_Help = LoadCursor(NULL, IDC_HELP);
	HCURSOR m_IBeam = LoadCursor(NULL, IDC_IBEAM);
	HCURSOR m_No = LoadCursor(NULL, IDC_NO);
	HCURSOR m_SizeAll = LoadCursor(NULL, IDC_SIZEALL);
	HCURSOR m_UpArrow = LoadCursor(NULL, IDC_UPARROW);
	HCURSOR m_SizeNESW = LoadCursor(NULL, IDC_SIZENESW);
	HCURSOR m_SizeNS = LoadCursor(NULL, IDC_SIZENS);
	HCURSOR m_SizeNWSE = LoadCursor(NULL, IDC_SIZENWSE);
	HCURSOR m_SizeWE = LoadCursor(NULL, IDC_SIZEWE);
	HCURSOR m_Wait = LoadCursor(NULL, IDC_WAIT);

public:
	bool GetCursor(advancedfx::afxhooksource::json::cursor cursor, HCURSOR & result)
	{
		switch (cursor)
		{
		case advancedfx::afxhooksource::json::cursor::cursor_auto:
			result = m_Arrow;
			return false;
		case advancedfx::afxhooksource::json::cursor::cursor_default:
			result = m_Arrow;
			break;
		case advancedfx::afxhooksource::json::cursor::none:
			result = m_None;
			break;
		case advancedfx::afxhooksource::json::cursor::context_menu:
			result = m_Arrow;
			break;
		case advancedfx::afxhooksource::json::cursor::help:
			result = m_Help;
			break;
		case advancedfx::afxhooksource::json::cursor::pointer:
			result = m_Hand;
			break;
		case advancedfx::afxhooksource::json::cursor::progress:
			result = m_AppStarting;
			break;
		case advancedfx::afxhooksource::json::cursor::wait:
			result = m_Wait;
			break;
		case advancedfx::afxhooksource::json::cursor::cell:
			result = m_Cross;
			break;
		case advancedfx::afxhooksource::json::cursor::crosshair:
			result = m_Cross;
			break;
		case advancedfx::afxhooksource::json::cursor::text:
			result = m_IBeam;
			break;
		case advancedfx::afxhooksource::json::cursor::vertical_text:
			result = m_IBeam;
			break;
		case advancedfx::afxhooksource::json::cursor::alias:
			result = m_Arrow;
			break;
		case advancedfx::afxhooksource::json::cursor::copy:
			result = m_Arrow;
			break;
		case advancedfx::afxhooksource::json::cursor::move:
			result = m_SizeAll;
			break;
		case advancedfx::afxhooksource::json::cursor::no_drop:
			result = m_Arrow;
			break;
		case advancedfx::afxhooksource::json::cursor::not_allowed:
			result = m_No;
			break;
		case advancedfx::afxhooksource::json::cursor::grab:
			result = m_Arrow;
			break;
		case advancedfx::afxhooksource::json::cursor::grabbing:
			result = m_Arrow;
			break;
		case advancedfx::afxhooksource::json::cursor::all_scroll:
			result = m_SizeAll;
			break;
		case advancedfx::afxhooksource::json::cursor::col_resize:
			result = m_SizeWE;
			break;
		case advancedfx::afxhooksource::json::cursor::row_resize:
			result = m_SizeNS;
			break;
		case advancedfx::afxhooksource::json::cursor::n_resize:
			result = m_SizeNS;
			break;
		case advancedfx::afxhooksource::json::cursor::e_resize:
			result = m_SizeWE;
			break;
		case advancedfx::afxhooksource::json::cursor::s_resize:
			result = m_SizeNS;
			break;
		case advancedfx::afxhooksource::json::cursor::w_resize:
			result = m_SizeWE;
			break;

		case advancedfx::afxhooksource::json::cursor::ne_resize:
			result = m_SizeNESW;
			break;
		case advancedfx::afxhooksource::json::cursor::nw_resize:
			result = m_SizeNWSE;
			break;
		case advancedfx::afxhooksource::json::cursor::se_resize:
			result = m_SizeNWSE;
			break;
		case advancedfx::afxhooksource::json::cursor::sw_resize:
			result = m_SizeNESW;
			break;

		case advancedfx::afxhooksource::json::cursor::ew_resize:
			result = m_SizeWE;
			break;
		case advancedfx::afxhooksource::json::cursor::ns_resize:
			result = m_SizeNS;
			break;
		case advancedfx::afxhooksource::json::cursor::nesw_resize:
			result = m_SizeNESW;
			break;
		case advancedfx::afxhooksource::json::cursor::nwse_resize:
			result = m_SizeNWSE;
			break;

		case advancedfx::afxhooksource::json::cursor::zoom_in:
			result = m_Arrow;
			break;
		case advancedfx::afxhooksource::json::cursor::zoom_out:
			result = m_Arrow;
			break;

		default:
			result = m_Arrow;
			break;
		}

		return true;
	}

	HCURSOR GetFromGame(HCURSOR gameCursor, bool inMouseLook)
	{
		if (!inMouseLook || gameCursor)
			return gameCursor;
		return m_Hand;
	}

} m_Cursors;

struct CUSTOMVERTEX
{
	float    pos[3];
	D3DCOLOR col;
	float    uv[2];
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

bool IsSupported()
{
	//return false;
	return SourceSdkVer_CSGO == g_SourceSdkVer;
}

bool IsInMouseLook()
{
	return m_InMouseLook;
}

static bool IsAnyMouseButtonDown()
{
	for (int n = 0; n < sizeof(m_MouseButtonDown)/sizeof(m_MouseButtonDown[0]); n++)
		if (m_MouseButtonDown[n])
			return true;
	return false;
}

enum EndPassThrougButtonUp
{
	EPTBU_Left,
	EPTBU_Right,
	EPTBU_Middle
};
EndPassThrougButtonUp m_EndPassThroughButtonUp = EPTBU_Left;
bool m_IgnoreNextWM_MOUSEMOVE = false;


std::mutex m_CursorMutex;

bool m_HasKeyBoardFocus = false;


void SetHasKeyboardFocus(bool value) {

	std::unique_lock<std::mutex> lock(m_CursorMutex);

	m_HasKeyBoardFocus = value;
}

void SetMouseCursorThreadSafe(advancedfx::afxhooksource::json::cursor cursor) {

	std::unique_lock<std::mutex> lock(m_CursorMutex);

	// Set OS mouse position if requested last frame by io.WantMoveMouse flag (used when io.NavMovesTrue is enabled by user and using directional navigation)
	//if (io.WantMoveMouse)
	//{
	//	POINT pos = { (int)io.MousePos.x, (int)io.MousePos.y };
	//	ClientToScreen(g_hWnd, &pos);
	//	SetCursorPos(pos.x, pos.y);
	//}

	HCURSOR hCursor = 0;
	m_WantMouseCapture = m_Cursors.GetCursor(cursor, hCursor);

	if (!m_WantMouseCapture)
	{
		hCursor = m_Cursors.GetFromGame(m_GameCursor, IsInMouseLook());
	}

	if (m_OwnCursor != hCursor)
	{
		m_OwnCursor = hCursor;
		SetCursor(m_OwnCursor);
	}

	m_InMouseLook = m_HadSetCursorMouseLook;
	m_HadSetCursorMouseLook = false;
}

void KeyFromWparam(advancedfx::afxhooksource::json::KeyboardInputEvent& ev, WPARAM wParam) {
	switch (wParam) {
	case 0x30:
		ev.keyCode = "0";
		break;
	case 0x31:
		ev.keyCode = "1";
		break;
	case 0x32:
		ev.keyCode = "2";
		break;
	case 0x33:
		ev.keyCode = "3";
		break;
	case 0x34:
		ev.keyCode = "4";
		break;
	case 0x35:
		ev.keyCode = "5";
		break;
	case 0x36:
		ev.keyCode = "6";
		break;
	case 0x37:
		ev.keyCode = "7";
		break;
	case 0x38:
		ev.keyCode = "8";
		break;
	case 0x39:
		ev.keyCode = "9";
		break;
	case 0x41:
		ev.keyCode = "A";
		break;
	case 0x42:
		ev.keyCode = "B";
		break;
	case 0x43:
		ev.keyCode = "C";
		break;
	case 0x44:
		ev.keyCode = "D";
		break;
	case 0x45:
		ev.keyCode = "E";
		break;
	case 0x46:
		ev.keyCode = "F";
		break;
	case 0x47:
		ev.keyCode = "G";
		break;
	case 0x48:
		ev.keyCode = "H";
		break;
	case 0x49:
		ev.keyCode = "I";
		break;
	case 0x4a:
		ev.keyCode = "J";
		break;
	case 0x4b:
		ev.keyCode = "K";
		break;
	case 0x4c:
		ev.keyCode = "L";
		break;
	case 0x4d:
		ev.keyCode = "M";
		break;
	case 0x4e:
		ev.keyCode = "N";
		break;
	case 0x4f:
		ev.keyCode = "O";
		break;
	case 0x50:
		ev.keyCode = "P";
		break;
	case 0x51:
		ev.keyCode = "Q";
		break;
	case 0x52:
		ev.keyCode = "R";
		break;
	case 0x53:
		ev.keyCode = "S";
		break;
	case 0x54:
		ev.keyCode = "T";
		break;
	case 0x55:
		ev.keyCode = "U";
		break;
	case 0x56:
		ev.keyCode = "V";
		break;
	case 0x57:
		ev.keyCode = "W";
		break;
	case 0x58:
		ev.keyCode = "X";
		break;
	case 0x59:
		ev.keyCode = "Y";
		break;
	case 0x5a:
		ev.keyCode = "Z";
		break;
	case VK_F1:
		ev.keyCode = "F1";
		break;
	case VK_F2:
		ev.keyCode = "F2";
		break;
	case VK_F3:
		ev.keyCode = "F3";
		break;
	case VK_F4:
		ev.keyCode = "F4";
		break;
	case VK_F5:
		ev.keyCode = "F5";
		break;
	case VK_F6:
		ev.keyCode = "F6";
		break;
	case VK_F7:
		ev.keyCode = "F7";
		break;
	case VK_F8:
		ev.keyCode = "F8";
		break;
	case VK_F9:
		ev.keyCode = "F9";
		break;
	case VK_F10:
		ev.keyCode = "F10";
		break;
	case VK_F11:
		ev.keyCode = "F11";
		break;
	case VK_F12:
		ev.keyCode = "F12";
		break;
	case VK_F13:
		ev.keyCode = "F13";
		break;
	case VK_F14:
		ev.keyCode = "F14";
		break;
	case VK_F15:
		ev.keyCode = "F15";
		break;
	case VK_F16:
		ev.keyCode = "F16";
		break;
	case VK_F17:
		ev.keyCode = "F17";
		break;
	case VK_F18:
		ev.keyCode = "F18";
		break;
	case VK_F19:
		ev.keyCode = "F19";
		break;
	case VK_F20:
		ev.keyCode = "F20";
		break;
	case VK_F21:
		ev.keyCode = "F21";
		break;
	case VK_F22:
		ev.keyCode = "F22";
		break;
	case VK_F23:
		ev.keyCode = "F23";
		break;
	case VK_F24:
		ev.keyCode = "F24";
		break;
	case VK_OEM_PLUS:
		ev.keyCode = "Plus";
		break;
	case VK_OEM_COMMA:
		ev.keyCode = ",";
		break;
	case VK_OEM_MINUS:
		ev.keyCode = "Minus";
		break;
	case VK_OEM_PERIOD:
		ev.keyCode = ".";
		break;
	case VK_SPACE:
		ev.keyCode = "Space";
		break;
	case VK_TAB:
		ev.keyCode = "Tab";
		break;
	case VK_CAPITAL:
		ev.keyCode = "Capslock";
		break;
	case VK_NUMLOCK:
		ev.keyCode = "Numlock";
		break;
	case VK_SCROLL:
		ev.keyCode = "Scrolllock";
		break;
	case VK_BACK:
		ev.keyCode = "Backspace";
		break;
	case VK_DELETE:
		ev.keyCode = "Delete";
		break;
	case VK_INSERT:
		ev.keyCode = "Insert";
		break;
	case VK_RETURN:
		ev.keyCode = "Return";
		break;
	case VK_UP:
		ev.keyCode = "Up";
		break;
	case VK_LEFT:
		ev.keyCode = "Down";
		break;
	case VK_DOWN:
		ev.keyCode = "Left";
		break;
	case VK_RIGHT:
		ev.keyCode = "Right";
		break;
	case VK_HOME:
		ev.keyCode = "Home";
		break;
	case VK_END:
		ev.keyCode = "End";
		break;
	case VK_PRIOR:
		ev.keyCode = "PageUp";
		break;
	case VK_NEXT:
		ev.keyCode = "PageDown";
		break;
	case VK_ESCAPE:
		ev.keyCode = "Escape";
		break;
	case VK_VOLUME_UP:
		ev.keyCode = "VolumeUp";
		break;
	case VK_VOLUME_DOWN:
		ev.keyCode = "VolumeDown";
		break;
	case VK_VOLUME_MUTE:
		ev.keyCode = "VolumeMute";
		break;
	case VK_MEDIA_NEXT_TRACK:
		ev.keyCode = "MediaNextTrack";
		break;
	case VK_MEDIA_PREV_TRACK:
		ev.keyCode = "MediaPreviousTrack";
		break;
	case VK_MEDIA_STOP:
		ev.keyCode = "MediaStop";
		break;
	case VK_MEDIA_PLAY_PAUSE:
		ev.keyCode = "MediaPlayPause";
		break;
	case VK_PRINT:
		ev.keyCode = "PrintScreen";
		break;
	case VK_NUMPAD0:
		ev.keyCode = "num0";
		break;
	case VK_NUMPAD1:
		ev.keyCode = "num1";
		break;
	case VK_NUMPAD2:
		ev.keyCode = "num2";
		break;
	case VK_NUMPAD3:
		ev.keyCode = "num3";
		break;
	case VK_NUMPAD4:
		ev.keyCode = "num4";
		break;
	case VK_NUMPAD5:
		ev.keyCode = "num5";
		break;
	case VK_NUMPAD6:
		ev.keyCode = "num6";
		break;
	case VK_NUMPAD7:
		ev.keyCode = "num7";
		break;
	case VK_NUMPAD8:
		ev.keyCode = "num8";
		break;
	case VK_NUMPAD9:
		ev.keyCode = "num9";
		break;
	case VK_DECIMAL:
		ev.keyCode = "numdec";
		break;
	case VK_ADD:
		ev.keyCode = "numadd";
		break;
	case VK_SUBTRACT:
		ev.keyCode = "numsub";
		break;
	case VK_MULTIPLY:
		ev.keyCode = "nummult";
		break;
	case VK_DIVIDE:
		ev.keyCode = "numdiv";
		break;
	}

	if ((GetKeyState(VK_LWIN) & 0x8000) != 0 || (GetKeyState(VK_RWIN) & 0x8000) != 0) ev.keyCode = "Super+" + ev.keyCode;
	if ((GetKeyState(VK_SHIFT) & 0x8000) != 0) ev.keyCode = "Shift+" + ev.keyCode;
	if ((GetKeyState(VK_MENU) & 0x8000) != 0) ev.keyCode = "Alt+" + ev.keyCode;
	if ((GetKeyState(VK_CONTROL) & 0x8000) != 0) ev.keyCode = "CommandOrControl+" + ev.keyCode;
}

void DllProcessAttach(void)
{
}

void DllProcessDetach(void)
{
}

bool WndProcHandler(HWND hwnd, UINT msg, WPARAM & wParam, LPARAM & lParam)
{
	if (!IsSupported())
		return false;

	if (!m_Active)
		return false;

	if (m_PassThrough)
	{
		bool end = false;

		switch (msg)
		{
		case WM_LBUTTONUP:
			if (m_EndPassThroughButtonUp == EPTBU_Left) end = true;
			break;
		case WM_RBUTTONUP:
			if (m_EndPassThroughButtonUp == EPTBU_Right) end = true;
			break;
		case WM_MBUTTONUP:
			if (m_EndPassThroughButtonUp == EPTBU_Middle) end = true;
			break;
		case WM_MOUSEMOVE:
			switch (m_EndPassThroughButtonUp)
			{
			case EPTBU_Left:
				wParam &= ~(WPARAM)MK_LBUTTON;
				break;
			case EPTBU_Right:
				wParam &= ~(WPARAM)MK_RBUTTON;
				break;
			case EPTBU_Middle:
				wParam &= ~(WPARAM)MK_MBUTTON;
				break;
			}
			break;
		}

		if (end)
		{
			std::unique_lock<std::mutex> lock(m_CursorMutex);

			GetCursorPos(&m_GameCursorPos);

			m_PassThrough = false;

			SetCursor(m_OwnCursor);
			SetCursorPos(m_OldCursorPos.x, m_OldCursorPos.y);

			if (m_GameCaptured && GetCapture() == hwnd)
				ReleaseCapture();

			return true;
		}

		return false;
	}

	switch (msg)
	{
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	{
		std::unique_lock<std::mutex> lock(m_CursorMutex);
		return m_WantMouseCapture;
	}

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		{
			advancedfx::afxhooksource::json::MouseInputEvent ev;
			ev.type = "mouseDown";
			POINT pt = { GET_X_LPARAM(lParam),  GET_Y_LPARAM(lParam) };
			ev.x = pt.x;
			ev.y = pt.y;
			if (g_hWnd) {
				POINT globalPt = pt;
				ClientToScreen(g_hWnd, &globalPt);
				ev.globalX = globalPt.x;
				ev.globalY = globalPt.y;
			}
			switch (msg) {
			case WM_LBUTTONDOWN:
				ev.button = "left";
				break;
			case WM_MBUTTONDOWN:
				ev.button = "middle";
				break;
			case WM_RBUTTONDOWN:
				ev.button = "right";
				break;
			}
			bool handled = JsonSendMouseInputEvent(ev);

			std::unique_lock<std::mutex> lock(m_CursorMutex);

			if (!handled && IsInMouseLook())
			{
				bool start = false;

				switch (msg)
				{
				case WM_LBUTTONDOWN:
					start = true;
					m_EndPassThroughButtonUp = EPTBU_Left;
					break;
				case WM_RBUTTONDOWN:
					start = true;
					m_EndPassThroughButtonUp = EPTBU_Right;
					break;
				case WM_MBUTTONDOWN:
					start = true;
					m_EndPassThroughButtonUp = EPTBU_Middle;
					break;
				}
				
				if (start)
				{
					std::unique_lock<std::mutex> lock(m_CursorMutex);

					GetCursorPos(&m_OldCursorPos);

					m_IgnoreNextWM_MOUSEMOVE = m_OldCursorPos.x != m_GameCursorPos.x || m_OldCursorPos.y != m_GameCursorPos.y;

					SetCursor(m_GameCursor);
					SetCursorPos(m_GameCursorPos.x, m_GameCursorPos.y);

					if (m_GameCaptured && GetCapture() == NULL)
						SetCapture(hwnd);

					m_PassThrough = true;
					return true;
				}
			}
		
			int button = 0;
			if (msg == WM_LBUTTONDOWN) button = 0;
			if (msg == WM_RBUTTONDOWN) button = 1;
			if (msg == WM_MBUTTONDOWN) button = 2;
			if (!IsAnyMouseButtonDown() && GetCapture() == NULL)
				SetCapture(hwnd);
			m_MouseButtonDown[button] = true;
			return handled || IsInMouseLook();
		}
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		{
			advancedfx::afxhooksource::json::MouseInputEvent ev;
			ev.type = "mouseUp";
			POINT pt = { GET_X_LPARAM(lParam),  GET_Y_LPARAM(lParam) };
			ev.x = pt.x;
			ev.y = pt.y;
			if (g_hWnd) {
				POINT globalPt = pt;
				ClientToScreen(g_hWnd, &globalPt);
				ev.globalX = globalPt.x;
				ev.globalY = globalPt.y;
			}
			switch (msg) {
			case WM_LBUTTONDOWN:
				ev.button = "left";
				break;
			case WM_MBUTTONDOWN:
				ev.button = "middle";
				break;
			case WM_RBUTTONDOWN:
				ev.button = "right";
				break;
			}
			bool handled = JsonSendMouseInputEvent(ev);

			std::unique_lock<std::mutex> lock(m_CursorMutex);

			int button = 0;
			if (msg == WM_LBUTTONUP) button = 0;
			if (msg == WM_RBUTTONUP) button = 1;
			if (msg == WM_MBUTTONUP) button = 2;
			m_MouseButtonDown[button] = false;
			if (!IsAnyMouseButtonDown() && GetCapture() == hwnd)
				ReleaseCapture();
			return handled || IsInMouseLook();
		}
	case WM_MOUSEWHEEL:
	case WM_MOUSEHWHEEL:
		{
			advancedfx::afxhooksource::json::MouseWheelInputEvent ev;
			ev.type = "mouseWheel";
			int zDelta = GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1 : -1;
			switch (msg) {
			case WM_MOUSEWHEEL:
				ev.wheelTicksX = zDelta;
				break;
			case WM_MOUSEHWHEEL:
				ev.wheelTicksY = zDelta;
				break;
			}
			bool handled = JsonSendMouseWheelInputEvent(ev);

			std::unique_lock<std::mutex> lock(m_CursorMutex);

			return handled || IsInMouseLook();
		}
	case WM_MOUSEMOVE:
	{
		advancedfx::afxhooksource::json::MouseInputEvent ev;
		ev.type = "mouseMove";
		POINT pt = { GET_X_LPARAM(lParam),  GET_Y_LPARAM(lParam) };
		ev.x = pt.x;
		ev.y = pt.y;
		if (g_hWnd) {
			POINT globalPt = pt;
			ClientToScreen(g_hWnd, &globalPt);
			ev.globalX = globalPt.x;
			ev.globalY = globalPt.y;
		}
		bool handled = JsonSendMouseInputEvent(ev);

		std::unique_lock<std::mutex> lock(m_CursorMutex);

		return handled || IsInMouseLook();
	}
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		{
			advancedfx::afxhooksource::json::KeyboardInputEvent ev;
			ev.type = "keyDown";
			KeyFromWparam(ev, wParam);
			bool handled = JsonSendKeyboardInputEvent(ev);

			if (wParam < 256)
			{
				m_KeyDownEaten[wParam] = handled;
			}
			return handled;
		}
	case WM_KEYUP:
	case WM_SYSKEYUP:
		{
			advancedfx::afxhooksource::json::KeyboardInputEvent ev;
			ev.type = "keyUp";
			KeyFromWparam(ev, wParam);
			bool handled = JsonSendKeyboardInputEvent(ev);

			bool wasKeyDownEaten = false;
			if (wParam < 256)
			{
				wasKeyDownEaten = m_KeyDownEaten[wParam];
				m_KeyDownEaten[wParam] = false;
			}
			return handled || wasKeyDownEaten;
		}
	case WM_CHAR:
	{
		bool handled = false;
		if (wParam > 0 && wParam < 0x10000)
		{
			// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
			wchar_t wszParam[2] = { (wchar_t)wParam, '\0' };
			std::string utf8Str;
			if (WideStringToUTF8String(wszParam, utf8Str)) {
				advancedfx::afxhooksource::json::KeyboardInputEvent ev;
				ev.type = "char";
				ev.keyCode = utf8Str;
				handled = JsonSendKeyboardInputEvent(ev);
			}
		}
		return handled;	
	}
	case WM_INPUT:
		{
			HRAWINPUT hRawInput = (HRAWINPUT)lParam;
			RAWINPUT inp;
			UINT size = sizeof(inp);

			GetRawInputData(hRawInput, RID_INPUT, &inp, &size, sizeof(RAWINPUTHEADER));

			switch (inp.header.dwType)
			{
			case RIM_TYPEMOUSE:
			{
				std::unique_lock<std::mutex> lock(m_CursorMutex);

				return IsInMouseLook();
			}
			case RIM_TYPEKEYBOARD:
			{
				std::unique_lock<std::mutex> lock(m_CursorMutex);

				return m_HasKeyBoardFocus;
			}
			}
			return false;
		}
	}
	return false;
}

bool OnSetCursorPos(__in int X, __in int Y)
{
	if (!IsSupported())
		return false;

	std::unique_lock<std::mutex> lock(m_CursorMutex);

	if (g_hWnd && m_GameCaptured && g_VEngineClient)
	{
		int width, height;
		POINT clientPoint = { X , Y };

		g_VEngineClient->GetScreenSize(width, height);

		ScreenToClient(g_hWnd, &clientPoint);

		m_HadSetCursorMouseLook = ((width >> 1) == clientPoint.x) && ((height >> 1) == clientPoint.y);

		//Tier0_Msg("%i == %i, %i == %i + (%i, %i) --> %i\n", width >> 1, clientPoint.x, height >> 1, clientPoint.y, X, Y, m_InMouseLook ? 1 : 0);
	}
	else
		m_HadSetCursorMouseLook = false;

	m_GameCursorPos.x = X;
	m_GameCursorPos.y = Y;

	if (!m_Active)
		return false;

	if (m_PassThrough)
		return false;

	if (!IsInMouseLook())
		return false;

	return true;
}

bool OnGetCursorPos(__out LPPOINT lpPoint)
{
	if (!IsSupported())
		return false;

	std::unique_lock<std::mutex> lock(m_CursorMutex);

	if (!m_Active || m_PassThrough || !IsInMouseLook())
	{
		if (lpPoint)
		{
			m_GameCursorPos.x = lpPoint->x;
			m_GameCursorPos.y = lpPoint->y;
		}

		return false;
	}

	if (lpPoint)
	{
		lpPoint->x = m_GameCursorPos.x;
		lpPoint->y = m_GameCursorPos.y;
	}

	return true;
}

bool OnSetCursor(__in_opt HCURSOR hCursor, HCURSOR & result)
{
	if (!IsSupported())
		return false;

	std::unique_lock<std::mutex> lock(m_CursorMutex);

	if (!m_CursorSet)
	{
		m_GameCursor = GetCursor();
		m_CursorSet = true;
	}

	HCURSOR oldCursor = m_GameCursor;

	m_GameCursor = hCursor;

	if (!m_Active)
		return false;

	if (m_PassThrough)
		return false;

	if (!m_WantMouseCapture)
	{
		m_OwnCursor = m_Cursors.GetFromGame(hCursor, IsInMouseLook());
	}

	result = oldCursor;
	SetCursor(m_OwnCursor);

	return true;
}

bool OnSetCapture(HWND hWnd, HWND & result)
{
	if (!IsSupported())
		return false;

	std::unique_lock<std::mutex> lock(m_CursorMutex);

	m_GameCaptured = hWnd && hWnd == g_hWnd;

	if (!m_Active)
		return false;

	if (m_PassThrough)
		return false;

	if (!IsInMouseLook())
		return false;

	result = 0; // TODO: maybe some smartass logic here to determine something (not really worth it probably atm)?
	return true;
}

bool OnReleaseCapture()
{
	if (!IsSupported())
		return false;

	std::unique_lock<std::mutex> lock(m_CursorMutex);

	bool wasInMouseLook = IsInMouseLook();

	m_GameCaptured = false;
	m_InMouseLook = false;

	if (!m_Active)
		return false;

	if (m_PassThrough)
		return false;

	if (!wasInMouseLook)
		return false;

	return true;
}

void LockSharedTextueThreadSafe() {
	m_SharedTextureMutex.lock();
}

void UnlockSharedTextureThreadSafe() {
	m_SharedTextureMutex.unlock();
}



void ReleaseSharedTexture() {
	std::unique_lock<std::mutex> lock(m_SharedTextureMutex);
	if (m_SharedTexture) {
		m_SharedTexture->Release();
		m_SharedTexture = nullptr;
	}
}

void CreateSharedTexture(HANDLE handle) {
	ReleaseSharedTexture();

	std::unique_lock<std::mutex> lock(m_SharedTextureMutex);
	m_SharedTextureHandle = handle;
	if (INVALID_HANDLE_VALUE != m_SharedTextureHandle && g_pd3dDevice && 0 < m_Width && 0 < m_Height) {
		if (SUCCEEDED(g_pd3dDevice->CreateTexture(m_Width, m_Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_SharedTexture, &m_SharedTextureHandle)))
		{

		}
	}
}

void SetSharedTextureHandleThreadSafe(HANDLE handle) {
	CreateSharedTexture(handle);
}

bool On_Direct3DDevice9_Init(void* hwnd, IDirect3DDevice9* device, UINT width, UINT height)
{
	if (!IsSupported())
		return false;

	g_hWnd = (HWND)hwnd;
	g_pd3dDevice = device;
	m_Width = width;
	m_Height = height;

	//if (!QueryPerformanceFrequency((LARGE_INTEGER *)&g_TicksPerSecond))
	//	return false;
	//if (!QueryPerformanceCounter((LARGE_INTEGER *)&g_Time))
	//	return false;

	return true;
}

void On_Direct3DDevice9_Shutdown()
{
	if (!IsSupported())
		return;

	{
		ReleaseSharedTexture();
		std::unique_lock<std::mutex> lock(m_SharedTextureMutex);
		m_SharedTextureHandle = INVALID_HANDLE_VALUE;
	}

	g_pd3dDevice = NULL;
	g_hWnd = 0;
	m_Width = 0;
	m_Height = 0;
}


void On_Direct3DDevice9_EndScene()
{
	if (!IsSupported())
		return;

	if (m_FirstEndScene && m_Active)
	{
		m_FirstEndScene = false;

		std::unique_lock<std::mutex> lock(m_SharedTextureMutex);
		if (m_SharedTexture) {
			AfxDrawRect(m_SharedTexture, 0, 0, m_Width, m_Height, 0, 1, 1, 0);
		}
	}
}

void On_Direct3DDevice9_Present(bool deviceLost)
{
	if (!IsSupported())
		return;

	m_FirstEndScene = true;
}

void On_Direct3DDevice9_Reset_Before()
{
	if (!IsSupported())
		return;

	ReleaseSharedTexture();
}

void On_Direct3DDevice9_Reset_After()
{
	if (!IsSupported())
		return;

	CreateSharedTexture(m_SharedTextureHandle);
}


bool OnGameFrameRenderEnd()
{
	if (!IsSupported())
		return false;

	if (!m_Active)
		return false;

	// Setup time step
	//INT64 current_time;
	//QueryPerformanceCounter((LARGE_INTEGER *)&current_time);
	//io.DeltaTime = (float)(current_time - g_Time) / g_TicksPerSecond;
	//g_Time = current_time;

	// Setup display size (every frame to accommodate for window resizing)
	//RECT rect;
	//GetClientRect(g_hWnd, &rect);
	//io.DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));

	// ...

	return true;
}

} // namespace Gui {
} // namespace AfxHookSource {

#else

#include "Gui.h"

namespace AfxHookSource {
namespace Gui {

void DllProcessAttach(void)
{
}

void DllProcessDetach(void)
{
}

bool WndProcHandler(HWND hwnd, UINT msg, WPARAM & wParam, LPARAM & lParam)
{
	return false;
}

bool OnSetCursorPos(__in int X, __in int Y)
{
	return false;
}

bool OnGetCursorPos(__out LPPOINT lpPoint)
{
	return false;
}

bool OnSetCursor(__in_opt HCURSOR hCursor, HCURSOR & result)
{
	return false;
}

bool OnSetCapture(HWND hWnd, HWND & result)
{
	return false;
}

bool OnReleaseCapture()
{
	return false;
}

bool OnGameFrameRenderEnd()
{
	return false;
}

bool On_Direct3DDevice9_Init(void* hwnd, IDirect3DDevice9* device)
{
	return false;
}

void On_Direct3DDevice9_Shutdown()
{

}

void On_Direct3DDevice9_EndScene()
{

}

void On_Direct3DDevice9_Present(bool deviceLost)
{

}

void On_Direct3DDevice9_Reset_Before()
{

}

void On_Direct3DDevice9_Reset_After()
{

}



} // namespace Gui {
} // namespace AfxHookSource {


#endif
