/** @file AvlTree.h
 *  
 *  C++ implementation of an AVL Tree
 */
#ifndef AVL_TREE_H_
#define AVL_TREE_H_
//=============================================================================
// INCLUDES
//=============================================================================
#include <stdint.h>
#include <vector>
#include "Logging.h"

//=============================================================================
// DEFINITIONS
//=============================================================================
#define MSG_TYPE_AVL_TREE (0x62912a67)

/**
 * The AvlNode object is the basic unit of the AVL tree.
 */
template <class KeyType, class ObjType>
class AvlNode {
public:
    AvlNode (void);
    ~AvlNode (void);

public:
    AvlNode* left;
    AvlNode* right;
    AvlNode* parent;
    KeyType key;
    ObjType data;
};

template <class KeyType, class ObjType>
class AvlTree {
public:
    AvlTree (void);
    ~AvlTree (void);

    /**
     * Search for key within the AVL tree and return a pointer of ObjType or NULL if 
     * the object is not found within the tree. 
     *  
     * @param key Key used for lookup.
     * @param pObj Object pointer or NULL if not found. 
     * @return true on success, false on failure 
     */
    bool find (KeyType key, ObjType* pObj);

    /**
     * Search for objects within a key range key1 : key2 inclusive. 
     *  
     * @param key1 Lower bound
     * @param key2 Upper bound
     * @return std::vector<ObjType> 
     */
    std::vector<ObjType> findRange(KeyType key1, KeyType key2);

    /**
     * Search for keys within a key range key1 : key2 inclusive. 
     *  
     * @param key1 Lower bound
     * @param key2 Upper bound
     * @return std::vector<KeyType> 
     */
    std::vector<KeyType> findKeyRange(KeyType key1, KeyType key2);

    /**
     * Returns the next key in a balanced AVL tree. 
     *  
     * @param key 
     * @return KeyType 
     */
    bool getNextKey (KeyType key, KeyType* pNextKey);

    /**
     * Returns the previous key in a balanced AVL tree. 
     *  
     * @param key 
     * @return KeyType 
     */
    bool getPrevKey (KeyType key, KeyType* pPrevKey);

    /**
     * Inserts an object with a specific key into the AVL tree. 
     *  
     * @param key Key used for insertion
     * @param data Object to be added to the tree.
     * @return int 
     */
    bool insert (KeyType key, ObjType data);

    /**
     * Removes an object from the tree by deleting its key. 
     *  
     * @param key Key used for deletion
     * @return bool true on success, false on failure
     */
    bool remove (KeyType key);

    /**
     * Clears the AVL tree of all contents and frees all memory. 
     *  
     * @return int 
     */
    void clear (void);

    /**
     * Prints out the AVL tree.
     */
    void print (void);

protected:
    /**
     * Recursively prints out the AVL tree.
     * @param pRootNode 
     */
    void print_r (AvlNode<KeyType, ObjType>* pRootNode, int nDepth);

    /**
     * Recursively removes a node from the AVL tree. 
     *  
     * @param ppRootNode 
     * @param key 
     * @return true on success, false on failure 
     */
    bool remove_r (AvlNode<KeyType, ObjType>** ppRootNode, KeyType key);

    /**
     * Recursively search for objects within a key range key1 : key2 inclusive. 
     *  
     * @param key1 Lower bound
     * @param key2 Upper bound
     */
    bool findRange_r (AvlNode<KeyType, ObjType>* pNode, std::vector<ObjType>* pList, KeyType key1, KeyType key2);

    /**
     * Recursively search for keys within a key range key1 : key2 inclusive. 
     *  
     * @param key1 Lower bound
     * @param key2 Upper bound
     */
    bool findKeyRange_r (AvlNode<KeyType, ObjType>* pNode, std::vector<KeyType>* pList, KeyType key1, KeyType key2);

    /**
     * Balances the AVL tree by shifting nodes either left or right.
     */
    void balanceTree (void);

    /**
     * Recursively balances the AVL tree by shifting nodes either left or right.
     *  
     * @param ppRootNode Root node
     * @return int 
     */
    int balanceTree_r (AvlNode<KeyType, ObjType>** ppRootNode);

    /**
     * Recursively clears the AVL tree.
     */
    void clear_r (AvlNode<KeyType, ObjType>* pRootNode);

    /**
     * Shifts a node to the left. 
     *  
     * @param ppRootNode Root node
     * @return int 
     */
    int avlShiftLeft (AvlNode<KeyType, ObjType>** ppRootNode);

    /**
     * Shifts a node to the right.
     *  
     * @param ppRootNode Root node
     * @return int 
     */
    int avlShiftRight (AvlNode<KeyType, ObjType>** ppRootNode);

    /**
     * Returns the right most node. 
     *  
     * @param pNode 
     * @return AvlNode<KeyType,ObjType>* 
     */
    AvlNode<KeyType, ObjType>* avlRight (AvlNode<KeyType, ObjType>* pNode);

    /**
     * Returns the left most node. 
     *  
     * @param pNode 
     * @return AvlNode<KeyType,ObjType>* 
     */
    AvlNode<KeyType, ObjType>* avlLeft (AvlNode<KeyType, ObjType>* pNode);

protected:
    AvlNode<KeyType,ObjType>* m_pRootNode;
    uint64_t m_nDepth;   
};

//=============================================================================
// NODE IMPLEMENTATION
//=============================================================================
template <class KeyType, class ObjType>
AvlNode<KeyType,ObjType>::AvlNode (void) :
    left(NULL),
    right(NULL),
    parent(NULL)
{
}

template <class KeyType, class ObjType>
AvlNode<KeyType,ObjType>::~AvlNode (void) {
}

//=============================================================================
// TREE IMPLEMENTATION
//=============================================================================
template <class KeyType, class ObjType>
AvlTree<KeyType,ObjType>::AvlTree (void) :
    m_pRootNode(NULL),
    m_nDepth(0)
{
}

template <class KeyType, class ObjType>
AvlTree<KeyType,ObjType>::~AvlTree (void) {
    clear();

    if ( m_pRootNode ) {
        delete m_pRootNode;
        m_pRootNode = NULL;
    }
}

template<class KeyType, class ObjType>
bool AvlTree<KeyType, ObjType>::find (KeyType key, ObjType* pObj) {
    bool retValue = false;
    AvlNode<KeyType, ObjType>* pNode = m_pRootNode;

#if 0
    PrintLogMessage(
       LEVEL_DEBUG,
       MSG_TYPE_AVL_TREE,
       "AvlTree::find(key=%llx, pObj=%llx)",
       key, pObj
    );
#endif

    if ( !pNode ) {
        #if 0
        PrintLogMessage(
            LEVEL_ERROR,
            MSG_TYPE_AVL_TREE,
            "AvlTree::find(key=%llx, pObj=%llx, pNode=%p)",
            (uint64_t)key,
            (uint64_t)pObj,
            pNode
        );
        #endif
        goto ErrorExit;
    }
    
    //Simple case
    if ( pNode->key == key ) {
        if (pObj) {
            *pObj = pNode->data;
        }
        retValue = true;
    } else {
        //Locate the key within the tree
        //If the entire tree is traversed then retValue will never be set, resulting in a NULL return value
        while ( pNode ) {

            #if 0
            PrintLogMessage(
                LEVEL_DEBUG,
                MSG_TYPE_AVL_TREE,
                "AvlTree::find(key=%llx, pObj=%llx, pNode=%p, pNode->key=%llx, right=%p, left=%p)",
                (uint64_t)key,
                (uint64_t)pObj,
                pNode,
                (uint64_t)pNode->key,
                pNode->right,
                pNode->left
            );
            #endif

            //Check for key equivalence
            if ( key == pNode->key ) {
                if (pObj) {
                    *pObj = pNode->data;
                }
                retValue = true;
                break;
            }

            //Select the next node
            if ( key > pNode->key ) {
                pNode = pNode->right;
            } else {
                pNode = pNode->left;
            }
        }
    }

ErrorExit:
    return retValue;
}

template<class KeyType, class ObjType>
bool AvlTree<KeyType, ObjType>::insert (KeyType key, ObjType data) {
    AvlNode<KeyType, ObjType>* pNode;
    AvlNode<KeyType, ObjType>* pPrevNode;
    ObjType objValue;
    bool retValue = false;

    //PrintSimpleLogMessage(LEVEL_ERROR, "---------------------------");
    //PrintSimpleLogMessage(LEVEL_ERROR, "Inserting %08llx", key);
    //print();

    //Check to see if the key is already in the tree
    if ( find(key, &objValue) ) {
        goto ErrorExit;
    }

    //Check to see if the root node is present.
    //If it isn't present, put the key in that location
    if ( !m_pRootNode ) {
        m_pRootNode = new AvlNode<KeyType, ObjType>();
        m_pRootNode->data = data;
        m_pRootNode->key = key;
    } else {
        //Locate the proper location to insert the node
        pNode = m_pRootNode;

        //Locate an empty leaf pointer
        while ( pNode ) {
            pPrevNode = pNode;
            if ( key > pNode->key ) {
                pNode = pNode->right;
            } else {
                pNode = pNode->left;
            }
        }

        //Using the previous node, add a new node in the correct location and update pNode
        if ( key > pPrevNode->key ) {
            pPrevNode->right = new AvlNode<KeyType, ObjType>();
            pNode = pPrevNode->right;
            pNode->parent = pPrevNode;
        } else {
            pPrevNode->left = new AvlNode<KeyType, ObjType>();
            pNode = pPrevNode->left;
            pNode->parent = pPrevNode;
        }
        pNode->key = key;
        pNode->data = data;

        //Balance the tree
        balanceTree();
    }

    //PrintSimpleLogMessage(LEVEL_ERROR, "+++++++++++++++++++++++++++++++++");
    //print();

    retValue = true;
ErrorExit:
    return retValue;
}

template<class KeyType, class ObjType>
bool AvlTree<KeyType, ObjType>::remove (KeyType key) {
    AvlNode<KeyType, ObjType>* pNode = m_pRootNode;
    bool retValue = false;

    if ( !pNode ) {
        goto ErrorExit;
    }

    //print();
    //PrintSimpleLogMessage(LEVEL_ERROR, "Removing key = %08llx", key);

    retValue = remove_r(&m_pRootNode, key);

    //PrintSimpleLogMessage(LEVEL_ERROR, "##########################", key);
    //print();

    //Balance the tree
    balanceTree();

    //PrintSimpleLogMessage(LEVEL_ERROR, "!!!!!!!!!!!!!!!!!!!!!!!!!!", key);
    //print();

ErrorExit:
    return retValue;
}

template<class KeyType, class ObjType>
bool AvlTree<KeyType, ObjType>::remove_r (AvlNode<KeyType, ObjType>** ppRootNode, KeyType key) {
    bool retValue = false;
    AvlNode<KeyType, ObjType>* k;

    if ( !ppRootNode ||
         !(*ppRootNode) ) {
        goto ErrorExit;
    }

    k = *ppRootNode;

    if ( k->key == key ) {
        if (k->right) {
            *ppRootNode = k->right;
            k->right->parent = k->parent;
            k->right->left = k->left;
        } else if ( k->left ) {
            //AvlNode<KeyType, ObjType>* p = avlRight(k->left);
            *ppRootNode = k->left;
            //(*ppRootNode)->
            k->left->right = k->right;
            k->left->parent = k->parent;

            //if (k->right) {
            //    p->right = (*ppRootNode)->left;
            //    p->right->parent = k->right;
            //} else {
            //    //k->left->right = p->right;
            //    //k->left->parent = (*ppRootNode)->parent;
            //    //(*ppRootNode) = k->left;
            //}
        } else {
            *ppRootNode = k->left;
        }

        delete k;

        retValue = true;
        goto ErrorExit;
    }

    retValue = (key > k->key) ? remove_r(&k->right, key) : remove_r(&k->left, key);
    
ErrorExit:
    return retValue;
}

template<class KeyType, class ObjType>
void AvlTree<KeyType, ObjType>::print (void) {
    PrintLogMessage(
       LEVEL_DEBUG,
       MSG_TYPE_AVL_TREE,
       "AvlTree(0x%llx)",
       this
    );

    print_r(m_pRootNode, 0);
}

template<class KeyType, class ObjType>
void AvlTree<KeyType, ObjType>::print_r (AvlNode<KeyType, ObjType>* pRootNode, int nDepth) {
    if ( pRootNode ) {
        //Print right
        print_r(pRootNode->right, nDepth + 1);
        
        PrintLogMessage(
           LEVEL_DEBUG,
           MSG_TYPE_AVL_TREE,
           "%sAvlNode(key=0x%08llx, value=0x%08llx depth=%d) : Parent(key=0x%08llx)",
           GetDepthSpaces(nDepth), pRootNode->key, pRootNode->data, nDepth,
           pRootNode->parent ? pRootNode->parent->key : 0xFFFFFFFFFFFFFFFF
        );

        //Print left
        print_r(pRootNode->left, nDepth + 1);
    }
}

template<class KeyType, class ObjType>
void AvlTree<KeyType, ObjType>::clear (void) {
    if ( m_pRootNode ) {
        clear_r(m_pRootNode);
        delete m_pRootNode;
        m_pRootNode = NULL;
    }
}

template<class KeyType, class ObjType>
void AvlTree<KeyType, ObjType>::clear_r (AvlNode<KeyType, ObjType>* pRootNode) {
    if ( pRootNode ) {
        #if 0
        PrintLogMessage(
            LEVEL_DEBUG,
            MSG_TYPE_AVL_TREE,
            "AvlTree::clear_r(rootKey=%llx, left=%p, right=%p)",
            pRootNode->key,
            pRootNode->left,
            pRootNode->right
        );
        #endif


        clear_r(pRootNode->left);
        clear_r(pRootNode->right);

        if ( pRootNode->left ) {
            delete pRootNode->left;
        }

        if ( pRootNode->right ) {
            delete pRootNode->right;
        }

        pRootNode->left = NULL;
        pRootNode->right = NULL;
        pRootNode->parent = NULL;
    }
}

template<class KeyType, class ObjType>
void AvlTree<KeyType, ObjType>::balanceTree (void) {
#if 0
    PrintLogMessage(
       LEVEL_DEBUG,
       MSG_TYPE_AVL_TREE,
       "AvlTree::balanceTree()"
    );
    PrintLogMessage(
       LEVEL_DEBUG,
       MSG_TYPE_AVL_TREE,
       "---------------------------------"
    );

    print();

    PrintLogMessage(
       LEVEL_DEBUG,
       MSG_TYPE_AVL_TREE,
       "+++++++++++++++++++++++++++++++++"
    );
#endif

    if ( m_pRootNode ) {
        balanceTree_r(&m_pRootNode);
    }

#if 0
    print();

    PrintLogMessage(
       LEVEL_DEBUG,
       MSG_TYPE_AVL_TREE,
       "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$"
    );
#endif
}

template<class KeyType, class ObjType>
int AvlTree<KeyType, ObjType>::balanceTree_r (AvlNode<KeyType, ObjType>** ppRootNode) {
    AvlNode<KeyType, ObjType>* pRoot = *ppRootNode;

    if ( !ppRootNode || 
         !(*ppRootNode) ) {
        return 0;
    }

    int rhs = balanceTree_r(&pRoot->right);
    int lhs = balanceTree_r(&pRoot->left);

#if 0
    PrintLogMessage(
       LEVEL_DEBUG,
       MSG_TYPE_AVL_TREE,
       "balanceTree_r (rhs=%d, lhs=%d, rootKey=0x%llx)",
       rhs, lhs, (*ppRootNode)->key
    );
#endif

    if ( (rhs - lhs) > 1 ) {
        avlShiftLeft(ppRootNode);
    } else if ( (rhs - lhs) < -1 ) {
        avlShiftRight(ppRootNode);
    }

    return (rhs >= lhs) ? rhs + 1 : lhs + 1;
}

template<class KeyType, class ObjType>
int AvlTree<KeyType, ObjType>::avlShiftLeft (AvlNode<KeyType, ObjType>** ppRootNode) { 
    AvlNode<KeyType, ObjType>* k;
    AvlNode<KeyType, ObjType>* r;
    uint64_t s = 0;   

    if ( !ppRootNode ||
         !(*ppRootNode) ||
         !(*ppRootNode)->right ) {
        return -1;
    }

#if 0
    PrintLogMessage(
       LEVEL_DEBUG,
       MSG_TYPE_AVL_TREE,
       "AvlTree::avlShiftLeft(key=0x%llx)",
       (*ppRootNode)->key
    );
#endif

    k = *ppRootNode;
    r = k->right;

    if ( r->right ) {
        s |= 0x00FF;
    }
    if ( r->left ) {
        s |= 0xFF00;
    }

    //Experimental change
    if (s == 0xFF00) {       
        r = k->right->left;
        r->parent->left = NULL;
        r->parent->right = NULL;
        //r->right = r->parent;
        r->right = k->right;
        //r->right = k->parent;
        r->left = k;
        k->right = NULL;

        #if 0
        PrintLogMessage(
            LEVEL_DEBUG,
            MSG_TYPE_AVL_TREE,
            "AvlTree::avlShiftLeft(left=%p, right=%p, key=%llx, key_k=%llx)",
            r->left,
            r->right,
            r->key,
            k->key
        );
        #endif
    }

    *ppRootNode = r;

    //Update parent information
    // 'k' is always the node that was previous parent
    // 'r' is always the new parent node
    r->parent = k->parent;
    k->parent = r;

    switch ( s ) {
    case 0xFF00:
        {
            //Left child only            
            //r->right = k->right->left;
            //k->right = NULL;
            //r->left = k;
            break;
        }
    case 0x00FF:
        {
            //Right child only
            k->right = NULL;
            r->left = k;           
            break;
        }
    case 0xFFFF:
        {
            //Both left and right children
            k->right = r->left;
            r->left->parent = k;
            r->left = k;
            break;
        }
    case 0x0000:
    default:
        {
            //No children
            k->right = NULL;
            r->left = k;
            break;
        }
    }
    
    return 0;
}

template<class KeyType, class ObjType>
int AvlTree<KeyType, ObjType>::avlShiftRight (AvlNode<KeyType, ObjType>** ppRootNode) {
    AvlNode<KeyType, ObjType>* k;
    AvlNode<KeyType, ObjType>* r;
    uint64_t s = 0;

    if ( !ppRootNode ||
         !(*ppRootNode) ||
         !(*ppRootNode)->left ) {
        return -1;
    }

#if 0
    PrintLogMessage(
       LEVEL_DEBUG,
       MSG_TYPE_AVL_TREE,
       "AvlTree::avlShiftRight(key=0x%llx)",
       (*ppRootNode)->key
    );
#endif

    k = *ppRootNode;
    r = k->left;

    if ( r->right ) {
        s |= 0x00FF;
    }
    if ( r->left ) {
        s |= 0xFF00;
    }

    //Experimental change
    #if 0
    if (s == 0x00FF) {       
        r = k->left->right;
        r->parent->left = NULL;
        r->parent->right = NULL;
        //r->right = r->parent;
        r->left = k->left;
        //r->right = k->parent;
        r->right = k;
        k->left = NULL;

        #if 1
        PrintLogMessage(
            LEVEL_DEBUG,
            MSG_TYPE_AVL_TREE,
            "AvlTree::avlShiftRight(left=%p, right=%p, key=%llx, key_k=%llx)",
            r->left,
            r->right,
            r->key,
            k->key
        );
        #endif
    }
    #endif

    *ppRootNode = r;

    //Update parent information
    // 'k' is always the node that was previous parent
    // 'r' is always the new parent node
    r->parent = k->parent;
    k->parent = r;

    switch ( s ) {
    case 0xFF00:
        {
            //Left child only            
            k->left = NULL;
            //r->left = r->right;
            r->right = k;
            break;
        }
    case 0x00FF:
        {
            //Right child only
            k->left = NULL;
            //r->left = r->right;
            k->left = r->right;
            r->right->parent = k;
            r->right = k;
            break;
        }
    case 0xFFFF:
        {
            //Both left and right children
            k->left = r->right;
            r->right->parent = k;
            r->right = k;
            break;
        }
    case 0x0000:
    default:
        {
            //No children
            k->left = NULL;
            r->right = k;
            break;
        }
    }

    return 0;
}

template<class KeyType, class ObjType>
AvlNode<KeyType, ObjType>* AvlTree<KeyType, ObjType>::avlRight (AvlNode<KeyType, ObjType>* pNode) {
    AvlNode<KeyType, ObjType>* k = pNode;
    while ( k->right ) {
        k = k->right;
    }
    return k;
}

template<class KeyType, class ObjType>
AvlNode<KeyType, ObjType>* AvlTree<KeyType, ObjType>::avlLeft (AvlNode<KeyType, ObjType>* pNode) {
    AvlNode<KeyType, ObjType>* k = pNode;
    while ( k->left ) {
        k = k->left;
    }
    return k;
}

template<class KeyType, class ObjType>
std::vector<ObjType> AvlTree<KeyType, ObjType>::findRange (KeyType key1, KeyType key2) {
    std::vector<ObjType> retValue;

    findRange_r(m_pRootNode, &retValue, key1, key2);

    return retValue;
}

template<class KeyType, class ObjType>
std::vector<KeyType> AvlTree<KeyType, ObjType>::findKeyRange (KeyType key1, KeyType key2) {
    std::vector<KeyType> retValue;

    findKeyRange_r(m_pRootNode, &retValue, key1, key2);

    return retValue;
}

template<class KeyType, class ObjType>
bool AvlTree<KeyType, ObjType>::findRange_r (AvlNode<KeyType, ObjType>* pNode, std::vector<ObjType>* pList, KeyType key1, KeyType key2) {
    bool retValue = false;
    if ( !pNode ) {
        goto ErrorExit;
    }

    //Begin traversing the tree starting with the least most value
    if ( pNode->left && key1 <= pNode->key ) {
        //Only recurse left if the minimum key is smaller than current key
        findRange_r(pNode->left, pList, key1, key2);
    }

    //Check the simple case of the current key being within range
    if ( key1 <= pNode->key &&
         key2 >= pNode->key ) {
        pList->push_back(pNode->data);
    }

    //Traverse the tree until the right most value is reached.
    if ( pNode->right && key2 >= pNode->key ) {
        //Only recurse left if the maximum key is larger than current key
        findRange_r(pNode->right, pList, key1, key2);
    }

ErrorExit:
    return retValue;
}

template<class KeyType, class ObjType>
bool AvlTree<KeyType, ObjType>::findKeyRange_r (AvlNode<KeyType, ObjType>* pNode, std::vector<KeyType>* pList, KeyType key1, KeyType key2) {
    bool retValue = false;
    if ( !pNode ) {
        goto ErrorExit;
    }

    //Begin traversing the tree starting with the least most value
    if ( pNode->left && key1 <= pNode->key ) {
        //Only recurse left if the minimum key is smaller than current key
        findKeyRange_r(pNode->left, pList, key1, key2);
    }

    //Check the simple case of the current key being within range
    if ( key1 <= pNode->key &&
         key2 >= pNode->key ) {
        pList->push_back(pNode->key);
    }

    //Traverse the tree until the right most value is reached.
    if ( pNode->right && key2 >= pNode->key ) {
        //Only recurse left if the maximum key is larger than current key
        findKeyRange_r(pNode->right, pList, key1, key2);
    }

ErrorExit:
    return retValue;
}

template<class KeyType, class ObjType>
bool AvlTree<KeyType, ObjType>::getNextKey (KeyType key, KeyType* pNextKey) {
    bool retValue = false;
    AvlNode<KeyType, ObjType>* pNode = m_pRootNode;
    AvlNode<KeyType, ObjType>* pNextNode;

#if 0
    PrintLogMessage(
       LEVEL_DEBUG,
       MSG_TYPE_AVL_TREE,
       "AvlTree::getNextKey(key=%llx)",
       key
    );
#endif

    if ( !pNode || !pNextKey ) {
        goto ErrorExit;
    }
    
    //Locate the key within the tree    
    while ( pNode ) {
        //Check for key equivalence
        if ( key == pNode->key ) {
            break;
        }
        //Select the next node
        if ( key > pNode->key ) {
            pNode = pNode->right;
        } else {
            pNode = pNode->left;
        }
    }
    
    if ( pNode && pNode->right ) {
        //Simple case involving the right child node
        pNextNode = avlLeft(pNode->right);
        retValue = true;
    } else if ( pNode ) {
        //Parent case when parents must be evaluated one by one until a key
        // of greater value than the current key is located.
        while ( pNode ) {
            if ( pNode->key > key ) {
                pNextNode = pNode;
                retValue = true;
                break;
            }
            pNode = pNode->parent;
        }        
    }

    if ( retValue ) {
        *pNextKey = pNextNode->key;
    }

ErrorExit:
    return retValue;
}

template<class KeyType, class ObjType>
bool AvlTree<KeyType, ObjType>::getPrevKey (KeyType key, KeyType* pPrevKey) {
    bool retValue = false;
    AvlNode<KeyType, ObjType>* pNode = m_pRootNode;
    AvlNode<KeyType, ObjType>* pPrevNode;

#if 0
    PrintLogMessage(
       LEVEL_DEBUG,
       MSG_TYPE_AVL_TREE,
       "AvlTree::getPrevKey(key=%llx)",
       key
    );
#endif

    if ( !pNode || !pPrevKey ) {
        goto ErrorExit;
    }
    
    //Locate the key within the tree    
    while ( pNode ) {
        //Check for key equivalence
        if ( key == pNode->key ) {
            break;
        }
        //Select the next node
        if ( key > pNode->key ) {
            pNode = pNode->right;
        } else {
            pNode = pNode->left;
        }
    }
    
    if ( pNode && pNode->left ) {
        //Simple case involving the right child node
        pPrevNode = avlRight(pNode->left);
        retValue = true;
    } else if ( pNode ) {
        //Parent case when parents must be evaluated one by one until a key
        // of greater value than the current key is located.
        while ( pNode ) {
            if ( pNode->key < key ) {
                pPrevNode = pNode;
                retValue = true;
                break;
            }
            pNode = pNode->parent;
        }        
    }

    if ( retValue ) {
        *pPrevKey = pPrevNode->key;
    }

ErrorExit:
    return retValue;
}

//=============================================================================
#endif //AVL_TREE_H_
