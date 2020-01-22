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
#include <iostream> // std::cout
#include <sstream>

#ifdef IMGUI_ENABLED

namespace imgui {

	RemoteImGuiServer::RemoteImGuiServer(int16_t port) 
		: mPort(port) {
	}

	RemoteImGuiServer::~RemoteImGuiServer() = default;

	void RemoteImGuiServer::init() {
	}

	void RemoteImGuiServer::connect() {
		const std::string address = "0.0.0.0";

		std::cout << "Hosting ImGui server @ http://" << address << ":" << mPort << std::endl;
		IWebSocketServer::Init(address.c_str(), mPort);
	}

	void RemoteImGuiServer::disconnect() {
		IWebSocketServer::Shutdown();
	}

	void RemoteImGuiServer::update() {
		RemoteImGui::update();
		IWebSocketServer::Update();
	}

	bool RemoteImGuiServer::getIsConnected() const {
		return _getIsActive();
	}

	void RemoteImGuiServer::OnMessage(IWebSocketServer::OpCode opcode, const void *data, int size) {
		switch (opcode) {
			case IWebSocketServer::Text: {
				if (size > 0) {
					const unsigned char * text = static_cast<const unsigned char *>(data);
					RemoteMessageType messageType = static_cast<RemoteMessageType>(text[0]);
					_handleMessage(messageType, &text[1], size - 1);
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

	void RemoteImGuiServer::_sendText(const std::string& str) {
		SendText(str.c_str(), static_cast<int>(str.size()));
	}

	void RemoteImGuiServer::_handleMessage(RemoteMessageType messageType, const void * data, int size) {
		switch (messageType) {
			case RemoteMessageType::ImInit: {
				// If not active, don't process input
				if (!_getIsActive()) {
					mIsClientActive = true;
					RemoteImGui::_handleMessage(messageType, data, size);
				}
				break;
			}
			case RemoteMessageType::RelayRoomJoined:
			case RemoteMessageType::RelayRoomUpdate:
			case RemoteMessageType::ImMouseMove:
			case RemoteMessageType::ImMousePress:
			case RemoteMessageType::ImMouseWheelDelta:
			case RemoteMessageType::ImKeyDown:
			case RemoteMessageType::ImKeyUp:
			case RemoteMessageType::ImKeyPress:
			case RemoteMessageType::ImClipboard:
			case RemoteMessageType::ImCanvasUpdate:
			default: {
				// Handling other messages is not specific to local servers
				// Unhandled messages will be logged in upward implementation
				RemoteImGui::_handleMessage(messageType, data, size);
				break;
			}
		}
	}
}

#endif // IMGUI_ENABLED