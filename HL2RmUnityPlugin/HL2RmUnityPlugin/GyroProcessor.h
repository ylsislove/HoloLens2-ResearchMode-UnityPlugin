#pragma once
class GyroProcessor
{
public:
    GyroProcessor(
        IResearchModeSensor* pGyroSensor,
        HANDLE hasData,
        ResearchModeSensorConsent* pCamAccessConsent);
    virtual ~GyroProcessor();

    void UpdateSample();
    void GetGyroSample(DirectX::XMFLOAT3* pGyroSample);

private:
    static void GyroUpdateThread(GyroProcessor* pGyroProcessor, HANDLE hasData, ResearchModeSensorConsent* pCamAccessConsent);
    void GyroUpdateLoop();

    IResearchModeSensor* m_pGyroSensor = nullptr;
    IResearchModeSensorFrame* m_pSensorFrame;
    DirectX::XMFLOAT3 m_gyroSample;
    std::mutex m_sampleMutex;

    std::thread* m_pGyroUpdateThread;
    bool m_fExit = { false };
};

