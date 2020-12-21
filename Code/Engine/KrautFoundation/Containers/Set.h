#ifndef AE_FOUNDATION_CONTAINERS_SET_H
#define AE_FOUNDATION_CONTAINERS_SET_H

#include "Deque.h"
#include "Stack.h"

namespace AE_NS_FOUNDATION
{
  /*
    erase (iterator, reverse_iterator)
  */
  
  //! A set container that only stores whether an element resides in it or not. Similar to STL::set
  /*! Sets are similar to maps that do not store a value (or only a bool that is always true).
      Sets can be used to reduce an unordered number of elements to only those that are unique.
      Insertion/erasure/lookup in sets is quite fast (O (log n)).
      This container is implemented with a red-black tree, so it will always be a balanced tree.
  */
  template < class KEY, class COMPARE = aeCompareLess<KEY> >
  class aeSet
  {
  private:
    //! Node class that stores one element.
    struct aeNode 
    {
      aeNode (void);

      KEY m_Key;

      aeUInt16 m_uiLevel;
      aeNode* m_pParent;
      aeNode* m_pLink[2];
    };

    //! Base class for all iterators.
    struct iterator_base
    {
      iterator_base (void)                    : m_pElement (NULL) {}
      iterator_base (const iterator_base& cc) : m_pElement (cc.m_pElement) {}
      iterator_base (aeNode* pInit)           : m_pElement (pInit) {}

      bool operator== (const typename aeSet<KEY, COMPARE>::iterator_base& it2) const { return (m_pElement == it2.m_pElement); }
      bool operator!= (const typename aeSet<KEY, COMPARE>::iterator_base& it2) const { return (m_pElement != it2.m_pElement); }

      const KEY&   key   (void) const { return (m_pElement->m_Key);   }

    protected:
      friend class aeSet<KEY, COMPARE>;
      aeNode* m_pElement;

      void forward (void);
      void backward (void);
    };

  public:

    //! Forward iterator to access all elements in sorted order.
    struct iterator : public iterator_base
    {
      iterator (void)               : iterator_base () {}
      iterator (const iterator& cc) : iterator_base (cc.m_pElement) {}
      iterator (aeNode* pInit)      : iterator_base (pInit) {}
      
      // prefix
      void operator++ () { forward ();  }
      void operator-- () { backward (); }
    };

    //! Reverse iterator to access all elements in reversed sorted order.
    struct reverse_iterator : public iterator_base
    {
      reverse_iterator (void)                       : iterator_base () {}
      reverse_iterator (const reverse_iterator& cc) : iterator_base (cc.m_pElement) {}
      reverse_iterator (aeNode* pInit)              : iterator_base (pInit) {}
      
      // prefix
      void operator++ () {backward ();}
      void operator-- () {forward ();}
    };

    //! Constant forward iterator to access all elements in sorted order. Only allows read-access.
    struct const_iterator : public iterator_base
    {
      const_iterator (void)                     : iterator_base () {}
      const_iterator (const const_iterator& cc) : iterator_base (cc.m_pElement) {}
      const_iterator (const iterator& cc)       : iterator_base (cc.m_pElement) {}
      const_iterator (aeNode* pInit)            : iterator_base (pInit) {}
      
      // prefix
      void operator++ () { forward ();  }
      void operator-- () { backward (); }
    };

    //! Constant reverse iterator to access all elements in reversed sorted order. Only allows read-access.
    struct const_reverse_iterator : public iterator_base
    {
      const_reverse_iterator (void)                             : iterator_base () {}
      const_reverse_iterator (const const_reverse_iterator& cc) : iterator_base (cc.m_pElement) {}
      const_reverse_iterator (const reverse_iterator& cc)       : iterator_base (cc.m_pElement) {}
      const_reverse_iterator (aeNode* pInit)                    : iterator_base (pInit) {}
      
      // prefix
      void operator++ () { backward (); }
      void operator-- () { forward ();  }
    };

  public:
    //! Initializes the set to be empty.
    aeSet (void);
    //! Copies the data from the given set into this one.
    aeSet (const aeSet<KEY, COMPARE>& cc);
    //! Destroys all elements in the set.
    ~aeSet ();

    //! Copies the data from the given set into this one.
    void operator= (const aeSet<KEY, COMPARE>& rhs);

    //! Returns whether the set holds no elements. O(1) operation.
    bool empty (void) const;
    //! Returns the number of elements stored in the set. O(1) operation.
    aeUInt32 size (void) const;

    //! Removes all elements from the set. Reduces memory consumption to zero.
    void clear (void);

    //! Returns a forward iterator to the very first element.
    iterator begin (void);
    //! Returns a forward iterator to the element after the last element.
    iterator end (void);
    //! Returns a constant forward iterator to the very first element.
    const_iterator begin (void) const;
    //! Returns a constant forward iterator to the element after the last element.
    const_iterator end (void) const;
    //! Returns an iterator to the last element. For reverse traversal.
    reverse_iterator rbegin (void);
    //! Returns an iterator to the element before the first element. For reverse traversal.
    reverse_iterator rend (void);
    //! Returns a constant iterator to the last element. For reverse traversal.
    const_reverse_iterator rbegin (void) const;
    //! Returns a constant iterator to the element before the first element. For reverse traversal.
    const_reverse_iterator rend (void) const;

    //! Inserts the given element, if not yet existing. O(log n) operation. Returns an iterator to that element.
    iterator insert (const KEY& key);

    //! Removes the given element from the set, if it existed. O(log n) operation.
    void erase (const KEY& key);
    //! Removes the given element from the set. Returns an iterator to the element after it. O(1) operation.
    iterator erase (const iterator& pos);
    //! Removes the given element from the set. Returns an iterator to the element after it. O(1) operation.
    reverse_iterator erase (const reverse_iterator& pos);

    //! Searches for the given element. Returns an iterator to it or "end ()" it it does not exist. O(log n) operation.
    iterator find (const KEY& key);
    //! Returns the iterator to the element with an equal or larger key. O(log n) operation.
    iterator lower_bound (const KEY& key);
    //! Returns the iterator to the element with a LARGER key. O(log n) operation.
    iterator upper_bound (const KEY& key);

    //! Searches for the given element. Returns an iterator to it or "end ()" it it does not exist. O(log n) operation.
    const_iterator find (const KEY& key) const;
    //! Returns the iterator to the element with an equal or larger key. O(log n) operation.
    const_iterator lower_bound (const KEY& key) const;
    //! Returns the iterator to the element with a LARGER key. O(log n) operation.
    const_iterator upper_bound (const KEY& key) const;

  private:
    aeNode* AccquireNode (const KEY& key, int m_uiLevel, aeNode* pParent);
    void ReleaseNode (aeNode* pNode);

    // Red-Black Tree stuff
    // Implementation taken from here:
    // http://eternallyconfuzzled.com/tuts/datastructures/jsw_tut_andersson.aspx

    aeNode* SkewNode (aeNode* root);
    aeNode* SplitNode (aeNode* root);
    aeNode* insert (aeNode* root, const KEY& key, aeNode*& pInsertedNode);
    aeNode* erase (aeNode* root, const KEY& key);

    aeNode* GetLeftMost (void) const;
    aeNode* GetRightMost (void) const;

    aeNode* m_pRoot;
    aeNode m_NilNode;

    aeUInt32 m_uiSize;

    aeDeque<aeNode, ((4096 / sizeof (aeNode)) < 32) ? 32 : (4096 / sizeof (aeNode)), false> m_Elements;
    aeStack<aeNode*> m_FreeElements;
  };
}

#include "Inline/Set.inl"

#endif


