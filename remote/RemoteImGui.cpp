/********************************************************
* (c) Mojang. All rights reserved                       *
* (c) Microsoft. All rights reserved.                   *
*********************************************************/
#include "RemoteImGui.h"

#include "RemoteImGuiFrameBuilder.h"

#ifdef IMGUI_ENABLED

#define IMGUI_REMOTE_KEY_FRAME    60  // send keyframe every 30 frames
#define IMGUI_REMOTE_INPUT_FRAMES 60 // input valid during 120 frames

namespace imgui {

	RemoteImGui::RemoteImGui()
		: mFrameBuilder(std::make_unique<RemoteImGuiFrameBuilder>()) {
	}

	RemoteImGui::~RemoteImGui() {
	}

	void RemoteImGui::update() {
		mFrame++;
	}

	void RemoteImGui::draw(ImDrawList** const cmd_lists, int cmd_lists_count) {
		if (_getIsActive()) {
			_sendDrawFrame(cmd_lists, cmd_lists_count);
		}
	}

	void RemoteImGui::_onImMouseMove(int x, int y, int left, int right) {
		mFrameReceived = mFrame;
		mInput.MousePos.x = (float)x;
		mInput.MousePos.y = (float)y;
		mInput.MouseButtons = left | (right << 1);
	}

	void RemoteImGui::_onImMousePress(int left, int right) {
		mFrameReceived = mFrame;
		mInput.MouseButtons = left | (right << 1);
	}

	void RemoteImGui::_onImMouseWheelDelta(float mouseWheel) {
		mFrameReceived = mFrame;
		mInput.MouseWheelDelta = mouseWheel * 0.01f;
	}

	void RemoteImGui::_onImKeyDown(int key, int shift, int ctrl) {
		mFrameReceived = mFrame;
		mInput.KeyShift = shift > 0;
		mInput.KeyCtrl = ctrl > 0;
		mapRemoteKey(&key, mInput.KeyCtrl);
		mInput.KeysDown[key] = true;
	}

	void RemoteImGui::_onImKeyUp(int key) {
		mFrameReceived = mFrame;
		mInput.KeysDown[key] = false;
		mInput.KeyShift = false;
		mInput.KeyCtrl = false;
	}

	void RemoteImGui::_onImKeyPress(int key) {
		ImGui::GetIO().AddInputCharacter(static_cast<ImWchar>(key));
	}

	void RemoteImGui::_onImClipboard(void* userData, const char* text) {
		ImGui::GetIO().SetClipboardTextFn(userData, text);
	}

	void RemoteImGui::_sendFontFrame() {
		Frame frame;
		mFrameBuilder->buildFontFrame(frame);
		_sendFrame(frame);
	}

	void RemoteImGui::_sendDrawFrame(ImDrawList** const cmd_lists, int cmd_lists_count) {
		Frame frame;
		if (mFrameBuilder->buildDrawFrame(cmd_lists, cmd_lists_count, (mFrame % IMGUI_REMOTE_KEY_FRAME) == 0, frame)) {
			_sendFrame(frame);
		}
	}

	bool RemoteImGui::getRemoteInput(RemoteInput &input) {
		bool res = false;
		if (_getIsActive()) {
			if (mFrame - mFrameReceived < IMGUI_REMOTE_INPUT_FRAMES) {
				input = mInput;
				res = true;
			}
		}

		memset(mInput.KeysDown, 0, 256 * sizeof(bool));
		mInput.MouseWheelDelta = 0;
		return res;
	}
}

#endif // IMGUI_ENABLED