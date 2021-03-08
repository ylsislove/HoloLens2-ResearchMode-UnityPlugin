#pragma once
class AccelProcessor
{
public:
    AccelProcessor(
        IResearchModeSensor* pAccelSensor,
        HANDLE hasData,
        ResearchModeSensorConsent* pCamAccessConsent);

    virtual ~AccelProcessor();

    void UpdateSample();
    void GetAccelSample(DirectX::XMFLOAT3* pAccelSample);

private:
    static void AccelUpdateThread(AccelProcessor* pAccelProcessor, HANDLE hasData, ResearchModeSensorConsent* pCamAccessConsent);
    void AccelUpdateLoop();


    IResearchModeSensor* m_pAccelSensor = nullptr;
    IResearchModeSensorFrame* m_pSensorFrame;
    DirectX::XMFLOAT3 m_accelSample;
    std::mutex m_sampleMutex;

    std::thread* m_pAccelUpdateThread;
    bool m_fExit = { false };
};
