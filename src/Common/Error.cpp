#include "pch.h"
#include "JunoInternal.h"

static thread_local JunoErrorCode sLastCode = JunoErrorCode_Ok;
static thread_local HRESULT sLastHResult = S_OK;
static thread_local char sLastMessage[4096] = "";

JunoErrorCode JunoSetError(JunoErrorCode code, HRESULT hr, const char* pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	vsnprintf(sLastMessage, sizeof(sLastMessage), pFormat, args);
	va_end(args);

	if (FAILED(hr))
	{
		size_t length = strlen(sLastMessage);
		snprintf(sLastMessage + length, sizeof(sLastMessage) - length, " (HRESULT 0x%08X)", (unsigned int)hr);
	}

	sLastCode = code;
	sLastHResult = hr;
	return code;
}

JunoErrorCode JunoFailHResult(JunoErrorCode fallback, HRESULT hr, const char* pWhat)
{
	JunoErrorCode code = (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) ? JunoErrorCode_DeviceLost : fallback;
	return JunoSetError(code, hr, "%s failed", pWhat);
}

JUNO_API const c_char* JunoGetLastErrorMessage(void)
{
	return sLastMessage;
}

JUNO_API JunoErrorCode JunoGetLastErrorCode(void)
{
	return sLastCode;
}

JUNO_API c_int JunoGetLastErrorHResult(void)
{
	return (c_int)sLastHResult;
}

JUNO_API void JunoClearLastError(void)
{
	sLastCode = JunoErrorCode_Ok;
	sLastHResult = S_OK;
	sLastMessage[0] = '\0';
}
