#pragma once
#pragma once

#include <memory>

#include "wrapper/plugin_api.h"

using namespace IOPlugin;

struct x265_encoder;
struct x265_param;

class UISettingsController;

class X265Encoder : public IPluginCodecRef
{
public:
	static const uint8_t s_UUID[];

public:
	X265Encoder();
	~X265Encoder();

	static StatusCode s_RegisterCodecs(HostListRef* p_pList);
	static StatusCode s_GetEncoderSettings(HostPropertyCollectionRef* p_pValues, HostListRef* p_pSettingsList);

	virtual bool IsNeedNextPass() override
	{
		return (m_IsMultiPass && (m_PassesDone < 2));
	}

	virtual bool IsAcceptingFrame(int64_t p_PTS) override
	{
		// accepts every frame in multipass, PTS is the frame number in track fps
		// return false after all passes finished
		return (m_IsMultiPass && (m_PassesDone < 3));
	}

protected:
	virtual void DoFlush() override;
	virtual StatusCode DoInit(HostPropertyCollectionRef* p_pProps) override;
	virtual StatusCode DoOpen(HostBufferRef* p_pBuff) override;
	virtual StatusCode DoProcess(HostBufferRef* p_pBuff) override;

private:
	void SetupContext(bool p_IsFinalPass);
	std::string X265Encoder::ConvertUINT8ToHexStr(const uint8_t* v, const size_t s);

private:
	x265_encoder* m_pContext;
	x265_param* m_pParam;
	int m_ColorModel;
	std::string m_sStatFileName;
	std::unique_ptr<UISettingsController> m_pSettings;
	HostCodecConfigCommon m_CommonProps;

	bool m_IsMultiPass;
	uint64_t m_FramesSubmitted;
	uint64_t m_FramesWritten;
	uint32_t m_PassesDone;
	StatusCode m_Error;

};
