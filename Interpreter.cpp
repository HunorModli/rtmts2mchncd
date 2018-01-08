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

    cout << endl << "Generated code:" << endl;

    vector<Node*> roots;

    for (auto t: trees) {
        roots.push_back(t.getRoot());
    }

    string codeLine;
    string currentOperator;
    int counter;
    while (hasNonLeafNode(roots)) {///
        counter = 0;///o-tol max parh muveletekig
        codeLine = "";///ures string->kodsor lesz
        currentOperator = "";
        map<string,int> operatorMap;///milyen muvelet legyen; osszes fat bejarjuk; osszes elvegezheto muveletet elvegezzuk <muvelet, hanyszor>
        if (CONCURRENT_OPERATIONS > 1) {
            // choose operation to make in simd;
            operatorMap["FMA3"] = 0; ///priorizaljuk;
            operatorMap["+"] = 0;
            operatorMap["-"] = 0;
            operatorMap["*"] = 0;
            operatorMap["/"] = 0;

            calculateOccurrances(roots, operatorMap);

            // if there is at least MAX_COMCURRENT FMA-s available, then we pick fma
            if (operatorMap["FMA3"] >= CONCURRENT_OPERATIONS) {///akkor az operator az fma 3 ha >=parhmuvszam
                currentOperator = "FMA3";
            } else {
                currentOperator = findMaxInMap(operatorMap);///maxkereses
            }
        }

        bool maxReached = false;///elertuk e a counterrel a max parh muveleteket
        for (int i = 0; i < roots.size(); ++i) {///vegigmegy az osszes kifejezesi fan (gyokerek->outpu); elvegezheto muveletek
            if (maxReached) {///ha elertuk a max parh muveletek szamat
                break;
            }
//            this->testTree();
            reduceTree(roots[i]);///egyszerusitjuk a fat
//            this->testTree();
//            return;
            // if we have fma as our target operation we make fma
            if (currentOperator == "FMA3") {
                vector<Node*> fmaNodes;///kivalasztjuk azokat a nodokat, akik fmavalidak (mutato az adott nodera)
                findFmaNodes(roots[i], fmaNodes);
                for (auto node : fmaNodes) {///vegigmegyunk rajtuk
                    if (counter == CONCURRENT_OPERATIONS) {///elertuk e a maxot?
                        maxReached = true;
                        break;///kibreakel
                    } else {
                        makeFma(node,codeLine,registers);///atadjuk a regisztereket is hogy fel tudjuk oket szabaditani
                        counter++;
                    }
                }
            } else {///ha nem fma volt a muveletunk
                vector<Node*> validNodes;///osszes valid nodeot belerakjuk, akinek mar csak levelei vannak
                findDistinctValidNodes(roots[i],validNodes);///
                for (auto node : validNodes) {///vegig a nodokon
                    if (counter == CONCURRENT_OPERATIONS) {///ha tobb a muvelet, mint pmsz
                        maxReached = true;
                        break;
                    } else {
                        if (node->symbol == currentOperator) {///adott muvelet-e(osszeadas)
                            makeOperation(node, roots, codeLine, registers, currentMemoryAddress,trees[i].getOutput());///megcsinaljuk a muveletet; pl reg2=reg1+mem0
                            counter++;
                        }
                    }
                }
            }
//            if (!maxReached && counter < CONCURRENT_OPERATIONS) { // if a tree has been reduced to a single leaf node
//                if (isLeafNode(roots[i])) {
//                    codeLine += trees[i].getOutput() + "=";
//                    if (trees[i].getRoot()->storage == "Reg" || trees[i].getRoot()->storage == "Mem") {
//                        codeLine += trees[i].getRoot()->storage + "[" + to_string(trees[i].getRoot()->index) + "];";
//                    } else {
//                        codeLine += trees[i].getRoot()->symbol + ";";
//                    }
//                    counter++;
//                }
//            }
        }
        cout << codeLine << endl;
    }
}

void Interpreter::freeUpMemory() {
    for (auto t : trees) {
        t.destroy(t.getRoot());
    }
}
