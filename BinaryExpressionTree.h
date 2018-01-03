#ifndef BINARYEXPRESSIONTREE_H
#define BINARYEXPRESSIONTREE_H

#include <fstream>
#include <iostream>
#include <deque>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include "myUtility.h"

class Node {
public:
    std::string type; // operator or expression
    std::string symbol;
    std::string storage; // Reg or Mem
    int index;        // index of storage
    Node *left, *right;
    Node(std::string &type, std::string &symbol) :
            left(nullptr),
            right(nullptr),
            type(type),
            symbol(symbol),
            storage("unknown"),
            index(-1){}
    Node(std::string &type, std::string &symbol, Node *right, Node *left) :
            left(left),
            right(right),
            type(type),
            symbol(symbol),
            storage("unknown"),
            index(-1){}
};

class BinaryExpressionTree {
    Node* root;

public:
    BinaryExpressionTree(std::vector<utility::Token>);

//    ~BinaryExpressionTree();

    void inorder(Node *i);

    void preorder(Node *i);

    void postorder(Node *i);

    Node *getRoot() const;

    void destroy(Node *i);

    std::string eval(Node* i);

    void generateMachineCode(Node* i, std::vector<utility::Register> &registers, int &m, const std::string &out);
//    void _destroy(Node* x);

    int maxHeight(Node *i);

    void printBranches(int branchLen, int nodeSpaceLen, int startLen, int nodesInThisLevel, const std::deque<Node*>& nodesQueue, std::ostream& out);

    void printNodes(int branchLen, int nodeSpaceLen, int startLen, int nodesInThisLevel, const std::deque<Node*>& nodesQueue, std::ostream& out);

    void printLeaves(int indentSpace, int level, int nodesInThisLevel, const std::deque<Node*>& nodesQueue, std::ostream& out);

    void printPretty(Node *root, int level, int indentSpace, std::ostream& out);
};


#endif //BINARYEXPRESSIONTREE_H
