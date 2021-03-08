#include "pch.h"
#include "HL2RmUnityPlugin.h"

#define DBG_ENABLE_VERBOSE_LOGGING 1
#define DBG_ENABLE_INFO_LOGGING 1

extern "C"
HMODULE LoadLibraryA(
	LPCSTR lpLibFileName
);

static ResearchModeSensorConsent camAccessCheck;
static HANDLE camConsentGiven;
static ResearchModeSensorConsent imuAccessCheck;
static HANDLE imuConsentGiven;


int __stdcall HL2Stream::GetAccelSample(OUTPUT* output)
{
	HRESULT hr = S_OK;
	DirectX::XMFLOAT3 accelSample;

	m_pAccelProcessor->GetAccelSample(&accelSample);
	output->x = accelSample.x;
	output->y = accelSample.y;
	output->z = accelSample.z;
	return 200;
}

int __stdcall HL2Stream::GetGyroSample(OUTPUT* output)
{
	HRESULT hr = S_OK;
	DirectX::XMFLOAT3 gyroSample;

	m_pGyroProcessor->GetGyroSample(&gyroSample);
	output->x = gyroSample.x;
	output->y = gyroSample.y;
	output->z = gyroSample.z;
	return 200;
}

int __stdcall HL2Stream::GetMagSample(OUTPUT* output)
{
	HRESULT hr = S_OK;
	DirectX::XMFLOAT3 magSample;

	m_pMagProcessor->GetMagSample(&magSample);
	output->x = magSample.x;
	output->y = magSample.y;
	output->z = magSample.z;
	return 200;
}

void __stdcall HL2Stream::StartStreaming()
{
#if DBG_ENABLE_INFO_LOGGING
	OutputDebugStringW(L"HL2Stream::StartStreaming: Initializing...\n");
#endif

	InitializeResearchModeSensors();
	InitializeResearchModeProcessing();

#if DBG_ENABLE_INFO_LOGGING
	OutputDebugStringW(L"HL2Stream::StartStreaming: Done.\n");
#endif
}

void HL2Stream::InitializeResearchModeSensors()
{
	HRESULT hr = S_OK;
	size_t sensorCount = 0;
	camConsentGiven = CreateEvent(nullptr, true, false, nullptr);

	// Load research mode library
	HMODULE hrResearchMode = LoadLibraryA("ResearchModeAPI");
	if (hrResearchMode)
	{
#if DBG_ENABLE_VERBOSE_LOGGING
		OutputDebugStringW(L"Image2Face::InitializeSensors: Creating sensor device...\n");
#endif
		// create the research mode sensor device
		typedef HRESULT(__cdecl* PFN_CREATEPROVIDER) (IResearchModeSensorDevice** ppSensorDevice);
		PFN_CREATEPROVIDER pfnCreate = reinterpret_cast<PFN_CREATEPROVIDER>
			(GetProcAddress(hrResearchMode, "CreateResearchModeSensorDevice"));
		if (pfnCreate)
		{
			winrt::check_hresult(pfnCreate(&m_pSensorDevice));
		}
		else
		{
			winrt::check_hresult(E_INVALIDARG);
		}
	}

	// manage consent
	winrt::check_hresult(m_pSensorDevice->QueryInterface(IID_PPV_ARGS(&m_pSensorDeviceConsent)));
	winrt::check_hresult(m_pSensorDeviceConsent->RequestCamAccessAsync(CamAccessOnComplete));
	winrt::check_hresult(m_pSensorDeviceConsent->RequestIMUAccessAsync(ImuAccessOnComplete));

	m_pSensorDevice->DisableEyeSelection();

	winrt::check_hresult(m_pSensorDevice->GetSensorCount(&sensorCount));
	m_sensorDescriptors.resize(sensorCount);

	winrt::check_hresult(m_pSensorDevice->GetSensorDescriptors(m_sensorDescriptors.data(),
		m_sensorDescriptors.size(), &sensorCount));

	for (const auto& sensorDescriptor : m_sensorDescriptors)
	{
		if (sensorDescriptor.sensorType == IMU_ACCEL)
		{
			winrt::check_hresult(m_pSensorDevice->GetSensor(sensorDescriptor.sensorType, &m_pAccelSensor));
		}
		if (sensorDescriptor.sensorType == IMU_GYRO)
		{
			winrt::check_hresult(m_pSensorDevice->GetSensor(sensorDescriptor.sensorType, &m_pGyroSensor));
		}
		if (sensorDescriptor.sensorType == IMU_MAG)
		{
			winrt::check_hresult(m_pSensorDevice->GetSensor(sensorDescriptor.sensorType, &m_pMagSensor));
		}
	}
	OutputDebugStringW(L"Image2Face::InitializeSensors: Done.\n");
	return;
}

void HL2Stream::InitializeResearchModeProcessing()
{
	if (m_pAccelSensor)
	{
		m_pAccelProcessor = std::make_shared<AccelProcessor>(m_pAccelSensor, imuConsentGiven, &imuAccessCheck);
	}
	if (m_pGyroSensor)
	{
		m_pGyroProcessor = std::make_shared<GyroProcessor>(m_pGyroSensor, imuConsentGiven, &imuAccessCheck);
	}
	if (m_pMagSensor)
	{
		m_pMagProcessor = std::make_shared<MagProcessor>(m_pMagSensor, imuConsentGiven, &imuAccessCheck);
	}
}

void HL2Stream::CamAccessOnComplete(ResearchModeSensorConsent consent)
{
	camAccessCheck = consent;
	SetEvent(camConsentGiven);
}

void HL2Stream::ImuAccessOnComplete(ResearchModeSensorConsent consent)
{
	imuAccessCheck = consent;
	SetEvent(imuConsentGiven);
}

void HL2Stream::DisableSensors()
{
#if DBG_ENABLE_VERBOSE_LOGGING
	OutputDebugString(L"Image2Face::DisableSensors: Disabling sensors...\n");
#endif // DBG_ENABLE_VERBOSE_LOGGING
	if (m_pAccelSensor)
	{
		m_pAccelSensor->Release();
	}
	if (m_pGyroSensor)
	{
		m_pGyroSensor->Release();
	}
	if (m_pMagSensor)
	{
		m_pMagSensor->Release();
	}
	if (m_pSensorDevice)
	{
		m_pSensorDevice->EnableEyeSelection();
		m_pSensorDevice->Release();
	}
	if (m_pSensorDeviceConsent)
	{
		m_pSensorDeviceConsent->Release();
	}
#if DBG_ENABLE_VERBOSE_LOGGING
	OutputDebugString(L"Image2Face::DisableSensors: Done.\n");
#endif // DBG_ENABLE_VERBOSE_LOGGING
}
