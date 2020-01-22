//-----------------------------------------------------------------------------
// Remote ImGui https://github.com/JordiRos/remoteimgui
// Uses
// ImGui https://github.com/ocornut/imgui 1.3
// Webby https://github.com/deplinenoise/webby
// LZ4   https://code.google.com/p/lz4/
//-----------------------------------------------------------------------------
#pragma once

#include "imgui_remote_webby.h"
#include "RemoteImGui.h"
#include <inttypes.h>

#ifdef IMGUI_ENABLED

namespace imgui {
	// Hosts a remote ImGUI interface through http://localhost:7002/
	class RemoteImGuiServer : public RemoteImGui, public IWebSocketServer {
	public:
		RemoteImGuiServer(int16_t port = 7002);
		~RemoteImGuiServer();

		void init() override;
		void connect() override;
		void disconnect() override;
		void update() override;
		bool getIsConnected() const override;

		virtual void OnMessage(OpCode opcode, const void *data, int size) override;

	protected:
		virtual bool _getIsActive() const override;
		virtual void _sendFrame(const Frame& frame) override;
		virtual void _sendText(const std::string& str) override;

		void _handleMessage(RemoteMessageType messageType, const void *data, int size);

	private:
		bool mIsClientActive = false;
		int16_t mPort;
	};
}

#endif // IMGUI_ENABLED