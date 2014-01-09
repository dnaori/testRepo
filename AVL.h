#ifndef AVL_H_
#define AVL_H_

#include <iostream>
#include <cassert>
#include <cstddef> // for NULL

class ELEMENT_NOT_FOUND: public std::exception{};

template<class First, class Second>
class Pair {
public:
    Pair(const First& first, const Second& second) :
        first(first), second(second) {
    }
    First first;
    Second second;
};

template<class T>
void swap(T& a, T& b) {
    T tmp(a);
    a = b;
    b = tmp;
}

template<class Key, class T>
class AVL {
public:
    AVL();
    AVL(const AVL& avl);
    AVL& operator=(const AVL& avl);
    ~AVL();

    class Iterator;
    Iterator end();
    Iterator max();

    int getSize() const;
    Pair<Iterator, bool> insert(const Key& key, const T& element);
    bool remove(const Key& key);
    Iterator find(const Key& key);

    template<class Functor>
    void inOrder(Functor& functor);

    template<class Functor>
    void preOrder(Functor& functor);

    template<class Functor>
    void postOrder(Functor& functor);

private:
    class Node;
    Node* root;
    int size;
    Node** findAux(const Key& key);
    void removeNodeWithOneSonOrLess(Node* node);
    void removeNodeWithTwoSons(Node* node);
    void swapNodes(Node*& a, Node*& b);
    void updateNodeRelatives(Node* node, Node* swappedWith);
    void fixSwapSelfAssignment(Node* node, Node* swappedWith);

    template<class Functor>
    void inOrderAux(Node* current, Functor& functor);

    template<class Functor>
    void preOrderAux(Node* current, Functor& functor);

    template<class Functor>
    void postOrderAux(Node* current, Functor& functor);

    void destructorAux(Node* current);
    void balance(Node* node, bool onInsert);
    void rotateRight(Node* node);
    void rotateLeft(Node* node);
    bool checkInvariant() const;
};

template<class Key, class T>
AVL<Key, T>::AVL() :
    root(NULL), size(0) {
}

template<class Key, class T>
AVL<Key, T>::AVL(const AVL& avl) : root(NULL), size(avl.size) {
    if (!avl.root) {
        return;
    }
    root = avl.root->copyNodes(NULL);
}

template<class Key, class T>
AVL<Key, T>& AVL<Key, T>::operator=(const AVL<Key,T>& avl) {
    if (this == &avl) {
        return *this;
    }

    Node* nodesCopy = NULL;
    if (avl.root) {
        nodesCopy = avl.root->copyNodes(NULL);
    }
    destructorAux(root);
    size = avl.size;
    root = nodesCopy;
    return *this;
}

template<class Key, class T>
Pair<typename AVL<Key, T>::Iterator, bool> AVL<Key, T>::insert(const Key& key,
                                                      const T& element) {
    size++;
    if (!root) {
        root = new Node(key, element);
        assert(checkInvariant());
        return Pair<Iterator, bool>(Iterator(this, root), true);
    }
    Node* current = root;
    Node** next = NULL;
    while (current) {
        if (!(current->key < key) && !(key < current->key)) { // same key
            break;
        }
        next = &current->rightSon;
        if (key < current->key) {
            next = &current->leftSon;
        }

        if (!*next) {
            *next = new Node(key, element, current, NULL, NULL);
            Pair<Iterator, bool> returnPair(Iterator(this, *next), true);
            balance(current, true);
            assert(checkInvariant());
            return returnPair;
        }
        current = *next;
    }
    size--;
    assert(checkInvariant());
    return Pair<Iterator, bool>(Iterator(this, current), false);
}

template<class Key, class T>
void AVL<Key, T>::balance(Node* node, bool onInsert = false) {
    node->updateHeight();
    int BF = node->calcBF();
    Node* next = node->parent;
    if (BF == 2) {
        if (node->leftSon->calcBF() > -1) {
            rotateRight(node->leftSon);
        } else {
            assert(node->leftSon->rightSon);
            rotateLeft(node->leftSon->rightSon);
            rotateRight(node->leftSon);
        }
        if (onInsert) {
            return;
        }
    }
    if (BF == -2) {
        if (node->rightSon->calcBF() < 1) {
            rotateLeft(node->rightSon);
        } else {
            assert(node->rightSon->leftSon);
            rotateRight(node->rightSon->leftSon);
            rotateLeft(node->rightSon);
        }
        if (onInsert) {
            return;
        }
    }
    if (next) {
        balance(next, onInsert);
    }
}


template<class Key, class T>
void AVL<Key, T>::swapNodes(Node*& a, Node*& b) {
    swap(a->parent, b->parent);
    swap(a->leftSon, b->leftSon);
    swap(a->rightSon, b->rightSon);
    swap(a->height, b->height);
    fixSwapSelfAssignment(a, b);
    updateNodeRelatives(a, b);
    updateNodeRelatives(b, a);
}

template<class Key, class T>
void AVL<Key, T>::fixSwapSelfAssignment(Node* node, Node* swappedWith) {
    if (node->parent == node) {
        node->parent = swappedWith;
        if (swappedWith->leftSon == swappedWith) {
            swappedWith->leftSon = node;
            return;
        }
        assert(swappedWith->rightSon == swappedWith);
        swappedWith->rightSon = node;
        return;
    }
    if (node->leftSon == node) {
        node->leftSon = swappedWith;
        assert(swappedWith->parent == swappedWith);
        swappedWith->parent = node;
        return;
    }
    if (node->rightSon == node) {
        node->rightSon = swappedWith;
        assert(swappedWith->parent == swappedWith);
        swappedWith->parent = node;
        return;
    }
}

template<class Key, class T>
void AVL<Key, T>::updateNodeRelatives(Node* node, Node* swappedWith) {
    if (node->leftSon) {
        node->leftSon->parent = node;
    }
    if (node->rightSon) {
        node->rightSon->parent = node;
    }
    if (node->parent) {
        if (node->parent->leftSon == swappedWith) {
            node->parent->leftSon = node;
        } else if (node->parent->rightSon == swappedWith) {
            node->parent->rightSon = node;
        } else {
            assert(node->parent == swappedWith);
        }
    } else {
        root = node;
    }
}


template<class Key, class T>
void AVL<Key, T>::rotateRight(Node* node) {
    assert(node->parent);
    Node* tmp = node->rightSon;

    if (tmp) {
        tmp->parent = node->parent;
    }

    node->rightSon = node->parent;

    Node* grandParent = node->parent->parent;
    if (!grandParent) {
        root = node;
    } else if (grandParent->rightSon == node->parent) {
        grandParent->rightSon = node;
    } else {
        grandParent->leftSon = node;
    }

    node->parent = node->parent->parent;
    node->rightSon->parent = node;
    node->rightSon->leftSon = tmp;

    node->rightSon->updateHeight();
    node->updateHeight();
}

template<class Key, class T>
void AVL<Key, T>::rotateLeft(Node* node) {
    assert(node->parent);
    Node* tmp = node->leftSon;

    if (tmp) {
        tmp->parent = node->parent;
    }

    node->leftSon = node->parent;

    Node* grandParent = node->parent->parent;
    if (!grandParent) {
        root = node;
    } else if (grandParent->rightSon == node->parent) {
        grandParent->rightSon = node;
    } else {
        grandParent->leftSon = node;
    }

    node->parent = node->parent->parent;
    node->leftSon->parent = node;
    node->leftSon->rightSon = tmp;

    node->leftSon->updateHeight();
    node->updateHeight();
}

template<class Key, class T>
typename AVL<Key, T>::Iterator AVL<Key, T>::find(const Key& key) {
    return AVL<Key, T>::Iterator(this, *findAux(key));
}

template<class Key, class T>
typename AVL<Key, T>::Node** AVL<Key, T>::findAux(const Key& key) {
    Node** tmp = &root;
    if (!root) {
        return tmp;
    }
    do {
        if (!((*tmp)->key < key) && !(key < (*tmp)->key)) { // same key
            return tmp;
        }
        if (key < (*tmp)->key) {
            tmp = &(*tmp)->leftSon;
            continue;
        }
        tmp = &(*tmp)->rightSon;
    } while (*tmp);
    return tmp;
}

template<class Key, class T>
bool AVL<Key, T>::remove(const Key& key) {
    Node* node = *findAux(key);
    if (!node) {
        assert(checkInvariant());
        return false;
    }
    if (node->leftSon && node->rightSon) {
        removeNodeWithTwoSons(node);
    } else {
        removeNodeWithOneSonOrLess(node);
    }
    size--;
    assert(checkInvariant());
    return true;
}

template<class Key, class T>
void AVL<Key, T>::removeNodeWithOneSonOrLess(Node* node) {
    Node* parent = node->parent;
    Node* son = node->rightSon;
    if (!son) {
        son = node->leftSon;
    }
    if (son) {
        son->parent = parent;
    }
    if (parent && (parent->leftSon == node)) {
        parent->leftSon = son;
    } else if (parent && (parent->rightSon == node)) {
        parent->rightSon = son;
    } else {
        assert(!parent);
        root = son;
    }
    delete node;
    if (parent) {
        balance(parent);
    }
}

template<class Key, class T>
void AVL<Key, T>::removeNodeWithTwoSons(Node* node) {
    assert(node->rightSon);
    Node* tmp = node->rightSon;
    while (tmp->leftSon) {
        tmp = tmp->leftSon;
    }
    swapNodes(node, tmp);
    removeNodeWithOneSonOrLess(node);
}


template<class Key, class T>
template<class Functor>
void AVL<Key, T>::inOrder(Functor& functor) {
    inOrderAux(root, functor);
}

template<class Key, class T>
template<class Functor>
void AVL<Key, T>::inOrderAux(Node* current, Functor& functor) {
    if (!current) {
        return;
    }
    inOrderAux(current->leftSon, functor);
    functor(current->element);
    inOrderAux(current->rightSon, functor);
}

template<class Key, class T>
template<class Functor>
void AVL<Key, T>::preOrder(Functor& functor) {
    preOrderAux(root, functor);
}

template<class Key, class T>
template<class Functor>
void AVL<Key, T>::preOrderAux(Node* current, Functor& functor) {
    if (!current) {
        return;
    }
    functor(current->element);
    preOrderAux(current->leftSon, functor);
    preOrderAux(current->rightSon, functor);
}

template<class Key, class T>
template<class Functor>
void AVL<Key, T>::postOrder(Functor& functor) {
    postOrderAux(root, functor);
}

template<class Key, class T>
template<class Functor>
void AVL<Key, T>::postOrderAux(Node* current, Functor& functor) {
    if (!current) {
        return;
    }
    postOrderAux(current->leftSon, functor);
    postOrderAux(current->rightSon, functor);
    functor(current->element);
}

template<class Key, class T> AVL<Key, T>::~AVL() {
    destructorAux(root);
}

template<class Key, class T>
void AVL<Key, T>::destructorAux(Node* current) {
    if (!current) {
        return;
    }
    destructorAux(current->leftSon);
    destructorAux(current->rightSon);
    delete current;
}

template<class Key, class T>
int AVL<Key, T>::getSize() const {
    return size;
}

template<class Key, class T>
bool AVL<Key, T>::checkInvariant() const {
    if (!root) {
        assert(!size);
        return true;
    }
    return root->validateBF();
}

template<class Key, class T>
typename AVL<Key, T>::Iterator AVL<Key, T>::end() {
    return Iterator(this, NULL);
}

template<class Key, class T>
typename AVL<Key, T>::Iterator AVL<Key, T>::max() {
    if (!root) {
        return Iterator(this, NULL);
    }
    Node* tmp = root;
    while (tmp->rightSon) {
        tmp = tmp->rightSon;
    }
    return Iterator(this, tmp);
}

/* Node */
template<class Key, class T>
class AVL<Key, T>::Node {

    void updateHeight();
    bool validateBF() const;
    int calcBF() const;
    Node* copyNodes(Node* father) const;

    Node(const Key& key,
         const T& element,
         Node* parent = NULL,
         Node* leftSon = NULL,
         Node* rightSon = NULL) : key(key),
                                  element(element),
                                  parent(parent),
                                  leftSon(leftSon),
                                  rightSon(rightSon),
                                  height(0) {
        updateHeight();
    }
    Key key;
    T element;
    Node* parent;
    Node* leftSon;
    Node* rightSon;
    int height;
    friend class AVL<Key, T>;
};


template<class Key, class T>
typename AVL<Key, T>::Node* AVL<Key, T>::Node::copyNodes(Node* father) const {
    Node* newNode = new Node(key, element, father, NULL, NULL);
    if (leftSon) {
        newNode->leftSon = leftSon->copyNodes(newNode);
    }
    if (rightSon) {
        newNode->rightSon = rightSon->copyNodes(newNode);
    }
	newNode->updateHeight();
    return newNode;
}

template<class Key, class T>
void AVL<Key, T>::Node::updateHeight() {
    height = 0;
    if (leftSon) {
        height = (leftSon->height + 1);
    }
    if (rightSon && (rightSon->height >= height)) {
        height = (rightSon->height + 1);
    }
}

template<class Key, class T>
int AVL<Key, T>::Node::calcBF() const {
    int BF = -1;
    if (leftSon) {
        BF = leftSon->height;
    }
    BF++;
    if (rightSon) {
        BF -= (rightSon->height + 1);
    }
    return BF;
}

template<class Key, class T>
bool AVL<Key, T>::Node::validateBF() const {
    int BF = calcBF();
    if (!(BF < 2 && BF > -2)) {
        return false;
    }
    if ( (leftSon && !leftSon->validateBF()) ||
         (rightSon && !rightSon->validateBF()) ) {
        return false;
    }
    return true;
}

/* Iterator */
template<class Key, class T>
class AVL<Key, T>::Iterator {
public:
	//throws ELEMENT_NOT_FOUND if current==NULL
    T& operator*() const;

    bool operator==(const Iterator& iterator2) const;
    bool operator!=(const Iterator& iterator2) const;

private:
    Iterator(AVL<Key, T>* avl, Node* current) :
        avl(avl), current(current) {
    }

    AVL<Key, T>* avl;
    Node* current;

    friend class AVL<Key, T>;
};

template<class Key, class T>
T& AVL<Key, T>::Iterator::operator*() const {
    if (!this->current) {
        throw ELEMENT_NOT_FOUND();
    }
    return this->current->element;
}

template<class Key, class T>
bool AVL<Key, T>::Iterator::operator==(const AVL<Key, T>::Iterator&
                                            iterator2) const {
    return (avl == iterator2.avl && current == iterator2.current);
}

template<class Key, class T>
bool AVL<Key, T>::Iterator::operator!=(const AVL<Key, T>::Iterator&
                                            iterator2) const {
    return !(*this == iterator2);
}

#endif /* AVL_H_ */
