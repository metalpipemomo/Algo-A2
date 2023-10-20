#include "../include/UnorderedSet.h"

#include <vector>

template <typename T>
UnorderedSet<T>::UnorderedSet()
{
    root = nullptr;
}

template <typename T>
UnorderedSet<T>::~UnorderedSet()
{
    clearRecursive(root);
}

template <typename T>
UnorderedSet<T>::Iterator UnorderedSet<T>::begin() const
{
    return Iterator{ root };
}

template <typename T>
UnorderedSet<T>::Iterator UnorderedSet<T>::end() const
{
    std::vector<Node<T>*> queue = { root };
    Node<T>* node = nullptr;
    while (queue.size())
    {
        node = queue.front();
        queue.erase(queue.begin());
        if (node->left)
        {
            queue.push_back(node->left);
        }
        if (node->right)
        {
            queue.push_back(node->right);
        }
    }

    return Iterator{ node };
}

template <typename T>
void insert_helper(Node<T>* root, Node<T>* node)
{
    if (node->key < root->key)
    {
        if (root->left != nullptr)
        {
            insert_helper(root->left, node);
        }
        else 
        {
            root->left = node;
            node->parent = root;
        }
    }
    else 
    {
        if (root->right != nullptr)
        {
            insert_helper(root->right, node);
        }
        else 
        {
            root->right = node;
            node->parent = root;
        }
    }
}

template <typename T>
bool UnorderedSet<T>::insert(const T& key)
{
    Node<T>* node = new Node<T>(key);
    node->color = Color::RED;

    if (search(key)) return false;

    if (root == nullptr) {
        node->color = Color::BLACK;
        root = node;
    }
    else
    {
        insert_helper(root, node);
        fixRedRedViolation(node);
    }
    updateSize();
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
    clearRecursive(root);
    updateSize();
}

template <typename T>
size_t UnorderedSet<T>::size() const
{
    return setSize;
}

template <typename T>
void UnorderedSet<T>::updateSize()
{
    setSize = getSize(root);
}

template <typename T>
size_t UnorderedSet<T>::getSize(Node<T>* node) const
{
    if (node == nullptr) return 0;
    return 1 + getSize(node->left) + getSize(node->right);
}

template <typename T>
void UnorderedSet<T>::fixRedRedViolation(Node<T>* node)
{
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
    node->parent->color = Color::BLACK;
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
void transplant(Node<T>* root, Node<T>* u, Node<T>* v)
{
    if (u == root)
    {
        root = v;
    }
    else if (u == u->parent->left)
    {
        u->parent->left = v;
    }
    else
    {
        u->parent->right = v;
    }
    v->parent = u->parent;
}

template <typename T>
Node<T>* treeMin(Node<T>* node)
{
    while (node->left != nullptr)
        {
            node = node->left;
        }
    return node;
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
        transplant(root, node, node->right);
    }
    else if (node->right == nullptr)
    {
        child = node->left;
        transplant(root, node, node->left);
    }
    else
    {
        original = treeMin(node->right);
        originalColor = original->color;
        child = original->right;
        if (original->parent == node)
        {
            child->parent = original;
        }
        else 
        {
            transplant(root, original, original->right);
            original->right = node->right;
            original->right->parent = original;
        }
        transplant(root, node, original);
        original->left = node->left;
        original->left->parent = original;
        original->color = node->color;
    }
    if (originalColor == Color::BLACK)
    {
        deleteFix(child);
    }
    updateSize();
}

template <typename T>
void UnorderedSet<T>::deleteFix(Node<T>* node)
{
    while (node != root && node->color == Color::BLACK)
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
