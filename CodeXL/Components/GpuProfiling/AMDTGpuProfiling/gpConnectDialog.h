//------------------------------ gpConnectDialog.h ------------------------------

#ifndef __GPCONNECTDIALOG_H
#define __GPCONNECTDIALOG_H

#include <AMDTApplicationComponents/Include/acDialog.h>

// Local:
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>

class GraphicsServerCommunication;
class acListCtrl;
class acProgressAnimationWidget;

struct gpConnectionData
{
    QString mName;
    QString mPID;
    QString mAPI;
    QString mAttached;
    float mTime;
};

class AMDT_GPU_PROF_API gpConnectDialog : public acDialog
{
    enum
    {
        eIndexColumn,
        eAPPNameColumn,
        eProcIDColumn,
        eAPIColumn,
        eAttachedColumn,
        eTimeColumn
    };
    Q_OBJECT
public:
    gpConnectDialog(GraphicsServerCommunication* pGraphicsServerCommunication, QWidget* pParent = 0, bool forceAutoConnect = false);
    virtual ~gpConnectDialog();

    // connect will display the dialog or show progress dialog depending on the setting
    bool Connect();

    QString PIDToConnectTo() { return m_pidToConnectTo; }
    QString APIToConnectTo() { return m_apiToConnectTo; }

protected slots:
    void onTimerTimeout();
    void OnAccept();
    void OnConnectionSelected(int);
    void OnTextEdited(const QString&);
    void OnTableSelectionChanged();

private:
    // init the view layout
    void InitLayout();

    /// check if auto connected to one of the connections
    bool CheckAutoConnected(QVector<gpConnectionData>& connections);

    /// The connections table
    acListCtrl* m_pConnectionsTable;

    /// The "remember selection" check box
    QCheckBox* m_pAutomaticCheckBox;

    // Selection combobox
    QComboBox* m_pOptionsComboBox;

    /// the data of the connection
    QLineEdit* m_pOptionsEdit;

    /// the animation widget
    acProgressAnimationWidget* m_pAnimationWidget;

    /// interface to the graphic server
    GraphicsServerCommunication* m_pGraphicsServerCommunication;

    /// sampling timer
    QTimer m_sampleTimer;

    /// total time counter
    float m_timePassed;

    /// process to connect to
    QString m_pidToConnectTo;
    /// api type to connect to (DX12 or Vulkan)
    QString m_apiToConnectTo;

    /// strings that temporary store the data
    QString m_processName;
    QString m_processNumber;

    /// integer validator
    QIntValidator m_validator;

    /// force auto connect to first dx12 application over writing user selection
    /// force autoconnect also removes the need to show the dialog
    bool m_forceAutoConnect;

    /// there was at least one process alive in the server. if after that we reached zero process again
    /// then parent process was killed and we should close the dialog with cancel
    bool m_oneProcessAlive;
};

#endif  // __GPCONNECTDIALOG_H