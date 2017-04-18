
#ifndef __ISAProgramGraph_H
#define __ISAProgramGraph_H

#include "Instruction.h"
#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <iostream>
#include <set>
#include <string>

// basic node in the isa program
class ISACodeBlock
{
private:
    int m_iLabel;           // the label no. of the loop, if any
    int m_iIterationCount ; // how many times to do the loop, if applicable
    std::vector<Instruction*> m_vInstructions; // how many nodes points on me
    ISACodeBlock* m_pNext; // next code block
    ISACodeBlock* m_pTrue; // next code block in case of branch is true. null if point to the upper loop to avoid loops in the graph
    ISACodeBlock* m_pFalse;// next code block in case of branch is false

public:
    ISACodeBlock()
    {
        m_iLabel = NO_LABEL;
        m_iIterationCount  = 1;
        m_pNext = NULL;
        m_pTrue = NULL;
        m_pFalse = NULL;
    }
    ~ISACodeBlock() {}

    int GetLabel() {return m_iLabel;}
    ISACodeBlock* GetNext() {return m_pNext;}
    ISACodeBlock* GetTrue() {return m_pTrue;}
    ISACodeBlock* GetFalse() {return m_pFalse;}
    int GetIterationCount() {return m_iIterationCount;}
    const std::vector<Instruction*>& GetIsaCodeBlockInstructions() const { return m_vInstructions;}

    void SetNext(ISACodeBlock* p) { m_pNext = p; }
    void SetTrue(ISACodeBlock*  p)  { m_pTrue = p; }
    void SetFalse(ISACodeBlock* p) { m_pFalse = p; }
    void SetIterationCount(int iIterationCount) { m_iIterationCount = iIterationCount; }

    //void SetLabel(int iLabel) { m_iLabel = iLabel; }

    friend class ISAProgramGraph;

};

// This class is a utility class that can build, destroy, search and traverse ISAProgramGraph.
// The LabelNodeSet is saved as well so finding a label will be efficient
class ISAProgramGraph
{
public:
    enum AnalyzeDataPath
    {
        CALC_ALL = 0,
        CALC_TRUE = 1,
        CALC_FALSE = 2,
        CALC_NUM_OF_PATHES = 3,
    };

    /// The number of instruction in each category
    struct NumOfInstructionsInCategory
    {
        unsigned int m_scalarMemoryReadInstCount;
        unsigned int m_scalarMemoryWriteInstCount;
        unsigned int m_scalarALUInstCount;
        unsigned int m_vectorMemoryReadInstCount;
        unsigned int m_vectorMemoryWriteInstCount;
        unsigned int m_vectorALUInstCount;
        unsigned int m_LDSInstCount;
        unsigned int m_GDSInstCount;
        unsigned int m_exportInstCount;
        unsigned int m_atomicsInstCount;
        unsigned int m_CalculatedCycles;
        unsigned int m_CalculatedCycesPerWevefronts;

        NumOfInstructionsInCategory()
        {
            m_scalarMemoryReadInstCount = 0;
            m_scalarMemoryWriteInstCount = 0;
            m_scalarALUInstCount = 0;
            m_vectorMemoryReadInstCount = 0;
            m_vectorMemoryWriteInstCount = 0;
            m_vectorALUInstCount = 0;
            m_LDSInstCount = 0;
            m_GDSInstCount = 0;
            m_exportInstCount = 0;
            m_atomicsInstCount = 0;
            m_CalculatedCycles = 0;
            m_CalculatedCycesPerWevefronts = 0;
        };

        NumOfInstructionsInCategory& operator=(const NumOfInstructionsInCategory& original)
        {
            m_scalarMemoryReadInstCount = original.m_scalarMemoryReadInstCount;
            m_scalarMemoryWriteInstCount = original.m_scalarMemoryWriteInstCount;
            m_scalarALUInstCount = original.m_scalarALUInstCount;
            m_vectorMemoryReadInstCount = original.m_vectorMemoryReadInstCount;
            m_vectorMemoryWriteInstCount = original.m_vectorMemoryWriteInstCount;
            m_vectorALUInstCount = original.m_vectorALUInstCount;
            m_LDSInstCount = original.m_LDSInstCount;
            m_GDSInstCount = original.m_GDSInstCount;
            m_exportInstCount = original.m_exportInstCount;
            m_atomicsInstCount = original.m_atomicsInstCount;
            m_CalculatedCycles = original.m_CalculatedCycles;
            m_CalculatedCycesPerWevefronts = original.m_CalculatedCycesPerWevefronts;
            return *this;
        };
    };

private:

    class LabelNodeSet
    {
    public:
        int iLabel;
        int iNumOfIteration;
        ISACodeBlock* pNode;

        LabelNodeSet()
        {
            iLabel = NO_LABEL;
            pNode = NULL;
            iNumOfIteration = 1;
        }
    };

    class LabelNodeSetCompare
    {
    public:
        bool operator()(const LabelNodeSet& lhs, const LabelNodeSet& rhs) const
        {
            if (lhs.iLabel < rhs.iLabel)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    };

private:
    ISACodeBlock* m_ISACodeBlock; // this is the head of the entire ISA graph
    std::set<LabelNodeSet, LabelNodeSetCompare> m_NodesSet; // the set is saved for easy find
    int m_iNextLabel; // we give a fake label for nodes with no original labbel in the ISA
    int m_iNumOfLoopIterations;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        BuildISAProgramStructureInternal
    /// \brief Description: Internal function that builds the graph out of the instruction vector.
    /// -----------------------------------------------------------------------------------------------
    bool BuildISAProgramStructureInternal(std::vector<Instruction*>& Instructions, std::vector<Instruction*>::const_iterator instIterator, ISACodeBlock* pHeadIsaNode);

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        UpdateNumOfIteration
    /// \brief Description: part of ISA program graph. when a loop is discovered update the default num of iteration
    /// -----------------------------------------------------------------------------------------------
    bool UpdateNumOfIteration(std::vector<Instruction*>& Instructions, ISACodeBlock* pLoop, const std::vector<Instruction*>::const_iterator iIterator);

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        GetInstructionsOfProgramPathInternal
    /// \brief Description: the internal recursive search
    /// -----------------------------------------------------------------------------------------------
    void GetInstructionsOfProgramPathInternal(std::set<LabelNodeSet, LabelNodeSetCompare>& PathInstructionsSet, ISACodeBlock* pHeadISACodeBlock, int iPath, int iNumOfIterations);

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        CreateNewNode
    /// \brief Description: Create a new node, updates it's label and update the nodeset
    /// -----------------------------------------------------------------------------------------------
    ISACodeBlock* CreateNewNode(int iLabel);

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        CountInstructions
    /// \brief Description: The main function that counts the instruction in the specific ISA graph. this is the main idea of the entire analysis
    /// -----------------------------------------------------------------------------------------------
    void CountInstructions(std::set<ISAProgramGraph::LabelNodeSet, ISAProgramGraph::LabelNodeSetCompare> PathInstructionsSet, ISAProgramGraph::NumOfInstructionsInCategory& NumOfInstructionsInCategory);


    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        GetBiggest
    /// \brief Description: standard bubble sort to retur the biggest number
    /// -----------------------------------------------------------------------------------------------
    int GetBiggest(int iScalarALU_ScalarMemoryRead_Write, int iVectorMemoryRead_Write, int iVectorALU, int iLDS_Atomics, int iGDS_Export);

public:
    ISAProgramGraph();
    ~ISAProgramGraph();

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        BuildISAProgramStructure
    /// \brief Description: goes through the instructions parsed in ParseToVector and build the ISA program graph
    /// \return true if succeeded
    /// -----------------------------------------------------------------------------------------------
    bool BuildISAProgramStructure(std::vector<Instruction*>& Instructions);

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        DestroyISAProgramStructure
    /// \brief Description: destroy ALL the program graph
    /// -----------------------------------------------------------------------------------------------
    void DestroyISAProgramStructure();

    /// -----------------------------------------------------------------------------------------------
    /// traverse the code block
    /// \return the node with the desired label
    /// -----------------------------------------------------------------------------------------------
    ISACodeBlock* LabelSearcher(int iLabel);

    /// -----------------------------------------------------------------------------------------------
    /// GetISAProgramGraph
    /// \return the head of the graph. who needs it?
    /// -----------------------------------------------------------------------------------------------
    ISACodeBlock* GetISAProgramGraph() { return m_ISACodeBlock; }

    /// -----------------------------------------------------------------------------------------------
    /// GetInstructionsOfProgramPath
    /// \return the vector of instruction in a specific path (all/true/false)
    /// -----------------------------------------------------------------------------------------------
    void GetInstructionsOfProgramPath(std::set<LabelNodeSet, LabelNodeSetCompare>& PathInstructionsSet, int iPath);

    /// -----------------------------------------------------------------------------------------------
    /// DumpGraph
    /// save the graph in GRAPHVIZ format
    /// -----------------------------------------------------------------------------------------------
    void DumpGraph(std::set<ISAProgramGraph::LabelNodeSet, ISAProgramGraph::LabelNodeSetCompare> PathInstructionsSet, std::string sFileName);

    /// -----------------------------------------------------------------------------------------------
    /// GetNumOfInstructionsInCategory
    /// Traverse through all Graph Paths (ALL/TRUE/FALSE) and count the instructions
    /// -----------------------------------------------------------------------------------------------
    void GetNumOfInstructionsInCategory(ISAProgramGraph::NumOfInstructionsInCategory NumOfInstructionsInCategory[CALC_NUM_OF_PATHES], std::string sDumpGraph);

    /// this is for the analysis
    void SetNumOfLoopIteration(int iNumOfLoopIteration);
    int GetNumOfLoopIteration();

};

#endif // __ISAProgramGraph_H`