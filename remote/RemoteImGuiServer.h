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

namespace imgui {
	class RemoteImGuiServer : public RemoteImGui, public IWebSocketServer {
	public:
		RemoteImGuiServer() = default;
		~RemoteImGuiServer();

		void init() override;
		void connect() override;
		void disconnect() override;
		void update() override;
		bool getIsConnected() const override;

		virtual void OnMessage(OpCode opcode, const void *data, int size);

	protected:
		virtual void _sendFrame(const Frame& frame) override;
		virtual bool _getIsActive() const override;

	private:
		bool mIsClientActive = false;
	};
}