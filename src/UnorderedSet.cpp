#include "../include/UnorderedSet.h"

#include <vector>

template <typename T>
UnorderedSet<T>::UnorderedSet()
{
    // Instantiates setSize (size of tree) and the root node
    setSize = 0;
    root = nullptr;
}

template <typename T>
UnorderedSet<T>::~UnorderedSet()
{
    // Sets the root to nullptr, making the rest of the nodes inaccessible
    if (root == nullptr) return;
    root = nullptr;
}

template <typename T>
Node<T>* GetLeftmostNode(Node<T>* root)
{
    // Recursively finds the leftmost node in the tree
    if (root == nullptr || root->left == nullptr) return root;

    return GetLeftmostNode(root->left);
}

template <typename T>
typename UnorderedSet<T>::Iterator UnorderedSet<T>::begin() const
{
    // Return an Iterator to the leftmost node in the tree
    return Iterator{ GetLeftmostNode(root) };
}

template <typename T>
typename UnorderedSet<T>::Iterator UnorderedSet<T>::end() const
{
    // Retun an Iterator to nothing, the end iterator in a tree is just nullptr
    return Iterator{ nullptr };
}

template <typename T>
Node<T>* insert_helper(Node<T>* node, const T& key)
{
    // Recursively tries to insert a key, uses binary tree search

    // Checks if the key is smaller than the node's key
    if (node->key > key)
    {
        // Traverse to the left
        // If the node exists, try again
        // If not, insert here
        if (node->left != nullptr)
        {
            return insert_helper(node->left, key);
        }
        else
        {
            node->left = new Node<T>(key);
            node->left->parent = node;
            return node->left;
        }
    }
    else
    {
        // Traverse to the right
        // If the node exists, try again
        // If not, insert here
        if (node->right != nullptr)
        {
            return insert_helper(node->right, key);
        }
        else
        {
            node->right = new Node<T>(key);
            node->right->parent = node;
            return node->right;
        }
    }
}

template <typename T>
bool UnorderedSet<T>::insert(const T& key)
{
    // If key exists, dont insert
    if (search(key)) return false;

    // If the tree does not have a root, insert key into the root
    if (root == nullptr)
    {
        root = new Node<T>(key);
        root->color = Color::BLACK;
    }
    else
    {
        // Insert key recursively, starting at root
        Node<T>* node = insert_helper(root, key);

        // Fix any violations after insertion
        fixRedRedViolation(node);
    }

    // Increase tree size
    setSize++;

    return true;
}

template <typename T>
Node<T>* search_helper(Node<T>* root, const T& key)
{
    // Recursively searches a tree for a node with the given key
    if (root == nullptr || key == root->key)
    {
        return root;
    }
    else if (key < root->key)
    {
        return search_helper(root->left, key);
    }
    else 
    {
        return search_helper(root->right, key);
    }
}

template <typename T>
bool UnorderedSet<T>::search(const T& key) const
{
    // Self-explanatory
    return search_helper(root, key);
}

template <typename T>
bool UnorderedSet<T>::erase(const T& key)
{
    // Find the node with the associated key
    Node<T>* result = search_helper(root, key);
    
    // Delete the node if it exists
    if (result == nullptr) return false;
    deleteOneChild(result);
    return true;
}

template <typename T>
void UnorderedSet<T>::clear()
{
    // Recursively deletes every node in the tree, sets root to nullpt and resets size of tree
    if (root == nullptr) return;
    clearRecursive(root);
    root = nullptr;
    setSize = 0;
}

template <typename T>
size_t UnorderedSet<T>::size() const
{
    // Self-explanatory
    return setSize;
}

template <typename T>
void UnorderedSet<T>::fixRedRedViolation(Node<T>* node)
{
    // If node is null, do nothing
    if (node == nullptr) return;

    // Continue fixing while the node isn't the root and its parent is colored red
    while (node != root && node->parent->color == Color::RED)
    {
        // If parent is the left child of its parent
        if (node->parent == node->parent->parent->left)
        {
            // Get the uncle of the node (right sibling of the parent)
            Node<T>* uncle = node->parent->parent->right;

            // If the uncle exists and is red, then it's a red-red violation
            if (uncle != nullptr && uncle->color == Color::RED)
            {
                // Color the parent and the uncle black
                node->parent->color = Color::BLACK;
                uncle->color = Color::BLACK;

                // Color the grandparent red and move up to the grandparent
                node->parent->parent->color = Color::RED;
                node = node->parent->parent;
            }
            else
            {
                // If the node is the right child of its parent
                if (node == node->parent->right)
                {
                    // Make the node its parent and rotate left
                    node = node->parent;
                    rotateLeft(node);
                }
                // Color the parent black and grandparent red, then rotate right
                node->parent->color = Color::BLACK;
                node->parent->parent->color = Color::RED;
                rotateRight(node->parent->parent);
            }
        }
        else // If parent is the right child of its parent
        {
            // Get the uncle of the node (left sibling of the parent)
            Node<T>* uncle = node->parent->parent->left;
            
            // If the uncle exists and is red, then it's a red-red violation
            if (uncle != nullptr && uncle->color == Color::RED)
            {
                // Color the parent and the uncle black
                node->parent->color = Color::BLACK;
                uncle->color = Color::BLACK;
                
                // Color the grandparent red and move up to the grandparent
                node->parent->parent->color = Color::RED;
                node = node->parent->parent;
            }
            else
            {
                // If the node is the left child of its parent
                if (node == node->parent->left)
                {
                    // Make the node its parent and rotate right
                    node = node->parent;
                    rotateRight(node);
                }
                // Color the parent black and grandparent red, then rotate left
                node->parent->color = Color::BLACK;
                node->parent->parent->color = Color::RED;
                rotateLeft(node->parent->parent);
            }
        }
    }

    // Ensure the root is always colored black
    root->color = Color::BLACK;
}

template <typename T>
void UnorderedSet<T>::rotateLeft(Node<T>* node)
{
    // Start with the right child of the node; this becomes the pivot for rotation
    Node<T>* child = node->right;

    // Update the right child of the node to the left child of the pivot (child)
    node->right = child->left;

    // If the left child of the pivot isn't null, update its parent pointer
    if (child->left != nullptr)
    {
        child->left->parent = node;
    }

    // Connect the pivot's parent pointer to the node's parent
    child->parent = node->parent;

    // If the node being rotated was the root, update the root to the pivot
    if (node->parent == nullptr)
    {
        root = child;
    }
    else if (node == node->parent->left) // If node was a left child
    {
        // Update the left child of the node's parent to the pivot
        node->parent->left = child;
    }
    else // If node was a right child
    {
        // Update the right child of the node's parent to the pivot
        node->parent->right = child;
    }

    // Set the left child of the pivot to the node
    child->left = node;

    // Finally, update the parent pointer of the node to the pivot
    node->parent = child;
}

template <typename T>
void UnorderedSet<T>::rotateRight(Node<T>* node)
{
    // Start with the left child of the node; this becomes the pivot for rotation
    Node<T>* child = node->left;

    // Update the left child of the node to the right child of the pivot (child)
    node->left = child->right;
    
    // If the right child of the pivot isn't null, update its parent pointer
    if (child->right != nullptr)
    {
        child->right->parent = node;
    }

    // Connect the pivot's parent pointer to the node's parent
    child->parent = node->parent;
    
    // If the node being rotated was the root, update the root to the pivot
    if (node->parent == nullptr)
    {
        root = child;
    }
    else if (node == node->parent->right) // If node was a right child
    {
        // Update the right child of the node's parent to the pivot
        node->parent->right = child;
    }
    else // If node was a left child
    {
        // Update the left child of the node's parent to the pivot
        node->parent->left = child;
    }

    // Set the right child of the pivot to the node
    child->right = node;

    // Finally, update the parent pointer of the node to the pivot
    node->parent = child;
}

template<typename T>
void transplant(UnorderedSet<T>* set, Node<T>* u, Node<T>* v)
{
    // If u is the root node, set v as the new root
    if (u->parent == nullptr)
    {
        set->root = v;
    }
    // If u is a left child, update its parent's left pointer to v
    else if (u == u->parent->left)
    {
        u->parent->left = v;
    }
    // If u is a right child, update its parent's right pointer to v
    else
    {
        u->parent->right = v;
    }

    // If v is not null, update its parent pointer to u's parent
    if (v != nullptr)
    {
        v->parent = u->parent;
    }
}

template <typename T>
void UnorderedSet<T>::deleteOneChild(Node<T>* node)
{
    // To keep track of the node being deleted or the node moved within the tree
    Node<T>* original = node;
    Color originalColor = original->color;
    Node<T>* child = nullptr;

    // Case where node has no left child
    // Take the right child as the only child
    // Replace node with its right child in the tree
    if (node->left == nullptr)
    {
        child = node->right;
        transplant(this, node, node->right);
    }
    // Case where node has no right child
    // Take the left child as the only child
    // Replace node with its left child in the tree
    else if (node->right == nullptr)
    {
        child = node->left;
        transplant(this, node, node->left);
    }
    // Case where node has two children
    else
    {
        // Find the smallest node in the right subtree (in-order successor)
        original = GetLeftmostNode(node->right);
        originalColor = original->color;
        child = original->right;

        // If the in-order successor is a direct child of the node
        if (original->parent == node)
        {
            if (child != nullptr)
            {
                // Ensure child's parent is pointed to in-order successor
                child->parent = original;
            }
        }
        else 
        {
            // Replace the successor with its own right child
            transplant(this, original, original->right);
            original->right = node->right;
            original->right->parent = original;
        }
        // Replace node with the in-order successor in the tree
        transplant(this, node, original);
        original->left = node->left;
        original->left->parent = original;
        original->color = node->color;
    }
    // If the color of the node (or the node replacing it) was black, then tree properties might be violated, call fix
    if (originalColor == Color::BLACK)
    {
        deleteFix(child);
    }
    // Decrement the size of the set
    setSize--;
}

template <typename T>
void UnorderedSet<T>::deleteFix(Node<T>* node)
{
    // If the node is null, do nothing
    if (node == nullptr) return;

    // Continue fixing until the node is either the root or is colored red
    while (node != nullptr && node != root && node->color == Color::BLACK)
    {
        // Handling the case when the node is a left child
        if (node == node->parent->left)
        {
            // Get the sibling of the node (right child of the parent)
            Node<T>* sibling = node->parent->right;

            // Case 1: If the sibling is red
            // Color the sibling black
            // Color the parent red
            // Left rotate the parent
            // Update the sibling reference
            if (sibling->color == Color::RED)
            {
                sibling->color = Color::BLACK;
                node->parent->color = Color::RED;
                rotateLeft(node->parent);
                sibling = node->parent->right;
            }
            // Case 2: Both of the sibling's children are black
            // Color the sibling red
            // Move up the tree
            if (sibling->left->color == Color::BLACK && sibling->right->color == Color::BLACK)
            {
                sibling->color = Color::RED;
                node = node->parent;
            }
            // Case 3: The right child of the sibling is black
            // Color left child black
            // Color the sibling red
            // Right rotate the sibling
            // Update the sibling reference
            else if (sibling->right->color == Color::BLACK)
            {
                sibling->left->color = Color::BLACK;
                sibling->color = Color::RED;
                rotateRight(sibling);
                sibling = node->parent->right;
            }

            // Case 4: Fixing colors and performing a left rotation
            sibling->color = node->parent->color;
            node->parent->color = Color::BLACK;
            sibling->right->color = Color::BLACK;
            rotateLeft(node->parent);
            node = root;
        }
        else // Mirrored logic for when the node is a right child
        {
            Node<T>* sibling = node->parent->left;
            if (sibling->color == Color::RED)
            {
                sibling->color = Color::BLACK;
                node->parent->color = Color::RED;
                rotateRight(node->parent);
                sibling = node->parent->left;
            }
            if (sibling->right->color == Color::BLACK && sibling->left->color == Color::BLACK)
            {
                sibling->color = Color::RED;
                node = node->parent;
            }
            else if (sibling->left->color == Color::BLACK)
            {
                sibling->right->color = Color::BLACK;
                sibling->color = Color::RED;
                rotateLeft(sibling);
                sibling = node->parent->left;
            }
            sibling->color = node->parent->color;
            node->parent->color = Color::BLACK;
            sibling->left->color = Color::BLACK;
            rotateRight(node->parent);
            node = root;
        }
    }

    // Ensure the root is always black
    node->color = Color::BLACK;
}

template <typename T>
void UnorderedSet<T>::clearRecursive(Node<T>* node)
{
    // Recursively iterate through the tree and delete each node
    if (node->left != nullptr)
    {
        clearRecursive(node->left);
    }
    if (node->right != nullptr)
    {
        clearRecursive(node->right);
    }
    delete node;
}
