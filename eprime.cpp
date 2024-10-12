#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <cstring>
#include <sstream>
// Lexer
enum Token {
    tok_eof = -1,
    //keywords
    tok_def = -2,
    tok_func = -3,
    tok_prec = -4,
    tok_derivative = -5,

    //primary
    tok_identifier = -6,

    //types
    tok_int = -7,
    tok_string = -8,
    tok_dec = -9
};
std::vector<std::string> split(const std::string& input, const char delim) {
    std::vector<std::string> result;
    std::stringstream ss(input);
    std::string item;
    
    while (std::getline(ss, item, delim)) {
        result.push_back(item);
    }
    
    return result;
}
static std::string IdentifierStr;
static long IntVal;
static long DecVal[2]; //to avoid errors like 0.1 + 0.1 + 0.1 = 0.30000000000000004 
static int getToken(){ //get the next token
    static int LastChar = ' ';
    while (isspace(LastChar))
        LastChar = getchar();
    if (isalpha(LastChar)){
        IdentifierStr = LastChar;
        while(isalnum(LastChar = getchar()))
            IdentifierStr += LastChar;
        if (IdentifierStr == "def")
            return tok_def;
        if (IdentifierStr == "func")
            return tok_func;
        if (IdentifierStr == "prec")
            return tok_prec;
        if (IdentifierStr == "derivative")
            return tok_derivative;
        return tok_identifier;        
    }
    if (isdigit(LastChar) || LastChar == '.') {
        std::string NumStr;
        do {
            NumStr += LastChar;
            LastChar = getchar();
        } while (isdigit(LastChar) || LastChar == '.');
        if (NumStr.find(".") != std::string::npos){
            std::vector DecParts = split(NumStr, '.');
            if (DecParts.size() > 2){
                printf("Decimal number cannot contain more than one period");
                exit(1);
            }
            DecVal[0] = strtod(DecParts[0].c_str(), 0);
            DecVal[1] = strtod(DecParts[1].c_str(), 0);
            return tok_dec;
        } else {
            IntVal = strtod(NumStr.c_str(), 0);
            return tok_int;
        }
        if (LastChar == '#') {
            do
                LastChar = getchar();
            while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');
            if (LastChar != EOF)
                return getToken();
        }
        if (LastChar == EOF)
            return tok_eof;
        int ThisChar = LastChar;
        LastChar = getchar();
        return ThisChar;
    }
}

//AST
class ExprAST {
public:
    virtual ~ExprAST() = default;
};
class IntExprAST : public ExprAST {
    long Val;
    public:
        IntExprAST(long Val) : Val(Val) {}
};
class DecimalExprAST: public ExprAST {
    long Val[2];
    public:
        DecimalExprAST(long (&Val)[2]) : Val{Val[0], Val[1]} {}
};
class VariableExprAST: public ExprAST {
    std::string Name;
    public:
        VariableExprAST(const std::string &Name) : Name(Name) {}
};
class BinaryExprAST : public ExprAST {
    char Op;
    std::unique_ptr<ExprAST> LHS, RHS;
    public:
        BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS,

        std::unique_ptr<ExprAST> RHS): Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
};
class CallExprAST : public ExprAST {
  std::string Callee;
  std::vector<std::unique_ptr<ExprAST>> Args;
    public:
    CallExprAST(const std::string &Callee, std::vector<std::unique_ptr<ExprAST>> Args)
        : Callee(Callee), Args(std::move(Args)) {}
};
class PrototypeAST {
    std::string Name;
    std::vector<std::string> Args;
    public:
        PrototypeAST(const std::string &Name, std::vector<std::string> Args): Name(Name), Args(std::move(Args)) {}
    const std::string &getName() const { return Name; }
};
class FunctionAST {
    std::unique_ptr<PrototypeAST> Proto;
    std::unique_ptr<ExprAST> Body;
    public:
        FunctionAST(std::unique_ptr<PrototypeAST> Proto, std::unique_ptr<ExprAST> Body)
        : Proto(std::move(Proto)), Body(std::move(Body)) {}
};
//Parser
static int CurTok;
static int getNextToken() {
  return CurTok = getToken();
}