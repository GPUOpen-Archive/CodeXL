#ifndef IProgressBarDataProvider_h__
#define IProgressBarDataProvider_h__

class IProgressBarEventHandler
{
public:
    virtual ~IProgressBarEventHandler() {}
    virtual void SetProgressInfo(const wchar_t* pProgBarLabel, bool isInProgress, unsigned long progBarComplete, unsigned long progBarRange) = 0;
    virtual bool ShouldUpdateProgress() = 0;
};

#endif // IProgressBarDataProvider_h__
