//  Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).
//
// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// Thread safety
// -------------
//
// Writes require external synchronization, most likely a mutex.
// Reads require a guarantee that the SkipList will not be destroyed
// while the read is in progress.  Apart from that, reads progress
// without any internal locking or synchronization.
//
// Invariants:
//
// (1) Allocated nodes are never deleted until the SkipList is
// destroyed.  This is trivially guaranteed by the code since we
// never delete any skip list nodes.
//
// (2) The contents of a Node except for the next/prev pointers are
// immutable after the Node has been linked into the SkipList.
// Only Insert() modifies the list, and it is careful to initialize
// a node and use release-stores to publish the nodes in one or
// more lists.
//
// ... prev vs. next pointer ordering ...
//

#pragma once
#include <assert.h>
#include <stdlib.h>
#include <atomic>
#include "memory/allocator.h"
#include "port/port.h"
#include "util/random.h"

#define MOD 16 // Signal.Jin

namespace ROCKSDB_NAMESPACE {

template<typename Key, class Comparator>
class SkipList {
 private:
  struct Node;

  enum Color {
    RED,
    BLACK
  };

  typedef struct T_node {
    Node* SL_node;
    struct T_node* left;
    struct T_node* right;
    Key key;
  } Tnode; // Tree node structure - Signal.Jin

  typedef struct RB_node {
    Node* SL_node;
    Key key;
    RB_node *left = nullptr;
    RB_node *right = nullptr;
    RB_node *parent = nullptr;
    Color color = BLACK;
  } RBnode; // RB-tree node structure - Signal.Jin
/*
  typedef struct V_node {
    Node* SL_node;
    int height;
    struct V_node* left;
    struct V_node* right;
    Key key;
  } Vnode; // Variable height Tree node structure - Signal.Jin

  typedef struct AVL_node {
    int height;
    Node* SL_node;
    struct AVL_node* left;
    struct AVL_node* right;
    Key key;
  } Anode; // AVL Tree node structure - Signal.Jin

  typedef struct Lifting_node {
    int max_height;
    Key key;
    int ref_cnt;
    int lookup_cnt;
    struct Lifting_node *next;
  } Lnode; // Lifting Table strucutre - Signal.Jin
*/
 public:
  // Create a new SkipList object that will use "cmp" for comparing keys,
  // and will allocate memory using "*allocator".  Objects allocated in the
  // allocator must remain allocated for the lifetime of the skiplist object.
  explicit SkipList(Comparator cmp, Allocator* allocator,
                    int32_t max_height = 7, int32_t branching_factor = 4);
  // No copying allowed
  SkipList(const SkipList&) = delete;
  void operator=(const SkipList&) = delete;

  // Insert key into the list.
  // REQUIRES: nothing that compares equal to key is currently in the list.
  void Insert(const Key& key);

  void Insert_Buf(const Key& key);

  void Insert_B2hSL(const Key& key); // B2hSL insert function - Signal.Jin
  void Insert_RB(const Key& key);
/*
  void Insert_AVL(const Key& key); // AVL insert function - Signal.Jin
  void Insert_Lift(const Key& key); // Dynamic Lifting insert function - Signal.Jin
  void Insert_VH(const Key& key); // Select more node for tree node - Signal.Jin
  void Batch_Insert(const Key& key); // For batch write - Signal.Jin
  void Insert_Seq(const Key* key_); // Lead to sequential write - Signal.Jin
*/
  uint64_t Estimate_Max() const; // Signal.Jin

  // Returns true iff an entry that compares equal to key is in the list.
  bool Contains(const Key& key) const;

  // Call a FindGreatorOrEqual_Cursor function - Signal.Jin
  //bool Contains_Cursor(const Key& key) const;

  // Signal.Jin
  bool Contains_Buf(const Key& key) const;
  bool Contains_B2hSL(const Key& key) const;
  bool Contains_RB(const Key& key) const;
/*
  bool Contains_AVL(const Key& key) const;
  bool Contains_Lift(const Key& key) const;
  bool Contains_VH(const Key& key) const;
*/

  // Return estimated number of entries smaller than `key`.
  uint64_t EstimateCount(const Key& key) const;

  // Iteration over the contents of a skip list
  class Iterator {
   public:
    // Initialize an iterator over the specified list.
    // The returned iterator is not valid.
    explicit Iterator(const SkipList* list);

    // Change the underlying skiplist used for this iterator
    // This enables us not changing the iterator without deallocating
    // an old one and then allocating a new one
    void SetList(const SkipList* list);

    // Returns true iff the iterator is positioned at a valid node.
    bool Valid() const;

    // Returns the key at the current position.
    // REQUIRES: Valid()
    const Key& key() const;

    // Advances to the next position.
    // REQUIRES: Valid()
    void Next();

    // Advances to the previous position.
    // REQUIRES: Valid()
    void Prev();

    // Advance to the first entry with a key >= target
    void Seek(const Key& target);

    // Retreat to the last entry with a key <= target
    void SeekForPrev(const Key& target);

    // Position at the first entry in list.
    // Final state of iterator is Valid() iff list is not empty.
    void SeekToFirst();

    // Position at the last entry in list.
    // Final state of iterator is Valid() iff list is not empty.
    void SeekToLast();

   private:
    const SkipList* list_;
    Node* node_;
    // Intentionally copyable
  };

 private:
  const uint16_t kMaxHeight_;
  const uint16_t kBranching_;
  const uint32_t kScaledInverseBranching_;

  // Immutable after construction
  Comparator const compare_;
  Allocator* const allocator_;    // Allocator used for allocations of nodes

  Node* const head_;

  //mutable Key buf[MOD]; // Signal.Jin
  
  mutable Tnode* root; // Root of Tree structure - Signal.Jin
  mutable RBnode* rootRB;
  mutable RBnode* leafNode;
/*
  mutable Anode* rootAVL; // Root of AVL Tree structure - Signal.Jin
  mutable Vnode* rootVH;
  mutable Lnode* table[MAX_TABLE];
  mutable Key batch[8]; // For batch write - Signal.Jin
  mutable int batch_count;

  mutable Node* single_cursor_; // Single Cursor based skiplist optimization - Signal.Jin
  mutable int cs_level; // To store single cursor's top level - Signal.Jin
*/
  // Modified only by Insert().  Read racily by readers, but stale
  // values are ok.
  std::atomic<int> max_height_;  // Height of the entire list

  // Used for optimizing sequential insert patterns.  Tricky.  prev_[i] for
  // i up to max_height_ is the predecessor of prev_[0] and prev_height_
  // is the height of prev_[0].  prev_[0] can only be equal to head before
  // insertion, in which case max_height_ and prev_height_ are 1.
  Node** prev_;
  int32_t prev_height_;

  inline int GetMaxHeight() const {
    return max_height_.load(std::memory_order_relaxed);
  }

  Node* NewNode(const Key& key, int height);
  int RandomHeight();
  bool Equal(const Key& a, const Key& b) const { return (compare_(a, b) == 0); }
  bool LessThan(const Key& a, const Key& b) const {
    return (compare_(a, b) < 0);
  }

  // Return true if key is greater than the data stored in "n"
  bool KeyIsAfterNode(const Key& key, Node* n) const;

  // Returns the earliest node with a key >= key.
  // Return nullptr if there is no such node.
  Node* FindGreaterOrEqual(const Key& key) const;

  //Node* FindGreaterOrEqual_Cursor(const Key& key) const; // Signal.Jin

  Node* FindGreaterOrEqual_Buf(const Key& key) const;
  Node* FindLessOrEqual_Buf(const Key& key, Node** prev = nullptr) const;

  Node* FindGreaterOrEqual_B2hSL(const Key& key) const; // Signal.Jin
  Node* FindLessThan_B2hSL(const Key& key, Node** prev = nullptr) const; // Signal.Jin

  void AddTreeNode(const Key& key, Node* M_target) const; // Signal.Jin
  Node* SearchTreeNode(const Key& key) const; // Signal.Jin

  Node* FindLessThan_RB(const Key& key, Node** prev = nullptr) const; // Signal.Jin
  Node* FindGreaterOrEqual_RB(const Key& key) const; // Signal.Jin

  void InsertRBTree(const Key& key, Node* sl_node) const;
  void InsertFixUp(RBnode* z) const;
  void RotateL(RBnode* x) const;
  void RotateR(RBnode* y) const;
  Node* SearchRBNode(const Key& key) const;
/*
  Node* FindGreaterOrEqual_AVL(const Key& key) const; // Signal.Jin
  Node* FindLessThan_AVL(const Key& key, Node** prev = nullptr) const; // Signal.Jin

  Node* FindGreaterOrEqual_Lift(const Key& key) const; // Signal.Jin
  Node* FindLessThan_Lift(const Key& key, Node** prev = nullptr) const; // Signal.Jin

  Node* FindGreaterOrEqual_VH(const Key& key) const;
  Node* FindLessThan_VH(const Key& key, Node** prev = nullptr) const;

  void AddLiftNode(const Key& key, int height, bool flag) const; // Signal.Jin
  int hashing(const Key& key) const; // Signal.Jin

  void AddVHNode(const Key& key, Node* M_target, int height) const; // Signal.Jin
  std::pair<Node*, int> SearchVHNode(const Key& key) const; // Signal.Jin

  Anode* AddAVLNode(const Key& key, Node* M_target, Anode* A_target) const; // Signal.Jin
  void AVLBalancing(const Key& key, Anode* A_target) const;
  int getHeight(Anode* A_target) const;
  int getBalanceFactor(Anode* A_target) const;
  Anode* RotateLeft(Anode* A_target) const;
  Anode* RotateRight(Anode* A_target) const;
  Node* SearchAVLNode(const Key& key) const; // Signal.Jin
*/
  // Return the latest node with a key < key.
  // Return head_ if there is no such node.
  // Fills prev[level] with pointer to previous node at "level" for every
  // level in [0..max_height_-1], if prev is non-null.
  Node* FindLessThan(const Key& key, Node** prev = nullptr) const;

  // Return the last node in the list.
  // Return head_ if list is empty.
  Node* FindLast() const;
};

// Implementation details follow
template<typename Key, class Comparator>
struct SkipList<Key, Comparator>::Node {
  explicit Node(const Key& k) : key(k) { }

  Key const key;

  //Key buf[MOD]; // Signal.Jin

  // Accessors/mutators for links.  Wrapped in methods so we can
  // add the appropriate barriers as necessary.
  Node* Next(int n) {
    assert(n >= 0);
    // Use an 'acquire load' so that we observe a fully initialized
    // version of the returned Node.
    return (next_[n].load(std::memory_order_acquire));
  }
  void SetNext(int n, Node* x) {
    assert(n >= 0);
    // Use a 'release store' so that anybody who reads through this
    // pointer observes a fully initialized version of the inserted node.
    next_[n].store(x, std::memory_order_release);
  }

  // No-barrier variants that can be safely used in a few locations.
  Node* NoBarrier_Next(int n) {
    assert(n >= 0);
    return next_[n].load(std::memory_order_relaxed);
  }
  void NoBarrier_SetNext(int n, Node* x) {
    assert(n >= 0);
    next_[n].store(x, std::memory_order_relaxed);
  }

 private:
  // Array of length equal to the node height.  next_[0] is lowest level link.
  std::atomic<Node*> next_[1];
};

template<typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node*
SkipList<Key, Comparator>::NewNode(const Key& key, int height) {
  char* mem = allocator_->AllocateAligned(
      sizeof(Node) + sizeof(std::atomic<Node*>) * (height - 1));
  /*char* mem = allocator_->AllocateAligned(
      sizeof(Node) + sizeof(Node*) * (height - 1));*/
  return new (mem) Node(key);
}

template<typename Key, class Comparator>
inline SkipList<Key, Comparator>::Iterator::Iterator(const SkipList* list) {
  SetList(list);
}

template<typename Key, class Comparator>
inline void SkipList<Key, Comparator>::Iterator::SetList(const SkipList* list) {
  list_ = list;
  node_ = nullptr;
}

template<typename Key, class Comparator>
inline bool SkipList<Key, Comparator>::Iterator::Valid() const {
  return node_ != nullptr;
}

template<typename Key, class Comparator>
inline const Key& SkipList<Key, Comparator>::Iterator::key() const {
  assert(Valid());
  return node_->key;
}

template<typename Key, class Comparator>
inline void SkipList<Key, Comparator>::Iterator::Next() {
  assert(Valid());
  node_ = node_->Next(0);
}

template<typename Key, class Comparator>
inline void SkipList<Key, Comparator>::Iterator::Prev() {
  // Instead of using explicit "prev" links, we just search for the
  // last node that falls before key.
  assert(Valid());
  node_ = list_->FindLessThan(node_->key);
  if (node_ == list_->head_) {
    node_ = nullptr;
  }
}

template<typename Key, class Comparator>
inline void SkipList<Key, Comparator>::Iterator::Seek(const Key& target) {
  node_ = list_->FindGreaterOrEqual(target);
}

template <typename Key, class Comparator>
inline void SkipList<Key, Comparator>::Iterator::SeekForPrev(
    const Key& target) {
  Seek(target);
  if (!Valid()) {
    SeekToLast();
  }
  while (Valid() && list_->LessThan(target, key())) {
    Prev();
  }
}

template <typename Key, class Comparator>
inline void SkipList<Key, Comparator>::Iterator::SeekToFirst() {
  node_ = list_->head_->Next(0);
}

template<typename Key, class Comparator>
inline void SkipList<Key, Comparator>::Iterator::SeekToLast() {
  node_ = list_->FindLast();
  if (node_ == list_->head_) {
    node_ = nullptr;
  }
}

template<typename Key, class Comparator>
int SkipList<Key, Comparator>::RandomHeight() {
  auto rnd = Random::GetTLSInstance();

  // Increase height with probability 1 in kBranching
  int height = 1;
  while (height < kMaxHeight_ && rnd->Next() < kScaledInverseBranching_) {
    height++;
  }
  assert(height > 0);
  assert(height <= kMaxHeight_);
  return height;
}

template<typename Key, class Comparator>
bool SkipList<Key, Comparator>::KeyIsAfterNode(const Key& key, Node* n) const {
  // nullptr n is considered infinite
  return (n != nullptr) && (compare_(n->key, key) < 0);
}

template<typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::
  FindGreaterOrEqual(const Key& key) const {
  // Note: It looks like we could reduce duplication by implementing
  // this function as FindLessThan(key)->Next(0), but we wouldn't be able
  // to exit early on equality and the result wouldn't even be correct.
  // A concurrent insert might occur after FindLessThan(key) but before
  // we get a chance to call Next(0).
  
  Node* x = head_;
  int level = GetMaxHeight() - 1;
  Node* last_bigger = nullptr;
  while (true) {
    assert(x != nullptr);
    Node* next = x->Next(level);
    /*if (next != nullptr) {
      PREFETCH(next->Next(level), 0, 1);
    }*/
    // Make sure the lists are sorted
    assert(x == head_ || next == nullptr || KeyIsAfterNode(next->key, x));
    // Make sure we haven't overshot during our search
    assert(x == head_ || KeyIsAfterNode(key, x));
    int cmp = (next == nullptr || next == last_bigger)
        ? 1 : compare_(next->key, key);
    if (cmp == 0 || (cmp > 0 && level == 0)) {
      return next;
    } else if (cmp < 0) {
      // Keep searching in this list
      x = next;
    } else {
      // Switch to next list, reuse compare_() result
      last_bigger = next;
      level--;
    }
  }
}

template<typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::
  FindGreaterOrEqual_Buf(const Key& key) const {
  // Note: It looks like we could reduce duplication by implementing
  // this function as FindLessThan(key)->Next(0), but we wouldn't be able
  // to exit early on equality and the result wouldn't even be correct.
  // A concurrent insert might occur after FindLessThan(key) but before
  // we get a chance to call Next(0).
  Key quot = key / MOD;

  Node* x = head_;
  int level = GetMaxHeight() - 1;
  Node* last_bigger = nullptr;
  while (true) {
    assert(x != nullptr);
    Node* next = x->Next(level);
    // Make sure the lists are sorted
    assert(x == head_ || next == nullptr || KeyIsAfterNode(next->key, x));
    // Make sure we haven't overshot during our search
    assert(x == head_ || KeyIsAfterNode(key, x));
    int cmp = (next == nullptr || next == last_bigger)
        ? 1 : compare_(next->key, quot);
    if (cmp == 0 || (cmp > 0 && level == 0)) {
      return next;
    } else if (cmp < 0) {
      // Keep searching in this list
      x = next;
    } else {
      // Switch to next list, reuse compare_() result
      last_bigger = next;
      level--;
    }
  }
}

// template<typename Key, class Comparator>
// typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::
//   FindGreaterOrEqual_Cursor(const Key& key) const { // Signal.Jin
//   // Note: This function is a single cursor algorithm in a skip-list
//   // The role of cursor is to remember where it was previsouly
//   // lookup to.
//   // In a single-cursor, only one cursor can exist at a time. - Signal.Jin
  
//   Node* x = head_;
//   int level = GetMaxHeight() - 1;
//   Node* last_bigger = nullptr;
//   /*
//   // Single Cursor Opt 1.
//   if (cs_level != -1) {
//     if (cs_level <= level/2) {
//       // Nothing to do. - Signal.Jin
//     } else {
//       if (compare_(single_cursor_->key, head_->key) > 0) {
//         if (compare_(single_cursor_->key, key) < 0) {
//           x = single_cursor_;
//           level = cs_level;
//         } else if (compare_(single_cursor_->key, key) == 0) {
//           return single_cursor_;
//         }
//         // We can start lookup from a cursor.
//         // Becasue there is a possibility of having the
//         // shortest distance from the cursor. - Signal.Jin
//       }
//     }
//   } 
//   // If Cursor's node height is too low,
//   // Will start from the head node. - Signal.Jin
//   */

//   // Single Cursor Opt 2
//   if (cs_level != -1) {
//     if ((key - single_cursor_->key) > 9) { /*TODO: Setting Value by number of inserts*/
//       // Nothing to do. - Signal.Jin
//     } else {
//       if (compare_(single_cursor_->key, head_->key) > 0) {
//         if (compare_(single_cursor_->key, key) < 0) {
//           x = single_cursor_;
//           level = cs_level;
//         } else if (compare_(single_cursor_->key, key) == 0) {
//           return single_cursor_;
//         }
//         // We can start lookup from a cursor.
//         // Becasue there is a possibility of having the
//         // shortest distance from the cursor. - Signal.Jin
//       }
//     }
//   }

//   while (true) {
//     assert(x != nullptr);
//     Node* next = x->Next(level);
//     // Make sure the lists are sorted
//     assert(x == head_ || next == nullptr || KeyIsAfterNode(next->key, x));
//     // Make sure we haven't overshot during our search
//     if (next != nullptr) {
//       assert(x == head_ || KeyIsAfterNode(key, x));
//     }
//     int cmp = (next == nullptr || next == last_bigger)
//         ? 1 : compare_(next->key, key);
//     if (cmp == 0 || (cmp > 0 && level == 0)) {
//       if (next != nullptr) {
//         single_cursor_ = next;
//         cs_level = level;
//       }
//       return next;
//     } else if (cmp < 0) {
//       // Keep searching in this list
//       x = next;
//     } else {
//       // Switch to next list, reuse compare_() result
//       last_bigger = next;
//       level--;
//     }
//   }
// }

template<typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::
  FindGreaterOrEqual_B2hSL(const Key& key) const {
  // Note: It looks like we could reduce duplication by implementing
  // this function as FindLessThan(key)->Next(0), but we wouldn't be able
  // to exit early on equality and the result wouldn't even be correct.
  // A concurrent insert might occur after FindLessThan(key) but before
  // we get a chance to call Next(0).
  
  Node* x = SearchTreeNode(key);
  if (compare_(x->key, key) == 0) {
    return x;
  }
  int level = GetMaxHeight() - 1; // Max height node + tree - Signal.Jin
  //int level = GetMaxHeight() - 2; // Max height node -> tree - Signal.Jin
  Node* last_bigger = nullptr;
  //printf("\n%lu\n", x->key);
  while (true) {
    assert(x != nullptr);
    Node* next = x->Next(level);
    /*if (next != nullptr) {
      PREFETCH(next->Next(level), 0, 1);
    }*/
    // Make sure the lists are sorted
    assert(x == head_ || next == nullptr || KeyIsAfterNode(next->key, x));
    // Make sure we haven't overshot during our search
    assert(x == head_ || KeyIsAfterNode(key, x));
    int cmp = (next == nullptr || next == last_bigger)
        ? 1 : compare_(next->key, key);
    if (cmp == 0 || (cmp > 0 && level == 0)) {
      return next;
    } else if (cmp < 0) {
      // Keep searching in this list
      x = next;
    } else {
      // Switch to next list, reuse compare_() result
      last_bigger = next;
      level--;
    }
  }
}

template<typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::
  FindGreaterOrEqual_RB(const Key& key) const {
  // Note: It looks like we could reduce duplication by implementing
  // this function as FindLessThan(key)->Next(0), but we wouldn't be able
  // to exit early on equality and the result wouldn't even be correct.
  // A concurrent insert might occur after FindLessThan(key) but before
  // we get a chance to call Next(0).
  
  Node* x = SearchRBNode(key);
  if (compare_(x->key, key) == 0) {
    return x;
  }
  int level = GetMaxHeight() - 1; // Max height node + tree - Signal.Jin
  //int level = GetMaxHeight() - 2; // Max height node -> tree - Signal.Jin
  Node* last_bigger = nullptr;
  //printf("\n%lu\n", x->key);
  while (true) {
    assert(x != nullptr);
    Node* next = x->Next(level);
    /*if (next != nullptr) {
      PREFETCH(next->Next(level), 0, 1);
    }*/
    // Make sure the lists are sorted
    assert(x == head_ || next == nullptr || KeyIsAfterNode(next->key, x));
    // Make sure we haven't overshot during our search
    assert(x == head_ || KeyIsAfterNode(key, x));
    int cmp = (next == nullptr || next == last_bigger)
        ? 1 : compare_(next->key, key);
    if (cmp == 0 || (cmp > 0 && level == 0)) {
      return next;
    } else if (cmp < 0) {
      // Keep searching in this list
      x = next;
    } else {
      // Switch to next list, reuse compare_() result
      last_bigger = next;
      level--;
    }
  }
}

// template<typename Key, class Comparator>
// typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::
//   FindGreaterOrEqual_AVL(const Key& key) const {
//   // Note: It looks like we could reduce duplication by implementing
//   // this function as FindLessThan(key)->Next(0), but we wouldn't be able
//   // to exit early on equality and the result wouldn't even be correct.
//   // A concurrent insert might occur after FindLessThan(key) but before
//   // we get a chance to call Next(0).
  
//   Node* x = SearchAVLNode(key);
//   if (compare_(x->key, key) == 0) {
//     return x;
//   }
//   int level = GetMaxHeight() - 1;
//   //int level = GetMaxHeight() - 2;
//   Node* last_bigger = nullptr;
//   //printf("\n%lu\n", x->key);
//   while (true) {
//     assert(x != nullptr);
//     Node* next = x->Next(level);
//     /*if (next != nullptr) {
//       PREFETCH(next->Next(level), 0, 1);
//     }*/
//     // Make sure the lists are sorted
//     assert(x == head_ || next == nullptr || KeyIsAfterNode(next->key, x));
//     // Make sure we haven't overshot during our search
//     assert(x == head_ || KeyIsAfterNode(key, x));
//     int cmp = (next == nullptr || next == last_bigger)
//         ? 1 : compare_(next->key, key);
//     if (cmp == 0 || (cmp > 0 && level == 0)) {
//       return next;
//     } else if (cmp < 0) {
//       // Keep searching in this list
//       x = next;
//     } else {
//       // Switch to next list, reuse compare_() result
//       last_bigger = next;
//       level--;
//     }
//   }
// }

// template<typename Key, class Comparator>
// typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::
//   FindGreaterOrEqual_Lift(const Key& key) const {
//   // Note: It looks like we could reduce duplication by implementing
//   // this function as FindLessThan(key)->Next(0), but we wouldn't be able
//   // to exit early on equality and the result wouldn't even be correct.
//   // A concurrent insert might occur after FindLessThan(key) but before
//   // we get a chance to call Next(0).
  
//   Node* x = head_;
//   int level = GetMaxHeight() - 1;
//   Node* last_bigger = nullptr;
//   while (true) {
//     assert(x != nullptr);
//     Node* next = x->Next(level);
//     /*if (next != nullptr) {
//       PREFETCH(next->Next(level), 0, 1);
//     }*/
//     // Make sure the lists are sorted
//     assert(x == head_ || next == nullptr || KeyIsAfterNode(next->key, x));
//     // Make sure we haven't overshot during our search
//     assert(x == head_ || KeyIsAfterNode(key, x));
//     int cmp = (next == nullptr || next == last_bigger)
//         ? 1 : compare_(next->key, key);
//     if (cmp == 0 || (cmp > 0 && level == 0)) {
//       return next;
//     } else if (cmp < 0) {
//       // Keep searching in this list
//       x = next;
//     } else {
//       // Switch to next list, reuse compare_() result
//       last_bigger = next;
//       level--;
//     }
//   }
// } // Signal.Jin

// template<typename Key, class Comparator>
// typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::
//   FindGreaterOrEqual_VH(const Key& key) const {
//   // Note: It looks like we could reduce duplication by implementing
//   // this function as FindLessThan(key)->Next(0), but we wouldn't be able
//   // to exit early on equality and the result wouldn't even be correct.
//   // A concurrent insert might occur after FindLessThan(key) but before
//   // we get a chance to call Next(0).

//   std::pair<typename SkipList<Key, Comparator>::Node*, int> tmp = SearchVHNode(key);
//   Node* x = tmp.first;
//   if (compare_(x->key, key) == 0) {
//     return x;
//   }
//   int level = tmp.second; 
//   Node* last_bigger = nullptr;
//   //printf("\n%lu\n", x->key);
//   while (true) {
//     assert(x != nullptr);
//     Node* next = x->Next(level);
//     /*if (next != nullptr) {
//       PREFETCH(next->Next(level), 0, 1);
//     }*/
//     // Make sure the lists are sorted
//     assert(x == head_ || next == nullptr || KeyIsAfterNode(next->key, x));
//     // Make sure we haven't overshot during our search
//     assert(x == head_ || KeyIsAfterNode(key, x));
//     int cmp = (next == nullptr || next == last_bigger)
//         ? 1 : compare_(next->key, key);
//     if (cmp == 0 || (cmp > 0 && level == 0)) {
//       return next;
//     } else if (cmp < 0) {
//       // Keep searching in this list
//       x = next;
//     } else {
//       // Switch to next list, reuse compare_() result
//       last_bigger = next;
//       level--;
//     }
//   }
// }

template<typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node*
SkipList<Key, Comparator>::FindLessThan(const Key& key, Node** prev) const {
  Node* x = head_;
  int level = GetMaxHeight() - 1;
  // KeyIsAfter(key, last_not_after) is definitely false
  Node* last_not_after = nullptr;
  while (true) {
    assert(x != nullptr);
    Node* next = x->Next(level);
    assert(x == head_ || next == nullptr || KeyIsAfterNode(next->key, x));
    assert(x == head_ || KeyIsAfterNode(key, x));
    if (next != last_not_after && KeyIsAfterNode(key, next)) {
      // Keep searching in this list
      x = next;
    } else {
      if (prev != nullptr) {
        prev[level] = x;
      }
      if (level == 0) {
        return x;
      } else {
        // Switch to next list, reuse KeyIUsAfterNode() result
        last_not_after = next;
        level--;
      }
    }
  }
}

template<typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node*
SkipList<Key, Comparator>::FindLessOrEqual_Buf(const Key& key, Node** prev) const {
  Node* x = head_;
  int level = GetMaxHeight() - 1;
  // KeyIsAfter(key, last_not_after) is definitely false
  Node* last_not_after = nullptr;
  //printf("key = %lu\n", key);
  while (true) {
    assert(x != nullptr);
    Node* next = x->Next(level);

    if (next != nullptr && compare_(key, next->key) == 0) {
      return next; // Signal.Jin
    }

    assert(x == head_ || next == nullptr || KeyIsAfterNode(next->key, x));
    assert(x == head_ || KeyIsAfterNode(key, x));
    if (next != last_not_after && KeyIsAfterNode(key, next)) {
      // Keep searching in this list
      x = next;
    } else {
      if (prev != nullptr) {
        prev[level] = x;
      }
      if (level == 0) {
        return nullptr;
      } else {
        // Switch to next list, reuse KeyIUsAfterNode() result
        last_not_after = next;
        level--;
      }
    }
  }
}

template<typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node*
SkipList<Key, Comparator>::FindLessThan_B2hSL(const Key& key, Node** prev) const {
  Node* x = SearchTreeNode(key);
  int level = GetMaxHeight() - 1;
  // KeyIsAfter(key, last_not_after) is definitely false
  Node* last_not_after = nullptr;
  while (true) {
    assert(x != nullptr);
    Node* next = x->Next(level);
    assert(x == head_ || next == nullptr || KeyIsAfterNode(next->key, x));
    assert(x == head_ || KeyIsAfterNode(key, x));
    if (next != last_not_after && KeyIsAfterNode(key, next)) {
      // Keep searching in this list
      x = next;
    } else {
      if (prev != nullptr) {
        prev[level] = x;
      }
      if (level == 0) {
        return x;
      } else {
        // Switch to next list, reuse KeyIUsAfterNode() result
        last_not_after = next;
        level--;
      }
    }
  }
} // Signal.Jin

template<typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node*
SkipList<Key, Comparator>::FindLessThan_RB(const Key& key, Node** prev) const {
  Node* x = SearchRBNode(key);
  //Node* x = head_;
  int level = GetMaxHeight() - 1;
  // KeyIsAfter(key, last_not_after) is definitely false
  Node* last_not_after = nullptr;
  while (true) {
    assert(x != nullptr);
    Node* next = x->Next(level);
    assert(x == head_ || next == nullptr || KeyIsAfterNode(next->key, x));
    assert(x == head_ || KeyIsAfterNode(key, x));
    if (next != last_not_after && KeyIsAfterNode(key, next)) {
      // Keep searching in this list
      x = next;
    } else {
      if (prev != nullptr) {
        prev[level] = x;
      }
      if (level == 0) {
        return x;
      } else {
        // Switch to next list, reuse KeyIUsAfterNode() result
        last_not_after = next;
        level--;
      }
    }
  }
} // Signal.Jin

// template<typename Key, class Comparator>
// typename SkipList<Key, Comparator>::Node*
// SkipList<Key, Comparator>::FindLessThan_AVL(const Key& key, Node** prev) const {
//   Node* x = SearchAVLNode(key);
//   int level = GetMaxHeight() - 1;
//   // KeyIsAfter(key, last_not_after) is definitely false
//   Node* last_not_after = nullptr;
//   while (true) {
//     assert(x != nullptr);
//     Node* next = x->Next(level);
//     assert(x == head_ || next == nullptr || KeyIsAfterNode(next->key, x));
//     assert(x == head_ || KeyIsAfterNode(key, x));
//     if (next != last_not_after && KeyIsAfterNode(key, next)) {
//       // Keep searching in this list
//       x = next;
//     } else {
//       if (prev != nullptr) {
//         prev[level] = x;
//       }
//       if (level == 0) {
//         return x;
//       } else {
//         // Switch to next list, reuse KeyIUsAfterNode() result
//         last_not_after = next;
//         level--;
//       }
//     }
//   }
// } // Signal.Jin

// template<typename Key, class Comparator>
// typename SkipList<Key, Comparator>::Node*
// SkipList<Key, Comparator>::FindLessThan_Lift(const Key& key, Node** prev) const {
//   Node* x = head_;
//   int level = GetMaxHeight() - 1;
//   // KeyIsAfter(key, last_not_after) is definitely false
//   Node* last_not_after = nullptr;
//   while (true) {
//     assert(x != nullptr);
//     Node* next = x->Next(level);
//     assert(x == head_ || next == nullptr || KeyIsAfterNode(next->key, x));
//     assert(x == head_ || KeyIsAfterNode(key, x));
//     if (next != last_not_after && KeyIsAfterNode(key, next)) {
//       // Keep searching in this list
//       x = next;
//       AddLiftNode(x->key, level, false);
//     } else {
//       if (prev != nullptr) {
//         prev[level] = x;
//       }
//       if (level == 0) {
//         AddLiftNode(x->key, level, true);
//         return x;
//       } else {
//         // Switch to next list, reuse KeyIUsAfterNode() result
//         last_not_after = next;
//         level--;
//       }
//     }
//   }
// } // Signal.Jin

// template<typename Key, class Comparator>
// typename SkipList<Key, Comparator>::Node*
// SkipList<Key, Comparator>::FindLessThan_VH(const Key& key, Node** prev) const {
//   std::pair<typename SkipList<Key, Comparator>::Node*, int> tmp = SearchVHNode(key);
//   Node* x = tmp.first;
//   printf("%lu, %lu\n", x->key, key);
//   int level = tmp.second;
//   // KeyIsAfter(key, last_not_after) is definitely false
//   Node* last_not_after = nullptr;
//   while (true) {
//     assert(x != nullptr);
//     Node* next = x->Next(level);
//     assert(x == head_ || next == nullptr || KeyIsAfterNode(next->key, x));
//     assert(x == head_ || KeyIsAfterNode(key, x));
//     if (next != last_not_after && KeyIsAfterNode(key, next)) {
//       // Keep searching in this list
//       x = next;
//     } else {
//       if (prev != nullptr) {
//         prev[level] = x;
//       }
//       if (level == 0) {
//         return x;
//       } else {
//         // Switch to next list, reuse KeyIUsAfterNode() result
//         last_not_after = next;
//         level--;
//       }
//     }
//   }
// } // Signal.Jin

template<typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::FindLast()
    const {
  Node* x = head_;
  int level = GetMaxHeight() - 1;
  while (true) {
    Node* next = x->Next(level);
    if (next == nullptr) {
      if (level == 0) {
        return x;
      } else {
        // Switch to next list
        level--;
      }
    } else {
      x = next;
    }
  }
}

template <typename Key, class Comparator>
uint64_t SkipList<Key, Comparator>::EstimateCount(const Key& key) const {
  uint64_t count = 0;

  Node* x = head_;
  int level = GetMaxHeight() - 1;
  while (true) {
    assert(x == head_ || compare_(x->key, key) < 0);
    Node* next = x->Next(level);
    if (next == nullptr || compare_(next->key, key) >= 0) {
      if (level == 0) {
        return count;
      } else {
        // Switch to next list
        count *= kBranching_;
        level--;
      }
    } else {
      x = next;
      count++;
    }
  }
}

template <typename Key, class Comparator>
uint64_t SkipList<Key, Comparator>::Estimate_Max() const {
  uint64_t count = 0;

  Node* x = head_;
  int level = GetMaxHeight() - 1;
  while (true) {
    Node* next = x->Next(level);
    if (next == nullptr) {
      return count;
    }
    count++;
    x = next;
  }
} // Signal.Jin

// template<typename Key, class Comparator>
// void SkipList<Key, Comparator>::
// AddLiftNode(const Key& key, int height, bool flag) const {
//   Lnode* newNode = new Lnode();
//   Lnode* tmpNode = new Lnode();

//   int h_key = hashing(key);
//   //printf("key = %d, h_key = %d, height = %d\n", (int)key, h_key, height);

//   if (table[h_key] == nullptr) {
//     newNode->key = key;
//     newNode->max_height = height+1;
//     if (flag == false) {
//       newNode->ref_cnt = 1;
//       newNode->lookup_cnt = 0;
//     } else if (flag == true) {
//       newNode->ref_cnt = 0;
//       newNode->lookup_cnt = 1;
//     }
//     newNode->next = nullptr;
//     table[h_key] = newNode;
//   } else {
//     tmpNode = table[h_key];
//     do {
//       if (compare_(tmpNode->key, key) == 0) {
//         if (flag == false)
//           tmpNode->ref_cnt++;
//         else if (flag == true)
//           tmpNode->lookup_cnt++;
//         break;
//       }
//       tmpNode = tmpNode->next;
//       if (tmpNode == nullptr) {
//         newNode->key = key;
//         newNode->max_height = height;
//         if (flag == false) {
//           newNode->ref_cnt = 1;
//           newNode->lookup_cnt = 0;
//         } else if (flag == true) {
//           newNode->ref_cnt = 0;
//           newNode->lookup_cnt = 1;
//         }
//         newNode->next = nullptr;
//         tmpNode = newNode;
//       }
//     } while(tmpNode->next != nullptr);
//   }
// } // Signal.Jin

// template<typename Key, class Comparator>
// int SkipList<Key, Comparator>::
// hashing(const Key& key) const {
//   int t_key = (int)key;
//   t_key = t_key & MAX_TABLE;

//   return t_key;
// } // Signal.Jin

template<typename Key, class Comparator>
void SkipList<Key, Comparator>::
AddTreeNode(const Key& key, Node* M_target) const {
  Tnode* newNode = new Tnode();
  Tnode* tmpRoot = nullptr;

  newNode->key = key;
  newNode->SL_node = M_target->Next(kMaxHeight_-1); // Max height node + tree - Signal.Jin
  //newNode->SL_node = M_target->Next(kMaxHeight_-2); // Max height node -> tree - Signal.Jin
  if (root == nullptr) {
    root = newNode;
  } else {
    Tnode* pos = root;
    while(pos != nullptr) {
      tmpRoot = pos;
      if (compare_(newNode->key, pos->key) < 0) {
        pos = pos->left;
      } else {
        pos = pos->right;
      }
    }
    if (compare_(newNode->key, tmpRoot->key) < 0)
      tmpRoot->left = newNode;
    else
      tmpRoot->right = newNode;
  }
} // Add Skip List Node into Tree Node - Signal.Jin

template<typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::
  SearchTreeNode(const Key& key) const {
  Tnode* pos = root;
  Tnode* parent = nullptr;
  Tnode* gparent = nullptr;

  while (pos != nullptr) {
    if (compare_(pos->key, key) == 0) {
      return pos->SL_node;
    } else if (compare_(pos->key, key) < 0) {
      if (pos->right == nullptr) {
        return pos->SL_node;
      } else if (gparent != nullptr) {
        gparent = nullptr;
        parent = pos;
        pos = pos->right;
      }else {
        parent = pos;
        pos = pos->right;
      }
    } else if (compare_(pos->key, key) > 0) {
      if (pos->left == nullptr) {
        if (parent == nullptr && gparent == nullptr) {
          return head_;
        } else if (parent != nullptr && gparent == nullptr) {
          return parent->SL_node;
        } else if (parent != nullptr && gparent != nullptr) {
          return gparent->SL_node;
        }
      } else if (parent != nullptr && gparent == nullptr) {
        gparent = parent;
        parent = pos;
        pos = pos->left;
      } else if (parent != nullptr && gparent != nullptr) {
        parent = pos;
        pos = pos->left;
      } else {
        pos = pos->left;
      }
    }
  }
  return head_;
} // Search Node from Tree Structure - Signal.Jin

template<typename Key, class Comparator>
void SkipList<Key, Comparator>::
InsertRBTree(const Key& key, Node* sl_node) const {
  RBnode* x = rootRB;
  RBnode* y = nullptr;
  RBnode* z = new RBnode();

  z->key = key;
  z->color = RED;
  z->SL_node = sl_node->Next(kMaxHeight_-1);
  z->parent = nullptr;
  z->left = leafNode;
  z->right = leafNode;

  while (x != leafNode) {
    y = x;
    if (compare_(x->key, key) < 0)
      x = x->right;
    else
      x = x->left;
  }

  z->parent = y;

  if (y == nullptr)
    rootRB = z;
  else if (compare_(z->key, y->key) > 0)
    y->right = z;
  else
    y->left = z;

  if (z->parent == nullptr) {
    z->color = BLACK;
    return;
  }

  if (z->parent->parent == nullptr)
    return;
  InsertFixUp(z);

  return;
} // Add Skip List Node into RB Tree Node - Signal.Jin

template<typename Key, class Comparator>
void SkipList<Key, Comparator>::
InsertFixUp(RBnode* z) const {
  while (z != rootRB && z->parent->color == RED) {
    RBnode* grandparent = z->parent->parent;
    RBnode* uncle = (z->parent == grandparent->left) ? grandparent->right : grandparent->left;
    bool side = (z->parent == grandparent->left) ? true : false;

    if (uncle && uncle->color == RED) {
      z->parent->color = BLACK;
      uncle->color = BLACK;
      grandparent->color = RED;
      z = grandparent;
    }
    else {
      if (z == (side ? z->parent->right : z->parent->left)) {
        z = z->parent;
        side ? RotateL(z) : RotateR(z);
      }
      z->parent->color = BLACK;
      grandparent->color = RED;
      side ? RotateR(grandparent) : RotateL(grandparent);
    }
  }
  rootRB->color = BLACK;
}

template<typename Key, class Comparator>
void SkipList<Key, Comparator>::
RotateL(RBnode* x) const {
  RBnode* y;

  y = x->right;
  x->right = y->left;
  if (y->left != leafNode)
    y->left->parent = x;
  y->parent = x->parent;

  if (!x->parent)
    rootRB = y;
  else if (x == x->parent->left)
    x->parent->left = y;
  else
    x->parent->right = y;
  x->parent = y;
  y->left = x;
}

template<typename Key, class Comparator>
void SkipList<Key, Comparator>::
RotateR(RBnode* y) const {
  RBnode* x;

  x = y->left;
  y->left = x->right;
  if (x->right != leafNode)
    x->right->parent = y;
  x->parent = y->parent;

  if (!y->parent)
    rootRB = x;
  else if (y == y->parent->left)
    y->parent->left = x;
  else
    y->parent->right = x;
  y->parent = x;
  x->right = y;
}

template<typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::
  SearchRBNode(const Key& key) const {
  RBnode* pos = rootRB;
  RBnode* parent = nullptr;
  RBnode* gparent = nullptr;

  while (pos != leafNode) {
    if (compare_(pos->key, key) == 0) {
      return pos->SL_node;
    } else if (compare_(pos->key, key) < 0) {
      if (pos->right == leafNode) {
        return pos->SL_node;
      } else if (gparent != nullptr) {
        gparent = nullptr;
        parent = pos;
        pos = pos->right;
      }else {
        parent = pos;
        pos = pos->right;
      }
    } else if (compare_(pos->key, key) > 0) {
      if (pos->left == leafNode) {
        if (parent == nullptr && gparent == nullptr) {
          return head_;
        } else if (parent != nullptr && gparent == nullptr) {
          return parent->SL_node;
        } else if (parent != nullptr && gparent != nullptr) {
          return gparent->SL_node;
        }
      } else if (parent != nullptr && gparent == nullptr) {
        gparent = parent;
        parent = pos;
        pos = pos->left;
      } else if (parent != nullptr && gparent != nullptr) {
        parent = pos;
        pos = pos->left;
      } else {
        pos = pos->left;
      }
    }
  }
  return head_;
}

// template<typename Key, class Comparator>
// void SkipList<Key, Comparator>::
// AddVHNode(const Key& key, Node* M_target, int height) const {
//   Vnode* newNode = new Vnode();
//   Vnode* tmpRoot = nullptr;

//   newNode->key = key;
//   newNode->SL_node = M_target->Next(height); // Max height node + tree - Signal.Jin
//   newNode->height = height;
//   if (rootVH == nullptr) {
//     rootVH = newNode;
//   } else {
//     Vnode* pos = rootVH;
//     while(pos != nullptr) {
//       tmpRoot = pos;
//       if (compare_(newNode->key, pos->key) < 0) {
//         pos = pos->left;
//       } else {
//         pos = pos->right;
//       }
//     }
//     if (compare_(newNode->key, tmpRoot->key) < 0)
//       tmpRoot->left = newNode;
//     else
//       tmpRoot->right = newNode;
//   }
// } // Add Skip List Node into variable height Tree Node - Signal.Jin

// template<typename Key, class Comparator>
// std::pair<typename SkipList<Key, Comparator>::Node*, int> SkipList<Key, Comparator>::
//   SearchVHNode(const Key& key) const {
//   Vnode* pos = rootVH;
//   Vnode* parent = nullptr;
//   Vnode* gparent = nullptr;
//   while (pos != nullptr) {
//     if (compare_(pos->key, key) == 0) {
//       return std::make_pair(pos->SL_node, pos->height);
//     } else if (compare_(pos->key, key) < 0) {
//       if (pos->right == nullptr) {
//         return std::make_pair(pos->SL_node, pos->height);
//       } else if (gparent != nullptr) {
//         gparent = nullptr;
//         parent = pos;
//         pos = pos->right;
//       }else {
//         parent = pos;
//         pos = pos->right;
//       }
//     } else if (compare_(pos->key, key) > 0) {
//       if (pos->left == nullptr) {
//         if (parent == nullptr && gparent == nullptr) {
//           return std::make_pair(head_, GetMaxHeight()-1);
//         } else if (parent != nullptr && gparent == nullptr) {
//           return std::make_pair(parent->SL_node, parent->height);
//         } else if (parent != nullptr && gparent != nullptr) {
//           return std::make_pair(gparent->SL_node, gparent->height);
//         }
//       } else if (parent != nullptr && gparent == nullptr) {
//         gparent = parent;
//         parent = pos;
//         pos = pos->left;
//       } else if (parent != nullptr && gparent != nullptr) {
//         parent = pos;
//         pos = pos->left;
//       } else {
//         pos = pos->left;
//       }
//     }
//   }
//   return std::make_pair(head_, GetMaxHeight()-1);
// } // Search Node from variable height Tree Structure - Signal.Jin

// template<typename Key, class Comparator>
// typename SkipList<Key, Comparator>::Anode* SkipList<Key, Comparator>::
// AddAVLNode(const Key& key, Node* M_target, Anode* A_target) const {
//   if (A_target == nullptr) {
//     Anode* newNode = new Anode();
//     newNode->key = key;
//     newNode->SL_node = M_target->Next(kMaxHeight_-2);
//     A_target = newNode;
//     return A_target;
//   } else if (compare_(A_target->key, key) < 0) {
//     A_target->right = AddAVLNode(key, M_target, A_target->right);
//   } else {
//     A_target->left = AddAVLNode(key, M_target, A_target->left);
//   }
//   A_target->height = std::max(getHeight(A_target->left), getHeight(A_target->right)) + 1;
  
//   AVLBalancing(key, A_target);
  
//   return A_target;
// } // Add Skip List Node into AVL Tree Node - Signal.Jin

// template<typename Key, class Comparator>
// void SkipList<Key, Comparator>::
// AVLBalancing(const Key& key, Anode* A_target) const {
//   int b_factor = getBalanceFactor(A_target);

//   if (b_factor > 1 && compare_(key, A_target->left->key) < 0) {
//     A_target = RotateRight(A_target);
//   } else if (b_factor > 1 && compare_(key, A_target->left->key) > 0) {
//     A_target->left = RotateLeft(A_target->left);
//     A_target = RotateRight(A_target);
//   } else if (b_factor < -1 && compare_(key, A_target->right->key) > 0) {
//     A_target =RotateLeft(A_target);
//   } else if (b_factor < -1 && compare_(key, A_target->right->key) < 0) {
//     A_target->right = RotateRight(A_target->right);
//     A_target = RotateLeft(A_target);
//   }
// } // Signal.Jin

// template<typename Key, class Comparator>
// int SkipList<Key, Comparator>::
// getHeight(Anode* A_target) const {
//   if (A_target == nullptr)
//     return 0;
//   else 
//     return A_target->height;
// } // Signal.Jin

// template<typename Key, class Comparator>
// int SkipList<Key, Comparator>::
// getBalanceFactor(Anode* A_target) const {
//   return getHeight(A_target->left) - getHeight(A_target->right);
// } // Signal.Jin

// template<typename Key, class Comparator>
// typename SkipList<Key, Comparator>::Anode* SkipList<Key, Comparator>::
// RotateLeft(Anode* A_target) const {
//   Anode* tmp = A_target->right;
//   A_target->right = tmp->left;
//   tmp->left = A_target;

//   A_target->height = std::max(getHeight(A_target->left), getHeight(A_target->right)) + 1;
//   tmp->height = std::max(getHeight(tmp->left), getHeight(tmp->right)) + 1;

//   return tmp;
// } // Signal.Jin

// template<typename Key, class Comparator>
// typename SkipList<Key, Comparator>::Anode* SkipList<Key, Comparator>::
// RotateRight(Anode* A_target) const {
//   Anode* tmp = A_target->left;
//   A_target->left = tmp->right;
//   tmp->right = A_target;

//   A_target->height = std::max(getHeight(A_target->left), getHeight(A_target->right)) + 1;
//   tmp->height = std::max(getHeight(tmp->left), getHeight(tmp->right)) + 1;

//   return tmp;
// } // Signal.Jin

// template<typename Key, class Comparator>
// typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::
// SearchAVLNode(const Key& key) const {
//   Anode* pos = rootAVL;
//   Anode* parent = nullptr;
//   Anode* gparent = nullptr;

//   while (pos != nullptr) {
//     if (compare_(pos->key, key) == 0) {
//       return pos->SL_node;
//     } else if (compare_(pos->key, key) < 0) {
//       if (pos->right == nullptr) {
//         return pos->SL_node;
//       } else if (gparent != nullptr) {
//         gparent = nullptr;
//         parent = pos;
//         pos = pos->right;
//       }else {
//         parent = pos;
//         pos = pos->right;
//       }
//     } else if (compare_(pos->key, key) > 0) {
//       if (pos->left == nullptr) {
//         if (parent == nullptr && gparent == nullptr) {
//           return head_;
//         } else if (parent != nullptr && gparent == nullptr) {
//           return parent->SL_node;
//         } else if (parent != nullptr && gparent != nullptr) {
//           return gparent->SL_node;
//         }
//       } else if (parent != nullptr && gparent == nullptr) {
//         gparent = parent;
//         parent = pos;
//         pos = pos->left;
//       } else if (parent != nullptr && gparent != nullptr) {
//         parent = pos;
//         pos = pos->left;
//       } else {
//         pos = pos->left;
//       }
//     }
//   }
//   return head_;
// } // Search Node from Tree Structure - Signal.Jin

template <typename Key, class Comparator>
SkipList<Key, Comparator>::SkipList(const Comparator cmp, Allocator* allocator,
                                    int32_t max_height,
                                    int32_t branching_factor)
    : kMaxHeight_(static_cast<uint16_t>(max_height)),
      kBranching_(static_cast<uint16_t>(branching_factor)),
      kScaledInverseBranching_((Random::kMaxNext + 1) / kBranching_),
      compare_(cmp),
      allocator_(allocator),
      head_(NewNode(0 /* any key will do */, max_height)),
      //single_cursor_(NewNode(0, max_height)),
      //cs_level(-1),
      max_height_(1),
      prev_height_(1) {
  assert(max_height > 0 && kMaxHeight_ == static_cast<uint32_t>(max_height));
  assert(branching_factor > 0 &&
         kBranching_ == static_cast<uint32_t>(branching_factor));
  assert(kScaledInverseBranching_ > 0);
  // Allocate the prev_ Node* array, directly from the passed-in allocator.
  // prev_ does not need to be freed, as its life cycle is tied up with
  // the allocator as a whole.
  prev_ = reinterpret_cast<Node**>(
            allocator_->AllocateAligned(sizeof(Node*) * kMaxHeight_));
  for (int i = 0; i < kMaxHeight_; i++) {
    head_->SetNext(i, nullptr);
    prev_[i] = head_;
  }
  root = nullptr; // Signal.Jin

  {
    leafNode = new RBnode();
    leafNode->color = BLACK;
    leafNode->left = leafNode->right = leafNode->parent = nullptr;
    leafNode->SL_node = nullptr;
    rootRB = leafNode;
  }
  // rootAVL = nullptr; // Setup for AVL tree with Skip List - Signal.Jin
  // for (int i = 0; i < MAX_TABLE; i++) {
  //   table[i] = NULL;
  // }
  // batch_count = 0;
}

template<typename Key, class Comparator>
void SkipList<Key, Comparator>::Insert(const Key& key) {
  // fast path for sequential insertion
  if (!KeyIsAfterNode(key, prev_[0]->NoBarrier_Next(0)) &&
      (prev_[0] == head_ || KeyIsAfterNode(key, prev_[0]))) {
    assert(prev_[0] != head_ || (prev_height_ == 1 && GetMaxHeight() == 1));

    // Outside of this method prev_[1..max_height_] is the predecessor
    // of prev_[0], and prev_height_ refers to prev_[0].  Inside Insert
    // prev_[0..max_height - 1] is the predecessor of key.  Switch from
    // the external state to the internal
    for (int i = 1; i < prev_height_; i++) {
      prev_[i] = prev_[0];
    }
  } else {
    // TODO(opt): we could use a NoBarrier predecessor search as an
    // optimization for architectures where memory_order_acquire needs
    // a synchronization instruction.  Doesn't matter on x86
    FindLessThan(key, prev_);
  }

  // Our data structure does not allow duplicate insertion
  assert(prev_[0]->Next(0) == nullptr || !Equal(key, prev_[0]->Next(0)->key));

  int height = RandomHeight(); // Height is defined randomly - Lee Jeyeon.
  if (height > GetMaxHeight()) { // Change Total skiplist heigth - Lee Jeyeon.
    for (int i = GetMaxHeight(); i < height; i++) {
      prev_[i] = head_;
    }
    //fprintf(stderr, "Change height from %d to %d\n", max_height_, height);

    // It is ok to mutate max_height_ without any synchronization
    // with concurrent readers.  A concurrent reader that observes
    // the new value of max_height_ will see either the old value of
    // new level pointers from head_ (nullptr), or a new value set in
    // the loop below.  In the former case the reader will
    // immediately drop to the next level since nullptr sorts after all
    // keys.  In the latter case the reader will use the new node.
    max_height_.store(height, std::memory_order_relaxed);
  }
  
  Node* x = NewNode(key, height);
  for (int i = 0; i < height; i++) {
    // NoBarrier_SetNext() suffices since we will add a barrier when
    // we publish a pointer to "x" in prev[i].
    x->NoBarrier_SetNext(i, prev_[i]->NoBarrier_Next(i));
    prev_[i]->SetNext(i, x);
  }

  prev_[0] = x;
  prev_height_ = height;
}

template<typename Key, class Comparator>
void SkipList<Key, Comparator>::Insert_Buf(const Key& key) {
  // fast path for sequential insertion
  Key quot, remain;
  Node* loc = nullptr;

  quot = key / MOD;
  remain = key % MOD;

  //printf("%lu, %lu\n", quot, remain);

  if (!KeyIsAfterNode(quot, prev_[0]->NoBarrier_Next(0)) &&
      (prev_[0] == head_ || KeyIsAfterNode(quot, prev_[0]))) {
    assert(prev_[0] != head_ || (prev_height_ == 1 && GetMaxHeight() == 1));
    // Outside of this method prev_[1..max_height_] is the predecessor
    // of prev_[0], and prev_height_ refers to prev_[0].  Inside Insert
    // prev_[0..max_height - 1] is the predecessor of key.  Switch from
    // the external state to the internal
    for (int i = 1; i < prev_height_; i++) {
      prev_[i] = prev_[0];
    }
  } else {
    // TODO(opt): we could use a NoBarrier predecessor search as an
    // optimization for architectures where memory_order_acquire needs
    // a synchronization instruction.  Doesn't matter on x86
    //loc = FindLessOrEqual_Buf(quot, prev_);
  }
  loc = FindLessOrEqual_Buf(quot, prev_);
  if (loc != nullptr && compare_(loc->key, quot) == 0) {
    loc->buf[remain] = key;
    return;
  } // Signal.Jin

  // Our data structure does not allow duplicate insertion
  assert(prev_[0]->Next(0) == nullptr || !Equal(key, prev_[0]->Next(0)->key));

  int height = RandomHeight(); // Height is defined randomly - Lee Jeyeon.
  if (height > GetMaxHeight()) { // Change Total skiplist heigth - Lee Jeyeon.
    for (int i = GetMaxHeight(); i < height; i++) {
      prev_[i] = head_;
    }
    //fprintf(stderr, "Change height from %d to %d\n", max_height_, height);

    // It is ok to mutate max_height_ without any synchronization
    // with concurrent readers.  A concurrent reader that observes
    // the new value of max_height_ will see either the old value of
    // new level pointers from head_ (nullptr), or a new value set in
    // the loop below.  In the former case the reader will
    // immediately drop to the next level since nullptr sorts after all
    // keys.  In the latter case the reader will use the new node.
    max_height_.store(height, std::memory_order_relaxed);
  }
  Node* x = NewNode(quot, height);
  for (int i = 0; i < height; i++) {
    // NoBarrier_SetNext() suffices since we will add a barrier when
    // we publish a pointer to "x" in prev[i].
    x->NoBarrier_SetNext(i, prev_[i]->NoBarrier_Next(i));
    prev_[i]->SetNext(i, x);
    if (i == 0) x->buf[remain] = key;
  }

  prev_[0] = x;
  prev_height_ = height;
}

template<typename Key, class Comparator>
void SkipList<Key, Comparator>::Insert_B2hSL(const Key& key) {
  // fast path for sequential insertion
  if (!KeyIsAfterNode(key, prev_[0]->NoBarrier_Next(0)) &&
      (prev_[0] == head_ || KeyIsAfterNode(key, prev_[0]))) {
    assert(prev_[0] != head_ || (prev_height_ == 1 && GetMaxHeight() == 1));

    // Outside of this method prev_[1..max_height_] is the predecessor
    // of prev_[0], and prev_height_ refers to prev_[0].  Inside Insert
    // prev_[0..max_height - 1] is the predecessor of key.  Switch from
    // the external state to the internal
    for (int i = 1; i < prev_height_; i++) {
      prev_[i] = prev_[0];
    }
  } else {
    // TODO(opt): we could use a NoBarrier predecessor search as an
    // optimization for architectures where memory_order_acquire needs
    // a synchronization instruction.  Doesn't matter on x86
    //FindLessThan(key, prev_);
    FindLessThan_B2hSL(key, prev_); // Signal.Jin
  }
  // Our data structure does not allow duplicate insertion
  assert(prev_[0]->Next(0) == nullptr || !Equal(key, prev_[0]->Next(0)->key));

  //int check = 0;
  //int height = RandomHeight_B2hSL(); // Height is defined randomly - Lee Jeyeon.
  int height = RandomHeight();
  //if (height == kMaxHeight_+1) {
  //  height--;
  //  check = 1;
  //} // Signal.Jin
  if (height > GetMaxHeight()) { // Change Total skiplist height - Lee Jeyeon.
    for (int i = GetMaxHeight(); i < height; i++) {
      prev_[i] = head_;
    }
    //fprintf(stderr, "Change height from %d to %d\n", max_height_, height);

    // It is ok to mutate max_height_ without any synchronization
    // with concurrent readers.  A concurrent reader that observes
    // the new value of max_height_ will see either the old value of
    // new level pointers from head_ (nullptr), or a new value set in
    // the loop below.  In the former case the reader will
    // immediately drop to the next level since nullptr sorts after all
    // keys.  In the latter case the reader will use the new node.
    max_height_.store(height, std::memory_order_relaxed);
  }
  Node* x = NewNode(key, height);
  for (int i = 0; i < height; i++) {
    // NoBarrier_SetNext() suffices since we will add a barrier when
    // we publish a pointer to "x" in prev[i].
    x->NoBarrier_SetNext(i, prev_[i]->NoBarrier_Next(i));
    prev_[i]->SetNext(i, x);
  }
  //if (height == kMaxHeight_ && check == 1) {
  if (height == kMaxHeight_) {
    AddTreeNode(key, prev_[height-1]);
  } // Signal.Jin

  prev_[0] = x;
  prev_height_ = height;
}

template<typename Key, class Comparator>
void SkipList<Key, Comparator>::Insert_RB(const Key& key) {
  // fast path for sequential insertion
  if (!KeyIsAfterNode(key, prev_[0]->NoBarrier_Next(0)) &&
      (prev_[0] == head_ || KeyIsAfterNode(key, prev_[0]))) {
    assert(prev_[0] != head_ || (prev_height_ == 1 && GetMaxHeight() == 1));

    // Outside of this method prev_[1..max_height_] is the predecessor
    // of prev_[0], and prev_height_ refers to prev_[0].  Inside Insert
    // prev_[0..max_height - 1] is the predecessor of key.  Switch from
    // the external state to the internal
    for (int i = 1; i < prev_height_; i++) {
      prev_[i] = prev_[0];
    }
  } else {
    // TODO(opt): we could use a NoBarrier predecessor search as an
    // optimization for architectures where memory_order_acquire needs
    // a synchronization instruction.  Doesn't matter on x86
    //FindLessThan(key, prev_);
    FindLessThan_RB(key, prev_); // Signal.Jin
  }
  // Our data structure does not allow duplicate insertion
  assert(prev_[0]->Next(0) == nullptr || !Equal(key, prev_[0]->Next(0)->key));

  //int check = 0;
  //int height = RandomHeight_B2hSL(); // Height is defined randomly - Lee Jeyeon.
  int height = RandomHeight();
  //if (height == kMaxHeight_+1) {
  //  height--;
  //  check = 1;
  //} // Signal.Jin
  if (height > GetMaxHeight()) { // Change Total skiplist height - Lee Jeyeon.
    for (int i = GetMaxHeight(); i < height; i++) {
      prev_[i] = head_;
    }
    //fprintf(stderr, "Change height from %d to %d\n", max_height_, height);

    // It is ok to mutate max_height_ without any synchronization
    // with concurrent readers.  A concurrent reader that observes
    // the new value of max_height_ will see either the old value of
    // new level pointers from head_ (nullptr), or a new value set in
    // the loop below.  In the former case the reader will
    // immediately drop to the next level since nullptr sorts after all
    // keys.  In the latter case the reader will use the new node.
    max_height_.store(height, std::memory_order_relaxed);
  }
  Node* x = NewNode(key, height);
  for (int i = 0; i < height; i++) {
    // NoBarrier_SetNext() suffices since we will add a barrier when
    // we publish a pointer to "x" in prev[i].
    x->NoBarrier_SetNext(i, prev_[i]->NoBarrier_Next(i));
    prev_[i]->SetNext(i, x);
  }
  
  if (height == kMaxHeight_) {
    InsertRBTree(key, prev_[height-1]);
  } // Signal.Jin

  prev_[0] = x;
  prev_height_ = height;
}

// template<typename Key, class Comparator>
// void SkipList<Key, Comparator>::Insert_AVL(const Key& key) {
//   // fast path for sequential insertion
//   if (!KeyIsAfterNode(key, prev_[0]->NoBarrier_Next(0)) &&
//       (prev_[0] == head_ || KeyIsAfterNode(key, prev_[0]))) {
//     assert(prev_[0] != head_ || (prev_height_ == 1 && GetMaxHeight() == 1));

//     // Outside of this method prev_[1..max_height_] is the predecessor
//     // of prev_[0], and prev_height_ refers to prev_[0].  Inside Insert
//     // prev_[0..max_height - 1] is the predecessor of key.  Switch from
//     // the external state to the internal
//     for (int i = 1; i < prev_height_; i++) {
//       prev_[i] = prev_[0];
//     }
//   } else {
//     // TODO(opt): we could use a NoBarrier predecessor search as an
//     // optimization for architectures where memory_order_acquire needs
//     // a synchronization instruction.  Doesn't matter on x86
//     //FindLessThan(key, prev_);
//     FindLessThan_AVL(key, prev_);
//   }

//   // Our data structure does not allow duplicate insertion
//   assert(prev_[0]->Next(0) == nullptr || !Equal(key, prev_[0]->Next(0)->key));

//   int height = RandomHeight(); // Height is defined randomly - Lee Jeyeon.
//   if (height > GetMaxHeight()) { // Change Total skiplist height - Lee Jeyeon.
//     for (int i = GetMaxHeight(); i < height; i++) {
//       prev_[i] = head_;
//     }
//     //fprintf(stderr, "Change height from %d to %d\n", max_height_, height);

//     // It is ok to mutate max_height_ without any synchronization
//     // with concurrent readers.  A concurrent reader that observes
//     // the new value of max_height_ will see either the old value of
//     // new level pointers from head_ (nullptr), or a new value set in
//     // the loop below.  In the former case the reader will
//     // immediately drop to the next level since nullptr sorts after all
//     // keys.  In the latter case the reader will use the new node.
//     max_height_.store(height, std::memory_order_relaxed);
//   }
//   Node* x = NewNode(key, height);
//   /*if (height == kMaxHeight_) {
//     for (int i = 0; i < height-1; i++) {
//       // NoBarrier_SetNext() suffices since we will add a barrier when
//       // we publish a pointer to "x" in prev[i].
//       x->NoBarrier_SetNext(i, prev_[i]->NoBarrier_Next(i));
//       prev_[i]->SetNext(i, x);
//     }
//     rootAVL = AddAVLNode(key, prev_[height-2], rootAVL); // Signal.Jin
//   } // If height == kMaxHeight, highest level node turn into tree node - Signal.Jin
//   else {
//     for (int i = 0; i < height; i++) {
//       // NoBarrier_SetNext() suffices since we will add a barrier when
//       // we publish a pointer to "x" in prev[i].
//       x->NoBarrier_SetNext(i, prev_[i]->NoBarrier_Next(i));
//       prev_[i]->SetNext(i, x);
//     }
//   }*/

//   for (int i = 0; i < height; i++) {
//     // NoBarrier_SetNext() suffices since we will add a barrier when
//     // we publish a pointer to "x" in prev[i].
//     x->NoBarrier_SetNext(i, prev_[i]->NoBarrier_Next(i));
//     prev_[i]->SetNext(i, x);
//   }
//   if (height == kMaxHeight_) {
//     rootAVL = AddAVLNode(key, prev_[height-1], rootAVL);
//   } // Signal.Jin

//   prev_[0] = x;
//   prev_height_ = height;
// }

// template<typename Key, class Comparator>
// void SkipList<Key, Comparator>::Insert_Lift(const Key& key) {
//   // fast path for sequential insertion
//   if (!KeyIsAfterNode(key, prev_[0]->NoBarrier_Next(0)) &&
//       (prev_[0] == head_ || KeyIsAfterNode(key, prev_[0]))) {
//     assert(prev_[0] != head_ || (prev_height_ == 1 && GetMaxHeight() == 1));

//     // Outside of this method prev_[1..max_height_] is the predecessor
//     // of prev_[0], and prev_height_ refers to prev_[0].  Inside Insert
//     // prev_[0..max_height - 1] is the predecessor of key.  Switch from
//     // the external state to the internal
//     for (int i = 1; i < prev_height_; i++) {
//       prev_[i] = prev_[0];
//     }
//   } else {
//     // TODO(opt): we could use a NoBarrier predecessor search as an
//     // optimization for architectures where memory_order_acquire needs
//     // a synchronization instruction.  Doesn't matter on x86
//     FindLessThan_Lift(key, prev_);
//   }

//   // Our data structure does not allow duplicate insertion
//   assert(prev_[0]->Next(0) == nullptr || !Equal(key, prev_[0]->Next(0)->key));

//   int height = RandomHeight(); // Height is defined randomly - Lee Jeyeon.
//   if (height > GetMaxHeight()) { // Change Total skiplist heigth - Lee Jeyeon.
//     for (int i = GetMaxHeight(); i < height; i++) {
//       prev_[i] = head_;
//     }
//     //fprintf(stderr, "Change height from %d to %d\n", max_height_, height);

//     // It is ok to mutate max_height_ without any synchronization
//     // with concurrent readers.  A concurrent reader that observes
//     // the new value of max_height_ will see either the old value of
//     // new level pointers from head_ (nullptr), or a new value set in
//     // the loop below.  In the former case the reader will
//     // immediately drop to the next level since nullptr sorts after all
//     // keys.  In the latter case the reader will use the new node.
//     max_height_.store(height, std::memory_order_relaxed);
//   }
  
//   Node* x = NewNode(key, height);
//   for (int i = 0; i < height; i++) {
//     // NoBarrier_SetNext() suffices since we will add a barrier when
//     // we publish a pointer to "x" in prev[i].
//     x->NoBarrier_SetNext(i, prev_[i]->NoBarrier_Next(i));
//     prev_[i]->SetNext(i, x);
//   }

//   prev_[0] = x;
//   prev_height_ = height;
// }

// template<typename Key, class Comparator>
// void SkipList<Key, Comparator>::Insert_VH(const Key& key) {
//   // fast path for sequential insertion
//   if (!KeyIsAfterNode(key, prev_[0]->NoBarrier_Next(0)) &&
//       (prev_[0] == head_ || KeyIsAfterNode(key, prev_[0]))) {
//     assert(prev_[0] != head_ || (prev_height_ == 1 && GetMaxHeight() == 1));

//     // Outside of this method prev_[1..max_height_] is the predecessor
//     // of prev_[0], and prev_height_ refers to prev_[0].  Inside Insert
//     // prev_[0..max_height - 1] is the predecessor of key.  Switch from
//     // the external state to the internal
//     for (int i = 1; i < prev_height_; i++) {
//       prev_[i] = prev_[0];
//     }
//   } else {
//     // TODO(opt): we could use a NoBarrier predecessor search as an
//     // optimization for architectures where memory_order_acquire needs
//     // a synchronization instruction.  Doesn't matter on x86
//     //FindLessThan(key, prev_);
//     FindLessThan_VH(key, prev_); // Signal.Jin
//   }

//   // Our data structure does not allow duplicate insertion
//   assert(prev_[0]->Next(0) == nullptr || !Equal(key, prev_[0]->Next(0)->key));

//   //int check = 0;
//   //int height = RandomHeight_B2hSL(); // Height is defined randomly - Lee Jeyeon.
//   int height = RandomHeight();
//   if (height > GetMaxHeight()) { // Change Total skiplist height - Lee Jeyeon.
//     for (int i = GetMaxHeight(); i < height; i++) {
//       prev_[i] = head_;
//     }
//     //fprintf(stderr, "Change height from %d to %d\n", max_height_, height);

//     // It is ok to mutate max_height_ without any synchronization
//     // with concurrent readers.  A concurrent reader that observes
//     // the new value of max_height_ will see either the old value of
//     // new level pointers from head_ (nullptr), or a new value set in
//     // the loop below.  In the former case the reader will
//     // immediately drop to the next level since nullptr sorts after all
//     // keys.  In the latter case the reader will use the new node.
//     max_height_.store(height, std::memory_order_relaxed);
//   }
//   Node* x = NewNode(key, height);

//   for (int i = 0; i < height; i++) {
//     // NoBarrier_SetNext() suffices since we will add a barrier when
//     // we publish a pointer to "x" in prev[i].
//     x->NoBarrier_SetNext(i, prev_[i]->NoBarrier_Next(i));
//     prev_[i]->SetNext(i, x);
//   }
//   //if (height == kMaxHeight_ && check == 1) {
//   if (height > 1 && height == max_height_) {
//     AddVHNode(key, prev_[height-1], height-1);
//   } // Signal.Jin

//   prev_[0] = x;
//   prev_height_ = height;
// }

// template<typename Key, class Comparator>
// void SkipList<Key, Comparator>::Insert_Seq(const Key* key_) {
//   Key key = key_[0];

//   // fast path for sequential insertion
//   if (!KeyIsAfterNode(key, prev_[0]->NoBarrier_Next(0)) &&
//       (prev_[0] == head_ || KeyIsAfterNode(key, prev_[0]))) {
//     assert(prev_[0] != head_ || (prev_height_ == 1 && GetMaxHeight() == 1));

//     // Outside of this method prev_[1..max_height_] is the predecessor
//     // of prev_[0], and prev_height_ refers to prev_[0].  Inside Insert
//     // prev_[0..max_height - 1] is the predecessor of key.  Switch from
//     // the external state to the internal
//     for (int i = 1; i < prev_height_; i++) {
//       prev_[i] = prev_[0];
//     }
//   } else {
//     // TODO(opt): we could use a NoBarrier predecessor search as an
//     // optimization for architectures where memory_order_acquire needs
//     // a synchronization instruction.  Doesn't matter on x86
//     FindLessThan(key, prev_);
//   }

//   // Our data structure does not allow duplicate insertion
//   assert(prev_[0]->Next(0) == nullptr || !Equal(key, prev_[0]->Next(0)->key));

//   for (int j = 0; j < 8; j++) { // Sequential batch write - Signal.Jin
//     int height = RandomHeight(); // Height is defined randomly - Lee Jeyeon.
//     if (height > GetMaxHeight()) { // Change Total skiplist heigth - Lee Jeyeon.
//       for (int i = GetMaxHeight(); i < height; i++) {
//         prev_[i] = head_;
//       }

//       // It is ok to mutate max_height_ without any synchronization
//       // with concurrent readers.  A concurrent reader that observes
//       // the new value of max_height_ will see either the old value of
//       // new level pointers from head_ (nullptr), or a new value set in
//       // the loop below.  In the former case the reader will
//       // immediately drop to the next level since nullptr sorts after all
//       // keys.  In the latter case the reader will use the new node.
//       max_height_.store(height, std::memory_order_relaxed);
//     }
  
//     Node* x = NewNode(key_[j], height);
//     for (int i = 0; i < height; i++) {
//       // NoBarrier_SetNext() suffices since we will add a barrier when
//       // we publish a pointer to "x" in prev[i].
//       x->NoBarrier_SetNext(i, prev_[i]->NoBarrier_Next(i));
//       prev_[i]->SetNext(i, x);
//     }

//     prev_[0] = x;
//     prev_height_ = height;
//     while(j != 7) {
//       if (prev_[0]->Next(0) == nullptr) {
//         break;
//       }
//       if (compare_(prev_[0]->Next(0)->key, key_[j+1]) < 0) {
//         prev_[0] = prev_[0]->Next(0);
//       } else if (compare_(prev_[0]->Next(0)->key, key_[j+1]) > 0) {
//         break;
//       }
//     }
//   }
// }

// template<typename Key, class Comparator>
// void SkipList<Key, Comparator>::Batch_Insert(const Key& key) {
//   if(batch_count != 8) {
//     batch[batch_count] = key;
//     batch_count++;
//   } 

//   if(batch_count == 8) {
//     Insert_Seq(batch);
//     batch_count = 0;
//   } 
// }

template<typename Key, class Comparator>
bool SkipList<Key, Comparator>::Contains(const Key& key) const {
  Node* x = FindGreaterOrEqual(key);
  if (x != nullptr && Equal(key, x->key)) {
    return true;
  } else {
    return false;
  }
}

template<typename Key, class Comparator>
bool SkipList<Key, Comparator>::Contains_Buf(const Key& key) const {
  Node* x = FindGreaterOrEqual_Buf(key);
  Key res;
  if (x != nullptr)
    res = x->buf[key % MOD];
  if (x != nullptr && Equal(key, res)) {
    return true;
  } else {
    return false;
  }
}

// template<typename Key, class Comparator>
// bool SkipList<Key, Comparator>::Contains_Cursor(const Key& key) const {
//   Node* x = FindGreaterOrEqual_Cursor(key);
//   if (x != nullptr && Equal(key, x->key)) {
//     return true;
//   } else {
//     return false;
//   }
// } // Signal.Jin

template<typename Key, class Comparator>
bool SkipList<Key, Comparator>::Contains_B2hSL(const Key& key) const {
  Node* x = FindGreaterOrEqual_B2hSL(key);
  if (x != nullptr && Equal(key, x->key)) {
    return true;
  } else {
    return false;
  }
} // Signal.Jin

template<typename Key, class Comparator>
bool SkipList<Key, Comparator>::Contains_RB(const Key& key) const {
  Node* x = FindGreaterOrEqual_RB(key);
  if (x != nullptr && Equal(key, x->key)) {
    return true;
  } else {
    return false;
  }
} // Signal.Jin

// template<typename Key, class Comparator>
// bool SkipList<Key, Comparator>::Contains_AVL(const Key& key) const {
//   Node* x = FindGreaterOrEqual_AVL(key);
//   if (x != nullptr && Equal(key, x->key)) {
//     return true;
//   } else {
//     return false;
//   }
// } // Signal.Jin

// template<typename Key, class Comparator>
// bool SkipList<Key, Comparator>::Contains_Lift(const Key& key) const {
//   Node* x = FindGreaterOrEqual_Lift(key);
//   if (x != nullptr && Equal(key, x->key)) {
//     return true;
//   } else {
//     return false;
//   }
// } // Signal.Jin

// template<typename Key, class Comparator>
// bool SkipList<Key, Comparator>::Contains_VH(const Key& key) const {
//   Node* x = FindGreaterOrEqual_VH(key);
//   if (x != nullptr && Equal(key, x->key)) {
//     return true;
//   } else {
//     return false;
//   }
// } // Signal.Jin

}  // namespace ROCKSDB_NAMESPACE
