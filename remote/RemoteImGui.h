/********************************************************
* (c) Mojang. All rights reserved                       *
* (c) Microsoft. All rights reserved.                   *
*********************************************************/
#pragma once

#include "../imgui.h"
#include <memory>
#include <string>

#ifdef IMGUI_ENABLED

namespace imgui {
	struct Frame;
	class RemoteImGuiFrameBuilder;

	//------------------
	// ImGuiRemoteInput
	// - a structure to store input received from remote imgui, so you can use it on your whole app (keys, mouse) or just in imgui engine
	// - use GetImGuiRemoteInput to read input data safely (valid for IMGUI_REMOTE_INPUT_FRAMES)
	//------------------
	struct RemoteInput
	{
		ImVec2	MousePos;
		int		MouseButtons;
		float	MouseWheelDelta;
		bool	KeyCtrl;
		bool	KeyShift;
		bool	KeysDown[256];
	};

	// Keys set in KeysDown to represent special keys
	enum class ImGuiKeyCode : size_t {
		Control = 21,
		Shift = 22
	};

	enum class RemoteMessageType : unsigned char {
		// System events fired by the relay server
		RelayRoomJoined = 0,
		RelayRoomUpdate,

		// ImGUI events owned by the client
		ImInit = 10,
		ImMouseMove,
		ImMousePress,
		ImMouseWheelDelta,
		ImKeyDown,
		ImKeyUp,
		ImKeyPress,
		ImClipboard
	};

	class RemoteImGui {
	public:
		RemoteImGui();
		virtual ~RemoteImGui();

		virtual void init() = 0;
		virtual void connect() = 0;
		virtual void disconnect() = 0;
		virtual void update();
		virtual bool getIsConnected() const = 0;

		bool getRemoteInput(RemoteInput &input);
		void draw(ImDrawList** const cmd_lists, int cmd_lists_count);

		// Used to log to console (cannot use normal system because of submodules)
		void (*mDebug)(const std::string & output);

	protected:
		virtual bool _getIsActive() const = 0;
		virtual void _sendFrame(const Frame& frame) = 0;

		void _sendFontFrame();
		void _sendDrawFrame(ImDrawList** const cmd_lists, int cmd_lists_count);

		void _handleMessage(RemoteMessageType messageType, const void *data, int size);

	private:
		void _onImMouseMove(int x, int y, int left, int right);
		void _onImMousePress(int left, int right);
		void _onImMouseWheelDelta(float mouseWheel);
		void _onImKeyDown(int key, int shift, int ctrl);
		void _onImKeyUp(int key, int shift, int ctrl);
		void _onImKeyPress(int key);
		void _onImClipboard(void* userData, const char* text);

		// Returns true if a "normal" key and false if a "special" key
		inline bool _mapRemoteKey(int & remoteKey, bool isCtrlPressed) {
			if (remoteKey == 9)
				remoteKey = ImGuiKey_Tab;
			else if (remoteKey == 37)
				remoteKey = ImGuiKey_LeftArrow;
			else if (remoteKey == 39)
				remoteKey = ImGuiKey_RightArrow;
			else if (remoteKey == 38)
				remoteKey = ImGuiKey_UpArrow;
			else if (remoteKey == 40)
				remoteKey = ImGuiKey_DownArrow;
			else if (remoteKey == 33)
				remoteKey = ImGuiKey_PageUp;
			else if (remoteKey == 34)
				remoteKey = ImGuiKey_PageDown;
			else if (remoteKey == 36)
				remoteKey = ImGuiKey_Home;
			else if (remoteKey == 35)
				remoteKey = ImGuiKey_End;
			else if (remoteKey == 45)
				remoteKey = ImGuiKey_Insert;
			else if (remoteKey == 46)
				remoteKey = ImGuiKey_Delete;
			else if (remoteKey == 8)
				remoteKey = ImGuiKey_Backspace;
			else if (remoteKey == 32)
				remoteKey = ImGuiKey_Space;
			else if (remoteKey == 13)
				remoteKey = ImGuiKey_Enter;
			else if (remoteKey == 27)
				remoteKey = ImGuiKey_Escape;
			else if (remoteKey == 65 && isCtrlPressed)
				remoteKey = ImGuiKey_A;
			else if (remoteKey == 67 && isCtrlPressed)
				remoteKey = ImGuiKey_C;
			else if (remoteKey == 86 && isCtrlPressed)
				remoteKey = ImGuiKey_V;
			else if (remoteKey == 88 && isCtrlPressed)
				remoteKey = ImGuiKey_X;
			else if (remoteKey == 89 && isCtrlPressed)
				remoteKey = ImGuiKey_Y;
			else if (remoteKey == 90 && isCtrlPressed)
				remoteKey = ImGuiKey_Z;
			else if (remoteKey == 16)
				remoteKey = static_cast<int>(ImGuiKeyCode::Shift);
			else if (remoteKey == 17)
				remoteKey = static_cast<int>(ImGuiKeyCode::Control);
			else
				return true;

			return false;
		}

	private:
		std::unique_ptr<RemoteImGuiFrameBuilder> mFrameBuilder;
		RemoteInput mInput;
		int mFrame = 0;
		int mFrameReceived = 0;
	};
}

#endif // IMGUI_ENABLED