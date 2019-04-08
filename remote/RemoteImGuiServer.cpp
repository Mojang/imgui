//-----------------------------------------------------------------------------
// Remote ImGui https://github.com/JordiRos/remoteimgui
// Uses
// ImGui https://github.com/ocornut/imgui 1.3
// Webby https://github.com/deplinenoise/webby
// LZ4   https://code.google.com/p/lz4/
//-----------------------------------------------------------------------------
#include "RemoteImGuiServer.h"

#include "RemoteImGuiFrameBuilder.h"
#include <cstdio>

#ifdef IMGUI_ENABLED

namespace imgui {
	RemoteImGuiServer::~RemoteImGuiServer() {
	}

	void RemoteImGuiServer::init() {
	}

	void RemoteImGuiServer::connect() {
		const std::string address = "0.0.0.0";
		const int port = 7002;

		IWebSocketServer::Init(address.c_str(), port);
	}

	void RemoteImGuiServer::disconnect() {
		IWebSocketServer::Shutdown();
	}

	void RemoteImGuiServer::update() {
		IWebSocketServer::Update();
	}

	bool RemoteImGuiServer::getIsConnected() const {
		return _getIsActive();
	}

	void RemoteImGuiServer::OnMessage(IWebSocketServer::OpCode opcode, const void *data, int size) {
		switch (opcode) {
			case IWebSocketServer::Text: {
				// If not active, don't process input
				if (!_getIsActive()) {
					// Wait for init instead
					if (!memcmp(data, "ImInit", sizeof("ImInit") - 1)) {
						mIsClientActive = true;

						// Send confirmation
						SendText("ImInit", 6);

						// Send font texture
						_sendFontFrame();
					}
				}
				// Handling other messages is not specific to local servers
				else if (strstr((char *)data, "ImMouseMove")) {
					_handleMessage(RemoteMessageType::ImMouseMove, (char*)data + sizeof("ImMouseMove"), size - sizeof("ImMouseMove"));
				}
				else if (strstr((char *)data, "ImMousePress")) {
					_handleMessage(RemoteMessageType::ImMousePress, (char*)data + sizeof("ImMousePress"), size - sizeof("ImMousePress"));
				}
				else if (strstr((char *)data, "ImMouseWheelDelta")) {
					_handleMessage(RemoteMessageType::ImMouseWheelDelta, (char*)data + sizeof("ImMouseWheelDelta"), size - sizeof("ImMouseWheelDelta"));
				}
				else if (strstr((char *)data, "ImKeyDown")) {
					_handleMessage(RemoteMessageType::ImKeyDown, (char*)data + sizeof("ImKeyDown"), size - sizeof("ImKeyDown"));
				}
				else if (strstr((char *)data, "ImKeyUp")) {
					_handleMessage(RemoteMessageType::ImKeyUp, (char*)data + sizeof("ImKeyUp"), size - sizeof("ImKeyUp"));
				}
				else if (strstr((char *)data, "ImKeyPress")) {
					_handleMessage(RemoteMessageType::ImKeyPress, (char*)data + sizeof("ImKeyPress"), size - sizeof("ImKeyPress"));
				}
				else if (strstr((char *)data, "ImClipboard")) {
					_handleMessage(RemoteMessageType::ImClipboard, (char*)data + sizeof("ImClipboard"), size - sizeof("ImClipboard"));
				}
				break;
			}
			case IWebSocketServer::Disconnect: {
				mIsClientActive = false;
				break;
			}
			case IWebSocketServer::Continuation:
			case IWebSocketServer::Binary:
			case IWebSocketServer::Ping:
			case IWebSocketServer::Pong: {
				// Valid, but unhandled
				break;
			}
			default: {
				// Unsupported message type
				assert(0);
				break;
			}
		}
	}

	bool RemoteImGuiServer::_getIsActive() const {
		return mIsClientActive;
	}

	void RemoteImGuiServer::_sendFrame(const Frame& frame) {
		SendBinary(frame.data, frame.size);
	}
}

#endif // IMGUI_ENABLED