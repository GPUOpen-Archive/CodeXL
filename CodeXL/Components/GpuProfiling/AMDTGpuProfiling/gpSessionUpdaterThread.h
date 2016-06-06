//------------------------------ gpSessionUpdaterThread.h ------------------------------

#ifndef __GPSESSIONUPDATERTHREAD_H
#define __GPSESSIONUPDATERTHREAD_H

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTRemoteClient/Include/RemoteClientDataTypes.h>


class GraphicsServerCommunication;

class gpSessionUpdaterThread : public QThread
{
    Q_OBJECT

public:
    enum {INVALID_FRAME_INDEX_INDICATING_NO_RENDER = -1};
    /// Constructor
    gpSessionUpdaterThread(QObject* pParent);

    /// Destructor
    virtual ~gpSessionUpdaterThread();

    void SendCaptureFrameRequest() { m_shouldCaptureFrame = true; }
    void SetCaptureIsComplete() { m_isCaptureInProgress = false; }

    bool EndedWithError() { return m_endedWithError;  }

    gtASCIIString m_sessionName;
    gtASCIIString m_projectName;

signals:

    /// Signals an update in the current session frame
    /// \param pCurrentFramePixmap the pixmap describing the frame. The memory of the pixmap should be handled by the user
    /// \param frameInfo structure describing the frame.
    void CurrentFrameUpdateReady(QPixmap* pCurrentFramePixmap, const FrameInfo& frameInfo);

    /// Signals an update for the current captured frame
    void CapturedFrameUpdateReady(int,int);
protected:

    /// Overriding QThread
    virtual void run();

    bool CaptureFrame();

protected slots:

    /// Is called for each timer timeout
    bool GetCurrentFrame();

protected:

    bool m_shouldCaptureFrame;

    // Capturing a frame includes the following steps:
    // 1. Sending the request to the graphics server and receiving its reply. This is done in the context of this gpSessionUpdaterThread, inside CaptureFrame().
    // 2. Emitting a 'CapturedFrameUpdateReady' signal.
    // 3. The UI main thread receives the signal and proceeds to request the capture frame files from the CodeXLRemoteAgent. This is done in gpSessionView::OnCapturedFrameData()
    // The purpose of m_isCaptureInProgress is to indicate when the 3 steps have been complete so that heartbeat requests can be resumed.
    bool m_isCaptureInProgress;

    /// if the thread ended because of an error
    bool m_endedWithError;

    GraphicsServerCommunication* m_pServerComm;
};
#endif // __GPSESSIONUPDATERTHREAD_H