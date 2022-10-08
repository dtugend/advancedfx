#include "main.h"

#include <jsonrpccxx/client.hpp>
#include <jsonrpccxx/server.hpp>

#include <string>
#include <vector>
#include <set>
#include <map>

#include <thread>
#include <atomic>
#include <mutex>

#include "Gui.h"

#include <Windows.h>

using namespace jsonrpccxx;
using namespace std;

namespace advancedfx {
	namespace afxhooksource {
		class CDuplexPipe {
		public:
			CDuplexPipe(HANDLE readHandle, HANDLE writeHandle)
				: m_ReadHandle(readHandle)
				, m_WriteHandle(writeHandle)
				, m_ReadBuffer(4096) {

			}

			~CDuplexPipe() {
				CloseHandle(m_WriteHandle);
				CloseHandle(m_ReadHandle);
			}

			void ReadBytes(void* pData, DWORD numBytes) {
				DWORD bytesRead = 0;
				while (bytesRead < numBytes) {
					DWORD curBytesRead;
					if (!ReadFile(m_ReadHandle, (unsigned char*)pData + bytesRead, numBytes - bytesRead, &curBytesRead, NULL)) throw "ReadFile failed";
					bytesRead += curBytesRead;
				}
			}

			void ReadString(std::string& inStr) {
				DWORD strLen;
				ReadBytes(&strLen, (DWORD)sizeof(strLen));

				while (inStr.length() < strLen)
				{
					DWORD bytesRead = 0;
					while (bytesRead < min(m_ReadBuffer.size(), strLen)) {
						DWORD curBytesRead;
						if (!ReadFile(m_ReadHandle, (unsigned char*)(&m_ReadBuffer[0]) + bytesRead, strLen - inStr.length(), &curBytesRead, NULL)) throw "ReadFile failed";
						bytesRead += curBytesRead;
					}
					inStr.append(m_ReadBuffer.begin(), m_ReadBuffer.begin() + bytesRead);
				}
			}

			void WriteBytes(void* pData, DWORD numBytes) {
				DWORD bytesWritten = 0;
				while (bytesWritten < numBytes) {
					DWORD curBytesWritten;
					if (!WriteFile(m_WriteHandle, (unsigned char*)pData + bytesWritten, numBytes - bytesWritten, &curBytesWritten, NULL)) throw "WriteFile failed";
					bytesWritten += curBytesWritten;
				}
			}

			void WriteString(const std::string& str) {
				DWORD strLen = (DWORD)str.size();
				WriteBytes(&strLen, sizeof(strLen));

				const char* pStrData = str.c_str();

				DWORD bytesWritten = 0;
				while (bytesWritten < strLen) {
					DWORD curBytesWritten;
					if (!WriteFile(m_WriteHandle, pStrData + bytesWritten, strLen - bytesWritten, &curBytesWritten, NULL)) throw "WriteFile failed";
					bytesWritten += curBytesWritten;
				}
			}

			DWORD PeekBytesAvail() {
				DWORD totalBytesAvail;
				if (!PeekNamedPipe(m_ReadHandle, NULL, 0, NULL, &totalBytesAvail, NULL)) throw "PeekNamedPipe failed";
				return totalBytesAvail;
			}

			bool PeekString(std::string& inStr) {
				if (PeekBytesAvail()) {
					ReadString(inStr);
					return true;
				}

				return false;
			}

		private:
			HANDLE m_ReadHandle;
			HANDLE m_WriteHandle;
			std::vector<char> m_ReadBuffer;
		};

		class CDuplexPipeJsonClientConnector : public jsonrpccxx::IClientConnector {
		public:
			explicit CDuplexPipeJsonClientConnector(HANDLE readHandle, HANDLE writeHandle) : m_DuplexPipe(readHandle, writeHandle) {}

			std::string Send(const std::string& request) override {

				try {
					m_DuplexPipe.WriteString(request);
				}
				catch (const std::exception& e) {
					throw jsonrpccxx::JsonRpcException(-32001, e.what());
				}

				std::string result;

				try {
					m_DuplexPipe.ReadString(result);
				}
				catch (const std::exception& e) {
					throw jsonrpccxx::JsonRpcException(-32002, e.what());
				}

				return result;
			}

		private:
			CDuplexPipe m_DuplexPipe;
		};

		namespace json {

			NLOHMANN_JSON_SERIALIZE_ENUM(cursor, { \
				{ cursor::cursor_auto, "auto" }, \
				{ cursor::cursor_default, "default" }, \
				{ cursor::none, "none" }, \
				{ cursor::context_menu, "context-menu" }, \
				{ cursor::help, "help" }, \
				{ cursor::pointer, "pointer" }, \
				{ cursor::progress, "progress" }, \
				{ cursor::wait, "wait" }, \
				{ cursor::cell, "cell" }, \
				{ cursor::crosshair, "crosshair" }, \
				{ cursor::text, "text" }, \
				{ cursor::vertical_text, "vertical-text" }, \
				{ cursor::alias, "alias" }, \
				{ cursor::copy, "copy" }, \
				{ cursor::move, "move" }, \
				{ cursor::no_drop, "no-drop" }, \
				{ cursor::not_allowed, "not-allowed" }, \
				{ cursor::grab, "grab" }, \
				{ cursor::grabbing, "grabbing" }, \
				{ cursor::all_scroll, "all-scroll" }, \
				{ cursor::col_resize, "col-resize" }, \
				{ cursor::row_resize, "row-resize" }, \
				{ cursor::n_resize, "n-resize" }, \
				{ cursor::e_resize, "e-resize" }, \
				{ cursor::s_resize, "s-resize" }, \
				{ cursor::w_resize, "w-resize" }, \
				{ cursor::ne_resize, "ne-resize" }, \
				{ cursor::nw_resize, "nw-resize" }, \
				{ cursor::se_resize, "se-resize" }, \
				{ cursor::sw_resize, "sw-resize" }, \
				{ cursor::ew_resize, "ew-resize" }, \
				{ cursor::ns_resize, "ns-resize" }, \
				{ cursor::nesw_resize, "nesw-resize" }, \
				{ cursor::nwse_resize, "nwse-resize" }, \
				{ cursor::zoom_in, "zoom-in" }, \
				{ cursor::zoom_out, "zoom-out" } \
			})

			inline void to_json(nlohmann::json& j, const Modifiers& ev) {
				auto jArray = nlohmann::json::array();
				if (ev.shift) jArray.push_back("shift");
				if (ev.control) jArray.push_back("control");
				if (ev.alt) jArray.push_back("alt");
				if (ev.meta) jArray.push_back("meta");
				j = jArray;
			}

			inline void to_json(nlohmann::json& j, const InputEvent& ev) {
				j = nlohmann::json{ {"type", ev.type}};
				nlohmann::json modifiers;
				to_json(j["modifiers"], ev.modifiers);
			}

			inline void to_json(nlohmann::json& j, const MouseInputEvent& ev) {
				to_json(j, static_cast<const InputEvent&>(ev));
				j["x"] = ev.x;
				j["y"] = ev.y;
				if (ev.button.has_value()) j["button"] = ev.button.value();
				if (ev.globalX.has_value()) j["globalX"] = ev.globalX.value();
				if (ev.globalY.has_value()) j["globalY"] = ev.globalY.value();
				if (ev.movementX.has_value()) j["movementX"] = ev.movementX.value();
				if (ev.movementY.has_value()) j["movementY"] = ev.movementY.value();
				if (ev.clickCount.has_value()) j["clickCount"] = ev.clickCount.value();
			}

			inline void to_json(nlohmann::json& j, const MouseWheelInputEvent& ev) {
				to_json(j, static_cast<const MouseInputEvent&>(ev));

				if (ev.deltaX.has_value()) j["deltaX"] = ev.deltaX.value();
				if (ev.deltaY.has_value()) j["deltaY"] = ev.deltaY.value();
				if (ev.wheelTicksX.has_value()) j["wheelTicksX"] = ev.wheelTicksX.value();
				if (ev.wheelTicksY.has_value()) j["wheelTicksY"] = ev.wheelTicksY.value();
				if (ev.accelerationRatioX.has_value()) j["accelerationRatioX"] = ev.accelerationRatioX.value();
				if (ev.accelerationRatioY.has_value()) j["accelerationRatioY"] = ev.accelerationRatioY.value();
				if (ev.hasPreciseScrollingDeltas.has_value()) j["hasPreciseScrollingDeltas"] = ev.hasPreciseScrollingDeltas.value();
				if (ev.canScroll.has_value()) j["canScroll"] = ev.canScroll.value();
			}

			inline void to_json(nlohmann::json& j, const KeyboardInputEvent& ev) {
				to_json(j, static_cast<const InputEvent&>(ev));
				j["keyCode"] = ev.keyCode;
			}

			class Handle {
			public:
				Handle() : lo(), hi() {}
				Handle(int lo, int hi) : lo(lo), hi(hi) {}
				int lo;
				int hi;
			};

			bool operator<(const Handle& h1, const Handle& h2) {
				int cmp = h1.hi - h2.hi;
				if (cmp != 0) return cmp < 0;
				return h1.lo < h2.lo;
			}

			inline void to_json(nlohmann::json& j, const Handle& h) {
				j = nlohmann::json{
					{"lo", h.lo},
					{"hi", h.hi}
				};
			}

			inline void from_json(const nlohmann::json& j, Handle& h) {
				j.at("lo").get_to(h.lo);
				j.at("hi").get_to(h.hi);
			}

			HANDLE HandleToHANDLE(const Handle& handle) {
				return Handle64ToHandle((void* __ptr64)((unsigned __int64)handle.lo | ((unsigned __int64)handle.hi)));
			}

			class Rect {
			public:
				Rect() : x0(), y0(), x1(), y1() {}
				Rect(int x0, int y0, int x1, int y1) : x0(x0), y0(y0), x1(x1), y1(y1) {}

				int x0;
				int y0;
				int x1;
				int y1;
			};

			inline void to_json(nlohmann::json& j, const Rect& w) {
				j = nlohmann::json{
					{"x0", w.x0},
					{"y0", w.y0},
					{"x1", w.x1},
					{"y1", w.y1}
				};
			}

			inline void from_json(const nlohmann::json& j, Rect& w) {
				j.at("x0").get_to(w.x0);
				j.at("y0").get_to(w.y0);
				j.at("x1").get_to(w.x1);
				j.at("y1").get_to(w.y1);
			}


			class AdvancedfxGuiClient {
			public:
				explicit AdvancedfxGuiClient(JsonRpcClient& client) : client(client) {}

				Handle GetAfxHookSourceServerReadHandle() {
					return client.CallMethod<Handle>(1, "GetAfxHookSourceServerReadHandle", {});
				}
				Handle GetAfxHookSourceServerWriteHandle() { return client.CallMethod<Handle>(1, "GetAfxHookSourceServerWriteHandle", {}); }

				void DrawingWindowCreated(int luidLo, int luidHi, int width, int height) {
					client.CallNotification("DrawingWindowCreated", { Handle(luidLo, luidHi), width, height });
				}

				void DrawingWindowDestroyed() {
					client.CallNotification("DrawingWindowDestroyed", {});
				}


				bool SendMouseInputEvent(const advancedfx::afxhooksource::json::MouseInputEvent& ev) {
					return client.CallMethod<bool>(1, "SendMouseInputEvent", { ev });
				}
				bool SendMouseWheelInputEvent(const advancedfx::afxhooksource::json::MouseWheelInputEvent& ev) {
					return client.CallMethod<bool>(1, "SendMouseWheelInputEvent", { ev });
				}
				bool SendKeyboardInputEvent(const advancedfx::afxhooksource::json::KeyboardInputEvent& ev) {
					return client.CallMethod<bool>(1, "SendKeyboardInputEvent", { ev });
				}

			private:
				JsonRpcClient& client;
			};


			class AfxHookSourceServer {
			public:
				void SetSharedTextureHandle(const Handle& sharedTextureHandle);
				void SetMouseCursor(advancedfx::afxhooksource::json::cursor cursor);
				void LockSharedTexture();
				void UnlockSharedTexture();
			private:
			};

			void AfxHookSourceServer::SetSharedTextureHandle(const Handle& sharedTextureHandle) {
				AfxHookSource::Gui::SetSharedTextureHandleThreadSafe(HandleToHANDLE(sharedTextureHandle));
			}

			void AfxHookSourceServer::SetMouseCursor(advancedfx::afxhooksource::json::cursor cursor) {
				AfxHookSource::Gui::SetMouseCursorThreadSafe(cursor);
			}

			void AfxHookSourceServer::LockSharedTexture() {
				AfxHookSource::Gui::LockSharedTextueThreadSafe();
			}
			void AfxHookSourceServer::UnlockSharedTexture() {
				AfxHookSource::Gui::UnlockSharedTextureThreadSafe();
			}

			void ThrowExcepetion(const std::exception& e) {
				if (!IsDebuggerPresent()) MessageBoxA(NULL, e.what(), "CJsonRpcServer exception", MB_OK | MB_ICONERROR);
				throw e;
			}

			class CJsonRpcServer {
			public:
				CJsonRpcServer(HANDLE readHandle, HANDLE writeHandle)
					: m_DuplexPipe(readHandle, writeHandle)
					, m_RpcServer()
					, m_Server()
				{
					m_RpcServer.Add("SetSharedTextureHandle", GetHandle(&AfxHookSourceServer::SetSharedTextureHandle, m_Server), {"handle"});
					m_RpcServer.Add("SetMouseCursor", GetHandle(&AfxHookSourceServer::SetMouseCursor, m_Server), {"cursor"});
					m_RpcServer.Add("LockSharedTexture", GetHandle(&AfxHookSourceServer::LockSharedTexture, m_Server), {});
					m_RpcServer.Add("UnlockSharedTexture", GetHandle(&AfxHookSourceServer::UnlockSharedTexture, m_Server), {});

					m_Thread = std::thread(&CJsonRpcServer::HandleRequests, this);
				}

				~CJsonRpcServer() {
					m_Quit = true;
					while (!m_HasQuit) CancelSynchronousIo(m_Thread.native_handle());
					if (m_Thread.joinable())
						m_Thread.join();
				}

				void HandleRequests(void) {
					std::string inStr;

					try {
						while (!m_Quit) {
							m_DuplexPipe.ReadString(inStr);
							std::string result = m_RpcServer.HandleRequest(inStr);
							inStr.clear();
							m_DuplexPipe.WriteString(result);
						}
					}
					catch (const std::exception& e) {
						if (!m_Quit) {
							ThrowExcepetion(e);
						}
					}

					m_HasQuit = true;
				}

			private:
				CDuplexPipe m_DuplexPipe;
				JsonRpc2Server m_RpcServer;
				AfxHookSourceServer m_Server;
				std::atomic_bool m_Quit = false;
				std::atomic_bool m_HasQuit = false;
				std::thread m_Thread;
			};


			class CJsonRpc {
			public:
				CJsonRpc(HANDLE readHandle, HANDLE writeHandle)
					: m_ClientConnector(readHandle, writeHandle)
					, m_RpcClient(m_ClientConnector, version::v2)
					, m_Client(m_RpcClient) {
				}

				~CJsonRpc() {
					delete m_Server;
				}

				void Connect() {
					if (m_Server) throw "Already connected";

					std::unique_lock<std::mutex> lock(m_ClientMutex);

					Handle readHandle = m_Client.GetAfxHookSourceServerReadHandle();
					Handle writeHandle = m_Client.GetAfxHookSourceServerWriteHandle();

					m_Server = new CJsonRpcServer(HandleToHANDLE(readHandle), HandleToHANDLE(writeHandle));
				}

				void DrawingWindowCreated(int luidLo, int luidHi, int width, int height) {
					std::unique_lock<std::mutex> lock(m_ClientMutex);

					m_Client.DrawingWindowCreated(luidLo, luidHi, width, height);
				}

				void DrawingWindowDestroyed() {
					std::unique_lock<std::mutex> lock(m_ClientMutex);

					m_Client.DrawingWindowDestroyed();
				}


				bool SendMouseInputEvent(const advancedfx::afxhooksource::json::MouseInputEvent& ev) {
					std::unique_lock<std::mutex> lock(m_ClientMutex);

					return m_Client.SendMouseInputEvent(ev);
				}
				bool SendMouseWheelInputEvent(const advancedfx::afxhooksource::json::MouseWheelInputEvent& ev) {
					std::unique_lock<std::mutex> lock(m_ClientMutex);

					return m_Client.SendMouseWheelInputEvent(ev);
				}
				bool SendKeyboardInputEvent(const advancedfx::afxhooksource::json::KeyboardInputEvent& ev) {
					std::unique_lock<std::mutex> lock(m_ClientMutex);

					return m_Client.SendKeyboardInputEvent(ev);
				}

			private:
				CDuplexPipeJsonClientConnector m_ClientConnector;
				jsonrpccxx::JsonRpcClient m_RpcClient;
				AdvancedfxGuiClient m_Client;
				std::mutex m_ClientMutex;
				CJsonRpcServer* m_Server = nullptr;
			};

		} // namespace json {
	} // namespace afxhooksource {
} // namespace advancedfx {

using namespace advancedfx::afxhooksource;
using namespace advancedfx::afxhooksource::json;

CJsonRpc *g_JsonRpc = nullptr;

bool InitJsonRpc(int hReadPipe, int hWritePipe) {
	ShutdownJsonRpc();

	g_JsonRpc = new CJsonRpc(LongToHandle(hReadPipe), LongToHandle(hWritePipe));

	try {
		g_JsonRpc->Connect();
	}
	catch (const std::exception& e) {
		ThrowExcepetion(e);
	}

	return true;
}

void ShutdownJsonRpc() {
	if (g_JsonRpc) {
		delete g_JsonRpc;
		g_JsonRpc = nullptr;
	}
}

void JsonDrawingWindowCreated(int luidLo, int luidHi, int width, int height) {
	if (!g_JsonRpc) return;

	try {
		g_JsonRpc->DrawingWindowCreated(luidLo, luidHi, width, height);
	}
	catch (const std::exception& e) {
		ThrowExcepetion(e);
	}
}

void JsonDrawingWindowDestroyed() {
	if (!g_JsonRpc) return;

	try {
		g_JsonRpc->DrawingWindowDestroyed();
	}
	catch (const std::exception& e) {
		ThrowExcepetion(e);
	}
}


bool JsonSendMouseInputEvent(const advancedfx::afxhooksource::json::MouseInputEvent& ev) {
	if (!g_JsonRpc) return false;

	try {
		return g_JsonRpc->SendMouseInputEvent(ev);
	}
	catch (const std::exception& e) {
		ThrowExcepetion(e);
	}
	return false;
}

bool JsonSendMouseWheelInputEvent(const advancedfx::afxhooksource::json::MouseWheelInputEvent& ev) {
	if (!g_JsonRpc) return false;

	try {
		return g_JsonRpc->SendMouseWheelInputEvent(ev);
	}
	catch (const std::exception& e) {
		ThrowExcepetion(e);
	}
	return false;
}
bool JsonSendKeyboardInputEvent(const advancedfx::afxhooksource::json::KeyboardInputEvent& ev) {
	if (!g_JsonRpc) return false;

	try {
		return g_JsonRpc->SendKeyboardInputEvent(ev);
	}
	catch (const std::exception& e) {
		ThrowExcepetion(e);
	}
	return false;
}