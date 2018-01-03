#include <iostream>
#include "Interpreter.h"

int main() {
    Interpreter i = Interpreter("input.txt");
    i.tokenizeRawCommands();
    i.commandToRpn();
    i.buildExpressionTrees();
    i.generateCode();

    // These can be commented out:
    // i.evalTree(); // evaluates the expressions in a postorder manner
    i.testTree(); // pretty prints the expression trees

    // DO NOT COMMENT OUT THIS LINE!
    i.freeUpMemory();
}