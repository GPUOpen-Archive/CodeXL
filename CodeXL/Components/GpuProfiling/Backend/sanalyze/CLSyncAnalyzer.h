//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This analyzer class detects excessive synchronization APIs
//==============================================================================

#ifndef _CL_SYNC_ANALYZER_H_
#define _CL_SYNC_ANALYZER_H_

#include <vector>
#include <string>
#include <set>
#include <list>
#include <map>
#include "CLAPIAnalyzer.h"

//------------------------------------------------------------------------------------
/// Dependency edge type
//------------------------------------------------------------------------------------
enum EdgeType
{
    ET_Implicit,   /// A Command in OpenCL inorder queue has implicit dependency on previous commands
    ET_Explicit    /// Explicit dependency introduced by APIs like clWaitForEvent, clFinish
};

//------------------------------------------------------------------------------------
/// Dependency edge
//------------------------------------------------------------------------------------
template<class T>
struct Edge
{
    /// Constructor
    /// \param pObj Object that is associated with the edge
    /// \param type Dependency type
    Edge(T* pObj, EdgeType type)
        : m_pObj(pObj),
          m_type(type)
    {}

    T* m_pObj;           ///< Object that is associated with the edge
    EdgeType m_type;     ///< Edge type
};

//------------------------------------------------------------------------------------
/// Dependency node
//------------------------------------------------------------------------------------
template<class NodeObjType, class EdgeObjType>
class Node
{
public:
    /// Constructor
    /// \param pObj Object that is associated with the node
    Node(NodeObjType* pObj) : m_pObj(pObj)
    {}

    /// Destructor
    ~Node()
    {
        for (typename std::vector<EdgeObjType*>::iterator it = m_dependencyList.begin(); it != m_dependencyList.end(); ++it)
        {
            if (*it)
            {
                delete *it;
            }
        }
    }

public:
    NodeObjType* m_pObj;                    ///< Object that is associated with the node.
    std::vector<EdgeObjType*> m_dependencyList;    ///< Edges that connect to other nodes which it depends on.
private:
    /// Disable copy constructor and assignment operator
    Node(const Node& obj);
    Node& operator= (const Node& obj);
};

typedef Edge<CLAPIInfo> CLDependencyEdge;
typedef Node<CLEnqueueAPI, CLDependencyEdge> CLDependencyNode;
typedef std::vector<CLDependencyEdge*> EdgeList;

//------------------------------------------------------------------------------------
/// CL object with reference counter
//------------------------------------------------------------------------------------
class CLObject
{
public:

    /// Constructor
    CLObject()
    {
        nRef = 1;
    }

    /// Release, reference--
    /// \return true if ref == 0
    bool Release()
    {
        if (nRef == 1)
        {
            return true;
        }
        else
        {
            nRef--;
            return false;
        }
    }

    /// Retain, reference++
    void Retain()
    {
        nRef++;
    }

    unsigned int nRef;   ///< Reference count
private:
    /// Disable copy constructor and assignment operator
    CLObject(const CLObject& obj);
    CLObject& operator= (const CLObject& obj);
};

typedef std::vector<CLDependencyNode*> CommandQueue;

class CLSyncAnalyzer;

//------------------------------------------------------------------------------------
/// OpenCL Command Queue object
//------------------------------------------------------------------------------------
class CLCommandQueue : public CLObject
{
    friend class CLSyncAnalyzer;
public:
    /// Constructor
    /// \param strHandle cl_command_queue handle string
    CLCommandQueue(const std::string& strHandle)
        : m_strHandle(strHandle)
    {}

    /// Destructor
    virtual ~CLCommandQueue()
    {
        for (CommandQueue::iterator it = m_queue.begin(); it != m_queue.end(); ++it)
        {
            if (*it)
            {
                delete *it;
            }
        }
    }

public:
    CommandQueue m_queue;      ///< Command queue
    std::string m_strHandle;   ///< cl_command_queue handle string

private:
    /// Disable copy constructor and assignment operator
    CLCommandQueue(const CLCommandQueue& obj);
    CLCommandQueue& operator= (const CLCommandQueue& obj);
};

typedef std::map<std::string, CLCommandQueue*> CommandQueueMap;

//------------------------------------------------------------------------------------
/// Synchronization problem detector
/// Step 1. Flatten related APIs if it's multi-threaded app
/// Setp 2. Scan flattened list, build dependency graph
///         2.1 Create nodes (Enq cmds)
///         2.2 Create edges (implicit dependency, inorder queue)
///         2.3 Create edges (explicit dependency - clFinish, clWaitForEvent, clGetEventInfo busy wait)
/// Setp 3. If edge degree is greater than 1, it means
///         redundant dependency, generate best practice message
//------------------------------------------------------------------------------------
class CLSyncAnalyzer :
    public CLAPIAnalyzer
{
public:
    /// Constructor
    /// \param p A pointer to CLAPIAnalyzerManager
    CLSyncAnalyzer(CLAPIAnalyzerManager* p);

    /// Destructor
    virtual ~CLSyncAnalyzer(void);

    /// Analyze API
    /// \param pAPIInfo APIInfo object
    void Analyze(APIInfo* pAPIInfo);

    /// Callback function for flattened APIs
    /// \param pAPIInfo APIInfo object
    void FlattenedAPIAnalyze(APIInfo* pAPIInfo);

    /// Generate APIAnalyzerMessage
    void EndAnalyze();

protected:
    /// Generate best practices message based on redundant sync API
    /// \param pAPIInfo redundant sync API object
    void GenerateMessage(CLAPIInfo* pAPIInfo);

    CommandQueueMap m_cmdQueueMap;   ///< Command queue.
    std::list<CLAPIInfo*> apiBuffer; ///< temp API buffer, cache sync apis, flush them when an enqueue command is encountered.
private:
    /// Disable copy constructor and assignment operator
    CLSyncAnalyzer(const CLSyncAnalyzer& obj);
    CLSyncAnalyzer& operator= (const CLSyncAnalyzer& obj);
};

#endif //_CL_SYNC_ANALYZER_H_
