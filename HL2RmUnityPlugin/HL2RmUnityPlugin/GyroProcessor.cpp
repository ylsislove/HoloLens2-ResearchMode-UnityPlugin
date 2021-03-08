#include "pch.h"
#include "GyroProcessor.h"

GyroProcessor::GyroProcessor(IResearchModeSensor* pGyroSensor, HANDLE hasData, ResearchModeSensorConsent* pCamAccessConsent)
{
    m_pGyroSensor = pGyroSensor;
    m_pGyroSensor->AddRef();
    m_pGyroUpdateThread = new std::thread(GyroUpdateThread, this, hasData, pCamAccessConsent);
}

GyroProcessor::~GyroProcessor()
{
    m_fExit = true;
    m_pGyroUpdateThread->join();

    if (m_pGyroSensor)
    {
        m_pGyroSensor->CloseStream();
        m_pGyroSensor->Release();
    }
}

void GyroProcessor::UpdateSample()
{
    HRESULT hr = S_OK;
}

void GyroProcessor::GetGyroSample(DirectX::XMFLOAT3* pGyroSample)
{
    std::lock_guard<std::mutex> guard(m_sampleMutex);

    *pGyroSample = m_gyroSample;
}

void GyroProcessor::GyroUpdateThread(GyroProcessor* pGyroProcessor, HANDLE hasData, ResearchModeSensorConsent* pCamAccessConsent)
{
    HRESULT hr = S_OK;

    if (hasData != nullptr)
    {
        DWORD waitResult = WaitForSingleObject(hasData, INFINITE);

        if (waitResult == WAIT_OBJECT_0)
        {
            switch (*pCamAccessConsent)
            {
            case ResearchModeSensorConsent::Allowed:
                OutputDebugString(L"Access is granted");
                break;
            case ResearchModeSensorConsent::DeniedBySystem:
                OutputDebugString(L"Access is denied by the system");
                hr = E_ACCESSDENIED;
                break;
            case ResearchModeSensorConsent::DeniedByUser:
                OutputDebugString(L"Access is denied by the user");
                hr = E_ACCESSDENIED;
                break;
            case ResearchModeSensorConsent::NotDeclaredByApp:
                OutputDebugString(L"Capability is not declared in the app manifest");
                hr = E_ACCESSDENIED;
                break;
            case ResearchModeSensorConsent::UserPromptRequired:
                OutputDebugString(L"Capability user prompt required");
                hr = E_ACCESSDENIED;
                break;
            default:
                OutputDebugString(L"Access is denied by the system");
                hr = E_ACCESSDENIED;
                break;
            }
        }
        else
        {
            hr = E_UNEXPECTED;
        }
    }

    if (FAILED(hr))
    {
        return;
    }

    pGyroProcessor->GyroUpdateLoop();
}

void GyroProcessor::GyroUpdateLoop()
{
    uint64_t lastSocTick = 0;
    uint64_t lastHupTick = 0;
    LARGE_INTEGER qpf;
    uint64_t lastQpcNow = 0;

    // Cache the QueryPerformanceFrequency
    QueryPerformanceFrequency(&qpf);

    winrt::check_hresult(m_pGyroSensor->OpenStream());

    while (!m_fExit)
    {
        char printString[1000];

        IResearchModeSensorFrame* pSensorFrame = nullptr;
        IResearchModeGyroFrame* pGyroFrame = nullptr;
        ResearchModeSensorTimestamp timeStamp;
        const GyroDataStruct* pGyroBuffer = nullptr;
        size_t BufferOutLength;

        winrt::check_hresult(m_pGyroSensor->GetNextBuffer(&pSensorFrame));

        winrt::check_hresult(pSensorFrame->QueryInterface(IID_PPV_ARGS(&pGyroFrame)));

        {
            std::lock_guard<std::mutex> guard(m_sampleMutex);

            winrt::check_hresult(pGyroFrame->GetCalibratedGyro(&m_gyroSample));
        }

        winrt::check_hresult(pGyroFrame->GetCalibratedGyroSamples(
            &pGyroBuffer,
            &BufferOutLength));

        lastHupTick = 0;
        std::string hupTimeDeltas = "";

        for (UINT i = 0; i < BufferOutLength; i++)
        {
            pSensorFrame->GetTimeStamp(&timeStamp);
            if (lastHupTick != 0)
            {
                if (pGyroBuffer[i].VinylHupTicks < lastHupTick)
                {
                    sprintf_s(printString, "####GYRO BAD HUP ORDERING\n");
                    OutputDebugStringA(printString);
                    DebugBreak();
                }
                sprintf_s(printString, " %I64d", (pGyroBuffer[i].VinylHupTicks - lastHupTick) / 1000); // Microseconds

                hupTimeDeltas = hupTimeDeltas + printString;

            }
            lastHupTick = pGyroBuffer[i].VinylHupTicks;
        }

        hupTimeDeltas = hupTimeDeltas + "\n";
        //OutputDebugStringA(hupTimeDeltas.c_str());

        pSensorFrame->GetTimeStamp(&timeStamp);
        LARGE_INTEGER qpcNow;
        uint64_t uqpcNow;
        QueryPerformanceCounter(&qpcNow);
        uqpcNow = qpcNow.QuadPart;

        if (lastSocTick != 0)
        {
            uint64_t timeInMilliseconds =
                (1000 *
                    (uqpcNow - lastQpcNow)) /
                qpf.QuadPart;

            if (timeStamp.HostTicks < lastSocTick)
            {
                DebugBreak();
            }

            sprintf_s(printString, "####Gyro: % 3.4f % 3.4f % 3.4f %I64d %I64d\n",
                m_gyroSample.x,
                m_gyroSample.y,
                m_gyroSample.z,
                (((timeStamp.HostTicks - lastSocTick) * 1000) / timeStamp.HostTicksPerSecond), // Milliseconds
                timeInMilliseconds);
            OutputDebugStringA(printString);
        }
        lastSocTick = timeStamp.HostTicks;
        lastQpcNow = uqpcNow;

        if (pSensorFrame)
        {
            pSensorFrame->Release();
        }

        if (pGyroFrame)
        {
            pGyroFrame->Release();
        }
    }

    winrt::check_hresult(m_pGyroSensor->CloseStream());
}
