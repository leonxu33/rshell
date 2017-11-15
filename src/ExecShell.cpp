#include "../header/ExecShell.h"
#include "../header/Bin.h"
#include "../header/And.h"
#include "../header/Or.h"
#include "../header/Semicolon.h"
#include <string>
#include <vector>
#include <stack>
#include <cstring>
#include <iostream>

using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::stack;

ExecShell::ExecShell() {
    shellTree = 0;
}
ExecShell::~ExecShell() {
    if (shellTree != 0) {
        delete shellTree;
        shellTree = 0;
    }
}

void ExecShell::execute(string userInput) {
    this->parseLine(userInput);
    
    if (shellTree != 0) {
        shellTree->execute();
        delete shellTree;
        shellTree = 0;
    }
}


void ExecShell::parseLine(string userInput) {
    if (userInput == "") {  // empty input, dont do anything
        return;
    }
    
    // Check number of parentheses, quotes and brackets
    if(!paranthesesCheck(userInput)) {
        cout << "Error: Unbalanced number of parantheses." << endl;
        return;
    }
    else if (!bracketCheck(userInput)) {
        cout << "Error: Unbalanced number of brackets." << endl;
        return;
    }
    else if (!quoteCheck(userInput)) {
        cout << "Error: Unbalanced number of quotes." << endl;
        return;
    }
    
    vector<string> vToken;
    string tok = "";
    bool isQuote = false;
    for (unsigned i = 0; i < userInput.size(); i++) {
        if (userInput.at(i) == '#' && !isQuote) {
            //cout << "-------------1------------" << endl;
            break;
        }
        else if (userInput.at(i) == ' ' && !isQuote) {
            //cout << "-------------2------------" << endl;
            
        }
        else if (userInput.at(i) == '\"') {
            //cout << "-------------3------------" << endl;
            string temp;
            temp.push_back('\"');
            vToken.push_back(temp);
            if (isQuote) {
                isQuote = false;
            }
            else {
                isQuote = true;
            }
        }
        else if (isCharOperator(userInput.at(i)) && !isQuote) {
            //cout << "-------------4------------" << endl;
            if (userInput.at(i) == '&') {
                if (i+1 < userInput.size()) {
                    if (userInput.at(i+1) == '&') {
                        vToken.push_back("&&");
                        i++;
                    }
                }
            }
            else if (userInput.at(i) == '|') {
                if (i+1 < userInput.size()) {
                    if (userInput.at(i+1) == '|') {
                        vToken.push_back("||");
                        i++;
                    }
                }
            }
            else {
                string temp;
                temp.push_back(userInput.at(i));
                vToken.push_back(temp);
            }
        }
        else if (isQuote) {
            //cout << "-------------5------------" << endl;
            tok.push_back(userInput.at(i));
            if (i+1 < userInput.size() && userInput.at(i+1) == '\"') {
                vToken.push_back(tok);
                tok = "";
            }
        }
        else {
            //cout << "-------------6------------" << endl;
            tok.push_back(userInput.at(i));
            if (i+1 == userInput.size()) {
                vToken.push_back(tok);
            }
            else {
                if (i+1 < userInput.size()) {
                    if (isCharOperator(userInput.at(i+1))) {
                        vToken.push_back(tok);
                        tok = "";
                    }
                }
            }
        }
    }
    
    // test
    // for(unsigned i = 0; i < vToken.size(); ++i) {
    //     cout << vToken.at(i) << " " << endl;
    // }
    // cout << "-----------------------------------" << endl;
    
    // Insert shell command from vector to queue
    vector<vector<string> > shellVec;
    vector<string> temp;
    bool is_quote = false;
    for (unsigned i = 0; i < vToken.size(); i++) {
        if (vToken.at(i) == "\"") {
            if (is_quote) {
                is_quote = false;
                shellVec.push_back(temp);
            }
            else {
                is_quote = true;
            }
        }
        else if (is_quote) {
            temp.push_back(vToken.at(i));
        }
        else if (!is_quote) {
            if (!isOperator(vToken.at(i))) {
                temp.push_back(vToken.at(i));
                if (i+1 < vToken.size()) {
                    if (isOperator(vToken.at(i+1))) {
                        shellVec.push_back(temp);
                    }
                }
                else {
                    shellVec.push_back(temp);
                }
            }
            else {
                temp.clear();
                temp.push_back(vToken.at(i));
                shellVec.push_back(temp);
                temp.clear();
            }
        }
    }
    if (shellVec.back().at(0) == ";") {
        shellVec.pop_back();
    }
    
    //test
    // for (unsigned i = 0; i < shellVec.size(); i++) {
    //     for (unsigned j = 0; j < shellVec.at(i).size(); j++) {
    //         cout << shellVec.at(i).at(j) << " ";
    //     }
    //     cout << endl;
    // }
    // cout << "----------------------------------- " << endl;
    
    shellVec = infixToPostfix(shellVec);
    
    //test
    // for (unsigned i = 0; i < shellVec.size(); i++) {
    //     for (unsigned j = 0; j < shellVec.at(i).size(); j++) {
    //         cout << shellVec.at(i).at(j) << " ";
    //     }
    //     cout << endl;
    // }
    // cout << "----------------------------------- " << endl;
    
    buildTree(shellVec);
}

vector<vector<string> > ExecShell::infixToPostfix(vector<vector<string> > shellVec) {
    stack<vector<string> > cntStack;
    vector<vector<string> > postfix;
    for (unsigned i = 0; i < shellVec.size(); i++) {
        if (!isOperator(shellVec.at(i).front())) {
            postfix.push_back(shellVec.at(i));
        }
        else if (shellVec.at(i).front() == "(") {
            cntStack.push(shellVec.at(i));
        }
        else if (shellVec.at(i).front() == ")") {
            vector<string> topVal = cntStack.top();
            cntStack.pop();
            while (topVal.front() != "(") {
                postfix.push_back(topVal);
                topVal = cntStack.top();
                cntStack.pop();
            }
        }
        else {
            while (!cntStack.empty() && (getPrecedence(cntStack.top()) >= getPrecedence(shellVec.at(i)))) {
                postfix.push_back(cntStack.top());
                cntStack.pop();
            }
            cntStack.push(shellVec.at(i));
        }
    }
    while (!cntStack.empty()) {
        postfix.push_back(cntStack.top());
        cntStack.pop();
    }
    
    return postfix;
    
}

void ExecShell::buildTree(std::vector<std::vector<std::string> > shellList) {
    stack<ShellComponent*> shellStack;
    for (unsigned i = 0; i < shellList.size(); i++) {
        if (!isOperator(shellList.at(i).front())) {
            shellStack.push(new Bin(shellList.at(i)));
        }
        else {
            Connector* op = 0;
            if (shellList.at(i).front() == "&&") {
                op = new And();
            }
            else if (shellList.at(i).front() == "||") {
                op = new Or();
            }
            else if (shellList.at(i).front() == ";") {
                op = new Semicolon();
            }
            if (!shellStack.empty()) {
                op->setRight(shellStack.top());
                shellStack.pop();
            }
            if (!shellStack.empty()) {
                op->setLeft(shellStack.top());
                shellStack.pop();
            }
            shellStack.push(op);
        }
    }
    if (!shellStack.empty()) {
        shellTree = shellStack.top();
        shellStack.pop();
    }
}

bool ExecShell::quoteCheck(string test) {
    int numQuotes = 0;
    for(unsigned i = 0; i < test.size(); ++i) {
        if(test.at(i) == '\"') {
            ++numQuotes;
        }
    }
    if(numQuotes % 2 != 0) {
        return false;
    }
    else {
        return true;
    }
}

bool ExecShell::paranthesesCheck(string test) {
    int numFrontP = 0;
    int numBackP = 0;
    for(unsigned i = 0; i < test.size(); ++i) {
        if(test.at(i) == '(') {
            ++numFrontP;
        }
        else if(test.at(i) == ')') {
            ++numBackP;
        }
    }
    if(numFrontP == numBackP) {
        return true;
    }
    else {
        return false;
    }
}

bool ExecShell::bracketCheck(string test) {
    int numFrontB = 0;
    int numBackB = 0;
    for(unsigned i = 0; i < test.size(); ++i) {
        if(test.at(i) == '[') {
            ++numFrontB;
        }
        else if(test.at(i) == ']') {
            ++numBackB;
        }
    }
    if(numFrontB == numBackB) {
        return true;
    }
    else {
        return false;
    }
}

int ExecShell::getPrecedence(vector<string> str) {
    if (str.empty()) {
        return 0;
    }
    if (str.front() == "(") {
        return 1;
    }
    else if (str.front() == "&&" || str.front() == "||" || str.front() == ";") {
        return 2;
    }
    else {
        return 0;
    }
}

bool ExecShell::isOperator(string str) {
    if (str == "||" || str == "&&" || str == ";" || str == "(" || str == ")") {
        return true;
    }
    else {
        return false;
    }
}

bool ExecShell::isCharOperator(char c) {
    if (c == '\"' || c == ' ' || c == '&' || c == '|' || c == ';' || c == '(' || c == ')' || c == '[' || c == ']') {
        return true;
    }
    else {
        return false;
    }
}