#include "plugin.h"

#include <assert.h>

#include <cstring>

#include "x265_encoder.h"

// NOTE: When creating a plugin for release, please generate a new Plugin UUID in order to prevent conflicts with other third-party plugins.
static const uint8_t pMyUUID[] = { 0x5c, 0x43, 0xce, 0x60, 0x45, 0x11, 0x4f, 0x58, 0x87, 0xde, 0xf3, 0x02, 0x80, 0x1e, 0x7b, 0xbd };

using namespace IOPlugin;

StatusCode g_HandleGetInfo(HostPropertyCollectionRef* p_pProps)
{
    StatusCode err = p_pProps->SetProperty(pIOPropUUID, propTypeUInt8, pMyUUID, 16);
    if (err == errNone)
    {
        err = p_pProps->SetProperty(pIOPropName, propTypeString, "Sample Plugin", strlen("Sample Plugin"));
    }

    return err;
}

StatusCode g_HandleCreateObj(unsigned char* p_pUUID, ObjectRef* p_ppObj)
{
    if (memcmp(p_pUUID, X265Encoder::s_UUID, 16) == 0)
    {
        *p_ppObj = new X265Encoder();
        return errNone;
    }

    return errUnsupported;
}

StatusCode g_HandlePluginStart()
{
    // perform libs initialization if needed
    return errNone;
}

StatusCode g_HandlePluginTerminate()
{
    return errNone;
}

StatusCode g_ListCodecs(HostListRef* p_pList)
{
    StatusCode err = X265Encoder::s_RegisterCodecs(p_pList);
    if (err != errNone)
    {
        return err;
    }

    return errNone;
}

StatusCode g_ListContainers(HostListRef* p_pList)
{
    return errNone;

}

StatusCode g_GetEncoderSettings(unsigned char* p_pUUID, HostPropertyCollectionRef* p_pValues, HostListRef* p_pSettingsList)
{
    if (memcmp(p_pUUID, X265Encoder::s_UUID, 16) == 0)
    {
        return X265Encoder::s_GetEncoderSettings(p_pValues, p_pSettingsList);
    }

    return errNoCodec;
}
