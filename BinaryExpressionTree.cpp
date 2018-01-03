#include "BinaryExpressionTree.h"
#include <cmath>

using namespace std;
using namespace utility;

BinaryExpressionTree::BinaryExpressionTree(vector<Token> expression) {
    vector<Node*> stack;

    for (Token t : expression) {
        if (isOperator(t.value)) {
            Node * rightOperand = stack.back();
            stack.pop_back();
            Node * leftOperand = stack.back();
            stack.pop_back();
            stack.push_back(new Node(t.type,t.value,rightOperand,leftOperand));
        }
        else {
            stack.push_back(new Node(t.type,t.value));
        }
    }
    root = stack.back();
}

void BinaryExpressionTree::destroy(Node *i) {
    if (i != nullptr) {
        destroy(i->left);
        destroy(i->right);
        delete i;
    }
}

void BinaryExpressionTree::inorder(Node *i) {
    if (i->left != nullptr) {
        inorder(i->left);
    }
    cout << i->symbol << ", ";
    if (i->right != nullptr) {
        inorder(i->right);
    }
}

void BinaryExpressionTree::postorder(Node *i) {
    if (i->left != nullptr) {
        postorder(i->left);
    }
    if (i->right != nullptr) {
        postorder(i->right);
    }
    cout << i->symbol << ", ";
}

void BinaryExpressionTree::preorder(Node *i) {
    cout << "Node: " << i->type << ": " << i->symbol << endl;
    if (i->left != nullptr && i->right != nullptr) {
        cout << "Left child: " << i->left->symbol << " | Right child: " << i->right->symbol << endl;
    }
//    cout << i->symbol << ", ";
    cout << "-------" << endl;
    if (i->left != nullptr) {
        preorder(i->left);
    }
    if (i->right != nullptr) {
        preorder(i->right);
    }
}

/*
 * WARNING: The code that follows may make you cry:
 *           A Safety Pig has been provided below for your benefit
 *                              _
 *      _._ _..._ .-',     _.._(`))
 *     '-. `     '  /-._.-'    ',/
 *       )         \            '.
 *      / _    _    |             \
 *     |  a    a    /              |
 *      \   .-.                     ;
 *       '-('' ).-'       ,'       ;
 *          '-;           |      .'
 *            \           \    /
 *            | 7  .__  _.-\   \
 *            | |  |  ``/  /`  /
 *           /,_|  |   /,_/   /
 *              /,_/      '`-'
 */
void BinaryExpressionTree::generateMachineCode(Node *i, std::vector<utility::Register> &registers, int &memoryAddress, const string &out) {
    if (i->left == nullptr && i->right == nullptr) {
//        cout << "Node is a leaf of type: " << i->type << ", and the value is: " << i->symbol << endl;
        return;
    }

    generateMachineCode(i->left, registers, memoryAddress, out);
    generateMachineCode(i->right, registers, memoryAddress, out);
    string codeLine = "";

    if (i == root) {
        codeLine += out + "=";
    } else {
        int regNumber = findFreeRegister(registers);
        if (regNumber != -1) {
            registers[regNumber].content = "expr";
            codeLine += "Reg[" + to_string(regNumber) + "]=";
            i->storage = "Reg";
            i->index = regNumber;
        } else { // there is no registers, so we use memory
            codeLine += "Mem[" + to_string(memoryAddress) + "]=";
            i->storage = "Mem";
            i->index = memoryAddress;
            memoryAddress++;
        }
    }
    i->type = "expression";
    if (i->left->type == "expression") {
        codeLine += i->left->storage + "[" + to_string(i->left->index) + "]";

        // Freeing up unnecessary register usage if left child used register
        if (i->left->storage == "Reg") {
            registers[i->left->index].content = "";
        }
    }
    else if (i->left->type == "input" || i->left->type == "literal") {
        codeLine += i->left->symbol;
    }
    codeLine += i->symbol;
    if (i->right->type == "expression") {
        codeLine += i->right->storage + "[" + to_string(i->right->index) + "]";

        // Freeing up unnecessary register usage if right child used register
        if (i->right->storage == "Reg") {
            registers[i->right->index].content = "";
        }
    }
    else if (i->right->type == "input" || i->right->type == "literal") {
        codeLine += i->right->symbol;
    }
    codeLine += ";";
    cout << codeLine << endl;
}

string BinaryExpressionTree::eval(Node *i) { // function not needed in the final version

    // leaf node, containing an expression:
    if (i->left == nullptr && i->right == nullptr) {
        cout << "Node is a leaf, and the value is: " << i->symbol << endl;
        return i->symbol;
    }

    string l_val = eval(i->left);
    i->left = nullptr; // todo: delete
    string r_val = eval(i->right);
    i->right = nullptr; // todo: delete

    cout << i->symbol << endl;
    if (i->symbol == "+") {
        i->symbol = l_val + " + " + r_val;
        i->type = "expression"; // might be useless here
    }

    if (i->symbol == "-") {
        i->symbol = l_val + " - " + r_val;
        i->type = "expression"; // might be useless here
    }

    if (i->symbol == "*") {
        i->symbol = l_val + " * " + r_val;
        i->type = "expression"; // might be useless here
    }

    if (i->symbol == "/") {
        i->symbol = l_val + " / " + r_val;
        i->type = "expression"; // might be useless here
    }
    cout << "Sub expr: " << i->symbol << endl;
    return i->symbol;
}

Node *BinaryExpressionTree::getRoot() const {
    return root;
}


void BinaryExpressionTree::printBranches(int branchLen, int nodeSpaceLen, int startLen, int nodesInThisLevel,
                                         const std::deque<Node *> &nodesQueue, std::ostream &out) {
    deque<Node*>::const_iterator iter = nodesQueue.begin();
    for (int i = 0; i < nodesInThisLevel / 2; i++) {
        out << ((i == 0) ? setw(startLen-1) : setw(nodeSpaceLen-2)) << "" << ((*iter++) ? "/" : " ");
        out << setw(2*branchLen+2) << "" << ((*iter++) ? "\\" : " ");
    }
    out << endl;
}

void BinaryExpressionTree::printNodes(int branchLen, int nodeSpaceLen, int startLen, int nodesInThisLevel,
                                      const std::deque<Node *> &nodesQueue, std::ostream &out) {
    deque<Node*>::const_iterator iter = nodesQueue.begin();
    for (int i = 0; i < nodesInThisLevel; i++, iter++) {
        out << ((i == 0) ? setw(startLen) : setw(nodeSpaceLen)) << "" << ((*iter && (*iter)->left) ? setfill('_') : setfill(' '));
        out << setw(branchLen+2) << ((*iter) ? (*iter)->symbol : "");
        out << ((*iter && (*iter)->right) ? setfill('_') : setfill(' ')) << setw(branchLen) << "" << setfill(' ');
    }
    out << endl;
}

void BinaryExpressionTree::printLeaves(int indentSpace, int level, int nodesInThisLevel,
                                       const std::deque<Node *> &nodesQueue, std::ostream &out) {
    deque<Node*>::const_iterator iter = nodesQueue.begin();
    for (int i = 0; i < nodesInThisLevel; i++, iter++) {
        out << ((i == 0) ? setw(indentSpace+2) : setw(2*level+2)) << ((*iter) ? (*iter)->symbol : "");
    }
    out << endl;
}

void BinaryExpressionTree::printPretty(Node *root, int level, int indentSpace, std::ostream &out) {
    int h = maxHeight(root);
    int nodesInThisLevel = 1;

    int branchLen = 2*((int)pow(2.0,h)-1) - (3-level)*(int)pow(2.0,h-1);  // eq of the length of branch for each node of each level
    int nodeSpaceLen = 2 + (level+1)*(int)pow(2.0,h);  // distance between left neighbor node's right arm and right neighbor node's left arm
    int startLen = branchLen + (3-level) + indentSpace;  // starting space to the first node to print of each level (for the left most node of each level only)

    deque<Node*> nodesQueue;
    nodesQueue.push_back(root);
    for (int r = 1; r < h; r++) {
        printBranches(branchLen, nodeSpaceLen, startLen, nodesInThisLevel, nodesQueue, out);
        branchLen = branchLen/2 - 1;
        nodeSpaceLen = nodeSpaceLen/2 + 1;
        startLen = branchLen + (3-level) + indentSpace;
        printNodes(branchLen, nodeSpaceLen, startLen, nodesInThisLevel, nodesQueue, out);

        for (int i = 0; i < nodesInThisLevel; i++) {
            Node *currNode = nodesQueue.front();
            nodesQueue.pop_front();
            if (currNode) {
                nodesQueue.push_back(currNode->left);
                nodesQueue.push_back(currNode->right);
            } else {
                nodesQueue.push_back(NULL);
                nodesQueue.push_back(NULL);
            }
        }
        nodesInThisLevel *= 2;
    }
    printBranches(branchLen, nodeSpaceLen, startLen, nodesInThisLevel, nodesQueue, out);
    printLeaves(indentSpace, level, nodesInThisLevel, nodesQueue, out);
}

int BinaryExpressionTree::maxHeight(Node *i) {
    if (!i) return 0;
    int leftHeight = maxHeight(i->left);
    int rightHeight = maxHeight(i->right);
    return (leftHeight > rightHeight) ? leftHeight + 1: rightHeight + 1;
}