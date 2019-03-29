/********************************************************
* (c) Mojang. All rights reserved                       *
* (c) Microsoft. All rights reserved.                   *
*********************************************************/
#include "RemoteImGuiFrameBuilder.h"

#include "lz4/lz4.h"
#include <fstream>
#include <streambuf>
#include <string>


namespace imgui {
	void RemoteImGuiFrameBuilder::buildFontFrame(Frame &frame) {
		mForceKeyFrame = true;

		unsigned char* pixels;
		int width, height;
		ImGui::GetIO().Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);
		_preparePacketTexFont(pixels, width, height);

		return _buildFrame(frame);
	}

	const bool RemoteImGuiFrameBuilder::buildDrawFrame(ImDrawList** const cmd_lists, int cmd_lists_count, bool isKeyFrame, Frame& frame) {
		static int sendframe = 0;
		if (++sendframe < 2) // every 2 frames, @TWEAK
		{
			return false;
		}
		sendframe = 0;

		unsigned int totalSize = sizeof(unsigned int); // cmd_lists_count
		for (int n = 0; n < cmd_lists_count; n++)
		{
			const ImDrawList* cmd_list = cmd_lists[n];
			int cmd_count = cmd_list->CmdBuffer.size();
			int vtx_count = cmd_list->VtxBuffer.size();
			int idx_count = cmd_list->IdxBuffer.size();
			totalSize += 3 * sizeof(unsigned int); //cmd_count, vtx_count and idx_count
			totalSize += cmd_count * sizeof(RemoteImGuiFrameBuilder::Cmd) + vtx_count * sizeof(RemoteImGuiFrameBuilder::Vtx) + idx_count * sizeof(RemoteImGuiFrameBuilder::Idx);
		}

		_preparePacketFrame(totalSize, isKeyFrame);
		_write((unsigned int)cmd_lists_count);

		for (int n = 0; n < cmd_lists_count; n++)
		{
			const ImDrawList* cmd_list = cmd_lists[n];
			const ImDrawVert * vtx_src = cmd_list->VtxBuffer.begin();
			const ImDrawIdx * idx_src = cmd_list->IdxBuffer.begin();
			unsigned int cmd_count = cmd_list->CmdBuffer.size();
			unsigned int vtx_count = cmd_list->VtxBuffer.size();
			unsigned int idx_count = cmd_list->IdxBuffer.size();
			_write(cmd_count);
			_write(vtx_count);
			_write(idx_count);
			// Send 
			// Add all draw cmds
			RemoteImGuiFrameBuilder::Cmd cmd;
			const ImDrawCmd* pcmd_end = cmd_list->CmdBuffer.end();
			for (const ImDrawCmd* pcmd = cmd_list->CmdBuffer.begin(); pcmd != pcmd_end; pcmd++)
			{
				cmd.set(*pcmd);
				_write(cmd);
			}
			// Add all vtx
			RemoteImGuiFrameBuilder::Vtx vtx;
			int vtx_remaining = vtx_count;
			while (vtx_remaining-- > 0)
			{
				vtx.set(*vtx_src++);
				_write(vtx);
			}

			// Add all idx
			RemoteImGuiFrameBuilder::Idx idx;

			int idx_remaining = idx_count;
			while (idx_remaining-- > 0)
			{
				idx.set(*idx_src++);
				_write(idx);
			}
		}

		_buildFrame(frame);
		return true;
	}

	void RemoteImGuiFrameBuilder::_buildFrame(Frame& frame) {
		static int buffer[65536];
		int size = (int)mPacket.size();
		int csize = LZ4_compress_limitedOutput((char *)&mPacket[0], (char *)(buffer + 3), size, 65536 * sizeof(int) - 12);
		buffer[0] = 0xBAADFEED; // Our LZ4 header magic number (used in custom lz4.js to decompress)
		buffer[1] = size;
		buffer[2] = csize;
		mPrevPacketSize = size;

		frame.data = buffer;
		frame.size = csize + 12;
	}

	void RemoteImGuiFrameBuilder::_write(unsigned char c) {
		mPacket.push_back(c);
	}

	void RemoteImGuiFrameBuilder::_write(unsigned int i) {
		if (mIsKeyFrame)
			_write(&i, sizeof(unsigned int));
		else
			_writeDiff(&i, sizeof(unsigned int));
	}

	void RemoteImGuiFrameBuilder::_write(Cmd const &cmd) {
		if (mIsKeyFrame)
			_write((void *)&cmd, sizeof(Cmd));
		else
			_writeDiff((void *)&cmd, sizeof(Cmd));
	}

	void RemoteImGuiFrameBuilder::_write(Vtx const &vtx) {
		if (mIsKeyFrame)
			_write((void *)&vtx, sizeof(Vtx));
		else
			_writeDiff((void *)&vtx, sizeof(Vtx));
	}

	void RemoteImGuiFrameBuilder::_write(Idx const &idx) {
		if (mIsKeyFrame)
			_write((void *)&idx, sizeof(Idx));
		else
			_writeDiff((void *)&idx, sizeof(Idx));
	}

	void RemoteImGuiFrameBuilder::_write(const void *data, int size) {
		unsigned char *src = (unsigned char *)data;
		for (int i = 0; i < size; i++)
		{
			int pos = (int)mPacket.size();
			_write(src[i]);
			mPrevPacket[pos] = src[i];
		}
	}

	void RemoteImGuiFrameBuilder::_writeDiff(const void *data, int size) {
		unsigned char *src = (unsigned char *)data;
		for (int i = 0; i < size; i++)
		{
			int pos = (int)mPacket.size();
			_write((unsigned char)(src[i] - (pos < mPrevPacketSize ? mPrevPacket[pos] : 0)));
			mPrevPacket[pos] = src[i];
		}
	}

	void RemoteImGuiFrameBuilder::_preparePacket(PacketType data_type, unsigned int data_size) {
		unsigned int size = sizeof(unsigned char) + data_size;
		mPacket.clear();
		mPacket.reserve(size);
		mPrevPacket.reserve(size);
		while (size > mPrevPacket.size())
			mPrevPacket.push_back(0);
		_write(static_cast<unsigned char>(data_type));
	}

	void RemoteImGuiFrameBuilder::_preparePacketTexFont(const void *data, unsigned int w, unsigned int h) {
		mIsKeyFrame = true;
		_preparePacket(PacketType::TEX_FONT, sizeof(unsigned int) * 2 + w * h);
		_write(w);
		_write(h);
		_write(data, w*h);
		mForceKeyFrame = true;
	}

	void RemoteImGuiFrameBuilder::_preparePacketFrame(unsigned int size, bool isKeyFrame) {
		mIsKeyFrame = isKeyFrame || mForceKeyFrame;
		_preparePacket(mIsKeyFrame ? PacketType::FRAME_KEY : PacketType::FRAME_DIFF, size);
		mForceKeyFrame = false;
	}

}