//------------------------------ gpRibbonDataCalculator.h ------------------------------

#ifndef __GPRIBBONDATACALCULATOR_H_
#define __GPRIBBONDATACALCULATOR_H_

// Qt
#include <QVector>

// Infra
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

class acTimeline;
class acTimelineGrid;
class gpTraceDataContainer;

class gpRibbonCallsData
{
public:
    /// the time of the draw call
    double m_startTime;

    /// the duration of the draw call
    double m_duration;

    /// sort function
    bool operator<(const gpRibbonCallsData& other) const { return m_startTime < other.m_startTime; };
};

class gpRibbonSegmentData
{
public:
    /// start time of segment
    double m_startTime;

    /// end time of the segment
    double m_endTime;

    /// id of the thread
    osThreadId m_threadID;
};

class gpRibbonDataCalculator
{
public:
    enum eCalculatorBucketMode
    {
        eBucketCountMode = 0,
        eBucketMaxValueMode
    };

    enum eConcurrencyBucketMode
    {
        eConcurrencyBucketMaxMode = 0,
        eConcurrencyBucketAvgMode,
        eConcurrencyBucketTotalMode
    };

    enum ePresentData
    {
        ePresentCPU = 0,
        ePresentGPU,
        eNumPresent
    };

    gpRibbonDataCalculator(gpTraceDataContainer* pSessionData, acTimeline* pTimeLine);
    virtual ~gpRibbonDataCalculator() {};

    /// Get the API calls vector
    /// \param dataVector data vector that is returned with the API calls.
    const QVector<gpRibbonCallsData>& GetApiCalls() const { return m_apiCallsSortedByTime; };

    /// Get the draw calls vector (API calls that are only draw calls)
    /// \param dataVector data vector that is returned with the Draw calls.
    const QVector<gpRibbonCallsData>& GetDrawCalls() const { return m_drawCallsSortedByTime; };

    /// Should be called after the timeline and data container are initialized
    void CalculateData();

    /// Calculates the concurrency of CPU. This calculation is not done as part of the general calculations, but only on demand, since it requires high performance
    void CalculateCPUConcurrency();

    /// Max threads concurrency buckets
    const QVector<double>& CpuMaxThreadConcurrency() const { return m_cpuMaxThreadConcurrency; };

    /// Average threads concurrency buckets
    const QVector<double>& CpuAverageThreadConcurrency() const { return m_cpuAverageThreadConcurrency; };

    /// Total threads concurrency buckets
    const QVector<double>& CpuTotalThreadConcurrency() const { return m_cpuTotalThreadConcurrency; };

    /// Get the GPU ops
    /// \param dataVector data vector that is returned with the GPU ops calls.
    void GetGPUOps(QVector<gpRibbonCallsData>& dataVector);

    /// Get the numTopCalls top calls
    /// \param dataVector calls that are used to get the top calls from
    /// \param callsTime top calls execution time
    /// \param callsTime top calls execution duration
    /// \param numTopCalls number of top calls to return
    void GetTopCalls(QVector<gpRibbonCallsData>& dataVector, QVector<double>& callsTime, QVector<double>& callsDuration, int numTopCalls);

    /// move the calls into bucket counting them or taking the maximum value in each bucket
    /// \param dataVector calls that is used to fill the buckets
    /// \param bucketsData returned data in buckets based on the bucket mode
    /// \param bucketMode how to fill the buckets: eBucketCountMode: count of items in the bucket or eBucketMaxValueMode: max value of items in the bucket
    void CallsToBuckets(QVector<gpRibbonCallsData>& dataVector, QVector<double>& bucketsData, eCalculatorBucketMode bucketMode);

    /// convert concurrency data to bucket data
    /// move the calls into bucket counting them or taking the maximum value in each bucket
    /// \param dataVector concurrency data that is used to fill the buckets
    /// \param bucketsData returned data in buckets based on the bucket mode
    /// \param bucketMode how to fill the buckets: eConcurrencyBucketMaxMode: maximum concurrent threads in the bucket or eConcurrencyBucketAvgMode: average concurrent threads in the bucket
    void ConcurrencyToBuckets(QVector<gpRibbonCallsData>& dataVector, QVector<double>& bucketsData, eConcurrencyBucketMode bucketMode);

    /// Get "Present" data. current assuming there is one for CPU and one for the GPU
    /// \param the present vector that holds the time of the data
    void GetPresentData(QVector<double>& presentData);

    /// Get the total count of API calls CPU & GPU
    int GetTotalApiCalls();

protected:

    /// Calculate and cache the api and draw calls sorted by time
    void CalculateCallsData();

private:
    /// Controlling Time line
    acTimeline* m_pTimeLine;

    /// Controlling Time line grid;
    acTimelineGrid* m_pTimeLineGrid;

    /// navigation data
    gpTraceDataContainer* m_pData;

    /// Contain the sorted by start time api calls:
    QVector<gpRibbonCallsData> m_apiCallsSortedByTime;

    /// Contain the sorted by start time draw calls:
    QVector<gpRibbonCallsData> m_drawCallsSortedByTime;

    /// Max threads concurrency buckets
    QVector<double> m_cpuMaxThreadConcurrency;

    /// Average threads concurrency buckets
    QVector<double> m_cpuAverageThreadConcurrency;

    /// Total threads concurrency buckets
    QVector<double> m_cpuTotalThreadConcurrency;

    /// flag to indicate of concurrency was calculated
    bool m_wasConcurrencyCalculated;
};
#endif // __GPRIBBONDATACALCULATOR_H_
