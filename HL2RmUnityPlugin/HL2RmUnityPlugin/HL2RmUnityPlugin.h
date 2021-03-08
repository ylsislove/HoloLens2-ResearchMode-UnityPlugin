#pragma once

#define FUNCTIONS_EXPORTS_API extern "C" __declspec(dllexport)   

extern "C" {
	typedef struct IMUOutputForUnity {
		float x;
		float y;
		float z;
	}OUTPUT;
}

namespace HL2Stream {

	FUNCTIONS_EXPORTS_API void __stdcall StartStreaming();
	FUNCTIONS_EXPORTS_API int __stdcall GetAccelSample(OUTPUT* output);
	FUNCTIONS_EXPORTS_API int __stdcall GetGyroSample(OUTPUT* output);
	FUNCTIONS_EXPORTS_API int __stdcall GetMagSample(OUTPUT* output);

	void InitializeResearchModeSensors();
	void InitializeResearchModeProcessing();

	static void CamAccessOnComplete(ResearchModeSensorConsent consent);
	static void ImuAccessOnComplete(ResearchModeSensorConsent consent);

	void DisableSensors();

	IResearchModeSensorDevice* m_pSensorDevice;
	IResearchModeSensorDeviceConsent* m_pSensorDeviceConsent;
	std::vector<ResearchModeSensorDescriptor> m_sensorDescriptors;

	// sensor
	IResearchModeSensor* m_pAccelSensor = nullptr;
	IResearchModeSensor* m_pGyroSensor = nullptr;
	IResearchModeSensor* m_pMagSensor = nullptr;

	// sensor processors
	std::shared_ptr<AccelProcessor> m_pAccelProcessor;
	std::shared_ptr<GyroProcessor> m_pGyroProcessor;
	std::shared_ptr<MagProcessor> m_pMagProcessor;

}