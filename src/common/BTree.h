/* @file BTree.h 
 * 
 * ============================================================================ 
 * Description: 
 * ============================================================================ 
 * - This B-Tree implementation is of a B-linked Tree with full 
 *   concurrency support, with minimal locking.
 * - All reads can be achieved with zero locking and no risk of dead-locking. 
 * - All insertions, deletions, splits, and merges require minimal locking. 
 * - A right-most pointer is maintained at the leaf-node level to allow for 
 *   concurrent traversal of the tree as well as simpler range searching.
 * - A maximum-key value is maintained for every node to simplify searching. 
 * - All code is based on B+-Trees, with B-linked Tree changes. 
 *  
 * ============================================================================ 
 * Usage: 
 * ============================================================================ 
 * - This is a template-based B-Tree implementation, requiring an 
 *   object data type with an (uint64_t addr()) method that is
 *   compared against an arbitrary key data type (usually uint64_t
 *   or int64_t -- preferrably unsigned).
 * - Instantiate a BTree<...> object and then begin to add user-defined 
 *   objects to the tree.
 *  
 * ============================================================================ 
 * Standards: 
 * ============================================================================ 
 * - All threading should be libpthread-based. 
 * - All locks and atomics should be C++11/C++14/C++17 based 
 *   where possible, unless this is incompatible with using
 *   libpthread-based threads.
 * - libpthread-based threads are used to maintain backwards 
 *   compatibility with ZMQ (which is incompatible with C++11 or later
 *   built-in threading features) -- the ZMQ threads just don't work
 *   at all if started from a newly created C++11 or later built-in thread. 
 */
#ifndef BTREE_H_
#define BTREE_H_
//=============================================================================
// INCLUDES
//=============================================================================
#include <stdint.h>
#include <deque>
#include <algorithm>
#include <memory>
#include <utility>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <tuple>
#include <functional>

//=============================================================================
// DEFINITIONS
//=============================================================================
#define DEFAULT_ORDER (10)

template <typename K, typename T>
class BTree;

/**
 * The type of the current node. 
 */
typedef enum {
    Leaf    = 0,
    Regular = 1
} BTreeNodeType;

/**
 * The direction of the current node. 
 */
typedef enum {
    Left    = 0,
    Right   = 1
} BTreeNodeDirection;

/**
 * Defines an individual B-Tree node that can be independently 
 * locked depending on the situation (insertion, deletion, 
 * splitting, merging, etc...). 
 *
 */
template <typename K, typename T>
class BTreeNode {
public:
    BTreeNode (void)
      : m_lock(),
        m_unstable(),
        m_entries(),
        m_children(),
        m_left(nullptr),
        m_right(nullptr),
        m_type(BTreeNodeType::Regular)
    {
    }

    BTreeNode (BTreeNodeType t)
      : m_lock(),
        m_unstable(),
        m_entries(),
        m_children(),
        m_left(nullptr),
        m_right(nullptr),
        m_type(t)
    {        
    }

    /**
     * Builds a new leaf node.
     * 
     * @return std::shared_ptr<BTreeNode<K,T>>
     */
    static std::shared_ptr<BTreeNode<K,T>> new_leaf (void) {
        return std::make_shared<BTreeNode<K,T>>(BTreeNodeType::Leaf);
    }

    /**
     * Builds a new regular interior node.
     * 
     * @return std::shared_ptr<BTreeNode<K,T>>
     */
    static std::shared_ptr<BTreeNode<K,T>> new_regular (void) {
        return std::make_shared<BTreeNode<K,T>>(BTreeNodeType::Regular);
    }

    /**
     * Retrieves the length of the node's children array. 
     *  
     * @return uint64_t Length of children array.
     */
    uint64_t len (void) {
        return m_children.size() + 1;
    }

    /**
     * Splits the current node. 
     *  
     * @note This is a locking operation. 
     * 
     * @return std::pair<std::shared_ptr<T>,
     *                   std::shared_ptr<BTreeNode<K,T>>> Pair data.
     * 
     */
    std::pair<std::shared_ptr<T>, std::shared_ptr<BTreeNode<K,T>>>
        split (void)
    {
        //Lock the current node.
        lock();

        std::shared_ptr<BTreeNode<K,T>> sibling =
            std::make_shared<BTreeNode<K,T>>(m_type);

        //Update right pointer first, since the sibling is new in
        //the overall B-Tree.
        sibling->m_right = m_right;

        //Always update this before beginning to erase things.
        //
        //This node starts out empty, so the searching algorithm
        //needs to take this situation into account when searching.
        m_right = sibling;
        
        unsigned int split_at = m_entries.size() / 2;

        //Locks m_unstable due to change in array ops.
        m_unstable.lock();

        std::shared_ptr<T> dev = m_entries[split_at];
        m_entries.erase(m_entries.begin() + split_at);

        std::shared_ptr<BTreeNode<K,T>> node = m_children[split_at];
        m_children.erase(m_children.begin() + split_at);

        //Unlocks m_unstable in preparation for next set of array ops.
        m_unstable.unlock();

        std::shared_ptr<T> device = nullptr;
        std::shared_ptr<BTreeNode<K,T>> child = nullptr;

        size_t count = m_entries.size();
        for (size_t i = (size_t)split_at; i < count; i++) {
            m_unstable.lock();

            device = m_entries[m_entries.size() - 1];
            child = m_children[m_children.size() - 1];
           
            m_entries.pop_back();
            m_children.pop_back();
            
            sibling->add_key(device->addr(), device, child);

            m_unstable.unlock();
        }

        sibling->add_left_child(node);

        //Unlock the current node.
        //
        //All locking for the sibling node occurs within the add_* 
        //routines and therefore doesn't need to occur in this routine
        //to avoid a dead-lock situation.
        unlock();

        return std::pair<std::shared_ptr<T>, std::shared_ptr<BTreeNode<K,T>>>(
            dev, sibling
        );
    }

    /**
     * Manually sets the m_left pointer to point to another shared 
     * pointer of another B-Tree node. 
     *  
     * @param node Left node shared pointer.
     */
    void add_left_child (
        std::shared_ptr<BTreeNode<K,T>> node
    ) {
        m_left = node;
    }

    /**
     * Adds a key to the B-Tree and automatically performs 
     * any splitting that is necessary. 
     * 
     * @param key Key
     * @param device Device
     * @param tree B-Tree
     * @return bool true on success, false on failure.
     */
    bool add_key (
        const K key,
        std::shared_ptr<T> device,
        std::shared_ptr<BTreeNode<K,T>> tree
    ) {
        size_t index = 0;
        auto pos = find_closest_index(key, index);

        if (pos == BTreeNodeDirection::Right) {
            index += 1;
        }       

        m_unstable.lock();

        if (index >= m_entries.size()) {
            m_entries.push_back(device);
            m_children.push_back(tree);
        } else {
            m_entries.insert(m_entries.begin() + index, device);
            m_children.insert(m_children.begin() + index, tree);
        }

        m_unstable.unlock();

        return true;
    }

    /**
     * Removes a key based on the key value. 
     *  
     * This recursively searches the B-Tree for compatible key 
     * values. 
     * 
     * @param id 
     */
    void remove_key (K id) {
        auto next_child = get_child(id);
        if (next_child != nullptr) {
            if (next_child->m_type != BTreeNodeType::Leaf) {
                next_child->remove_key(id);
            } else {
                next_child->remove_key_r(id);
            }
        }
    }

    void remove_key_empty (K id) {
        auto next_child = get_child(id);
        if (next_child != nullptr) {
            if (next_child->m_type != BTreeNodeType::Leaf) {
                next_child->remove_key_empty(id);
            } else {
                next_child->remove_key_empty_r(id);
            }
        }
    }

    /**
     * Actually performs the recursive key removal operation. 
     *  
     * @param key Key to remove recursively. 
     *  
     * @return std::tuple<K, 
     *                    std::shared_ptr<T>,
     *                    std::shared_ptr<BTreeNode<K,T>>> Removed
     *                    key data.
     */
    std::tuple<K, std::shared_ptr<T>, std::shared_ptr<BTreeNode<K,T>>>
        remove_key_r (const K key)
    {
        size_t index = 0;
        auto k = find_closest_index(key, index);

        if (k == BTreeNodeDirection::Left) {
            m_unstable.lock();

            auto tree = m_left;
            if (m_left != nullptr) {
                m_left = nullptr;
            }

            m_unstable.unlock();

            return std::make_tuple(key, nullptr, tree);
        } else {
            m_unstable.lock();

            auto dev = m_entries[index];
            auto tree = m_children[index];

            m_entries.erase(m_entries.begin() + index);
            m_children.erase(m_children.begin() + index);

            m_unstable.unlock();

            return std::make_tuple(dev->addr(), dev, tree);
        }
    }

    std::tuple<K, std::shared_ptr<T>, std::shared_ptr<BTreeNode<K,T>>>
        remove_key_empty_r (const K key)
    {
        size_t index = 0;
        auto k = find_closest_index_empty(key, index);

        if (k == BTreeNodeDirection::Left) {
            m_unstable.lock();

            auto tree = m_left;
            if (m_left != nullptr) {
                m_left = nullptr;
            }

            m_unstable.unlock();

            return std::make_tuple(key, nullptr, tree);
        } else {
            m_unstable.lock();

            auto dev = m_entries[index];
            auto tree = m_children[index];

            m_entries.erase(m_entries.begin() + index);
            m_children.erase(m_children.begin() + index);

            m_unstable.unlock();

            return std::make_tuple(dev->addr(), dev, tree);
        }
    }

    /**
     * Locates the next closest index to the given search key. 
     *  
     * @note This method returns the right most entry that is <= the
     *       search key.
     *  
     * @param key Search key.
     * @param index Index. 
     *  
     * @return BTreeNodeDirection Current node's direction.
     */
    BTreeNodeDirection find_closest_index (const K key, size_t &index) {
        BTreeNodeDirection retVal = BTreeNodeDirection::Left;
        index = 0;

        for (size_t i = 0; i < m_entries.size(); i++) {
            auto p = m_entries[i];
            if (p->addr() <= key) {
                index = i;
                retVal = BTreeNodeDirection::Right;
            } else {
                break;
            }
        }

        return retVal;
    }

    BTreeNodeDirection find_closest_index_empty (const K key, size_t &index) {
        BTreeNodeDirection retVal = BTreeNodeDirection::Left;
        index = 0;

        for (size_t i = 0; i < m_entries.size(); i++) {
            auto p = m_entries[i];
            if ((p->addr() <= key)) {
                index = i;
                retVal = BTreeNodeDirection::Right;
            } else {
                break;
            }
        }

        return retVal;
    }

    /**
     * Retrieves a specific device from the m_entries list.
     *  
     * @note This method depends on m_unstable being unlocked. 
     *  
     * @param key Key to look up.
     * @return std::shared_ptr<T> Device shared pointer. 
     */
    std::shared_ptr<T> get_device (const K key) {
        const std::lock_guard<std::mutex> _lock_guard(m_unstable);

        if (m_entries.size() == 0) {
            return nullptr;
        }

        for (int i = (m_entries.size() - 1); i >= 0; i--) {
            auto device = m_entries[i];

            if (device != nullptr) {
                if (((device->addr() == key) && (device->size() == 0)) ||
                    ((device->addr() <= key) &&
                    ((device->addr() + device->size()) > key))) 
                {
                    return device;
                }
            }
        }

        return nullptr;
    }

    /**
     * Retrieves the appropriate child node shared pointer based on 
     * the relevant key search value. 
     * 
     * @param key Key to search for.
     * @return std::shared_ptr<BTreeNode<K,T>> Child node shared 
     *         pointer.
     */
    std::shared_ptr<BTreeNode<K,T>> get_child (const K key) {
        const std::lock_guard<std::mutex> _lock_guard(m_unstable);
        size_t index = 0;

        auto pos = find_closest_index(key, index);

        if (pos == BTreeNodeDirection::Left) {
            return m_left;
        } else {
            return m_children[index];
        }
    }

    std::shared_ptr<BTreeNode<K,T>> get_child_empty (const K key) {
        //const std::lock_guard<std::mutex> _lock_guard(m_unstable);
        size_t index = 0;

        auto pos = find_closest_index_empty(key, index);
        
        auto retVal = m_left;

        if (pos == BTreeNodeDirection::Left) {
            PrintSimpleLogMessage(LEVEL_DEBUG, "gc_empty==%d, addr=0x%llx (1)",
                              pos, m_left != nullptr ? m_left->m_entries[0]->addr() : NULL);
            retVal = m_left;            
        } else {
            PrintSimpleLogMessage(LEVEL_DEBUG, "gc_empty==%d, addr=0x%llx (2)",
                              pos, m_children[index] != nullptr ? m_children[index]->m_entries[0]->addr() : NULL);
            retVal = m_children[index];
        }

        for (auto child : m_children) {
            if (child != nullptr) {
                child->get_child_empty(key);
            }
        }

        if (m_left != nullptr && m_left->m_children.size() > 0) {
            for (auto child : m_left->m_children) {
                if (child != nullptr) {
                    child->get_child_empty(key);
                }
            }
        }

        size_t c = 0;
        bool isDone = false;
        bool modRetVal = false;
        while (!isDone) {
            c = 0;
            isDone = true;

            if (retVal == nullptr) {
                break;
            }

            for (; c < retVal->m_entries.size(); c++) {
                if (retVal->m_entries[c]->size() == 0) {
                    //m_unstable.lock();
                    retVal->m_entries.erase(retVal->m_entries.begin() + c);
                    retVal->m_children.erase(retVal->m_children.begin() + c);
                    //m_unstable.unlock();
                    isDone = false;
                    modRetVal = true;
                    c = retVal->m_entries.size() + 1;
                    continue;
                }
            }
        }

        return modRetVal ? retVal : retVal;
    }

    /**
     * Merges the current node with another node 'b'.
     *  
     * @note This method locks m_unstable and will block any 
     *       split or other merge that occurs while this method
     *       is still running. 
     *  
     * @param b Secondary node to merge into this node.
     */
    void merge (std::shared_ptr<BTreeNode<K,T>> b) {
        const std::lock_guard<std::mutex> _lock_guard(m_unstable);

        for (auto e : b->m_entries) {
            m_entries.push_back(e);
        }
        b->m_entries.clear();

        for (auto c : b->m_children) {
            m_children.push_back(c);
        }
        b->m_children.clear();

        //@todo This method should split the current node into
        //      chunks that are the size of the B-Tree's maximum
        //      order size.
    }

    /**
     * Locks the node's mutex. 
     *  
     * This should only be done for either 
     * an insertion or a deletion operation. 
     *  
     * Searching shouldn't require the use of 
     * any locks whatsoever. 
     */ 
    void lock (void) {
        m_lock.lock();
    }

    /**
     * Attempts to lock the node's mutex. 
     *  
     * @return bool true if lock was successful, false if lock 
     *         unavailable.
     */
    bool try_lock (void) {
        return m_lock.try_lock();
    }

    /**
     * Unlocks the node's mutex. 
     *  
     * This should only rarely occur. 
     */
    void unlock (void) {
        m_lock.unlock();
    }

    friend BTree<K,T>;
   
protected:
    //These are the only mutexes for this node.
    //m_unstable is only ever locked when there is a 
    //critical section of multiple changes to m_entries/m_children
    //that need to happen at the same time.
    std::mutex m_lock;
    std::mutex m_unstable;

    //Entries array stores actual data in the B-Tree.
    std::deque<std::shared_ptr<T>> m_entries;

    //Children array stores shared pointers to other B-Tree nodes.
    std::deque<std::shared_ptr<BTreeNode<K,T>>> m_children;

    //The left pointer is a hold-over from the Rust code
    //for the original B-Tree implementation. This shouldn't
    //really be necessary any more.
    std::shared_ptr<BTreeNode<K,T>> m_left;

    //The right pointer gets updated periodically to 
    //allow for concurrent searching while inserting or
    //deleting nodes from the tree.
    std::shared_ptr<BTreeNode<K,T>> m_right;

    //This variable defines the type of node.
    BTreeNodeType m_type;
};

/**
 * This is the full B-linked Tree implementation. 
 */ 
template <typename K, typename T>
class BTree {
public:
    /**
     * Creates a default B-Tree with a default hard-coded order 
     * value (e.g. maximum node width or length). 
     */
    BTree (void)
      : m_length(0),
        m_order(DEFAULT_ORDER)
    {
        m_rootNode = nullptr;
    }

    /**
     * Creates a B-Tree of a specific order (e.g. maximum node width 
     * or length). 
     * 
     * @param order 
     */
    BTree (size_t order) 
      : m_length(0),
        m_order(order)
    {
        m_rootNode = nullptr;
    }

public:
    /* Adds a value into the BTree.
     *  
     * This is an alias of the insert method. 
     *  
     * @note This routine doesn't properly handle duplicate values, 
     *       so don't do this since its currently unsupported. 
     *  
     * @param value Value to add to the BTree.
     */
    void add (T value) {
        insert(value);
    }

    void add (std::shared_ptr<T> value) {
        insert(value);
    }

    /**
     * Inserts a value into the BTree.
     *  
     * @note This routine doesn't properly handle duplicate values, 
     *       so don't do this since its currently unsupported. 
     *  
     * @param value Value to add to the BTree.
     */
    void insert (T value) {
        auto node = m_rootNode;

        if (node == nullptr) {
            node = BTreeNode<K,T>::new_leaf();
        } else {
            m_rootNode = nullptr;
        }

        auto root = add_r(node, std::make_shared<T>(value), true);

        cond.notify_one();
        m_rootNode = std::get<0>(root);
    }

    void insert (std::shared_ptr<T> value) {
        auto node = m_rootNode;

        if (node == nullptr) {
            node = BTreeNode<K,T>::new_leaf();
        } else {
            m_rootNode = nullptr;
        }

        auto root = add_r(node, value, true);

        cond.notify_one();
        m_rootNode = std::get<0>(root);
    }

private:
    /**
     * Recursively adds an entry to the current B-Tree. 
     * 
     * @param node Current B-Tree node.
     * @param device Device to add.
     * @param is_root Is the root node?
     * @return std::tuple<std::shared_ptr<BTreeNode<K,T>>, 
     *                    std::shared_ptr<T>,
     *                    std::shared_ptr<BTreeNode<K,T>>> Added
     *                    node information/pointers.
     */
    std::tuple<std::shared_ptr<BTreeNode<K,T>>, std::shared_ptr<T>, std::shared_ptr<BTreeNode<K,T>>>
        add_r (std::shared_ptr<BTreeNode<K,T>> node,
               std::shared_ptr<T> device,
               bool is_root)
    {
        auto id = device->addr();

        switch (node->m_type) {
        case BTreeNodeType::Leaf:
            {
                node->add_key(id, device, nullptr);
                m_length += 1;
                break;
            }
        case BTreeNodeType::Regular:
            {
                auto x = node->remove_key_r(id);
                auto n = add_r(std::get<2>(x), device, false);
                if (std::get<1>(x) == nullptr) {
                    node->add_left_child(std::get<0>(n));
                } else {
                    node->add_key(std::get<0>(x),
                                  std::get<1>(x),
                                  std::get<0>(n));
                }

                //auto split_result = std::get<1>(n);
                if (std::get<1>(n) != nullptr) {
                    auto new_id = std::get<1>(n);
                    node->add_key(new_id->addr(),
                                  std::get<1>(n),
                                  std::get<2>(n));
                }
                break;
            }
        default:
            {
                break;
            }
        }

        if (node->len() > m_order) {
            auto p = node->split();
            if (is_root) {
                auto parent = BTreeNode<K,T>::new_regular();

                parent->add_left_child(node);
                parent->add_key(p.first->addr(), p.first, p.second);

                return std::make_tuple(parent, nullptr, nullptr);
            } else {
                return std::make_tuple(node, p.first, p.second);
            }
        } else {
            return std::make_tuple(node, nullptr, nullptr);
        }
    }

public:
    /**
     * Removes a key or value based on its address. 
     * 
     * @param key Key or address to remove.
     */
    void remove (K key) {
        remove_key(key);
    }

    /**
     * Removes a key or value based on its address. 
     * 
     * @param key Key or address to remove.
     */
    void remove_empty (K key) {
        //remove_key_empty(key);
        PrintSimpleLogMessage(LEVEL_DEBUG, "remove_empty(0x%llx)", key);
        delete_empty_r(m_rootNode, key);
    }

    /**
     * Determines if the current BTree is valid or not.
     * 
     * @return bool true if valid, false if invalid
     */
    bool is_a_valid_btree (void) {
        auto root = m_rootNode;

        if (root != nullptr) {
            auto total = validate(root, 0);
            return (std::get<0>(total) && 
                    std::get<1>(total)) == std::get<2>(total);
        } else {
            return false;
        }
    }

private:
    /**
     * Validates that the current B-Tree is valid. 
     * 
     * @param node Current B-Tree node pointer.
     * @param level Current B-Tree level. 
     * @return std::tuple<bool,size_t,size_t> 
     */
    std::tuple<bool,size_t,size_t> validate (
        std::shared_ptr<BTreeNode<K,T>> node,
        size_t level
    ) {
        switch (node->m_type) {
        case BTreeNodeType::Leaf:
            {
                return std::make_tuple(
                    (node->len() <= m_order), level, level
                );
            }
        case BTreeNodeType::Regular:
            {
                auto min_children = level > 0 ? (m_order / 2) : 2;
                auto key_rules = node->len() <= m_order && node->len() >= min_children;
                auto total = std::make_tuple(key_rules, ((2^64) - 1), level);

                for (size_t i=0; i<node->m_children.size(); i++) {
                    auto tree = node->m_children[i];
                    auto stats = validate(tree, level+1);

                    total = std::make_tuple(
                        std::get<0>(total) && std::get<1>(total),
                        std::min(std::get<1>(stats), std::get<2>(total)),
                        std::min(std::get<2>(stats), std::get<2>(total))
                    );
                }

                if (node->m_left != nullptr) {
                    auto tree = node->m_left;
                    auto stats = validate(tree, level+1);

                    total = std::make_tuple(
                        std::get<0>(total) && std::get<1>(total),
                        std::min(std::get<1>(stats), std::get<2>(total)),
                        std::min(std::get<2>(stats), std::get<2>(total))
                    );
                }

                return total;                
            }
        default:
            {
                return std::make_tuple(false,0,0);
            }
        }
    }

public:
    /**
     * Finds a specific value that contains the specified address 
     * based on an address offset and size. 
     *  
     * @param id Address to lookup.
     * @return std::shared_ptr<T> Return Value 
     */
    std::shared_ptr<T> find (K id) {
        if (m_rootNode != nullptr) {
            return find_r(m_rootNode, id);
        } else {
            return nullptr;
        }
    }

    /**
     * Finds a range of data based on address and size within 
     * the range (start,stop) inclusive. 
     * 
     * 
     * @param start Start address (inclusive)
     * @param stop Stop address (inclusive)
     * @return std::deque<std::shared_ptr<T>> retVal
     * 
     */
    std::deque<std::shared_ptr<T>> find_range (K start, K stop) {
        std::deque<std::shared_ptr<T>> retVal;

        if (m_rootNode != nullptr) {
            size_t nextAddr = start;
            while (nextAddr <= stop) {
                auto k = find_r(m_rootNode, nextAddr);

                if (k != nullptr) {
                    nextAddr = k->addr() + k->size();
                } else {
                    nextAddr += 1;
                }

                retVal.push_back(k);
            }
        }

        //Sort the results. This should be nearly O(n) duration at most in the normal case.
        std::sort(retVal.begin(), retVal.end(), [](const std::shared_ptr<T> &a,
                                                   const std::shared_ptr<T> &b) -> bool {
                                                    return a->addr() < b->addr();
                                                   });
        return retVal;
    }


private:
    /**
     * Performs a recursive search based on a given search key. 
     * 
     * @param node Current B-Tree node pointer.
     * @param id Current search key.
     * @return std::shared_ptr<T> Found value pointer.
     */
    std::shared_ptr<T> find_r (
        std::shared_ptr<BTreeNode<K,T>> node,
        K id
    ) {
        if (node == nullptr) {
            return nullptr;
        }

        auto device = node->get_device(id);

        if (device != nullptr) {
            return device;
        }

        if (node->m_type != BTreeNodeType::Leaf) {
            auto tree = node->get_child(id);

            return find_r(tree, id);
        }

        return nullptr;
    }

    std::shared_ptr<BTreeNode<K,T>> delete_empty_r (
        std::shared_ptr<BTreeNode<K,T>> node,
        K id
    ) {
        if (node == nullptr) {
            return nullptr;
        }

        auto device = node->get_device(id);

        if (device != nullptr && device->size() == 0) {
            node->get_child_empty(id);
            return node;
        } 
        
        if (/*node->m_type != BTreeNodeType::Leaf*/ true) {
            auto tree = node->get_child_empty(id);
            delete_empty_r(tree, id);
        }

        return nullptr;
    }

public:
    /**
     * Walks the entire BTree and calls a callback function 
     * for each value duriang a depth first search of the tree. 
     * 
     * @param callback Callback function
     */
    void walk (std::function<void(T&,int)> callback) {
        if (m_rootNode != nullptr) {
            walk_in_order(m_rootNode, callback, 0);
        }
    }

    /**
     * Walks the entire BTree and calls a callback function 
     * for each value during a depth first search of the tree. 
     * 
     * @param callback Callback function
     */
    void walk_shared (std::function<void(std::shared_ptr<T>&,int)> &callback) {
        if (m_rootNode != nullptr) {
            walk_in_order_shared(m_rootNode, callback, 0);
        }
    }

    /**
     * Walks the B-Tree in pairs. 
     *  
     * @param callback 
     */
    void walk_pairs (std::function<bool(T&,T&,int)> callback) {
        if (m_rootNode != nullptr) {
            while(walk_pairs_in_order(
                m_rootNode,
                m_rootNode,
                0,
                callback,
                0) == true) {
                continue;
            }
        }
    }

private:
    void walk_in_order (
        std::shared_ptr<BTreeNode<K,T>> node,
        std::function<void(T&,int)> &callback,
        int depth
    ) {
        if (node->m_left != nullptr) {
            walk_in_order(node->m_left, callback, depth+1);            
        }

        for (size_t i = 0; i < node->m_entries.size(); i++) {
            auto k = node->m_entries[i];
            if (k != nullptr) {
                callback(*k.get(), depth);
            }
            auto c = node->m_children[i];
            if (c != nullptr) {
                walk_in_order(c, callback, depth+1);
            }
        }
    }

    void walk_in_order_shared (
        std::shared_ptr<BTreeNode<K,T>> node,
        std::function<void(std::shared_ptr<T>&,int)> &callback,
        int depth
    ) {
        if (node->m_left != nullptr) {
            walk_in_order_shared(node->m_left, callback, depth+1);            
        }

        for (size_t i = 0; i < node->m_entries.size(); i++) {
            auto k = node->m_entries[i];
            if (k != nullptr) {
                callback(k, depth);
            }
            auto c = node->m_children[i];
            if (c != nullptr) {
                walk_in_order_shared(c, callback, depth+1);
            }
        }
    }


    bool walk_pairs_in_order (
        std::shared_ptr<BTreeNode<K,T>> node,
        std::shared_ptr<BTreeNode<K,T>> parentNode,
        size_t index,
        std::function<bool(T&,T&,int)> &callback,
        int depth
    ) {
        bool retVal = false;       
        bool isDone = false;

        while (!isDone) {
            isDone = true;          

            if (node->m_left != nullptr) {
                retVal = walk_pairs_in_order(node->m_left, node, 0, callback, depth+1);

                if (node->m_left != nullptr &&
                    ((node->m_left->m_entries[0]->addr() + node->m_left->m_entries[0]->size()) == node->m_entries[0]->addr()) &&
                    node->m_left->m_entries.size() > 0) {
                    node->m_entries[0]->merge(node->m_left->m_entries[0]);
                    node->m_left = nullptr;
                    isDone = false;
                    continue;
                } else {
                    if (retVal == true) {
                        isDone = false;
                        continue;
                    }
                }
            }
            
            if (node != parentNode &&
                node != parentNode->m_left &&
                node->m_entries.size() > 0 &&
                parentNode->m_entries.size() > index &&
                parentNode->m_children.size() > index &&
                node->m_children.size() > 0 &&
                ((parentNode->m_entries[index]->addr() + parentNode->m_entries[index]->size()) == node->m_entries[0]->addr())) {
                
                while (((parentNode->m_entries[index]->addr() + parentNode->m_entries[index]->size()) == node->m_entries[0]->addr())) {
                    parentNode->m_entries[index]->merge(node->m_entries[0]);
                    if (node->m_entries.size() != 0) {
                        node->m_entries.erase(node->m_entries.begin());
                        node->m_children.erase(node->m_children.begin());
                    }
                }

                retVal = true;
                return retVal;
            }
            
            for (size_t i = 0; i < (node->m_entries.size()); i++) {
                if ((node->m_entries.size() > 0) &&
                    (i < (node->m_entries.size() - 1))) {
                    auto k = node->m_entries[i];
                    auto k2 = node->m_entries[i + 1];
                    if (k != nullptr &&
                        k2 != nullptr) {

                        //If callback returns true then merge both
                        //child nodes.
                        if (callback(*k.get(), *k2.get(), depth)) {
                            k->merge(k2);
                            node->m_entries.erase(node->m_entries.begin() + i + 1);

                            auto c = node->m_children[i];
                            auto c2 = node->m_children[i + 1];

                            if (c != nullptr && 
                                c2 != nullptr) {
                                c->merge(c2);
                                node->m_children.erase(node->m_children.begin() + i + 1);
                            }

                            retVal = true;
                            isDone = false;
                            break;
                        }
                    }
                }

                auto c = node->m_children[i];
                if (c != nullptr) {
                    retVal = walk_pairs_in_order(c, node, i, callback, depth+1);

                    if ((node->m_entries[i]->addr() + 
                         node->m_entries[i]->size()) == parentNode->m_entries[index]->addr()) {
                        parentNode->m_entries[index]->merge(node->m_entries[i]);
                        parentNode->m_children[index]->merge(node->m_children[i]);
                        node->m_entries.erase(node->m_entries.begin() + i);
                        node->m_children.erase(node->m_children.begin() + i);
                        isDone = false;
                        break;
                    }

                    if (retVal == true) {
                        isDone = false;
                        break;
                    }          
                }
            }            
        }

        return retVal;
    }

protected:
    std::shared_ptr<BTreeNode<K,T>> m_rootNode;
    size_t m_length;
    size_t m_order;

    std::condition_variable cond;
};

//=============================================================================
#endif //BTREE_H_
