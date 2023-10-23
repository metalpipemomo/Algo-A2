#include "../include/UnorderedSet.h"

#include <vector>

template <typename T>
UnorderedSet<T>::UnorderedSet()
{
    setSize = 0;
    root = nullptr;
}

template <typename T>
UnorderedSet<T>::~UnorderedSet()
{
    if (root != nullptr) clearRecursive(root);
}

template <typename T>
Node<T>* GetLeftmostNode(Node<T>* root)
{
    if (root == nullptr || root->left == nullptr) return root;

    return GetLeftmostNode(root->left);
}

template <typename T>
typename UnorderedSet<T>::Iterator UnorderedSet<T>::begin() const
{
    return Iterator{ GetLeftmostNode(root) };
}

template <typename T>
typename UnorderedSet<T>::Iterator UnorderedSet<T>::end() const
{
    return Iterator{ nullptr };
}

template <typename T>
Node<T>* insert_helper(Node<T>* node, const T& key)
{
    if (node->key > key)
    {
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
    if (search(key)) return false;

    if (root == nullptr)
    {
        root = new Node<T>(key);
        root->color = Color::BLACK;
    }
    else
    {
        Node<T>* node = insert_helper(root, key);
        fixRedRedViolation(node);
    }

    setSize++;

    return true;
}

template <typename T>
Node<T>* search_helper(Node<T>* root, const T& key)
{
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
    return search_helper(root, key);
}

template <typename T>
bool UnorderedSet<T>::erase(const T& key)
{
    Node<T>* result = search_helper(root, key);
    if (result == nullptr) return false;

    deleteOneChild(result);
    return true;
}

template <typename T>
void UnorderedSet<T>::clear()
{
    if (root == nullptr) return;
    clearRecursive(root);
    root = nullptr;
    setSize = 0;
}

template <typename T>
size_t UnorderedSet<T>::size() const
{
    return setSize;
}

template <typename T>
void UnorderedSet<T>::fixRedRedViolation(Node<T>* node)
{
    if (node == nullptr) return;
    while (node != root && node->parent->color == Color::RED)
    {
        if (node->parent == node->parent->parent->left)
        {
            Node<T>* uncle = node->parent->parent->right;
            if (uncle != nullptr && uncle->color == Color::RED)
            {
                node->parent->color = Color::BLACK;
                uncle->color = Color::BLACK;
                node->parent->parent->color = Color::RED;
                node = node->parent->parent;
            }
            else
            {
                if (node == node->parent->right)
                {
                    node = node->parent;
                    rotateLeft(node);
                }
                node->parent->color = Color::BLACK;
                node->parent->parent->color = Color::RED;
                rotateRight(node->parent->parent);
            }
        }
        else
        {
            Node<T>* uncle = node->parent->parent->left;
            if (uncle != nullptr && uncle->color == Color::RED)
            {
                node->parent->color = Color::BLACK;
                uncle->color = Color::BLACK;
                node->parent->parent->color = Color::RED;
                node = node->parent->parent;
            }
            else
            {
                if (node == node->parent->left)
                {
                    node = node->parent;
                    rotateRight(node);
                }
                node->parent->color = Color::BLACK;
                node->parent->parent->color = Color::RED;
                rotateLeft(node->parent->parent);
            }
        }
    }
    root->color = Color::BLACK;
}

template <typename T>
void UnorderedSet<T>::rotateLeft(Node<T>* node)
{
    Node<T>* child = node->right;
    node->right = child->left;
    if (child->left != nullptr)
    {
        child->left->parent = node;
    }
    child->parent = node->parent;
    if (node->parent == nullptr)
    {
        root = child;
    }
    else if (node == node->parent->left)
    {
        node->parent->left = child;
    }
    else
    {
        node->parent->right = child;
    }
    child->left = node;
    node->parent = child;
}

template <typename T>
void UnorderedSet<T>::rotateRight(Node<T>* node)
{
    Node<T>* child = node->left;
    node->left = child->right;
    if (child->right != nullptr)
    {
        child->right->parent = node;
    }
    child->parent = node->parent;
    if (node->parent == nullptr)
    {
        root = child;
    }
    else if (node == node->parent->right)
    {
        node->parent->right = child;
    }
    else
    {
        node->parent->left = child;
    }
    child->right = node;
    node->parent = child;
}

template<typename T>
void transplant(UnorderedSet<T>* set, Node<T>* u, Node<T>* v)
{
    if (u->parent == nullptr)
    {
        set->root = v;
    }
    else if (u == u->parent->left)
    {
        u->parent->left = v;
    }
    else
    {
        u->parent->right = v;
    }

    if (v != nullptr)
    {
        v->parent = u->parent;
    }
}

template <typename T>
void UnorderedSet<T>::deleteOneChild(Node<T>* node)
{
    Node<T>* original = node;
    Color originalColor = original->color;
    Node<T>* child = nullptr;

    if (node->left == nullptr)
    {
        child = node->right;
        transplant(this, node, node->right);
    }
    else if (node->right == nullptr)
    {
        child = node->left;
        transplant(this, node, node->left);
    }
    else
    {
        original = GetLeftmostNode(node->right);
        originalColor = original->color;
        child = original->right;
        if (original->parent == node)
        {
            if (child != nullptr)
            {
                child->parent = original;
            }
        }
        else 
        {
            transplant(this, original, original->right);
            original->right = node->right;
            original->right->parent = original;
        }
        transplant(this, node, original);
        original->left = node->left;
        original->left->parent = original;
        original->color = node->color;
    }
    if (originalColor == Color::BLACK)
    {
        deleteFix(child);
    }
    setSize--;
}

template <typename T>
void UnorderedSet<T>::deleteFix(Node<T>* node)
{
    if (node == nullptr) return;

    while (node != nullptr && node != root && node->color == Color::BLACK)
    {
        if (node == node->parent->left)
        {
            Node<T>* sibling = node->parent->right;
            if (sibling->color == Color::RED)
            {
                sibling->color = Color::BLACK;
                node->parent->color = Color::RED;
                rotateLeft(node->parent);
                sibling = node->parent->right;
            }
            if (sibling->left->color == Color::BLACK && sibling->right->color == Color::BLACK)
            {
                sibling->color = Color::RED;
                node = node->parent;
            }
            else if (sibling->right->color == Color::BLACK)
            {
                sibling->left->color = Color::BLACK;
                sibling->color = Color::RED;
                rotateRight(sibling);
                sibling = node->parent->right;
            }
            sibling->color = node->parent->color;
            node->parent->color = Color::BLACK;
            sibling->right->color = Color::BLACK;
            rotateLeft(node->parent);
            node = root;
        }
        else
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
    node->color = Color::BLACK;
}

template <typename T>
void UnorderedSet<T>::clearRecursive(Node<T>* node)
{
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
