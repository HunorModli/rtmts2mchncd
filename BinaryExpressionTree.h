#ifndef BINARYEXPRESSIONTREE_H
#define BINARYEXPRESSIONTREE_H

#include <fstream>
#include <iostream>
#include <deque>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <map>
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
    Node(std::string &_type, std::string &_symbol, Node *_right, Node *_left) :
            left(_left),
            right(_right),
            type(_type),
            symbol(_symbol),
            storage("unknown"),
            index(-1){}
};

class BinaryExpressionTree {

    Node *root;

    std::string output;

public:
    BinaryExpressionTree(std::vector<utility::Token>, std::string out);

//    ~BinaryExpressionTree();

    void inorder(Node *i);

    void preorder(Node *i);

    void postorder(Node *i);

    Node *getRoot() const;

    std::string getOutput() const;

    void destroy(Node *i);

    std::string eval(Node *i);

//    void generateMachineCode(Node *i, std::vector<utility::Register> &registers, int &m, const std::string &out);
//
//    void generateMachineCode(std::vector<Node *> roots, std::vector<utility::Register> &registers, int &m,
//                             const std::string &out, const int &MAX_CONCURRENT);

    int maxHeight(Node *i);

    void printBranches(int branchLen, int nodeSpaceLen, int startLen, int nodesInThisLevel,
                       const std::deque<Node *> &nodesQueue, std::ostream &out);

    void printNodes(int branchLen, int nodeSpaceLen, int startLen, int nodesInThisLevel,
                    const std::deque<Node *> &nodesQueue, std::ostream &out);

    void printLeaves(int indentSpace, int level, int nodesInThisLevel, const std::deque<Node *> &nodesQueue,
                     std::ostream &out);

    void printPretty(Node *root, int level, int indentSpace, std::ostream &out);

};

void reduceTree(Node *i);

bool isReducibleNode(Node *n);

bool hasNonLeafNode(std::vector<Node *> v);

void makeFma(Node *i, std::string &codeLine, std::vector<utility::Register> &registers, std::vector<int> &registerIndexesInUse);

void makeOperation(Node* i, std::vector<Node*> roots, std::string &codeLine, std::vector<utility::Register> & registers, int & memoryAddress, const std::string &output, std::vector<int> &registerIndexesInUse);

void calculateOccurrances(std::vector<Node *> roots, std::map<std::string, int> &operatorMap);

void findDistinctValidNodes(Node *i, std::vector<Node *> &validNodes);

void findFmaNodes(Node *i, std::vector<Node *> &nodes);

bool isFmaNode(Node *i);

bool isValidNode(Node *i);

bool isLeafNode(Node *i);


#endif //BINARYEXPRESSIONTREE_H
