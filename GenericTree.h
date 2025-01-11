
#pragma once

#include <stdexcept> // for std::runtime_error
#include <stack> // for std::stack
#include <queue> // for std::queue
#include <vector> // for std::vector
#include <iostream> // for std::cerr, std::cout
#include <ostream> // for std::ostream


template <typename T>
class GenericTree {
public:

  // This toggles whether to output extra debugging messages.
  // We'll set it to false by default.
  bool showDebugMessages;

  // An internal class type for tree nodes.
  class TreeNode {
  public:
    // Pointer to the node's parent (nullptr if there is no parent)
    TreeNode* parentPtr;

    std::vector< TreeNode* > childrenPtrs;

    T data;

    // Add a rightmost child to this node storing a copy of the provided data.
    // Returns a pointer to the new child node.
    TreeNode* addChild(const T& childData);

    // Default constructor: Indicate that there is no parent.
    TreeNode() : parentPtr(nullptr) {}

    // Constructor based on data argument:
    // Specifies no parent, but does copy in the data member by value.
    TreeNode(const T& dataArg) : parentPtr(nullptr), data(dataArg) {}

    TreeNode(const TreeNode& other) = delete;

    // Copy assignment operator: We will disable it.
    TreeNode& operator=(const TreeNode& other) = delete;

    // Destructor: Although we don't seem to write anything here,
    // the members of the node class will have their own destructors
    // automatically called afterward.
    ~TreeNode() {}

  };

private:

  TreeNode* rootNodePtr;

public:
  TreeNode* createRoot(const T& rootData);

  // Get a copy of the raw pointer to the root node of this class instance.
  TreeNode* getRootPtr() {
    return rootNodePtr;
  }

  void deleteSubtree(TreeNode* targetRoot);


  void compress();

  // Default constructor: Indicate that there is no root (empty tree).
  GenericTree() : showDebugMessages(false), rootNodePtr(nullptr) {}

  // Parameter constructor: Creates an empty tree, then adds a root node
  // with the provided data.
  GenericTree(const T& rootData) : GenericTree() {
    createRoot(rootData);
  }

  // Copy constructor: We will disable it.
  GenericTree(const GenericTree& other) = delete;

  // Copy assignment operator: We will disable it.
  GenericTree& operator=(const GenericTree& other) = delete;

  void clear() {
    // Use our special function to deallocate the entire tree
    deleteSubtree(rootNodePtr);

 

    if (rootNodePtr) {
      throw std::runtime_error("clear() detected that deleteSubtree() had not reset rootNodePtr");
    }
  }

  // Destructor
  ~GenericTree() {
    clear();
  }

  // Print the tree to the output stream (for example, std::cout) in a vertical text format
  std::ostream& Print(std::ostream& os) const;

};

// Operator overload that allows stream output syntax
template <typename T>
std::ostream& operator<<(std::ostream& os, const GenericTree<T>& tree) {
  return tree.Print(os);
}



template <typename T>
typename GenericTree<T>::TreeNode* GenericTree<T>::createRoot(const T& rootData) {
  
  // If the rootNodePtr member variable already has a nonzero value assigned,
  // then the root node already exists, and it's an error to try to recreate it.
  if (nullptr != rootNodePtr) {
    // We don't always need to import <string> just to reuse a short error message.
    // We can store short strings as constant arrays of char type at compile time.
    // (A constexpr is similar to const, but must be entirely evaluated at compile time.)
    constexpr char ERROR_MESSAGE[] = "Tried to createRoot when root already exists";
    // Display the error message to the standard error stream.
    std::cerr << ERROR_MESSAGE << std::endl;
    // Throw an exception containing the error message.
    // We won't catch the exception, so this terminates the program.
    throw std::runtime_error(ERROR_MESSAGE);
  }

  rootNodePtr = new TreeNode(rootData);

  // Return a copy of the root node pointer.
  return rootNodePtr;
}

template <typename T>
typename GenericTree<T>::TreeNode* GenericTree<T>::TreeNode::addChild(const T& childData) {

  // We prepare a new child node with the given data.
  TreeNode* newChildPtr = new TreeNode(childData);


  newChildPtr->parentPtr = this;

  childrenPtrs.push_back(newChildPtr);

  // Return a copy of the pointer to the new child.
  return newChildPtr;
}

template <typename T>
void GenericTree<T>::deleteSubtree(TreeNode* targetRoot) {

 
  if (nullptr == targetRoot) {
    return;
  }

  // Check that the specified node to delete is in the same tree as this
  // class instance that's calling the function.
  {
    TreeNode* walkBack = targetRoot;
    while (walkBack->parentPtr) {
      // Walk back from the targeted node to its ultimate parent, the root.
      // (The root has no parent, so the walk ends there.)
      walkBack = walkBack->parentPtr;
    }
    // The ultimate root found must be this tree's root. Otherwise we're in
    // a different tree.
    if (walkBack != rootNodePtr) {
      throw std::runtime_error("Tried to delete a node from a different tree");
    }
  }

  // We'll take note whether this is the root of the entire tree.
  bool targetingWholeTreeRoot = (rootNodePtr == targetRoot);

  if (targetRoot->parentPtr) {

    // A flag for error checking: We need to find the target node
    // listed as a child of its parent. We will keep track as we search.
    bool targetWasFound = false;
    

    for (auto& currentChildPtr : targetRoot->parentPtr->childrenPtrs) {
      if (currentChildPtr == targetRoot) {
        // We found where the parent node is pointing to the target
        // node as its child. Replace that pointer with a null pointer.
        currentChildPtr = nullptr;
        // Flag that our search succeeded, for error checking.
        targetWasFound = true;
        // Stop looping early. The "break" statement exits the current "for"
        // loop and moves on to the next statement outside.
        break;
      }
    }

    // If the target node was not found, our tree is malformed somehow.
    if (!targetWasFound) {
      // If this flag is still false, we have some kind of bug.
      // The target should have been listed as a child of its parent.
      constexpr char ERROR_MESSAGE[] = "Target node to delete was not listed as a child of its parent";
      std::cerr << ERROR_MESSAGE << std::endl;
      throw std::runtime_error(ERROR_MESSAGE);
    }
  }

 
  std::stack<TreeNode*> nodesToExplore;

  // We also need a stack for the pointers that need to be deleted:
  std::stack<TreeNode*> nodesToDelete;

  // To begin with, we'll record that the target node, which is the root of
  // the subtree, needs to be explored:
  nodesToExplore.push(targetRoot);

  // Keep looping as long as there are nodes left to explore and delete.
  while (!nodesToExplore.empty()) {

    // Get a copy of the top pointer on the explore stack.
    TreeNode* curNode = nodesToExplore.top();

    // Now that we've retrieved the top pointer, we can pop it from the explore stack.
    nodesToExplore.pop();

    if (showDebugMessages) {
      std::cerr << "Exploring node: ";
      if (curNode) {
        // if curNode isn't null, we can show what it contains
        std::cerr << curNode->data << std::endl;
      }
      else {
        std::cerr << "[null]" << std::endl;
      }
    }

    // If nullptr...
    if (!curNode) {
      // The "continue" statement jumps to the top of the next iteration
      // of the while loop.
      continue; 
    }

    // Record that we need to delete this node later, by pushing it onto the delete stack.
    nodesToDelete.push(curNode);

    // Loop through the current node's children pointers from first to last,
    // which we interpret as left to right
    for (auto childPtr : curNode->childrenPtrs) {
      // Push a copy of the child pointer onto the stack of children to explore
      nodesToExplore.push(childPtr);
    }

  } // End of explore loop.

  // We're done exploring all the nodes in the tree now, so now we need
  // to delete the nodes one at a time from the delete stack.
  while (!nodesToDelete.empty()) {
    
    // Get a copy of the top pointer on the delete stack.
    TreeNode* curNode = nodesToDelete.top();

    // Now that we've retrieved the top pointer, we can pop it from the stack.
    nodesToDelete.pop();

    if (showDebugMessages) {
      std::cerr << "Deleting node: ";
      if (curNode) {
        // if curNode isn't null, we can show what it contains
        std::cerr << curNode->data << std::endl;
      }
      else {
        std::cerr << "[null]" << std::endl;
      }
    }

    // Delete the current node pointer.
    delete curNode;

    curNode = nullptr;


  }

  // If we deleted the root node of this class instance,
  //  then reset the root pointer.
  if (targetingWholeTreeRoot) {
    rootNodePtr = nullptr;
  }

  return;
}

template <typename T>
void GenericTree<T>::compress() {

  if (!rootNodePtr) return;

  // Queue of node pointers that we still need to explore (constructed empty)
  std::queue<TreeNode*> nodesToExplore;

  // Begin by pushing our root pointer onto the queue
  nodesToExplore.push(rootNodePtr);

  // Loop while there are still nodes to explore
  while (!nodesToExplore.empty()) {

    // Make a copy of the front pointer on the queue, then pop it to decrease the queue
    TreeNode* frontNode = nodesToExplore.front();
    nodesToExplore.pop();

    if (!frontNode) {
      // The front node pointer should not be null, because we're designing this
      // function so that no null pointers should ever get pushed onto the exploration queue.
      throw std::runtime_error("Error: Compression exploration queued a null pointer");
    }

 
    std::vector<TreeNode*> compressedChildrenPtrs;
    // Now loop through the currently recorded children pointers...
    for (auto childPtr : frontNode->childrenPtrs) {
      if (childPtr) {
        // If this child pointer is not null, then push it onto the back
        // of our new, compressed pointers vector.
        compressedChildrenPtrs.push_back(childPtr);
        // Also put this child pointer onto the end of the exploration queue.
        nodesToExplore.push(childPtr);
      }
    }


    frontNode->childrenPtrs.swap(compressedChildrenPtrs);
  }

}

template <typename T>
std::ostream& GenericTree<T>::Print(std::ostream& os) const {


  const TreeNode* rootNodePtr = this->rootNodePtr;

  // Now that the pointer is const, if we tried to do this, the compiler
  // would give an error and stop us:
  // rootNodePtr->data = "This won't compile";

  // Base case: When the tree is empty
  if (nullptr == rootNodePtr) {
    return os << "[empty tree]" << std::endl;
  }

  // Our stack of nodes that still need to be explored and printed
  std::stack<const TreeNode*> nodesToExplore;
  nodesToExplore.push(rootNodePtr);

  // We'll have a depth number associated with every node we find.
  // Begin tracking the current depth at 0.
  std::stack<int> depthStack;
  depthStack.push(0);

  std::stack< std::vector<bool> > curMarginStack;
  // Begin with no margin for depth 0.
  curMarginStack.push( std::vector<bool>() );
  // Each node's margin graphics will be based on the trailing lines that
  // are still running parallel in the margin.
  std::stack< std::vector<bool> > trailingMarginStack;
  trailingMarginStack.push( std::vector<bool>() );

 

  while (!nodesToExplore.empty()) {

    // Get a copy of the top pointer on the explore stack.
    const TreeNode* curNode = nodesToExplore.top();

    // Now that we've retrieved the top pointer, we can pop it from the explore stack.
    nodesToExplore.pop();

    // Pop the current depth for the node being explored.
    int curDepth = depthStack.top();
    depthStack.pop();

    // Pop the current and trailing margin graphic flags for this node.
    std::vector<bool> curMargin = curMarginStack.top();
    curMarginStack.pop();
    std::vector<bool> trailingMargin = trailingMarginStack.top();
    trailingMarginStack.pop();

    if (showDebugMessages) {
      // Simplified numerical output for debugging.
      os << "Depth: " << curDepth;
      std::cerr << " Data: ";
      if (curNode) {
        // if curNode isn't null, we can show what it contains
        std::cerr << curNode->data << std::endl;
      }
      else {
        std::cerr << "[null]" << std::endl;
      }
    }
    else {


      constexpr int LAST_ROW = 2;

      for (int row = 1; row<=LAST_ROW; row++) {
        // Iterate forward through the margin display flags to fill in the margin.
        for (auto stemIt = curMargin.begin(); stemIt != curMargin.end(); stemIt++) {

          bool showStem = *stemIt;
          std::string stemSymbol = "|";
          if (!showStem) {
            stemSymbol = " ";
          }

          bool isLastCol = false;
          if (stemIt + 1 == curMargin.end()) {
            isLastCol = true;
          }

          if (isLastCol) {
            if (LAST_ROW==row) {
              // The stem before the data item should always be "|_ " in effect:
              os << stemSymbol << "_ ";
            }
            else if (showStem) {
              // Display a stem and a newline
              os << stemSymbol << std::endl;
            }
            else {
              // Don't bother displaying trailing spaces before the newline
              os << std::endl;
            }
          }
          else {
            // Display a stem (or a blank) and some padding spaces
            os << stemSymbol << "  ";
          }

          // Bottom of loop for margin stems
        }

        // Bottom of loop for multi-row display
      }

      if (curNode) {
        os << curNode->data << std::endl;
      }
      else {
        os << "[null]" << std::endl;;
      }

    }

    // If this node is non-null and has any children...
    if (curNode && curNode->childrenPtrs.size() > 0) {

 
      for (auto it = curNode->childrenPtrs.rbegin(); it != curNode->childrenPtrs.rend(); it++) {
        
     
        const TreeNode* childPtr = *it;

        // Now, push the child pointer onto the stack of children to explore.
        nodesToExplore.push(childPtr);
        
        // Record the depth that corresponds to the child node.
        depthStack.push(curDepth+1);
        
        // Prepare a working copy of the margin for the node we're pushing.
        auto nextMargin = trailingMargin;
        // All nodes get an extra stem glyph next to their printout.
        nextMargin.push_back(true);
        curMarginStack.push(nextMargin);
        
        // But for the trailing margin, we need to leave the rightmost child
        // with a blank trailing in the margin, because it's displayed lowest.
        auto nextTrailingMargin = trailingMargin;
        if (curNode->childrenPtrs.rbegin() == it) {
          // This is the rightmost child. Leave a blank trailing.
          nextTrailingMargin.push_back(false);
        }
        else {
          // Other children leave a vertical stem symbol trailing in the margin.
          nextTrailingMargin.push_back(true);
        }
        trailingMarginStack.push(nextTrailingMargin);

      }
    }

  }

  return os;
}
