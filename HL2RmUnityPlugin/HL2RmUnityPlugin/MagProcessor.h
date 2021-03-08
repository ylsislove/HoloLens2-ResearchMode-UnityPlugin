#pragma once
class MagProcessor
{
public:
    MagProcessor(
        IResearchModeSensor* pMagSensor,
        HANDLE hasData,
        ResearchModeSensorConsent* pCamAccessConsent);
    virtual ~MagProcessor();

    void UpdateSample();
    void GetMagSample(DirectX::XMFLOAT3* pMagSample);

private:
    static void MagUpdateThread(MagProcessor* pMagProcessor, HANDLE hasData, ResearchModeSensorConsent* pCamAccessConsent);
    void MagUpdateLoop();

    IResearchModeSensor* m_pMagSensor = nullptr;
    IResearchModeSensorFrame* m_pSensorFrame;
    DirectX::XMFLOAT3 m_magSample;
    std::mutex m_sampleMutex;

    std::thread* m_pMagUpdateThread;
    bool m_fExit = { false };
};

