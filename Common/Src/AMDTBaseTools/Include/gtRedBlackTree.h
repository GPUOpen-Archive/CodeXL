//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gtRedBlackTree.h
///
//=====================================================================

//------------------------------ gtRedBlackTree.h ------------------------------

#ifndef __GTREDBLACKTREE_H
#define __GTREDBLACKTREE_H

// Local:
#include <AMDTBaseTools/Include/gtAutoPtr.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtGRBaseToolsDLLBuild.h>
#include <AMDTBaseTools/Include/gtStack.h>


// ----------------------------------------------------------------------------------
// Class Name:           gtRedBlackTreeValue
// General Description:
//  An abstract base class representing a single value contained in the red black tree.
//  Sub class must implement a getKey() function to return an integer key representing the value.
//  The key for a tree value should never change.
//
// Author:      AMD Developer Tools Team
// Creation Date:        28/6/2010
// ----------------------------------------------------------------------------------
class GT_API gtRedBlackTreeValue
{
public:
    gtRedBlackTreeValue();
    virtual ~gtRedBlackTreeValue();
    virtual gtUInt64 getKey() const = 0;
};


// ----------------------------------------------------------------------------------
// Class Name:           gtRedBlackTreeNode
// General Description:
//  Red black tree internal node type.
//  Holds a gtRedBlackTreeValue and tree related data.
//
// Author:      AMD Developer Tools Team
// Creation Date:        28/6/2010
// ----------------------------------------------------------------------------------
class GT_API gtRedBlackTreeNode
{
public:
    gtRedBlackTreeValue* getValue() const { return _pValue; };

protected:
    gtRedBlackTreeNode();
    gtRedBlackTreeNode(gtRedBlackTreeValue* pValue);
    ~gtRedBlackTreeNode();

protected:
    // The value which this node represents:
    gtRedBlackTreeValue* _pValue;

    // An integer key representing the value that this node represents:
    gtUInt64 _key;

    // Is the node "red" or "black" colored:
    // true - the node is red
    // false - the node is black
    bool _isRed;

    gtRedBlackTreeNode* _pLeftChild;
    gtRedBlackTreeNode* _pRightChild;
    gtRedBlackTreeNode* _pParent;

private:
    friend class gtRedBlackTree;
};


// ----------------------------------------------------------------------------------
// Class Name:           gtRedBlackTree
// General Description:
//   Represents a red black tree: a special type of self-balancing binary tree, ensuring
//   that the longest path from the _pRootNode to any leaf is no more than twice as long as the
//   shortest path from the _pRootNode to any other leaf in that tree. This yields the following
//   run-time complexity:
//   - insert value - O(log n)
//   - remove value - O(log n)
//   - search - O(log n)
//   - Memory consumption - O(n)
//
//   For more details see: http://en.wikipedia.org/wiki/Red-black_tree
//
// Author:      AMD Developer Tools Team
// Creation Date:        29/6/2010
// ----------------------------------------------------------------------------------
class GT_API gtRedBlackTree
{
public:
    gtRedBlackTree();
    ~gtRedBlackTree();

    void clear();
    void deleteNode(gtRedBlackTreeNode* pNode);
    gtRedBlackTreeNode* insert(gtAutoPtr<gtRedBlackTreeValue>& aptrNode);
    gtRedBlackTreeNode* search(gtUInt64 key) const;
    gtRedBlackTreeNode* searchEqualOrLowerThan(gtUInt64 key) const;
    gtRedBlackTreeNode* getPredecessorOf(gtRedBlackTreeNode* pNode) const;
    gtRedBlackTreeNode* getSuccessorOf(gtRedBlackTreeNode* pNode) const;
    void enumerate(gtUInt64 low, gtUInt64 high, gtStack<gtRedBlackTreeNode*>& nodes) const;

protected:
    void checkAssumptions() const;
    void leftRotate(gtRedBlackTreeNode* pNode);
    void rightRotate(gtRedBlackTreeNode* pNode);
    void treeInsertHelp(gtRedBlackTreeNode* pNode);
    void deleteFixUp(gtRedBlackTreeNode* pNode);

private:
    void initializeRootAndNilNodes();

protected:
    // A sentinel is used for root and for nil.
    // _pRootNode->_pLeftChild should always point to the node which is the root of the "real" tree.
    // _pNilNode points a node which should always be black but has arbitrary children and parent and no key or info.
    //  The point of using these sentinels is so that the root and nil nodes do not require special cases in the code.
    gtRedBlackTreeNode* _pRootNode;
    gtRedBlackTreeNode* _pNilNode;
};


#endif //__GTREDBLACKTREE_H

