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

	void RemoteImGuiServer::OnMessage(IWebSocketServer::OpCode opcode, const void *data, int) {
		switch (opcode)
		{
			// Text message
		case IWebSocketServer::Text:
			if (!_getIsActive()) {
				if (!memcmp(data, "ImInit", 6)) {
					mIsClientActive = true;

					// Send confirmation
					SendText("ImInit", 6);

					// Send font texture
					_sendFontFrame();
				}
			}
			else if (strstr((char *)data, "ImMouseMove")) {
				int x, y, mouse_left, mouse_right;
				if (sscanf((char *)data, "ImMouseMove=%d,%d,%d,%d", &x, &y, &mouse_left, &mouse_right) == 4) {
					_onImMouseMove(x, y, mouse_left, mouse_right);
				}
			}
			else if (strstr((char *)data, "ImMousePress")) {
				int l, r;
				if (sscanf((char *)data, "ImMousePress=%d,%d", &l, &r) == 2) {
					_onImMousePress(l, r);
				}
			}
			else if (strstr((char *)data, "ImMouseWheelDelta")) {
				float mouseWheelDelta;
				if (sscanf((char *)data, "ImMouseWheelDelta=%f", &mouseWheelDelta) == 1) {
					_onImMouseWheelDelta(mouseWheelDelta);
				}
			}
			else if (strstr((char *)data, "ImKeyDown")) {
				int key, shift, ctrl;
				if (sscanf((char *)data, "ImKeyDown=%d,%d,%d", &key, &shift, &ctrl) == 3) {
					_onImKeyDown(key, shift, ctrl);
				}
			}
			else if (strstr((char *)data, "ImKeyUp")) {
				int key;
				if (sscanf((char *)data, "ImKeyUp=%d", &key) == 1) {
					_onImKeyUp(key);
				}
			}
			else if (strstr((char *)data, "ImKeyPress")) {
				unsigned int key;
				if (sscanf((char *)data, "ImKeyPress=%d", &key) == 1)
					_onImKeyPress(key);
			}
			else if (strstr((char *)data, "ImClipboard=")) {
				_onImClipboard(nullptr, &((char *)data)[strlen("ImClipboard=")]);
			}
			break;

		
			break;

		case IWebSocketServer::Disconnect:
			mIsClientActive = false;
			break;

		case IWebSocketServer::Binary:
		case IWebSocketServer::Ping:
		case IWebSocketServer::Pong:
			break;
		
		default:
			assert(0);
			break;
		}
	}

	void RemoteImGuiServer::_sendFrame(const Frame& frame) {
		SendBinary(frame.data, frame.size);
	}

	bool RemoteImGuiServer::_getIsActive() const {
		return mIsClientActive;
	}
}