//------------------------------ osDoubleBufferQueue.h ------------------------------

#ifndef __OS_DOUBLE_BUFFER_QUEUE
#define __OS_DOUBLE_BUFFER_QUEUE

#include <queue>

// Local:
#include <AMDTOSWrappers/Include/osCriticalSection.h>
#include <AMDTOSWrappers/Include/osCriticalSectionLocker.h>

// ----------------------------------------------------------------------------------
// Class Name:           osDoubleBufferQueue
// General Description:
//   A thread-safe container that allows multiple producers to insert items asynchronously
//   of a single consumer that reads items from the container.
//   Producers block each other when inserting. The consumer can continue popping items
//   and does not block the producers because when it begins reading from one queue all
//   the producers are directed to write to the other queue.
//
// Author:               Doron Ofek
// Creation Date:        Dec-17, 2015
// ----------------------------------------------------------------------------------
template <class value_type>
class osDoubleBufferQueue
{
    friend class popper;

public:

    // Self functions:
    osDoubleBufferQueue();
    virtual ~osDoubleBufferQueue();

    bool isEmpty() const;
    unsigned int size() const;
    void push(const value_type& newItem);

    // Use this class to extract items from the queue.
    // Internally it switches the producers and consumer queues in a thread-safe manner,
    // and then allows to reference and pop items from the consumer queue.
    // It uses a guard pattern allow only a single consumer to pop items at any point in time,
    // locking the consumer lock in the constructor and releasing it in the destructor.
    class popper
    {
    public:
        popper(osDoubleBufferQueue& q);
        ~popper();
        const value_type& front();
        void pop();
        bool isConsumerBufferEmpty();
    private:
        osDoubleBufferQueue& m_doubleBufferQ;
        unsigned int m_consumerQueueIndex;
    };

private:
    osCriticalSection m_producersLock;
    osCriticalSection m_consumersLock;

    std::queue<value_type> m_queues[2];
    unsigned int m_producerQueueIndex;
};

template <class value_type>
osDoubleBufferQueue<value_type>::osDoubleBufferQueue() : m_producerQueueIndex(0)
{
}

template <class value_type>
osDoubleBufferQueue<value_type>::~osDoubleBufferQueue()
{
}

template <class value_type>
void osDoubleBufferQueue<value_type>::push(const value_type& newItem)
{
    osCriticalSectionLocker guard(m_producersLock);
    m_queues[m_producerQueueIndex].push(newItem);
}

template <class value_type>
bool osDoubleBufferQueue<value_type>::isEmpty() const
{
    return size() == 0;
}

template <class value_type>
unsigned int osDoubleBufferQueue<value_type>::size() const
{
    osCriticalSectionLocker producersGuard(m_producersLock);
    osCriticalSectionLocker consumersGuard(m_consumersLock);
    unsigned int sumOfQSizes = m_queues[0].size() + m_queues[1].size();
    return sumOfQSizes;
}

template <class value_type>
osDoubleBufferQueue<value_type>::popper::popper(osDoubleBufferQueue& q) : m_doubleBufferQ(q)
{
    osCriticalSectionLocker guard(m_doubleBufferQ.m_producersLock);
    m_consumerQueueIndex = m_doubleBufferQ.m_producerQueueIndex;

    if (0 == m_doubleBufferQ.m_producerQueueIndex)
    {
        m_doubleBufferQ.m_producerQueueIndex = 1;
    }
    else
    {
        m_doubleBufferQ.m_producerQueueIndex = 0;
    }

    m_doubleBufferQ.m_consumersLock.enter();
}

template <class value_type>
osDoubleBufferQueue<value_type>::popper::~popper()
{
    m_doubleBufferQ.m_consumersLock.leave();
}

template <class value_type>
const value_type& osDoubleBufferQueue<value_type>::popper::front()
{
    return m_doubleBufferQ.m_queues[m_consumerQueueIndex].front();
}

template <class value_type>
void osDoubleBufferQueue<value_type>::popper::pop()
{
    m_doubleBufferQ.m_queues[m_consumerQueueIndex].pop();
}

template <class value_type>
bool osDoubleBufferQueue<value_type>::popper::isConsumerBufferEmpty()
{
    return m_doubleBufferQ.m_queues[m_consumerQueueIndex].empty();
}

#endif  // __OS_DOUBLE_BUFFER_QUEUE

