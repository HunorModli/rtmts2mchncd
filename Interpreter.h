#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <map>
#include "myUtility.h"
#include "BinaryExpressionTree.h"

class Interpreter {
private:
    int REGISTER_COUNT;
    int CONCURRENT_OPERATIONS;
    std::vector<std::string> rawCommands;
    std::vector<std::vector<utility::Token>> tokeinzedCommands;
    std::vector<std::vector<utility::Token>> rpnCommands;
    std::vector<std::string> outputVariables; // temporary store output expressions in a vector, later might be refactored into a map, or pairs
    std::vector<utility::Register> registers;
    int currentMemoryAddress;
    std::map<char, int> operators{
            {'+', 2},
            {'-', 2},
            {'*', 3},
            {'/', 3},
    };
    std::vector<BinaryExpressionTree> trees;

public:

    Interpreter(std::string const &filename);

    void tokenizeRawCommands();

    void commandToRpn();

    void buildExpressionTrees();

    void testTree();

    void evalTree();

    void generateCode();

    void generateCodes();

    bool hasNonLeafNode(std::vector<Node*> v);

    void freeUpMemory();
};

#endif //INTERPRETER_H
