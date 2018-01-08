#include "myUtility.h"
#include <algorithm>
#include <map>

using namespace std;

namespace utility {

    bool isOperator(string s) {
        if (s.size() != 1) {
            return false;
        } else {
            string op = "+-*/";
            size_t found = op.find(s);
            if (found == string::npos) {
                return false;
            } else {
                return true;
            }
        }
    }

    bool isOnlyLowerCase(string s) {
        bool onlyLower = true;
        for (int i = 0; i < s.length(); ++i) {
            if (!islower(s[i])) {
                onlyLower = false;
                break;
            }
        }
        return onlyLower;
    }

    bool isValidLiteral(string s) {
        string::const_iterator it = s.begin();
        int minSize = 0;
        if(s.size()>0 && (s[0] == '-' || s[0] == '+')) {
            it++; minSize++;
        }
        while (it != s.end() && std::isdigit(*it)) {
            it++;
        }
        return s.size()>minSize && it == s.end();
    }

    bool isLeftParenthesis(string s) {
        return s == "(";
    }

    bool isRightParenthesis(string s) {
        return s == ")";
    }

    vector<string> split(string str, const char &separator) {
        vector<string> results;
        size_t found;
        found = str.find_first_of(separator);
        while(found != string::npos){
            if(found > 0){
                results.push_back(str.substr(0,found));
            }
            str = str.substr(found+1);
            found = str.find_first_of(separator);
        }
        if(str.length() > 0){
            results.push_back(str);
        }
        return results;
    }

    vector<string> split(string str, const string &separators) {
        vector<string> results;
        size_t pos;
        size_t prev = 0;
        while ((pos = str.find_first_of(separators, prev)) != string::npos) {
            if (pos > prev) {
                results.push_back(str.substr(prev, pos-prev));
            }
            results.push_back(str.substr(pos,1));
            prev = pos+1;
        }
        if (prev < str.length()) {
            results.push_back(str.substr(prev, string::npos));
        }
        return results;
    }

    ostream& operator<<(ostream& os, const vector<string>& v) {
        os << "[";
        for (int i = 0; i < v.size(); ++i) {
            os << v[i];
            if (i != v.size() - 1) {
                os << ", ";
            }
        }
        os << "]\n";
        return os;
    }

    ostream& operator<<(ostream& os, const vector<Token>& v) {
        os << "[";
        for (int i = 0; i < v.size(); ++i) {
            os << v[i].value;
            if (i != v.size() - 1) {
                os << ", ";
            }
        }
        os << "]\n";
        return os;
    }

    int findFreeRegister(vector<Register> &r) {
        for (Register reg : r) {
            if (reg.content == "") {
                return reg.number;
            }
        }
        return -1;
    }

    string findMaxInMap(map<string,int> m) {
        int max = 0;
        string s = "undefined";
        for (auto i : m) {
            if (i.second > max) {
                max = i.second;
                s = i.first;
            }
        }
        return s;
    }
}
