/********************************************************
* (c) Mojang. All rights reserved                       *
* (c) Microsoft. All rights reserved.                   *
*********************************************************/
#pragma once

#include "../imgui.h"
#include <memory>

namespace imgui {
	struct RemoteInput;
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

	class RemoteImGui {
	public:
		RemoteImGui();
		virtual ~RemoteImGui();

		virtual void init() = 0;
		virtual void connect() = 0;
		virtual void disconnect() = 0;
		virtual void update();
		virtual void draw(ImDrawList** const cmd_lists, int cmd_lists_count);

		bool getRemoteInput(RemoteInput &input);
		virtual bool getIsConnected() const = 0;

	protected:
		void _onImMouseMove(int x, int y, int left, int right);
		void _onImMousePress(int left, int right);
		void _onImMouseWheelDelta(float mouseWheel);
		void _onImKeyDown(int key, int shift, int ctrl);
		void _onImKeyUp(int key);
		void _onImKeyPress(int key);
		void _onImClipboard(void* userData, const char* text);
		void _sendFontFrame();
		void _sendDrawFrame(ImDrawList** const cmd_lists, int cmd_lists_count);

		virtual void _sendFrame(const Frame& frame) = 0;
		virtual bool _getIsActive() const = 0;

	private:
		inline bool mapRemoteKey(int* remoteKey, bool isCtrlPressed) {
			if (*remoteKey == 37)
				*remoteKey = ImGuiKey_LeftArrow;
			else if (*remoteKey == 40)
				*remoteKey = ImGuiKey_DownArrow;
			else if (*remoteKey == 38)
				*remoteKey = ImGuiKey_UpArrow;
			else if (*remoteKey == 39)
				*remoteKey = ImGuiKey_RightArrow;
			else if (*remoteKey == 46)
				*remoteKey = ImGuiKey_Delete;
			else if (*remoteKey == 9)
				*remoteKey = ImGuiKey_Tab;
			else if (*remoteKey == 8)
				*remoteKey = ImGuiKey_Backspace;
			else if (*remoteKey == 65 && isCtrlPressed)
				*remoteKey = 'a';
			else if (*remoteKey == 67 && isCtrlPressed)
				*remoteKey = 'c';
			else if (*remoteKey == 86 && isCtrlPressed)
				*remoteKey = 'v';
			else if (*remoteKey == 88 && isCtrlPressed)
				*remoteKey = 'x';
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