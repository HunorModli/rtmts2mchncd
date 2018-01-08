#ifndef MYUTILITY_H
#define MYUTILITY_H

#include <string>
#include <vector>
#include <ostream>
#include <map>

namespace utility {

    struct Token {
        std::string type; // operator expression input literal
        std::string value;
        Token(const std::string &type, const std::string &value) : type(type), value(value) {}
    };

    struct Register {
        int number;
        std::string content;
        Register(const int number) : number(number), content("") {}
    };

    bool isOperator(std::string s);

    bool isOnlyLowerCase(std::string s);

    bool isValidLiteral(std::string s);

    bool isLeftParenthesis(std::string s);

    bool isRightParenthesis(std::string s);

    int findFreeRegister(std::vector<Register> &r);

    std::vector<std::string> split(std::string str, const char &separator);

    std::vector<std::string> split(std::string str, const std::string &separators);

    std::ostream& operator<<(std::ostream& os, const std::vector<std::string>& v);

    std::ostream& operator<<(std::ostream& os, const std::vector<Token>& v);

    std::string findMaxInMap(std::map<std::string,int> m);
}

#endif //MYUTILITY_H
