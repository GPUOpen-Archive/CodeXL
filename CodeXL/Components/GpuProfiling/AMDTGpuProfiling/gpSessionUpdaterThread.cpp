//------------------------------ gpSessionUpdaterThread.cpp ------------------------------

#include <qtIgnoreCompilerWarnings.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osStopWatch.h>

#include <AMDTGraphicsServerInterface/Include/AMDTGraphicsServerInterface.h>

// Local:
#include <AMDTGpuProfiling/gpExecutionMode.h>
#include <AMDTGpuProfiling/gpSessionUpdaterThread.h>
#include <AMDTGpuProfiling/ProfileManager.h>

#define GP_FRAME_UPDATE_INTERVAL_IN_SECONDS 1.0
#define GP_FRAME_CAPTURE_TIMEOUT_IN_SECONDS 25.0
#define GP_THREAD_SLEEP_TIME_BETWEEN_CHECKING_FOR_NEW_REQUEST_IN_MS 50
#define GP_GRAPHICS_SERVER_STATE_STALLED "GRAPHICS_SERVER_STATE_STALLED"
gpSessionUpdaterThread::gpSessionUpdaterThread(QObject* pParent) : QThread(pParent), m_shouldCaptureFrame(false), m_isCaptureInProgress(false), m_endedWithError(false)
{
}

gpSessionUpdaterThread::~gpSessionUpdaterThread()
{

}


void gpSessionUpdaterThread::run()
{
    m_endedWithError = false;

    // Use a stopwatch to check if it is time to requests an update from the server
    osStopWatch stopwatch;
    stopwatch.start();
    double elapsedTime = 0.0;

    gpExecutionMode* pModeManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();
    GT_IF_WITH_ASSERT((pModeManager != nullptr) && (pModeManager->GetGraphicsServerComminucation() != nullptr))
    {
        // Get the image buffer from the server
        m_pServerComm = pModeManager->GetGraphicsServerComminucation();
    }

    // First send the session name and project name to the server
    GT_IF_WITH_ASSERT(m_pServerComm != nullptr)
    {
        bool rc = m_pServerComm->SetSessionName(m_sessionName);
        GT_ASSERT(rc);
        rc = m_pServerComm->SetProjectName(m_projectName);
        GT_ASSERT(rc);
    }

    while (true)
    {
        if (isInterruptionRequested())
        {
            return;
        }

        if (m_shouldCaptureFrame)
        {
            m_isCaptureInProgress = true;
            CaptureFrame();
            m_shouldCaptureFrame = false;
        }
        else if (!m_isCaptureInProgress)
        {
            // Is it time to requests an update from the server?
            stopwatch.getTimeInterval(elapsedTime);

            if (elapsedTime >= GP_FRAME_UPDATE_INTERVAL_IN_SECONDS)
            {
                stopwatch.stop();
                bool rc = GetCurrentFrame();

                if (!rc)
                {
                    m_endedWithError = true;
                    break;
                }

                stopwatch.start();
            }
        }
        else
        {
            stopwatch.getTimeInterval(elapsedTime);

            if (elapsedTime >= GP_FRAME_CAPTURE_TIMEOUT_IN_SECONDS)
            {
                // Too much time has passed since the capture operation began. Tt is unreasonable that frame capture has not completed
                // yet so we assume it has failed and resume the heartbeat requests
                m_isCaptureInProgress = false;
            }
        }

        msleep(GP_THREAD_SLEEP_TIME_BETWEEN_CHECKING_FOR_NEW_REQUEST_IN_MS);
    }
}

bool gpSessionUpdaterThread::CaptureFrame()
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(m_pServerComm != nullptr)
    {
        gtASCIIString frameAsXML;
        retVal = m_pServerComm->CaptureFrame(frameAsXML);

        if (frameAsXML.find(GP_GRAPHICS_SERVER_STATE_STALLED) >= 0)
        {
            // Send a signal indicating that the profiled application is not currently rendering
            emit CapturedFrameUpdateReady(INVALID_FRAME_INDEX_INDICATING_NO_RENDER);
        }

        if (retVal)
        {
            // Translate the XML to a frame info
            FrameInfo frameInfo;
            retVal = gpUIManager::Instance()->FrameInfoFromXML(frameAsXML, frameInfo);
            GT_IF_WITH_ASSERT(retVal)
            {
                emit CapturedFrameUpdateReady(frameInfo.m_frameIndex);

            }
        }
    }

    return retVal;
}

bool gpSessionUpdaterThread::GetCurrentFrame()
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(m_pServerComm != nullptr)
    {
        // Get the image buffer from the server
        unsigned char* pImageBuffer = nullptr;
        unsigned long imageSize = 0;
        gtASCIIString infoXML;

        bool rc = m_pServerComm->GetCurrentFrameInfo(infoXML, pImageBuffer, imageSize);
        FrameInfo frameInfo;

        if (rc)
        {
            // Extract the frame info from the XML string
            bool rc2 = gpUIManager::Instance()->FrameInfoFromXML(infoXML, frameInfo);

            if (pImageBuffer != nullptr && rc2)
            {
                QPixmap* pThumbnailPixmap = new QPixmap;
                bool rcLoad = pThumbnailPixmap->loadFromData(pImageBuffer, imageSize, GP_thumbnailImageExtension);
                GT_IF_WITH_ASSERT(rcLoad)
                {
                    emit CurrentFrameUpdateReady(pThumbnailPixmap, frameInfo);

                    delete[] pImageBuffer;
                }
                retVal = true;
            }
            else
            {
                // Emit a signal that specifies the application is dead or server communication failure
                emit CurrentFrameUpdateReady(nullptr, frameInfo);
            }
        }
        else
        {
            // Emit a signal that specifies the application is dead or server communication failure
            emit CurrentFrameUpdateReady(nullptr, frameInfo);
        }

        // Check if there is a list of captured frames that had been captured by the user
        // Disabling this feature as a workaround for
        // CODEXL-2246 CXL throws "Memory Allocation Failure" for the second frame capture of Dandia app
        // http://ontrack-internal.amd.com/browse/CODEXL-2246
        // Calling this function causes an infinite loops when performing a capture in Dandia.
        // When this line is uncommented, please make sure that this bug is resolved.
        // pModeManager->UpdateCapturedFrameFromServer();

    }
    return retVal;

}