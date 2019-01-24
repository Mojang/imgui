/********************************************************
* (c) Mojang. All rights reserved                       *
* (c) Microsoft. All rights reserved.                   *
*********************************************************/
#pragma once

#include "../imgui.h"
#include <vector>

namespace imgui {
	struct Frame {
		const void* data;
		int size;
	};

	class RemoteImGuiFrameBuilder {
	public:
		RemoteImGuiFrameBuilder() = default;
		~RemoteImGuiFrameBuilder() = default;

		void buildFontFrame(Frame &frame);
		const bool buildDrawFrame(ImDrawList** const cmd_lists, int cmd_lists_count, bool isKeyFrame, Frame &frame);

	private:
#pragma pack(1)
		struct Cmd {
			int elemCount;
			float clip_rect[4];
			void set(const ImDrawCmd &draw_cmd) {
				elemCount = draw_cmd.ElemCount;
				clip_rect[0] = draw_cmd.ClipRect.x;
				clip_rect[1] = draw_cmd.ClipRect.y;
				clip_rect[2] = draw_cmd.ClipRect.z;
				clip_rect[3] = draw_cmd.ClipRect.w;
			}
		};

		struct Vtx {
			short x, y; // 16 short
			short u, v; // 16 fixed point
			unsigned char r, g, b, a; // 8*4
			void set(const ImDrawVert &vtx) {
				x = (short)(vtx.pos.x);
				y = (short)(vtx.pos.y);
				u = (short)(vtx.uv.x * 32767.f);
				v = (short)(vtx.uv.y * 32767.f);
				r = (vtx.col >> 0) & 0xff;
				g = (vtx.col >> 8) & 0xff;
				b = (vtx.col >> 16) & 0xff;
				a = (vtx.col >> 24) & 0xff;
			}
		};

		struct Idx {
			unsigned short idx;
			void set(ImDrawIdx _idx) {
				idx = _idx;
			}
		};
#pragma pack()

		enum class PacketType : unsigned char { TEX_FONT = 255, FRAME_KEY = 254, FRAME_DIFF = 253 };

		void _write(unsigned char c);
		void _write(unsigned int i);
		void _write(Cmd const &cmd);
		void _write(Vtx const &vtx);
		void _write(Idx const &idx);
		void _write(const void *data, int size);
		void _writeDiff(const void *data, int size);

		void _preparePacket(PacketType data_type, unsigned int data_size);
		void _preparePacketTexFont(const void *data, unsigned int w, unsigned int h);
		void _preparePacketFrame(unsigned int size, bool isKeyFrame);
		void _buildFrame(Frame &frame);

	private:
		int mPrevPacketSize = 0;
		bool mIsKeyFrame = false;
		bool mForceKeyFrame = false;
		std::vector<unsigned char> mPacket;
		std::vector<unsigned char> mPrevPacket;
	};
}