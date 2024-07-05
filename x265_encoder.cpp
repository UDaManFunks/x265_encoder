#include "x265_encoder.h"

#include <assert.h>
#include <cstring>
#include <vector>
#include <stdint.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>

#include "x265.h"

const uint8_t X265Encoder::s_UUID[] = { 0x6a, 0x88, 0xe8, 0x41, 0xd8, 0xe4, 0x41, 0x4b, 0x87, 0x9e, 0xa4, 0x80, 0xfc, 0x90, 0xda, 0xb5 };

class UISettingsController
{
public:
	UISettingsController()
	{
		InitDefaults();
	}

	explicit UISettingsController(const HostCodecConfigCommon& p_CommonProps)
		: m_CommonProps(p_CommonProps)
	{
		InitDefaults();
	}

	~UISettingsController()
	{
	}

	void Load(IPropertyProvider* p_pValues)
	{
		uint8_t val8 = 0;
		p_pValues->GetUINT8("x265_reset", val8);
		if (val8 != 0) {
			*this = UISettingsController();
			return;
		}

		p_pValues->GetINT32("x265_enc_preset", m_EncPreset);
		p_pValues->GetINT32("x265_tune", m_Tune);
		p_pValues->GetINT32("x265_profile", m_Profile);
		p_pValues->GetINT32("x265_num_passes", m_NumPasses);
		p_pValues->GetINT32("x265_q_mode", m_QualityMode);
		p_pValues->GetINT32("x265_qp", m_QP);
		p_pValues->GetINT32("x265_bitrate", m_BitRate);
		p_pValues->GetString("x265_enc_markers", m_MarkerColor);
	}

	StatusCode Render(HostListRef* p_pSettingsList)
	{
		StatusCode err = RenderGeneral(p_pSettingsList);
		if (err != errNone) {
			return err;
		}

		{
			HostUIConfigEntryRef item("x265_separator");
			item.MakeSeparator();
			if (!item.IsSuccess() || !p_pSettingsList->Append(&item)) {
				g_Log(logLevelError, "X265 Plugin :: Failed to add a separator entry");
				return errFail;
			}
		}

		err = RenderQuality(p_pSettingsList);
		if (err != errNone) {
			return err;
		}

		{
			HostUIConfigEntryRef item("x265_reset");
			item.MakeButton("Reset");
			item.SetTriggersUpdate(true);
			if (!item.IsSuccess() || !p_pSettingsList->Append(&item)) {
				g_Log(logLevelError, "X265 Plugin :: Failed to populate the button entry");
				return errFail;
			}
		}

		return errNone;
	}

private:
	void InitDefaults()
	{
		m_EncPreset = 4;
		m_Tune = -1;
		m_Profile = 0;
		m_NumPasses = 1;
		m_QualityMode = X265_RC_CRF;
		m_QP = 25;
		m_BitRate = 0;
	}

	StatusCode RenderGeneral(HostListRef* p_pSettingsList)
	{
		if (0) {
			HostUIConfigEntryRef item("x265_lbl_general");
			item.MakeLabel("General Settings");

			if (!item.IsSuccess() || !p_pSettingsList->Append(&item)) {
				g_Log(logLevelError, "X265 Plugin :: Failed to populate general label entry");
				return errFail;
			}
		}

		// Markers selection
		if (m_CommonProps.GetContainer().size() >= 32) {
			HostUIConfigEntryRef item("x265_enc_markers");
			item.MakeMarkerColorSelector("Chapter Marker", "Marker 1", m_MarkerColor);
			if (!item.IsSuccess() || !p_pSettingsList->Append(&item)) {
				g_Log(logLevelError, "X265 Plugin :: Failed to populate encoder preset UI entry");
				assert(false);
				return errFail;
			}
		}

		// Preset combobox
		{
			HostUIConfigEntryRef item("x265_enc_preset");

			std::vector<std::string> textsVec;
			std::vector<int> valuesVec;

			int32_t curVal = 1;
			const char* const* pPresets = x265_preset_names;
			while (*pPresets != 0) {
				valuesVec.push_back(curVal++);
				textsVec.push_back(*pPresets);
				++pPresets;
			}

			item.MakeComboBox("Encoder Preset", textsVec, valuesVec, m_EncPreset);
			if (!item.IsSuccess() || !p_pSettingsList->Append(&item)) {
				g_Log(logLevelError, "X265 Plugin :: Failed to populate encoder preset UI entry");
				return errFail;
			}
		}

		// Tune combobox
		{
			HostUIConfigEntryRef item("x265_tune");

			std::vector<std::string> textsVec;
			std::vector<int> valuesVec;

			int32_t curVal = 0;

			// default None
			valuesVec.push_back(curVal++ + 0);
			textsVec.push_back("none");

			const char* const* pPresets = x265_tune_names;
			while (*pPresets != 0) {
				valuesVec.push_back(curVal++);
				textsVec.push_back(*pPresets);
				++pPresets;
			}

			item.MakeComboBox("Encoder Tune", textsVec, valuesVec, m_Tune);
			if (!item.IsSuccess() || !p_pSettingsList->Append(&item)) {
				g_Log(logLevelError, "X265 Plugin :: Failed to populate tune UI entry");
				return errFail;
			}
		}

		// Profile combobox
		{
			HostUIConfigEntryRef item("x265_profile");

			std::vector<std::string> textsVec;
			std::vector<int> valuesVec;

			valuesVec.push_back(0);
			textsVec.push_back("main");

			// valuesVec.push_back(1);
			// textsVec.push_back("main10");

			item.MakeComboBox("Encoder Profile", textsVec, valuesVec, m_Profile);
			if (!item.IsSuccess() || !p_pSettingsList->Append(&item)) {
				g_Log(logLevelError, "X265 Plugin :: Failed to populate profile UI entry");
				return errFail;
			}
		}

		return errNone;
	}

	StatusCode RenderQuality(HostListRef* p_pSettingsList)
	{
		if (0) {
			HostUIConfigEntryRef item("x265_lbl_quality");
			item.MakeLabel("Quality Settings");

			if (!item.IsSuccess() || !p_pSettingsList->Append(&item)) {
				g_Log(logLevelError, "X265 Plugin :: Failed to populate quality label entry");
				return errFail;
			}
		}

		{
			HostUIConfigEntryRef item("x265_num_passes");

			std::vector<std::string> textsVec;
			std::vector<int> valuesVec;

			textsVec.push_back("1-Pass");
			valuesVec.push_back(1);
			textsVec.push_back("2-Pass");
			valuesVec.push_back(2);

			item.MakeComboBox("Passes", textsVec, valuesVec, m_NumPasses);
			item.SetTriggersUpdate(true);
			if (!item.IsSuccess() || !p_pSettingsList->Append(&item)) {
				g_Log(logLevelError, "X265 Plugin :: Failed to populate passes UI entry");
				return errFail;
			}
		}

		if (m_NumPasses < 2) {
			HostUIConfigEntryRef item("x265_q_mode");

			std::vector<std::string> textsVec;
			std::vector<int> valuesVec;

			textsVec.push_back("Constant Rate Factor");
			valuesVec.push_back(X265_RC_CRF);

			textsVec.push_back("Average Bitrate");
			valuesVec.push_back(X265_RC_ABR);

			item.MakeRadioBox("Quality Control", textsVec, valuesVec, GetQualityMode());
			item.SetTriggersUpdate(true);

			if (!item.IsSuccess() || !p_pSettingsList->Append(&item)) {
				g_Log(logLevelError, "X265 Plugin :: Failed to populate quality UI entry");
				return errFail;
			}
		}

		{
			HostUIConfigEntryRef item("x265_qp");
			const char* pLabel = NULL;
			if (m_QP < 17) {
				pLabel = "(high)";
			} else if (m_QP < 34) {
				pLabel = "(medium)";
			} else {
				pLabel = "(low)";
			}
			item.MakeSlider("Factor", pLabel, m_QP, 1, 51, 25);
			item.SetTriggersUpdate(true);
			item.SetHidden((m_QualityMode == X265_RC_ABR) || (m_NumPasses > 1));
			if (!item.IsSuccess() || !p_pSettingsList->Append(&item)) {
				g_Log(logLevelError, "X265 Plugin :: Failed to populate qp slider UI entry");
				return errFail;
			}
		}

		{
			HostUIConfigEntryRef item("x265_bitrate");
			item.MakeSlider("Bit Rate", "Kbps", m_BitRate, 8000, 100000, 1);
			item.SetHidden((m_QualityMode != X265_RC_ABR) && (m_NumPasses < 2));

			if (!item.IsSuccess() || !p_pSettingsList->Append(&item)) {
				g_Log(logLevelError, "X265 Plugin :: Failed to populate bitrate slider UI entry");
				return errFail;
			}
		}

		return errNone;
	}

public:
	int32_t GetNumPasses()
	{
		return m_NumPasses;
	}

	const char* GetEncPreset() const
	{
		return x265_preset_names[m_EncPreset];
	}

	const char* GetTune() const
	{

		if (m_Tune > 0) {
			return x265_tune_names[(m_Tune - 1)];
		}

		return NULL;
	}

	const char* GetProfile() const
	{
		return x265_profile_names[m_Profile];
	}

	int32_t GetQualityMode() const
	{
		return (m_NumPasses == 2) ? X265_RC_ABR : m_QualityMode;
	}

	int32_t GetQP() const
	{
		return std::max<int>(0, m_QP);
	}

	int32_t GetBitRate() const
	{
		return m_BitRate;
	}

	int32_t GetBitDepth() const
	{
		return m_Profile == 0 ? 8 : 10;
	}

	const std::string& GetMarkerColor() const
	{
		return m_MarkerColor;
	}

private:
	HostCodecConfigCommon m_CommonProps;
	std::string m_MarkerColor;
	int32_t m_EncPreset;
	int32_t m_Tune;
	int32_t m_Profile;

	int32_t m_NumPasses;
	int32_t m_QualityMode;
	int32_t m_QP;
	int32_t m_BitRate;
};

StatusCode X265Encoder::s_GetEncoderSettings(HostPropertyCollectionRef* p_pValues, HostListRef* p_pSettingsList)
{
	HostCodecConfigCommon commonProps;
	commonProps.Load(p_pValues);

	UISettingsController settings(commonProps);
	settings.Load(p_pValues);

	return settings.Render(p_pSettingsList);
}

StatusCode X265Encoder::s_RegisterCodecs(HostListRef* p_pList)
{
	HostPropertyCollectionRef codecInfo;
	if (!codecInfo.IsValid()) {
		return errAlloc;
	}

	codecInfo.SetProperty(pIOPropUUID, propTypeUInt8, X265Encoder::s_UUID, 16);

	const char* pCodecName = "Auto";
	codecInfo.SetProperty(pIOPropName, propTypeString, pCodecName, strlen(pCodecName));

	const char* pCodecGroup = "X265 (8-bit)";
	codecInfo.SetProperty(pIOPropGroup, propTypeString, pCodecGroup, strlen(pCodecGroup));

	uint32_t vFourCC = 'hvc1';
	codecInfo.SetProperty(pIOPropFourCC, propTypeUInt32, &vFourCC, 1);

	uint32_t vMediaVideo = mediaVideo;
	codecInfo.SetProperty(pIOPropMediaType, propTypeUInt32, &vMediaVideo, 1);

	uint32_t vDirection = dirEncode;
	codecInfo.SetProperty(pIOPropCodecDirection, propTypeUInt32, &vDirection, 1);

	uint32_t vColorModel = clrUYVY;
	codecInfo.SetProperty(pIOPropColorModel, propTypeUInt32, &vColorModel, 1);

	uint8_t hSampling = 2;
	uint8_t vSampling = 1;
	codecInfo.SetProperty(pIOPropHSubsampling, propTypeUInt8, &hSampling, 1);
	codecInfo.SetProperty(pIOPropVSubsampling, propTypeUInt8, &vSampling, 1);

	// Optionally enable both Data Ranges, Video will be default for "Auto" thus "0" value goes first
	std::vector<uint8_t> dataRangeVec;
	dataRangeVec.push_back(0);
	dataRangeVec.push_back(1);
	codecInfo.SetProperty(pIOPropDataRange, propTypeUInt8, dataRangeVec.data(), dataRangeVec.size());

	uint32_t vBitDepth = 8;
	codecInfo.SetProperty(pIOPropBitDepth, propTypeUInt32, &vBitDepth, 1);

	vBitDepth = 8;
	codecInfo.SetProperty(pIOPropBitsPerSample, propTypeUInt32, &vBitDepth, 1);

	const uint32_t temp = 0;
	codecInfo.SetProperty(pIOPropTemporalReordering, propTypeUInt32, &temp, 1);

	const uint8_t fieldSupport = (fieldProgressive | fieldTop | fieldBottom);
	codecInfo.SetProperty(pIOPropFieldOrder, propTypeUInt8, &fieldSupport, 1);

	std::vector<std::string> containerVec;
	containerVec.push_back("mp4");
	containerVec.push_back("mov");
	std::string valStrings;
	for (size_t i = 0; i < containerVec.size(); ++i) {
		valStrings.append(containerVec[i]);
		if (i < (containerVec.size() - 1)) {
			valStrings.append(1, '\0');
		}
	}

	codecInfo.SetProperty(pIOPropContainerList, propTypeString, valStrings.c_str(), valStrings.size());

	if (!p_pList->Append(&codecInfo)) {
		return errFail;
	}

	return errNone;
}

X265Encoder::X265Encoder()
	: m_pContext(0)
	, m_ColorModel(-1)
	, m_IsMultiPass(false)
	, m_PassesDone(0)
	, m_Error(errNone)
{
}

X265Encoder::~X265Encoder()
{
	if (m_pContext) {
		x265_encoder_close(m_pContext);
		x265_param_free(m_pParam);
		m_pContext = 0;
		m_pParam = 0;
	}
}

void X265Encoder::DoFlush()
{

	g_Log(logLevelInfo, "X265 Plugin :: DoFlush");

	if (m_Error != errNone) {
		return;
	}

	StatusCode sts = DoProcess(NULL);
	while (sts == errNone) {
		sts = DoProcess(NULL);
	}

	++m_PassesDone;

	if (!m_IsMultiPass || (m_PassesDone > 1)) {
		return;
	}

	if (m_PassesDone == 1) {
		SetupContext(true /* isFinalPass */);
	}
}

StatusCode X265Encoder::DoInit(HostPropertyCollectionRef* p_pProps)
{
	g_Log(logLevelInfo, "X265 Plugin :: DoInit");

	uint32_t vColorModel = clrUYVY;
	p_pProps->SetProperty(pIOPropColorModel, propTypeUInt32, &vColorModel, 1);

	uint8_t hSampling = 2;
	uint8_t vSampling = 1;
	p_pProps->SetProperty(pIOPropHSubsampling, propTypeUInt8, &hSampling, 1);
	p_pProps->SetProperty(pIOPropVSubsampling, propTypeUInt8, &vSampling, 1);


	return errNone;
}

void X265Encoder::SetupContext(bool p_IsFinalPass)
{

	std::string logMessagePrefix = "X265 Plugin :: SetupContext :: ";
	std::ostringstream logMessage;

	{
		// debug only, to DELETE

		logMessage << logMessagePrefix << "p_isFinalPass = " << p_IsFinalPass;
		g_Log(logLevelInfo, logMessage.str().c_str());
	}

	if (m_pContext) {
		g_Log(logLevelInfo, "X265 Plugin :: SetupContext :: cleaning m_pContext, m_pParam, and env");
		x265_encoder_close(m_pContext);
		x265_param_free(m_pParam);
		x265_cleanup();
		m_pContext = 0;
		m_pParam = 0;
	}

	m_pParam = x265_param_alloc();

	const char* pProfile = m_pSettings->GetProfile();
	m_ColorModel = X265_CSP_I420;

	if (x265_param_default_preset(m_pParam, m_pSettings->GetEncPreset(), m_pSettings->GetTune()) != 0) {
		g_Log(logLevelInfo, "X265 Plugin :: SetupContext :: setting x265 param default presets failed");
		m_Error = errFail;
		return;
	}

	m_pParam->internalCsp = m_ColorModel;
	m_pParam->sourceWidth = m_CommonProps.GetWidth();
	m_pParam->sourceHeight = m_CommonProps.GetHeight();
	m_pParam->sourceBitDepth = m_pSettings->GetBitDepth();
	m_pParam->fpsNum = m_CommonProps.GetFrameRateNum();
	m_pParam->fpsDenom = m_CommonProps.GetFrameRateDen();
	m_pParam->vui.bEnableVideoFullRangeFlag = m_CommonProps.IsFullRange();
	m_pParam->rc.rateControlMode = m_pSettings->GetQualityMode();

	if (!m_IsMultiPass && (m_pParam->rc.rateControlMode != X265_RC_ABR)) {
		const int qp = m_pSettings->GetQP();

		m_pParam->rc.qp = qp;
		m_pParam->rc.rfConstant = std::min<int>(50, qp);
		m_pParam->rc.rfConstantMax = std::min<int>(51, qp + 5);
	} else if (m_pParam->rc.rateControlMode == X265_RC_ABR) {
		m_pParam->rc.bitrate = m_pSettings->GetBitRate();
		m_pParam->rc.vbvBufferSize = m_pSettings->GetBitRate();
		m_pParam->rc.vbvMaxBitrate = m_pSettings->GetBitRate();
	}

	if (m_IsMultiPass) {
		if (p_IsFinalPass && (m_PassesDone > 0)) {
			m_pParam->rc.bStatRead = 1;
			m_pParam->rc.bStatWrite = 0;
		} else if (!p_IsFinalPass) {
			m_pParam->rc.bStatRead = 0;
			m_pParam->rc.bStatWrite = 1;
		}

		m_pParam->rc.statFileName = &m_sTmpFileName[0];
	}

	if (pProfile != NULL) {
		if (x265_param_apply_profile(m_pParam, pProfile) != 0) {
			m_Error = errFail;
			return;
		}
	}

	m_pContext = x265_encoder_open(m_pParam);

	m_Error = ((m_pContext != NULL) ? errNone : errFail);

	{
		// debug only, to DELETE

		logMessage.str("");
		logMessage.clear();
		logMessage << logMessagePrefix << " m_pContext = " << m_pContext << " :: m_Error = " << m_Error;
		g_Log(logLevelInfo, logMessage.str().c_str());

	}


}

StatusCode X265Encoder::DoOpen(HostBufferRef* p_pBuff)
{

	std::string logMessagePrefix = "X265 Plugin :: DoOpen :: ";
	std::ostringstream logMessage;

	{
		// debug only, to DELETE

		logMessage << logMessagePrefix;
		g_Log(logLevelInfo, logMessage.str().c_str());

	}

	assert(m_pContext == NULL);

	m_CommonProps.Load(p_pBuff);

	const std::string& path = m_CommonProps.GetPath();
	if (!path.empty()) {
		m_sTmpFileName = path;
		m_sTmpFileName.append(".pass.log");
	} else {
		m_sTmpFileName = "/tmp/x265_multipass.log";
	}

	m_pSettings.reset(new UISettingsController(m_CommonProps));
	m_pSettings->Load(p_pBuff);

	uint8_t isMultiPass = 0;
	if (m_pSettings->GetNumPasses() == 2) {
		m_IsMultiPass = true;
		isMultiPass = 1;
	}

	uint8_t vBitDepth = m_pSettings->GetBitDepth();
	p_pBuff->SetProperty(pIOPropBitDepth, propTypeUInt32, &vBitDepth, 1);
	vBitDepth = m_pSettings->GetBitDepth();
	p_pBuff->SetProperty(pIOPropBitsPerSample, propTypeUInt32, &vBitDepth, 1);

	{
		// debug only, to DELETE

		logMessage.str("");
		logMessage.clear();
		logMessage << logMessagePrefix << " bitDepth = " << m_pSettings->GetBitDepth();
		g_Log(logLevelInfo, logMessage.str().c_str());
	}

	StatusCode sts = p_pBuff->SetProperty(pIOPropMultiPass, propTypeUInt8, &isMultiPass, 1);
	if (sts != errNone) {
		return sts;
	}

	SetupContext(true);
	if (m_Error != errNone) {
		return m_Error;
	}

	x265_nal* pNals;
	uint32_t numNals = 0;
	int hdrBytes = x265_encoder_headers(m_pContext, &pNals, &numNals);

	if (hdrBytes > 0) {

		{
			logMessage.str("");
			logMessage.clear();
			logMessage << logMessagePrefix;
			logMessage << " hdrBytes = " << hdrBytes;

			g_Log(logLevelInfo, logMessage.str().c_str());

			logMessage << " :: numNals = ";
			logMessage << numNals;

			g_Log(logLevelInfo, logMessage.str().c_str());
		}

		std::vector<uint8_t> cookie;

		/**
		*
		* the following code copies NAL payload data until it reaches SEI looks like the order is VPS -> SPS -> PPS -> (STOP @ SEI)
		*
		*/

		for (int i = 0; i < numNals; i++) {

			if (pNals[i].type == NAL_UNIT_PREFIX_SEI) {
				g_Log(logLevelInfo, "X265 Plugin :: DoOpen :: continue");
				continue;
			}

			pNals[i].payload[0] = 0;
			pNals[i].payload[1] = 0;
			pNals[i].payload[2] = 0;
			pNals[i].payload[3] = 1;

			cookie.insert(cookie.end(), pNals[i].payload, pNals[i].payload + pNals[i].sizeBytes);

		}

		if (!cookie.empty()) {

			{

				// debug only, to DELETE

				logMessage.str("");
				logMessage.clear();
				logMessage << logMessagePrefix;
				logMessage << " cookie size = " << cookie.size();
				g_Log(logLevelInfo, logMessage.str().c_str());
			}

			p_pBuff->SetProperty(pIOPropMagicCookie, propTypeUInt8, &cookie[0], cookie.size());
			uint32_t fourCC = 0;
			p_pBuff->SetProperty(pIOPropMagicCookieType, propTypeUInt32, &fourCC, 1);
		}
	}

	uint32_t temporal = 2;
	p_pBuff->SetProperty(pIOPropTemporalReordering, propTypeUInt32, &temporal, 1);

	if (isMultiPass) {
		SetupContext(false /* isFinalPass */);
		if (m_Error != errNone) {
			return m_Error;
		}
	}

	return errNone;
}

std::string X265Encoder::ConvertUINT8ToHexStr(const uint8_t* v, const size_t s) {

	std::stringstream ss;

	ss << std::hex << std::setfill('0');

	for (int i = 0; i < s; i++) {
		ss << std::hex << std::setw(2) << static_cast<int>(v[i]);
	}

	return ss.str();
}

StatusCode X265Encoder::DoProcess(HostBufferRef* p_pBuff)
{
	std::string logMessagePrefix = "X265 Plugin :: DoOpen :: ";
	std::ostringstream logMessage;

	{
		// debug only, to DELETE

		logMessage << logMessagePrefix << " :: p_pBuff = " << p_pBuff << " :: m_Error = " << m_Error << " :: m_ColorModel = " << m_ColorModel;
		g_Log(logLevelInfo, logMessage.str().c_str());
	}

	if (m_Error != errNone) {
		return m_Error;
	}

	/*
	const int numDelayedFrames = x265_encoder_delayed_frames(m_pContext);
	if (((p_pBuff == NULL) || !p_pBuff->IsValid()) && (numDelayedFrames == 0))
	{
		return errMoreData;
	}

	*/

	x265_picture outPic;

	g_Log(logLevelInfo, "X265 Plugin :: DoProcess :: x265_picture_init (outPic)");
	x265_picture_init(m_pParam, &outPic);

	x265_nal* pNals = 0;
	uint32_t numNals = 0;
	int bytes = 0;
	int encoderRet = 0;
	int64_t pts = -1;

	if (!p_pBuff->IsValid()) {
		g_Log(logLevelInfo, "X265 Plugin :: DoProcess :: p_pBuff is NOT VALID");
	}

	if (p_pBuff == NULL || !p_pBuff->IsValid()) {

		g_Log(logLevelInfo, "X265 Plugin :: DoProcess :: flushing?");
		encoderRet = x265_encoder_encode(m_pContext, &pNals, &numNals, 0, &outPic);

	} else {

		g_Log(logLevelInfo, "X265 Plugin :: DoProcess :: trying");

		char* pBuf = NULL;
		size_t bufSize = 0;
		if (!p_pBuff->LockBuffer(&pBuf, &bufSize)) {
			g_Log(logLevelError, "X265 Plugin :: Failed to lock the buffer");
			return errFail;
		}

		if (pBuf == NULL || bufSize == 0) {
			g_Log(logLevelError, "X265 Plugin :: No data to encode");
			p_pBuff->UnlockBuffer();
			return errUnsupported;
		}

		uint32_t width = 0;
		uint32_t height = 0;

		if (!p_pBuff->GetUINT32(pIOPropWidth, width) || !p_pBuff->GetUINT32(pIOPropHeight, height)) {
			g_Log(logLevelError, "X265 Plugin :: Width/Height not set when encoding the frame");
			return errNoParam;
		}

		if (!p_pBuff->GetINT64(pIOPropPTS, pts)) {
			g_Log(logLevelError, "X265 Plugin :: PTS not set when encoding the frame");
			return errNoParam;
		}

		g_Log(logLevelInfo, "X265 Plugin :: DoProcess :: inPic");

		x265_picture inPic;

		g_Log(logLevelInfo, "X265 Plugin :: DoProcess :: x265_picture_init (inPic)");
		x265_picture_init(m_pParam, &inPic);

		// UYVY > I420

		uint8_t* pSrc = reinterpret_cast<uint8_t*>(const_cast<char*>(pBuf));

		int ySize = width * height;

		std::vector<uint8_t> yPlane;
		yPlane.reserve(ySize);
		std::vector<uint8_t> uPlane;
		uPlane.reserve(ySize / 4);
		std::vector<uint8_t> vPlane;
		vPlane.reserve(ySize / 4);

		for (int h = 0; h < height; h++) {

			for (int w = 0; w < width; w += 2) {

				yPlane.push_back(pSrc[1]);
				yPlane.push_back(pSrc[3]);

				if ((h % 2) == 0) {
					uPlane.push_back(pSrc[0]);
					vPlane.push_back(pSrc[2]);
				}

				pSrc += 4;
			}
		}

		inPic.pts = pts;
		inPic.planes[0] = yPlane.data();
		inPic.planes[1] = uPlane.data();
		inPic.planes[2] = vPlane.data();
		inPic.stride[0] = width;
		inPic.stride[1] = width / 2;
		inPic.stride[2] = width / 2;

		encoderRet = x265_encoder_encode(m_pContext, &pNals, &numNals, &inPic, &outPic);

		{

			// debug only, to DELETE

			logMessage.str("");
			logMessage.clear();
			logMessage << logMessagePrefix << " encoderRet = " << encoderRet << " :: bytes = " << bytes << " :: ySize = " << yPlane.size() << " :: uSize = " << uPlane.size() << " :: vSize = " << vPlane.size();
			g_Log(logLevelInfo, logMessage.str().c_str());

		}

		p_pBuff->UnlockBuffer();


		g_Log(logLevelInfo, "X265 Plugin :: DoOpen :: encoded a frame");

	}

	if (encoderRet == -1) {
		return errFail;
	} else if (encoderRet == 0) {
		return errMoreData;
	} else if (m_IsMultiPass && (m_PassesDone == 0)) {
		return errNone;
	}

	bytes = pNals[0].sizeBytes;

	HostBufferRef outBuf(false);
	if (!outBuf.IsValid() || !outBuf.Resize(bytes)) {
		return errAlloc;
	}

	char* pOutBuf = NULL;
	size_t outBufSize = 0;
	if (!outBuf.LockBuffer(&pOutBuf, &outBufSize)) {
		return errAlloc;
	}

	assert(outBufSize == bytes);

	memcpy(pOutBuf, pNals[0].payload, bytes);

	int64_t ts = outPic.pts;
	outBuf.SetProperty(pIOPropPTS, propTypeInt64, &ts, 1);

	ts = outPic.dts;
	outBuf.SetProperty(pIOPropDTS, propTypeInt64, &ts, 1);

	uint8_t isKeyFrame = IS_X265_TYPE_I(outPic.sliceType) ? 1 : 0;
	outBuf.SetProperty(pIOPropIsKeyFrame, propTypeUInt8, &isKeyFrame, 1);

	return m_pCallback->SendOutput(&outBuf);
}
