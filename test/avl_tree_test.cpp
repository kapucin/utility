/* Copyright (C) 2018 Bolt Robotics <info@boltrobotics.com> */

// SYSTEM INCLUDES
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <limits>

// PROJECT INCLUDES
#include "utility/avl_tree.hpp"
#include "utility/test_helpers.hpp"

namespace btr
{

//------------------------------------------------------------------------------

class Server : public NodeBase<Server>, public NodeObserver<Server>
{
public:

// LIFECYCLE

  Server(uint16_t key) :
    NodeBase(key)
  {}

// OPERATIONS

  /**
   * @override NodeObserver::onTraverse
   */
  virtual void onTraverse(Server* node)
  {
    keys_.push_back(node->key());
  }

// ATTRIBUTES

  std::vector<uint16_t> keys_;
};

//------------------------------------------------------------------------------

class AvlTreeTest : public testing::Test
{
public:

// LIFECYCLE

  AvlTreeTest() :
    tree_(),
    observer_(0),
    keys_()
  {
    keys_ = { 1, 9, 3, 7 };

    for (auto key : keys_) {
      tree_.insert(key);
    }
  }

  ~AvlTreeTest()
  {
    tree_.eraseBranch(tree_.root());
  }

// ATTRIBUTES

  AvlTree<Server> tree_;
  Server observer_;
  std::vector<uint16_t> keys_;
};

//------------------------------------------------------------------------------

TEST_F(AvlTreeTest, traverseInOrder)
{
  tree_.traverseInOrder(tree_.root(), &observer_);

  ASSERT_EQ(keys_[0], observer_.keys_[0]);
  ASSERT_EQ(keys_[2], observer_.keys_[1]);
  ASSERT_EQ(keys_[3], observer_.keys_[2]);
  ASSERT_EQ(keys_[1], observer_.keys_[3]);
}

TEST_F(AvlTreeTest, search)
{
  ASSERT_EQ(3, tree_.root()->key());

  for (auto key : keys_) {
    Server* n = tree_.search(tree_.root(), key);
    ASSERT_TRUE(nullptr != n) << "Key: " << key << std::endl;
  }
}

TEST_F(AvlTreeTest, erase)
{
  Server* s_node = tree_.search(tree_.root(), 3);
  ASSERT_TRUE(nullptr != s_node);
  ASSERT_TRUE(tree_.root() == s_node);
  ASSERT_EQ(3, s_node->key());
  s_node->keys_.clear();
  s_node->keys_.push_back(91);
  s_node->keys_.push_back(88);

  // After deleting root node 3, node 7 becomes root. We expect data members to be copied over
  // from 3 to 7.
  Server* e_node = tree_.erase(3);
  s_node = tree_.search(tree_.root(), 3);
  ASSERT_TRUE(nullptr == s_node);

  e_node = tree_.erase(9);
  s_node = tree_.search(tree_.root(), 9);
  ASSERT_TRUE(nullptr == s_node);

  s_node = tree_.search(tree_.root(), 7);
  ASSERT_TRUE(tree_.root() == s_node);
  ASSERT_EQ(tree_.root()->key(), s_node->key());
  ASSERT_EQ(1, s_node->left()->key());
  ASSERT_TRUE(nullptr == s_node->right());
  ASSERT_EQ(91, s_node->keys_[0]);
  ASSERT_EQ(88, s_node->keys_[1]);
}

TEST_F(AvlTreeTest, eraseBranch)
{
  tree_.eraseBranch(tree_.root());
  ASSERT_TRUE(nullptr == tree_.root());
}

} // namespace btr