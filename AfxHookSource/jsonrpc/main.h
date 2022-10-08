#pragma once

#include <string>
#include <optional>
#include <variant>

namespace advancedfx {
	namespace afxhooksource {
		namespace json {
			// https://developer.mozilla.org/en-US/docs/Web/CSS/cursor
			enum class cursor {
				cursor_auto,
				cursor_default,
				none,
				context_menu,
				help,
				pointer,
				progress,
				wait,
				cell,
				crosshair,
				text,
				vertical_text,
				alias,
				copy,
				move,
				no_drop,
				not_allowed,
				grab,
				grabbing,
				all_scroll,
				col_resize,
				row_resize,
				n_resize,
				e_resize,
				s_resize,
				w_resize,
				ne_resize,
				nw_resize,
				se_resize,
				sw_resize,
				ew_resize,
				ns_resize,
				nesw_resize,
				nwse_resize,
				zoom_in,
				zoom_out
			};

			class Modifiers {
			public:
				Modifiers() : shift(), control(), alt(), meta() {}
				bool shift;
				bool control;
				bool alt;
				bool meta;
			};

			class InputEvent {
			public:
				std::string type;
				Modifiers modifiers;
			};

			class MouseInputEvent : public InputEvent {
			public:
				MouseInputEvent() :  x(), y() {}
				int x;
				int y;
				std::optional<std::string> button;
				std::optional<int> globalX;
				std::optional<int> globalY;
				std::optional<int> movementX;
				std::optional<int> movementY;
				std::optional<int> clickCount;
			};

			class MouseWheelInputEvent : public MouseInputEvent {
			public:
				MouseWheelInputEvent() {}
				std::optional<int> deltaX;
				std::optional<int> deltaY;
				std::optional<int> wheelTicksX;
				std::optional<int> wheelTicksY;
				std::optional<int> accelerationRatioX;
				std::optional<int> accelerationRatioY;
				std::optional<bool> hasPreciseScrollingDeltas;
				std::optional<bool> canScroll;
			};

			class KeyboardInputEvent : public InputEvent {
			public:
				KeyboardInputEvent() {}
				std::string keyCode;
			};
		}
	}
}

bool InitJsonRpc(int hReadPipe, int hWritePipe);
void ShutdownJsonRpc();

void JsonDrawingWindowCreated(int luidLo, int luidHi, int width, int height);
void JsonDrawingWindowDestroyed();

bool JsonSendMouseInputEvent(const advancedfx::afxhooksource::json::MouseInputEvent& ev);
bool JsonSendMouseWheelInputEvent(const advancedfx::afxhooksource::json::MouseWheelInputEvent& ev);
bool JsonSendKeyboardInputEvent(const advancedfx::afxhooksource::json::KeyboardInputEvent& ev);
