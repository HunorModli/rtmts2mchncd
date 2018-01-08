#include <algorithm>
#include "Interpreter.h"
#include "BinaryExpressionTree.h"

using namespace utility;
using namespace std;

Interpreter::Interpreter(std::string const &filename) {
    cout << filename << endl;
    ifstream inputFile(filename);
    if (!inputFile.good()) {
        cerr << "Missing file, or error with reading file. Exiting program";
        return;
    }
    string line;
    getline(inputFile, line);
    vector<string> firstLine = split(line,' ');
    if (firstLine.size() == 1) {
        REGISTER_COUNT = std::stoi(firstLine[0]);
        CONCURRENT_OPERATIONS = std::stoi(firstLine[0]);
    } else if (firstLine.size() == 2) {
        REGISTER_COUNT = std::stoi(firstLine[0]);
        CONCURRENT_OPERATIONS = std::stoi(firstLine[1]);
    } else {
        cerr << "Malformed input text. Exiting program";
        return;
    }

    for (int i = 0; i < REGISTER_COUNT; ++i) {
        Register r(i);
        registers.push_back(r);
    }
    currentMemoryAddress = 0;
    while (inputFile >> line) {
        rawCommands.push_back(line);
    }
    cout << endl << "Constuctor set:" << endl << "REG_C to " << REGISTER_COUNT << endl << "CONC_OP to " << CONCURRENT_OPERATIONS << endl;
}

void Interpreter::tokenizeRawCommands() {
    //        cout << "--- TOKENIZER ---" << endl;
    for (string command : this->rawCommands) {
        vector<Token> tokenVector;
//            cout << command << endl;
        vector<string> s = split(command,"=+-*/;()");
//            cout << "Tokenized into: " << endl;
        for (string t : s) {
//                cout << t << endl;
            if (isOperator(t) || isLeftParenthesis(t) || isRightParenthesis(t)) {
                Token token("operator", t);
                tokenVector.push_back(token);
            } else {
//                    Token token("expression", t);
                if (isOnlyLowerCase(t)) {
                    Token token("input", t);
                    tokenVector.push_back(token);
                } else if (isValidLiteral(t)) { // todo: if variable contains digits then the machine code will exit with runtime error!
                    Token token("literal", t);
                    tokenVector.push_back(token);
                } else {
                    Token token("expression", t);
                    tokenVector.push_back(token);
                }
            }
        }
        this->tokeinzedCommands.push_back(tokenVector);
    }
}


void Interpreter::commandToRpn() {
    vector<Token> opStack;
    vector<Token> output;
    for (vector<Token> command : tokeinzedCommands) {
        opStack.clear(); // just in case, might be useless here - the opStack should be empty after each iteration
        output.clear();
        cout << endl << "EXPRESSION:" << endl << command;
        command.pop_back(); // getting rid of the terminating ; symbol
        reverse(command.begin(), command.end()); // reversing the command, to emulate stack-like behaviour
        outputVariables.push_back(command.back().value);
        command.pop_back(); // getting rid of the output variable
        command.pop_back(); // and the equation operator
        while (!command.empty()) {
            Token t = command.back();
            command.pop_back();
            if (t.type == "input" || t.type == "literal") {
                output.push_back(t);
            }
            else if (isOperator(t.value)) {
                if (!opStack.empty()) {
                    while (isOperator(opStack.back().value)) {
                        // tmp.value[0] beacuse the key of the map is of char type
                        if (operators[t.value[0]] <= operators[opStack.back().value[0]]) {
                            output.push_back(opStack.back());
                            opStack.pop_back();
                        } else {
                            break;
                        }
                        if (opStack.empty()) {
                            break;
                        }
                    }
                }
                opStack.push_back(t);
            }
            else if (isLeftParenthesis(t.value)) {
                opStack.push_back(t);
            }
            else if (isRightParenthesis(t.value)) {
                while (opStack.back().value != "(") {
                    output.push_back(opStack.back());
                    opStack.pop_back();
                }
                opStack.pop_back();
            }
        }

        while (!opStack.empty()) {
            output.push_back(opStack.back());
            opStack.pop_back();
        }
        cout << "POSTFIX: " << endl << output;
        this->rpnCommands.push_back(output);
    }
}
void Interpreter::buildExpressionTrees() {
    for (int i = 0; i < rpnCommands.size(); ++i) {
        BinaryExpressionTree tree(rpnCommands[i], outputVariables[i]);
        trees.push_back(tree);
    }
}

void Interpreter::testTree() {
//        cout << endl << "___PREORDER TRAVERSAL OF THE OPERATOR TREE___" << endl;
//        trees.back().preorder(trees.back().getRoot());
    for (auto b : trees) {
        b.printPretty(b.getRoot(),1,0,cout);
    }
}

void Interpreter::evalTree() {
    cout << "___EVAL___" << endl;
    BinaryExpressionTree tree = trees.back();
    tree.eval(tree.getRoot());
}

void Interpreter::generateCode() {
    cout << endl << "Generated machine code:" << endl;

    for (int i = 0; i < trees.size(); ++i) {
        BinaryExpressionTree tree = trees[i];
        trees[i].generateMachineCode(trees[i].getRoot(), registers, currentMemoryAddress, outputVariables[i]);
    }
}

bool Interpreter::hasNonLeafNode(vector<Node*> v) {
    bool onlyLeaf = true;
    for (auto n : v) {
        if (isLeafNode(n)) onlyLeaf = false;
        break;
    }
    return onlyLeaf;
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
void Interpreter::generateCodes(){
    vector<Node*> roots;

    for (auto t: trees) {
        roots.push_back(t.getRoot());
    }

    string codeLine;
    string currentOperator;
    int counter;
    while (hasNonLeafNode(roots)) {
        counter = 0;
        codeLine = "";
        currentOperator = "";
        map<string,int> operatorMap;
        if (CONCURRENT_OPERATIONS > 1) {
            // choose operation to make in simd;
            operatorMap["FMA3"] = 0;
            operatorMap["+"] = 0;
            operatorMap["-"] = 0;
            operatorMap["*"] = 0;
            operatorMap["/"] = 0;

            calculateOccurrances(roots, operatorMap);

            // if there is at least MAX_COMCURRENT FMA-s available, then we pick fma
            if (operatorMap["FMA3"] >= CONCURRENT_OPERATIONS) {
                currentOperator = "FMA3";
            } else {
                currentOperator = findMaxInMap(operatorMap);
            }
        }

        bool maxReached = false;
        for (int i = 0; i < roots.size(); ++i) {
            if (maxReached) {
                break;
            }
//            reduceTree(tree); // todo: nem működik még
//            this->testTree();
            // if we have fma as our target operation we make fma
            if (currentOperator == "FMA3") {
                vector<Node*> fmaNodes;
                findFmaNodes(roots[i], fmaNodes);
                for (auto node : fmaNodes) {
                    if (counter == CONCURRENT_OPERATIONS) {
                        maxReached = true;
                        break;
                    } else {
                        makeFma(node,codeLine,registers);
                        counter++;
                    }
                }
            } else {
                vector<Node*> validNodes;
                findDistinctValidNodes(roots[i],validNodes);
                for (auto node : validNodes) {
                    if (counter == CONCURRENT_OPERATIONS) {
                        maxReached = true;
                        break;
                    } else {
                        if (node->symbol == currentOperator) {
                            makeOperation(node, roots, codeLine, registers, currentMemoryAddress,trees[i].getOutput());
                            counter++;
                        }
                    }
                }
            }
        }
        cout << codeLine << endl;
    }
}























void Interpreter::freeUpMemory() {
    for (auto t : trees) {
        t.destroy(t.getRoot());
    }
}

void asd () {

    vector<int> v;
    v.push_back(1);
    v.push_back(5);
    v.push_back(4);
    v.push_back(3);

    for (int j = 0; j < v.size(); ++j) {
        cout << v[j] << endl;
    }

    for (auto i : v) {
        cout << i << endl;
    }
}
