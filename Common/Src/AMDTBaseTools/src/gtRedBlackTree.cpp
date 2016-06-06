//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gtRedBlackTree.cpp
///
//=====================================================================

//------------------------------ gtRedBlackTree.cpp ------------------------------

// Local:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtRedBlackTree.h>

// Perform extra sanity checking to make sure certain assumptions are satisfied while in debug build configuration:
#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
    #define GT_CHECK_RB_TREE_ASSUMPTIONS 1
#endif

// ---------------------------------------------------------------------------
// Name:        gtRedBlackTreeValue::gtRedBlackTreeValue
// Description: Constructor
// Author:      AMD Developer Tools Team
// Date:        29/6/2010
// ---------------------------------------------------------------------------
gtRedBlackTreeValue::gtRedBlackTreeValue()
{
}


// ---------------------------------------------------------------------------
// Name:        gtRedBlackTreeValue::~gtRedBlackTreeValue
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        29/6/2010
// ---------------------------------------------------------------------------
gtRedBlackTreeValue::~gtRedBlackTreeValue()
{
}


// ---------------------------------------------------------------------------
// Name:        gtRedBlackTreeNode::gtRedBlackTreeNode
// Description: Constructor
// Author:      AMD Developer Tools Team
// Date:        29/6/2010
// ---------------------------------------------------------------------------
gtRedBlackTreeNode::gtRedBlackTreeNode()
    : _pValue(NULL), _key(0), _isRed(false), _pLeftChild(NULL), _pRightChild(NULL), _pParent(NULL)
{
}


// ---------------------------------------------------------------------------
// Name:        gtRedBlackTreeNode::gtRedBlackTreeNode
// Description: Constructor
// Arguments:   pValue - A value to be contained in the tree node.
// Author:      AMD Developer Tools Team
// Date:        29/6/2010
// ---------------------------------------------------------------------------
gtRedBlackTreeNode::gtRedBlackTreeNode(gtRedBlackTreeValue* pValue)
    : _pValue(pValue), _key(pValue->getKey()), _isRed(false), _pLeftChild(NULL), _pRightChild(NULL), _pParent(NULL)
{
}


// ---------------------------------------------------------------------------
// Name:        gtRedBlackTreeNode::~gtRedBlackTreeNode
// Description: Destructor.
// Author:      AMD Developer Tools Team
// Date:        29/6/2010
// ---------------------------------------------------------------------------
gtRedBlackTreeNode::~gtRedBlackTreeNode()
{
    if (_pValue != NULL)
    {
        delete _pValue;
        _pValue = NULL;
    }
}


// ---------------------------------------------------------------------------
// Name:        gtRedBlackTree::gtRedBlackTree
// Description: Constructor
// Author:      AMD Developer Tools Team
// Date:        29/6/2010
// ---------------------------------------------------------------------------
gtRedBlackTree::gtRedBlackTree()
    : _pRootNode(NULL), _pNilNode(NULL)
{
    // Create the Nil node:
    _pNilNode = new gtRedBlackTreeNode;


    // Create the root node:
    _pRootNode = new gtRedBlackTreeNode;


    // Initialize the root and Nil nodes:
    initializeRootAndNilNodes();
}


// ---------------------------------------------------------------------------
// Name:        gtRedBlackTree::~gtRedBlackTree
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        29/6/2010
// ---------------------------------------------------------------------------
gtRedBlackTree::~gtRedBlackTree()
{
    // Initialize the tree:
    clear();

    // Delete the nil and root nodes:
    delete _pNilNode;
    _pNilNode = NULL;
    delete _pRootNode;
    _pRootNode = NULL;
}


// ---------------------------------------------------------------------------
// Name:        gtRedBlackTree::clear
// Description: Deletes all tree nodes and initializes the tree.
// Author:      AMD Developer Tools Team
// Date:        12/7/2010
// ---------------------------------------------------------------------------
void gtRedBlackTree::clear()
{
    // Contains nodes that should be deleted:
    gtStack<gtRedBlackTreeNode*> nodesToBeDeleted;

    // Initialize the destruction by marking the root's direct children as "should be deleted":
    if (_pRootNode->_pLeftChild != _pNilNode)
    {
        nodesToBeDeleted.push(_pRootNode->_pLeftChild);
    }

    if (_pRootNode->_pRightChild != _pNilNode)
    {
        nodesToBeDeleted.push(_pRootNode->_pRightChild);
    }

    // Traverse the sub trees and destruct them:
    while (!nodesToBeDeleted.empty())
    {
        gtRedBlackTreeNode* pCurrNode = nodesToBeDeleted.top();
        nodesToBeDeleted.pop();

        if (pCurrNode != NULL)
        {
            if (pCurrNode->_pLeftChild != _pNilNode)
            {
                nodesToBeDeleted.push(pCurrNode->_pLeftChild);
            }

            if (pCurrNode->_pRightChild != _pNilNode)
            {
                nodesToBeDeleted.push(pCurrNode->_pRightChild);
            }

            delete pCurrNode;
            pCurrNode = NULL;
        }
    }

    // Initialize the fields of the root and the Nil nodes:
    initializeRootAndNilNodes();
}


// ---------------------------------------------------------------------------
// Name:        gtRedBlackTree::deleteNode
// Description:
//  Removes a node from the tree, deletes it and it's held data.
//
// Arguments:   pNode - The node to be deleted from the tree.
// Author:      AMD Developer Tools Team
// Date:        29/6/2010
// ---------------------------------------------------------------------------
void gtRedBlackTree::deleteNode(gtRedBlackTreeNode* pNode)
{
    gtRedBlackTreeNode* pAidNodeB = ((pNode->_pLeftChild == _pNilNode) || (pNode->_pRightChild == _pNilNode)) ? pNode : getSuccessorOf(pNode);
    gtRedBlackTreeNode* pAidNodeA = (pAidNodeB->_pLeftChild == _pNilNode) ? pAidNodeB->_pRightChild : pAidNodeB->_pLeftChild;

    // Assignment of pAidNodeB->_pParent to pAidNodeA->_pParent is intentional:
    if (_pRootNode == (pAidNodeA->_pParent = pAidNodeB->_pParent))
    {
        _pRootNode->_pLeftChild = pAidNodeA;
    }
    else
    {
        if (pAidNodeB == pAidNodeB->_pParent->_pLeftChild)
        {
            pAidNodeB->_pParent->_pLeftChild = pAidNodeA;
        }
        else
        {
            pAidNodeB->_pParent->_pRightChild = pAidNodeA;
        }
    }

    if (pAidNodeB != pNode)
    {
        // pAidNodeB should not be _pNilNode in this case
        GT_ASSERT_EX((pAidNodeB != _pNilNode), L"pAidNodeB is _pNilNode in deleteNode \n");

        // pAidNodeB is the node to splice out and pAidNodeA is its child:
        pAidNodeB->_pLeftChild = pNode->_pLeftChild;
        pAidNodeB->_pRightChild = pNode->_pRightChild;
        pAidNodeB->_pParent = pNode->_pParent;
        pNode->_pLeftChild->_pParent = pNode->_pRightChild->_pParent = pAidNodeB;

        if (pNode == pNode->_pParent->_pLeftChild)
        {
            pNode->_pParent->_pLeftChild = pAidNodeB;
        }
        else
        {
            pNode->_pParent->_pRightChild = pAidNodeB;
        }

        if (!(pAidNodeB->_isRed))
        {
            pAidNodeB->_isRed = pNode->_isRed;
            deleteFixUp(pAidNodeA);
        }
        else
        {
            pAidNodeB->_isRed = pNode->_isRed;
        }

        delete pNode;
        pNode = NULL;

        // Sanity check:
        GT_ASSERT_EX(!_pNilNode->_isRed, L"_pNilNode not black in gtRedBlackTree::Delete");

#ifdef GT_CHECK_RB_TREE_ASSUMPTIONS
        {
            // Deeper sanity check:
            checkAssumptions();
        }
#endif
    }
    else
    {
        if (!(pAidNodeB->_isRed))
        {
            deleteFixUp(pAidNodeA);
        }

        delete pAidNodeB;
        pAidNodeB = NULL;

        // Sanity check:
        GT_ASSERT_EX(!_pNilNode->_isRed, L"_pNilNode not black in gtRedBlackTree::Delete");

#ifdef GT_CHECK_RB_TREE_ASSUMPTIONS
        {
            // Deeper sanity check:
            checkAssumptions();
        }
#endif
    }
}


// ---------------------------------------------------------------------------
// Name:        gtRedBlackTree::insert
// Description:
//  Inserts a new value into the tree. Before calling insert pValue should have its _key set.
//  This function returns a pointer to the newly inserted node which is guaranteed
//  to be valid until this node is deleted. What this means is if another data structure
//  stores this pointer then the tree does not need to be searched when this is to be deleted.
//
// Arguments:   pValue - The value to be inserted into the tree.
// Return Val:  gtRedBlackTreeNode* - The tree node representing the inserted value.
// Author:      AMD Developer Tools Team
// Date:        29/6/2010
// ---------------------------------------------------------------------------
gtRedBlackTreeNode* gtRedBlackTree::insert(gtAutoPtr<gtRedBlackTreeValue>& aptrNode)
{
    // Will point the tree node associated with the input value:
    gtRedBlackTreeNode* pValueAssociatedNode = NULL;

    // Get ownership on the value's memory:
    gtRedBlackTreeValue* pValue = aptrNode.releasePointedObjectOwnership();
    GT_IF_WITH_ASSERT(pValue != NULL)
    {
        // Create a node that will contain the value to be inserted:
        pValueAssociatedNode = new gtRedBlackTreeNode(pValue);


        // Insert the node into the tree and mark it as "red":
        treeInsertHelp(pValueAssociatedNode);
        pValueAssociatedNode->_isRed = true;

        gtRedBlackTreeNode* pAidNode = NULL;

        while (pValueAssociatedNode->_pParent->_isRed)
        {
            // Use sentinel instead of checking for _pRootNode:
            if (pValueAssociatedNode->_pParent == pValueAssociatedNode->_pParent->_pParent->_pLeftChild)
            {
                pAidNode = pValueAssociatedNode->_pParent->_pParent->_pRightChild;

                if (pAidNode->_isRed)
                {
                    pValueAssociatedNode->_pParent->_isRed = false;
                    pAidNode->_isRed = false;
                    pValueAssociatedNode->_pParent->_pParent->_isRed = true;
                    pValueAssociatedNode = pValueAssociatedNode->_pParent->_pParent;
                }
                else
                {
                    if (pValueAssociatedNode == pValueAssociatedNode->_pParent->_pRightChild)
                    {
                        pValueAssociatedNode = pValueAssociatedNode->_pParent;
                        leftRotate(pValueAssociatedNode);
                    }

                    pValueAssociatedNode->_pParent->_isRed = false;
                    pValueAssociatedNode->_pParent->_pParent->_isRed = true;
                    rightRotate(pValueAssociatedNode->_pParent->_pParent);
                }
            }
            else
            {
                // Case for pValueAssociatedNode->_pParent == pValueAssociatedNode->_pParent->_pParent->_pRightChild:
                // This part is just like the section above with _pLeftChild and _pRightChild interchanged
                pAidNode = pValueAssociatedNode->_pParent->_pParent->_pLeftChild;

                if (pAidNode->_isRed)
                {
                    pValueAssociatedNode->_pParent->_isRed = false;
                    pAidNode->_isRed = false;
                    pValueAssociatedNode->_pParent->_pParent->_isRed = true;
                    pValueAssociatedNode = pValueAssociatedNode->_pParent->_pParent;
                }
                else
                {
                    if (pValueAssociatedNode == pValueAssociatedNode->_pParent->_pLeftChild)
                    {
                        pValueAssociatedNode = pValueAssociatedNode->_pParent;
                        rightRotate(pValueAssociatedNode);
                    }

                    pValueAssociatedNode->_pParent->_isRed = false;
                    pValueAssociatedNode->_pParent->_pParent->_isRed = true;
                    leftRotate(pValueAssociatedNode->_pParent->_pParent);
                }
            }
        }

        _pRootNode->_pLeftChild->_isRed = false;

#ifdef GT_CHECK_RB_TREE_ASSUMPTIONS
        {
            // Deeper sanity check:
            checkAssumptions();
        }
#endif

        // Sanity check:
        GT_ASSERT_EX(!_pNilNode->_isRed, L"_pNilNode not red in gtRedBlackTree::insert");
        GT_ASSERT_EX(!_pRootNode->_isRed, L"_pRootNode not _isRed in gtRedBlackTree::insert");
    }

    return pValueAssociatedNode;
}


// ---------------------------------------------------------------------------
// Name:        gtRedBlackTree::search
// Description: Inputs a key and searches the tree for a node representing this key.
// Arguments:   key - The input key.
// Return Val:  gtRedBlackTreeNode* - Will get the node representing the input key, or NULL if such
//                                    key does not exist in the tree.
// Author:      AMD Developer Tools Team
// Date:        12/7/2010
// ---------------------------------------------------------------------------
gtRedBlackTreeNode* gtRedBlackTree::search(gtUInt64 key) const
{
    gtRedBlackTreeNode* retVal = NULL;

    // Start the search from the root node:
    gtRedBlackTreeNode* pCurrNode = _pRootNode;

    // While we didn't get to the Nil node:
    while (pCurrNode != _pNilNode)
    {
        // Get the current node's key:
        gtUInt64 currNodeKey = pCurrNode->_key;

        if (currNodeKey == key)
        {
            // The current node's key is the key that we are looking for:
            retVal = pCurrNode;
            break;
        }
        else if (key < currNodeKey)
        {
            // Drill into the left node:
            pCurrNode = pCurrNode->_pLeftChild;
        }
        else
        {
            // Drill into the right node:
            pCurrNode = pCurrNode->_pRightChild;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gtRedBlackTree::searchEqualOrLowerThan
// Description: Inputs a key and searches the tree for a node representing this key.
//              If such node does not exist, returns (if exists) the node who's key
//              is the highest key which is lower than the input key.
// Arguments:   key - The input key.
// Return Val:  gtRedBlackTreeNode* - Will get the found node, or NULL if such node does not exist.
// Author:      AMD Developer Tools Team
// Date:        14/7/2010
// ---------------------------------------------------------------------------
gtRedBlackTreeNode* gtRedBlackTree::searchEqualOrLowerThan(gtUInt64 key) const
{
    gtRedBlackTreeNode* retVal = NULL;

    // Start the search from the root node:
    gtRedBlackTreeNode* pCurrNode = _pRootNode;

    // While we didn't get to the Nil node:
    while (pCurrNode != _pNilNode)
    {
        // Get the current node's key:
        gtUInt64 currNodeKey = pCurrNode->_key;

        if (currNodeKey == key)
        {
            // The current node's key is the key that we are looking for:
            retVal = pCurrNode;
            break;
        }
        else if (key < currNodeKey)
        {
            // Drill into the left node:
            pCurrNode = pCurrNode->_pLeftChild;
        }
        else
        {
            // We found a node who's key is lower than the key we are looking for.
            // Use it as our return candidate until we find a better one:
            retVal = pCurrNode;

            // Drill into the right node:
            pCurrNode = pCurrNode->_pRightChild;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gtRedBlackTree::getPredecessorOf
// Description: Inputs a node and returns it's predecessor.
// Arguments:   pNode - The input node.
// Return Val:  gtRedBlackTreeNode* - Will get the node's predecessor, or NULL if no predecessor exist.
// Author:      AMD Developer Tools Team
// Date:        29/6/2010
// ---------------------------------------------------------------------------
gtRedBlackTreeNode* gtRedBlackTree::getPredecessorOf(gtRedBlackTreeNode* pNode) const
{
    gtRedBlackTreeNode* pPredecessorNode = NULL;

    if (_pNilNode != (pPredecessorNode = pNode->_pLeftChild))
    {
        // Assignment to pPredecessorNode is intentional:
        while (pPredecessorNode->_pRightChild != _pNilNode)
        {
            // Returns the maximum of the _pLeftChild subtree of pNode:
            pPredecessorNode = pPredecessorNode->_pRightChild;
        }
    }
    else
    {
        pPredecessorNode = pNode->_pParent;

        while (pNode == pPredecessorNode->_pLeftChild)
        {
            if (pPredecessorNode == _pRootNode)
            {
                return NULL;
            }

            pNode = pPredecessorNode;
            pPredecessorNode = pPredecessorNode->_pParent;
        }
    }

    if (pPredecessorNode == _pNilNode)
    {
        pPredecessorNode = NULL;
    }

    return pPredecessorNode;
}


// ---------------------------------------------------------------------------
// Name:        gtRedBlackTree::getSuccessorOf
// Description: Inputs a node and returns it's successor.
// Arguments:   pNode - The input node.
// Return Val:  gtRedBlackTreeNode* - Will get the node's successor, or NULL if no successor exist.
// Author:      AMD Developer Tools Team
// Date:        29/6/2010
// ---------------------------------------------------------------------------
gtRedBlackTreeNode* gtRedBlackTree::getSuccessorOf(gtRedBlackTreeNode* pNode) const
{
    gtRedBlackTreeNode* pSuccessorNode = NULL;

    if (_pNilNode != (pSuccessorNode = pNode->_pRightChild))
    {
        // Assignment to pSuccessorNode is intentional:
        while (pSuccessorNode->_pLeftChild != _pNilNode)
        {
            // Returns the minimum of the _pRightChild subtree of pNode:
            pSuccessorNode = pSuccessorNode->_pLeftChild;
        }
    }
    else
    {
        pSuccessorNode = pNode->_pParent;

        while (pNode == pSuccessorNode->_pRightChild)
        {
            // Sentinel used instead of checking for _pNilNode:
            pNode = pSuccessorNode;
            pSuccessorNode = pSuccessorNode->_pParent;
        }

        if (pSuccessorNode == _pRootNode)
        {
            pSuccessorNode = _pNilNode;
        }
    }

    if (pSuccessorNode == _pNilNode)
    {
        pSuccessorNode = NULL;
    }

    return pSuccessorNode;
}


// ---------------------------------------------------------------------------
// Name:        gtRedBlackTree::enumerate
// Description:
//   Returns a stack containing pointers to tree nodes containing values which are
//   in the [low,high] range.
//   The caller is responsible for deallocating the stack.
//
// Arguments:   low, high - Defined the [low,high] range.
//              nodes - Will get the output stack.
// Author:      AMD Developer Tools Team
// Date:        29/6/2010
// ---------------------------------------------------------------------------
void gtRedBlackTree::enumerate(gtUInt64 low, gtUInt64 high, gtStack<gtRedBlackTreeNode*>& nodes) const
{
    gtRedBlackTreeNode* pCurrNode = _pRootNode->_pLeftChild;
    gtRedBlackTreeNode* pLastBest = NULL;

    while (_pNilNode != pCurrNode)
    {
        if (pCurrNode->_key > high)
        {
            pCurrNode = pCurrNode->_pLeftChild;
        }
        else
        {
            pLastBest = pCurrNode;
            pCurrNode = pCurrNode->_pRightChild;
        }
    }

    while ((pLastBest != NULL) && (low <= pLastBest->_key))
    {
        nodes.push(pLastBest);
        pLastBest = getPredecessorOf(pLastBest);
    }
}


// ---------------------------------------------------------------------------
// Name:        gtRedBlackTree::checkAssumptions
// Description: Verifies that the tree meets the basic red-black tree assumptions.
// Author:      AMD Developer Tools Team
// Date:        29/6/2010
// ---------------------------------------------------------------------------
void gtRedBlackTree::checkAssumptions() const
{
    GT_ASSERT(_pNilNode->_key == 0);
    GT_ASSERT(_pRootNode->_key == GT_UINT64_MAX);
    GT_ASSERT(_pNilNode->_pValue == NULL);
    GT_ASSERT(_pRootNode->_pValue == NULL);
    GT_ASSERT(_pNilNode->_isRed == false);
    GT_ASSERT(_pRootNode->_isRed == false);
}


// ---------------------------------------------------------------------------
// Name:        gtRedBlackTree::leftRotate
// Description:
//  Rotates as described in "Introduction To Algorithms" by Cormen, Leiserson, Rivest (Chapter 14).
//  Basically this makes the _pParent of pNode be to the _pLeftChild of pNode, pNode the _pParent of
//  its _pParent before the rotation and fixes other pointers accordingly.
// Arguments:   pNode - The node to be "rotated"
// Author:      AMD Developer Tools Team
// Date:        29/6/2010
// ---------------------------------------------------------------------------
void gtRedBlackTree::leftRotate(gtRedBlackTreeNode* pNode)
{
    // I originally wrote this function to use the sentinel for _pNilNode to avoid checking for _pNilNode.  However this introduces a
    // very subtle bug because sometimes this function modifies the _pParent pointer of _pNilNode.  This can be a problem if a
    // function which calls leftRotate also uses the _pNilNode sentinel and expects the _pNilNode sentinel's _pParent pointer to be unchanged
    // after calling this function. For example, when deleteFixUP calls leftRotate it expects the _pParent pointer of _pNilNode to be unchanged.
    gtRedBlackTreeNode* pAidNode = pNode->_pRightChild;
    pNode->_pRightChild = pAidNode->_pLeftChild;

    // Used to use sentinel here and do an unconditional assignment instead of testing for _pNilNode:
    if (pAidNode->_pLeftChild != _pNilNode)
    {
        pAidNode->_pLeftChild->_pParent = pNode;
    }

    pAidNode->_pParent = pNode->_pParent;

    // Instead of checking if pNode->_pParent is the _pRootNode as in the book, we count on the _pRootNode sentinel to implicitly take care of this case:
    if (pNode == pNode->_pParent->_pLeftChild)
    {
        pNode->_pParent->_pLeftChild = pAidNode;
    }
    else
    {
        pNode->_pParent->_pRightChild = pAidNode;
    }

    pAidNode->_pLeftChild = pNode;
    pNode->_pParent = pAidNode;

    // Sanity check:
    GT_ASSERT_EX(!_pNilNode->_isRed, L"_pNilNode not red in gtRedBlackTree::leftRotate");

#ifdef GT_CHECK_RB_TREE_ASSUMPTIONS
    {
        // Deeper sanity check:
        checkAssumptions();
    }
#endif
}


// ---------------------------------------------------------------------------
// Name:        gtRedBlackTree::rightRotate
// Description:
//   Rotates as described in "Introduction To Algorithms" by Cormen, Leiserson, Rivest (Chapter 14).
//   Basically this makes the _pParent of pNode be to the _pLeftChild of pNode, pNode the _pParent of
//   its _pParent before the rotation and fixes other pointers accordingly.
// Arguments:   pNode - The node to be "rotated"
// Author:      AMD Developer Tools Team
// Date:        29/6/2010
// ---------------------------------------------------------------------------
void gtRedBlackTree::rightRotate(gtRedBlackTreeNode* pNode)
{
    // I originally wrote this function to use the sentinel for _pNilNode to avoid checking for _pNilNode.
    // However this introduces a very subtle bug because sometimes this function modifies the _pParent pointer of _pNilNode.
    // This can be a problem if a function which calls leftRotate also uses the _pNilNode sentinel and expects the _pNilNode
    // sentinel's _pParent pointer to be unchanged after calling this function. For example, when deleteFixUP calls leftRotate
    // it expects the _pParent pointer of _pNilNode to be unchanged.
    gtRedBlackTreeNode* pAidNode = pNode->_pLeftChild;
    pNode->_pLeftChild = pAidNode->_pRightChild;

    // Used to use sentinel here and do an unconditional assignment instead of testing for _pNilNode:
    if (_pNilNode != pAidNode->_pRightChild)
    {
        pAidNode->_pRightChild->_pParent = pNode;
    }

    // Instead of checking if pAidNode->_pParent is the _pRootNode as in the book, we count on the _pRootNode sentinel
    // to implicitly take care of this case:
    pAidNode->_pParent = pNode->_pParent;

    if (pNode == pNode->_pParent->_pLeftChild)
    {
        pNode->_pParent->_pLeftChild = pAidNode;
    }
    else
    {
        pNode->_pParent->_pRightChild = pAidNode;
    }

    pAidNode->_pRightChild = pNode;
    pNode->_pParent = pAidNode;

    // Sanity check:
    GT_ASSERT_EX(!_pNilNode->_isRed, L"_pNilNode not red in gtRedBlackTree::rightRotate");

#ifdef GT_CHECK_RB_TREE_ASSUMPTIONS
    {
        // Deeper sanity check:
        checkAssumptions();
    }
#endif
}


// ---------------------------------------------------------------------------
// Name:        gtRedBlackTree::treeInsertHelp
// Description:
//   Inserts pNode into the tree as if it were a regular binary tree using the
//   algorithm described in "Introduction To Algorithms" by Cormen et al.
//   This function is only intended to be called by the insert function and not by the user.
// Arguments:   pNode - The node to be inserted.
// Author:      AMD Developer Tools Team
// Date:        29/6/2010
// ---------------------------------------------------------------------------
void gtRedBlackTree::treeInsertHelp(gtRedBlackTreeNode* pNode)
{
    // This function should only be called by gtRedBlackTree::insert

    pNode->_pLeftChild = _pNilNode;
    pNode->_pRightChild = _pNilNode;

    gtRedBlackTreeNode* pAidNodeB = _pRootNode;
    gtRedBlackTreeNode* pAidNodeA = _pRootNode->_pLeftChild;

    while (pAidNodeA != _pNilNode)
    {
        pAidNodeB = pAidNodeA;

        if (pAidNodeA->_key > pNode->_key)
        {
            pAidNodeA = pAidNodeA->_pLeftChild;
        }
        else
        {
            // pAidNodeA->_key <= pNode->_key:
            pAidNodeA = pAidNodeA->_pRightChild;
        }
    }

    pNode->_pParent = pAidNodeB;

    if ((pAidNodeB == _pRootNode) || (pAidNodeB->_key > pNode->_key))
    {
        pAidNodeB->_pLeftChild = pNode;
    }
    else
    {
        pAidNodeB->_pRightChild = pNode;
    }

    // Sanity check:
    GT_ASSERT_EX(!_pNilNode->_isRed, L"_pNilNode not _isRed in gtRedBlackTree::treeInsertHelp");
}


// ---------------------------------------------------------------------------
// Name:        gtRedBlackTree::deleteFixUp
// Description:
//  Performs rotations and changes colors to restore _isRed-black properties
//  after a node is deleted.
// Arguments:   pNode - the child of the spliced out node in deleteNode.
// Author:      AMD Developer Tools Team
// Date:        29/6/2010
// ---------------------------------------------------------------------------
void gtRedBlackTree::deleteFixUp(gtRedBlackTreeNode* pNode)
{
    gtRedBlackTreeNode* pRootLeftChild = _pRootNode->_pLeftChild;
    gtRedBlackTreeNode* pAidNode = NULL;

    while ((!pNode->_isRed) && (pRootLeftChild != pNode))
    {
        if (pNode == pNode->_pParent->_pLeftChild)
        {
            pAidNode = pNode->_pParent->_pRightChild;

            if (pAidNode->_isRed)
            {
                pAidNode->_isRed = false;
                pNode->_pParent->_isRed = true;
                leftRotate(pNode->_pParent);
                pAidNode = pNode->_pParent->_pRightChild;
            }

            if ((!pAidNode->_pRightChild->_isRed) && (!pAidNode->_pLeftChild->_isRed))
            {
                pAidNode->_isRed = true;
                pNode = pNode->_pParent;
            }
            else
            {
                if (!pAidNode->_pRightChild->_isRed)
                {
                    pAidNode->_pLeftChild->_isRed = false;
                    pAidNode->_isRed = true;
                    rightRotate(pAidNode);
                    pAidNode = pNode->_pParent->_pRightChild;
                }

                pAidNode->_isRed = pNode->_pParent->_isRed;
                pNode->_pParent->_isRed = false;
                pAidNode->_pRightChild->_isRed = false;
                leftRotate(pNode->_pParent);

                // This is done to exit while loop:
                pNode = pRootLeftChild;
            }
        }
        else
        {
            // The code below is has _pLeftChild and _pRightChild switched from above:
            pAidNode = pNode->_pParent->_pLeftChild;

            if (pAidNode->_isRed)
            {
                pAidNode->_isRed = false;
                pNode->_pParent->_isRed = true;
                rightRotate(pNode->_pParent);
                pAidNode = pNode->_pParent->_pLeftChild;
            }

            if ((!pAidNode->_pRightChild->_isRed) && (!pAidNode->_pLeftChild->_isRed))
            {
                pAidNode->_isRed = true;
                pNode = pNode->_pParent;
            }
            else
            {
                if (!pAidNode->_pLeftChild->_isRed)
                {
                    pAidNode->_pRightChild->_isRed = false;
                    pAidNode->_isRed = true;
                    leftRotate(pAidNode);
                    pAidNode = pNode->_pParent->_pLeftChild;
                }

                pAidNode->_isRed = pNode->_pParent->_isRed;
                pNode->_pParent->_isRed = false;
                pAidNode->_pLeftChild->_isRed = false;
                rightRotate(pNode->_pParent);

                // This is done to exit while loop:
                pNode = pRootLeftChild;
            }
        }
    }

    pNode->_isRed = false;

#ifdef GT_CHECK_RB_TREE_ASSUMPTIONS
    {
        // Deeper sanity check:
        checkAssumptions();
    }
#endif

    // Sanity check:
    GT_ASSERT_EX(!_pNilNode->_isRed, L"_pNilNode not black in gtRedBlackTree::deleteFixUp");
}


// ---------------------------------------------------------------------------
// Name:        gtRedBlackTree::initializeRootAndNilNodes
// Description: Initializes the fields of the root and the Nil nodes.
// Author:      AMD Developer Tools Team
// Date:        15/7/2010
// ---------------------------------------------------------------------------
void gtRedBlackTree::initializeRootAndNilNodes()
{
    // Initialize the Nil node:
    _pNilNode->_pLeftChild = _pNilNode;
    _pNilNode->_pRightChild = _pNilNode;
    _pNilNode->_pParent = _pNilNode;
    _pNilNode->_isRed = false;
    _pNilNode->_key = 0;
    _pNilNode->_pValue = NULL;

    // Initialize the root node:
    _pRootNode->_pParent = _pNilNode;
    _pRootNode->_pLeftChild = _pNilNode;
    _pRootNode->_pRightChild = _pNilNode;
    _pRootNode->_key = GT_UINT64_MAX;
    _pRootNode->_isRed = false;
    _pRootNode->_pValue = NULL;
}

