#include "BinaryExpressionTree.h"
#include <cmath>
#include <map>
#include "algorithm"

using namespace std;
using namespace utility;

BinaryExpressionTree::BinaryExpressionTree(vector<Token> expression, string out) {

    this->output = out;

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
//
//bool isLeafNode(Node *i) {
//    return (i->left == nullptr && i->right == nullptr);
//}

bool hasNonLeafNode(vector<Node*> v) {
    bool onlyLeaf = true;
    for (auto n : v) {
        if (!isLeafNode(n)) onlyLeaf = false; break;
    }
    return onlyLeaf;
}

bool isReducibleNode(Node * n) {
    // if there is the same literal, or input variable in
    // both sides of a subtraction we can write a 0 literal.
    if (n->type == "operator" && n->symbol == "-") {
        if (n->left->type == n->right->type &&
            n->left->symbol == n->right->symbol) {
            return true;
        }
    }
    // if there is a 0 literal in either side of a
    // multiplication we can write a 0 literal
    if (n->type == "operator" && n->symbol == "*") {
        if ((n->left->type == "literal" && n->left->symbol == "0") ||
            (n->right->type == "literal" && n->right->symbol == "0")) {
            return true;
        }
    }
    return false;
}

void reduceTree(Node* i) {
    if (i->left != nullptr) {
        reduceTree(i->left);
    }
    if (i->right != nullptr) {
        reduceTree(i->right);
    }
    if (isReducibleNode(i)) {
        i->type = "literal";
        i->symbol = "0";
        delete(i->left);
        delete(i->right);
        i->left = nullptr;
        i->right = nullptr;
    }
}

bool isFmaNode(Node *i) {
    if (i->type == "operator" && i->symbol == "+") {
        // if left is a register and right is a multiplication:
        if (i->left->storage == "Reg" && i->right->type == "operator" && i->right->symbol == "*") {
            return true;
        }

        // if right is a register and left is a multiplication:
        if (i->right->storage == "Reg" && i->left->type == "operator" && i->left->symbol == "*") {
            return true;
        }
    }
    return false;
}

void findFmaNodes(Node* i, vector<Node*> &nodes) {
    if (i->left != nullptr) {
        findFmaNodes(i->left, nodes);
    }
    if (i->right != nullptr) {
        findFmaNodes(i->right, nodes);
    }
    if (isFmaNode(i)) {
        nodes.push_back(i);
    }
}

void makeOperation(Node* i, vector<Node*> roots, string &codeLine, vector<Register> & registers, int & memoryAddress, const string &output) {
    cout << "as" << endl;
    if (find(roots.begin(), roots.end(), i) != roots.end()) {
        codeLine += output + "=";
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
    if (isLeafNode(i)) {
        cout << "KEKE" << endl;
        if (i->storage == "Reg" || i->storage == "Mem") {
            codeLine += i->storage + "[" + to_string(i->index) + "];";
        } else {
            codeLine += i->symbol + ";";
        }
    } else {
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

        delete(i->left);
        i->left = nullptr;
        delete(i->right);
        i->right = nullptr;
    }
//    cout << codeLine << endl;
}

void makeFma(Node* i, string & codeLine, vector<Register> &registers) {

    // if left is a register, right is a multiplication
    if (i->left->storage == "Reg" && i->right->type == "operator" && i->right->symbol == "*") {
        codeLine += "Reg[" + to_string(i->left->index) + "] += ";

        // if the left operand of the * is stored in Reg or Mem
        if (i->right->left->storage == "Mem" || i->right->left->storage == "Reg") {
            codeLine += i->right->left->storage + "[" + to_string(i->right->left->index) + "]*";
        } else {
            // else it must be literal or input variable
            codeLine += i->right->left->symbol + "*";
        }

        // if the right operand of the * is stored in Reg or Mem
        if (i->right->right->storage == "Mem" || i->right->right->storage == "Reg") {
            codeLine += i->right->right->storage + "[" + to_string(i->right->right->index) + "];";
        } else {
            // else it must be literal or input variable
            codeLine += i->right->right->symbol + ";";
        }

        i->symbol = "expr";
        i->storage = "Reg";
        i->index = i->left->index;

        // free up registers
        if (i->right->left->storage == "Reg") {
            registers[i->right->left->index].content = "";
        }
        if (i->right->right->storage == "Reg") {
            registers[i->right->right->index].content = "";
        }

        // free up memory
        delete(i->left);
        i->left = nullptr;
        delete(i->right->left);
        delete(i->right->right);
        delete(i->right);
        i->right = nullptr;
    }


    // if right is a register, left is a multiplication
    if (i->right->storage == "Reg" && i->left->type == "operator" && i->left->symbol == "*") {
        codeLine += "Reg[" + to_string(i->right->index) + "] += ";

        if (i->left->left->storage == "Mem" || i->left->left->storage == "Reg") {
            codeLine += i->left->left->storage + "[" + to_string(i->left->left->index) + "]*";
        } else {
            codeLine += i->left->left->symbol + "*";
        }

        if (i->left->right->storage == "Mem" || i->left->right->storage == "Reg") {
            codeLine += i->left->right->storage + "[" + to_string(i->left->right->index) + "];";
        } else {
            codeLine += i->left->right->symbol + ";";
        }

        i->symbol = "expr";
        i->storage = "Reg";
        i->index = i->right->index;

        // free up registers
        if (i->left->left->storage == "Reg") {
            registers[i->left->left->index].content = "";
        }
        if (i->left->right->storage == "Reg") {
            registers[i->left->right->index].content = "";
        }

        delete(i->right);
        i->right = nullptr;
        delete(i->left->left);
        delete(i->left->right);
        delete(i->left);
        i->left = nullptr;
    }
}

bool isLeafNode(Node * i) {
    return (i->left == nullptr && i->right == nullptr);
}

bool isValidNode(Node* i) {

    if (i->left == nullptr || i->right == nullptr) {
        return false;
    }
    // Fma nodes are evaluated beforehand, therefore when we encounter a valid
    // fma-able node we need not to include it, or any of its subtrees.
    if (isFmaNode(i)) {
        return false;
    }

    if (isLeafNode(i->left) && isLeafNode(i->right)) {
        return true;
    }
}

void findDistinctValidNodes(Node* i, vector<Node*> &validNodes) {
    if (i->left != nullptr) {
        findDistinctValidNodes(i->left, validNodes);
    }
    if (i->right != nullptr) {
        findDistinctValidNodes(i->right, validNodes);
    }
    if (isValidNode(i)) {
        validNodes.push_back(i);
    }
}

void calculateOccurrances(vector<Node*> roots, map<string,int> &operatorMap) {
    for (auto r : roots) {
        vector<Node*> valids;
        vector<Node*> fmas;
        findDistinctValidNodes(r,valids);
        findFmaNodes(r,fmas);
        for (auto v : valids) {
            operatorMap[v->symbol]++;
        }
        operatorMap["FMA3"] += fmas.size();
    }
}

void BinaryExpressionTree::generateMachineCode(
        vector<Node *> roots,
        std::vector<utility::Register> &registers,
        int &memoryAddress,
        const string &out,
        const int &MAX_CONCURRENT) {

    string codeLine;
    string currentOperator;
    int counter;
    while (hasNonLeafNode(roots)) {
        counter = 0;
        codeLine = "";
        currentOperator = "";
        if (MAX_CONCURRENT > 1) {
            // choose operation to make in simd;
            map<string,int> operatorMap;
            operatorMap["+"] = 0;
            operatorMap["-"] = 0;
            operatorMap["*"] = 0;
            operatorMap["/"] = 0;
            operatorMap["FMA3"] = 0;

            calculateOccurrances(roots, operatorMap);

            // if there is at least MAX_COMCURRENT FMA-s available, then we pick fma
            if (operatorMap["FMA3"] >= MAX_CONCURRENT) {
                currentOperator = "FMA3";
            } else {
                currentOperator = std::max_element(operatorMap.begin(), operatorMap.end())->second;
            }
        }
        while (counter < MAX_CONCURRENT) {
            for (auto tree : roots) {
                reduceTree(tree);
                vector<Node*> fmaNodes;
                vector<Node*> validNodes;
                findFmaNodes(tree, fmaNodes);
                findDistinctValidNodes(tree,validNodes);
                for (auto node : fmaNodes) {
                    if (counter == MAX_CONCURRENT) {
                        break;
                    } else {
                        makeFma(node,codeLine,registers);
                        counter++;
                    }
                }
            }
        }
    }
}

void BinaryExpressionTree::generateMachineCode(Node *i, std::vector<utility::Register> &registers, int &memoryAddress, const string &out) {
    if (i->left == nullptr && i->right == nullptr) {
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

std::string BinaryExpressionTree::getOutput() const {
    return output;
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

