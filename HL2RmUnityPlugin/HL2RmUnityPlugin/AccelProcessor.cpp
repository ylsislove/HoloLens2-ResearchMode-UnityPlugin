#include "pch.h"
#include "AccelProcessor.h"

AccelProcessor::AccelProcessor(IResearchModeSensor* pAccelSensor, HANDLE hasData, ResearchModeSensorConsent* pCamAccessConsent)
{
    m_pAccelSensor = pAccelSensor;
    m_pAccelSensor->AddRef();
    m_pAccelUpdateThread = new std::thread(AccelUpdateThread, this, hasData, pCamAccessConsent);
}

AccelProcessor::~AccelProcessor()
{
    m_fExit = true;
    m_pAccelUpdateThread->join();

    if (m_pAccelSensor)
    {
        m_pAccelSensor->CloseStream();
        m_pAccelSensor->Release();
    }
}

void AccelProcessor::UpdateSample()
{
    HRESULT hr = S_OK;
}

void AccelProcessor::GetAccelSample(DirectX::XMFLOAT3* pAccelSample)
{
    std::lock_guard<std::mutex> guard(m_sampleMutex);

    *pAccelSample = m_accelSample;
}

void AccelProcessor::AccelUpdateThread(AccelProcessor* pAccelProcessor, HANDLE hasData, ResearchModeSensorConsent* pCamAccessConsent)
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

    pAccelProcessor->AccelUpdateLoop();
}

void AccelProcessor::AccelUpdateLoop()
{
    uint64_t lastSocTick = 0;
    uint64_t lastHupTick = 0;
    LARGE_INTEGER qpf;
    uint64_t lastQpcNow = 0;

    // Cache the QueryPerformanceFrequency
    QueryPerformanceFrequency(&qpf);

    winrt::check_hresult(m_pAccelSensor->OpenStream());

    while (!m_fExit)
    {
        char printString[1000];

        IResearchModeSensorFrame* pSensorFrame = nullptr;
        IResearchModeAccelFrame* pSensorAccelFrame = nullptr;
        ResearchModeSensorTimestamp timeStamp;
        const AccelDataStruct* pAccelBuffer = nullptr;
        size_t BufferOutLength;

        winrt::check_hresult(m_pAccelSensor->GetNextBuffer(&pSensorFrame));

        winrt::check_hresult(pSensorFrame->QueryInterface(IID_PPV_ARGS(&pSensorAccelFrame)));

        {
            std::lock_guard<std::mutex> guard(m_sampleMutex);

            winrt::check_hresult(pSensorAccelFrame->GetCalibratedAccelaration(&m_accelSample));
        }

        winrt::check_hresult(pSensorAccelFrame->GetCalibratedAccelarationSamples(
            &pAccelBuffer,
            &BufferOutLength));

        lastHupTick = 0;
        std::string hupTimeDeltas = "";

        for (UINT i = 0; i < BufferOutLength; i++)
        {
            pSensorFrame->GetTimeStamp(&timeStamp);
            if (lastHupTick != 0)
            {
                if (pAccelBuffer[i].VinylHupTicks < lastHupTick)
                {
                    sprintf_s(printString, "####ACCEL BAD HUP ORDERING\n");
                    OutputDebugStringA(printString);
                    DebugBreak();
                }
                sprintf_s(printString, " %I64d", (pAccelBuffer[i].VinylHupTicks - lastHupTick) / 1000); // Microseconds

                hupTimeDeltas = hupTimeDeltas + printString;

            }
            lastHupTick = pAccelBuffer[i].VinylHupTicks;
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

            sprintf_s(printString, "####Accel: % 3.4f % 3.4f % 3.4f %f %I64d %I64d\n",
                m_accelSample.x,
                m_accelSample.y,
                m_accelSample.z,
                sqrt(m_accelSample.x * m_accelSample.x + m_accelSample.y * m_accelSample.y + m_accelSample.z * m_accelSample.z),
                (((timeStamp.HostTicks - lastSocTick) * 1000) / timeStamp.HostTicksPerSecond), // Milliseconds
                timeInMilliseconds);
            //OutputDebugStringA(printString);
        }
        lastSocTick = timeStamp.HostTicks;
        lastQpcNow = uqpcNow;

        if (pSensorFrame)
        {
            pSensorFrame->Release();
        }

        if (pSensorAccelFrame)
        {
            pSensorAccelFrame->Release();
        }
    }

    winrt::check_hresult(m_pAccelSensor->CloseStream());
}
