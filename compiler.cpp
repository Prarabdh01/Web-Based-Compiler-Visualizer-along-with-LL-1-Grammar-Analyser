#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <algorithm>
#include <cctype>

using namespace std;

// ======================== TOKEN DEFINITIONS ========================
enum TokenType {
    INT, FLOAT,
    ID, NUM,
    PLUS, MINUS, MUL, DIV,
    LPAREN, RPAREN, SEMI, ASSIGN,
    END, UNKNOWN
};

struct Token {
    TokenType type;
    string value;
    Token(TokenType t = UNKNOWN, string v = "") : type(t), value(v) {}
};

// ======================== LEXICAL ANALYZER ========================
class Lexer {
private:
    string input;
    size_t pos;

    bool isKeyword(const string& word) {
        return word == "int" || word == "float";
    }

    TokenType getKeywordType(const string& word) {
        if (word == "int") return INT;
        if (word == "float") return FLOAT;
        return UNKNOWN;
    }

public:
    Lexer(const string& source) : input(source), pos(0) {}

    vector<Token> tokenize() {
        vector<Token> tokens;

        while (pos < input.length()) {
            char ch = input[pos];

            if (isspace(ch)) {
                pos++;
                continue;
            }

            if (ch == '/' && pos + 1 < input.length() && input[pos + 1] == '/') {
                while (pos < input.length() && input[pos] != '\n') pos++;
                continue;
            }

            if (isalpha(ch) || ch == '_') {
                string word;
                while (pos < input.length() && (isalnum(input[pos]) || input[pos] == '_')) {
                    word += input[pos++];
                }

                if (isKeyword(word)) {
                    tokens.push_back(Token(getKeywordType(word), word));
                } else {
                    tokens.push_back(Token(ID, word));
                }
                continue;
            }

            if (isdigit(ch)) {
                string num;
                while (pos < input.length() && isdigit(input[pos])) {
                    num += input[pos++];
                }
                tokens.push_back(Token(NUM, num));
                continue;
            }

            switch (ch) {
                case '+': tokens.push_back(Token(PLUS, "+"));  pos++; break;
                case '-': tokens.push_back(Token(MINUS, "-")); pos++; break;
                case '*': tokens.push_back(Token(MUL, "*"));   pos++; break;
                case '/': tokens.push_back(Token(DIV, "/"));   pos++; break;
                case '(': tokens.push_back(Token(LPAREN, "(")); pos++; break;
                case ')': tokens.push_back(Token(RPAREN, ")")); pos++; break;
                case ';': tokens.push_back(Token(SEMI, ";"));   pos++; break;
                case '=': tokens.push_back(Token(ASSIGN, "=")); pos++; break;
                default:
                    tokens.push_back(Token(UNKNOWN, string(1, ch)));
                    pos++;
            }
        }

        tokens.push_back(Token(END, "$"));
        return tokens;
    }

    static string tokenTypeToString(TokenType type) {
        switch (type) {
            case INT: return "INT";
            case FLOAT: return "FLOAT";
            case ID: return "ID";
            case NUM: return "NUM";
            case PLUS: return "PLUS";
            case MINUS: return "MINUS";
            case MUL: return "MUL";
            case DIV: return "DIV";
            case LPAREN: return "LPAREN";
            case RPAREN: return "RPAREN";
            case SEMI: return "SEMI";
            case ASSIGN: return "ASSIGN";
            case END: return "END";
            default: return "UNKNOWN";
        }
    }
};

// ======================== PARSE TREE NODE ========================
struct ParseNode {
    string type;
    string value;
    vector<ParseNode*> children;

    ParseNode(string t, string v = "") : type(t), value(v) {}

    ~ParseNode() {
        for (auto child : children) delete child;
    }
};

// ======================== PARSER ========================
class Parser {
private:
    vector<Token> tokens;
    size_t current;

    Token peek() {
        if (current < tokens.size()) return tokens[current];
        return Token(END, "$");
    }

    Token advance() {
        if (current < tokens.size()) return tokens[current++];
        return Token(END, "$");
    }

    bool match(TokenType type) {
        if (peek().type == type) {
            advance();
            return true;
        }
        return false;
    }

    // Program -> Decl* Stmt*
    ParseNode* parseProgram() {
        ParseNode* node = new ParseNode("Program");

        while (peek().type == INT || peek().type == FLOAT) {
            node->children.push_back(parseDeclaration());
        }

        while (peek().type == ID) {
            node->children.push_back(parseAssignment());
        }

        return node;
    }

    ParseNode* parseDeclaration() {
        ParseNode* node = new ParseNode("Declaration");

        Token typeToken = advance();
        node->children.push_back(new ParseNode("Type", typeToken.value));

        Token idToken = advance();
        node->children.push_back(new ParseNode("ID", idToken.value));

        advance(); // ;

        return node;
    }

    ParseNode* parseAssignment() {
        ParseNode* node = new ParseNode("Assignment");

        Token idToken = advance();
        node->children.push_back(new ParseNode("ID", idToken.value));

        advance(); // =

        node->children.push_back(parseExpr());

        advance(); // ;

        return node;
    }

    ParseNode* parseExpr() {
        ParseNode* node = new ParseNode("Expr");
        node->children.push_back(parseTerm());

        while (peek().type == PLUS || peek().type == MINUS) {
            Token op = advance();
            ParseNode* opNode = new ParseNode("Operator", op.value);
            node->children.push_back(opNode);
            node->children.push_back(parseTerm());
        }

        return node;
    }

    ParseNode* parseTerm() {
        ParseNode* node = new ParseNode("Term");
        node->children.push_back(parseFactor());

        while (peek().type == MUL || peek().type == DIV) {
            Token op = advance();
            ParseNode* opNode = new ParseNode("Operator", op.value);
            node->children.push_back(opNode);
            node->children.push_back(parseFactor());
        }

        return node;
    }

    ParseNode* parseFactor() {
        ParseNode* node = new ParseNode("Factor");

        if (peek().type == NUM) {
            Token num = advance();
            node->children.push_back(new ParseNode("NUM", num.value));
        } else if (peek().type == ID) {
            Token id = advance();
            node->children.push_back(new ParseNode("ID", id.value));
        } else if (match(LPAREN)) {
            node->children.push_back(parseExpr());
            advance(); // )
        }

        return node;
    }

public:
    Parser(const vector<Token>& toks) : tokens(toks), current(0) {}

    ParseNode* parse() {
        return parseProgram();
    }
};

// ======================== SEMANTIC ANALYZER ========================
struct SymbolInfo {
    string type;
    bool declared;

    SymbolInfo(string t = "unknown") : type(t), declared(true) {}
};

class SemanticAnalyzer {
private:
    map<string, SymbolInfo> symbolTable;
    vector<string> errors;

    void analyzeNode(ParseNode* node) {
        if (!node) return;

        if (node->type == "Declaration") {
            string varType = node->children[0]->value;
            string varName = node->children[1]->value;

            if (symbolTable.find(varName) != symbolTable.end()) {
                errors.push_back("Variable '" + varName + "' already declared");
            } else {
                symbolTable[varName] = SymbolInfo(varType);
            }
        }

        if (node->type == "Assignment") {
            string varName = node->children[0]->value;

            if (symbolTable.find(varName) == symbolTable.end()) {
                errors.push_back("Variable '" + varName + "' not declared");
            }
        }

        if (node->type == "Factor" || node->type == "ID") {
            if (!node->children.empty() && node->children[0]->type == "ID") {
                string varName = node->children[0]->value;
                if (symbolTable.find(varName) == symbolTable.end()) {
                    errors.push_back("Variable '" + varName + "' used before declaration");
                }
            }
        }

        for (auto child : node->children) {
            analyzeNode(child);
        }
    }

public:
    void analyze(ParseNode* root) {
        analyzeNode(root);
    }

    map<string, SymbolInfo> getSymbolTable() { return symbolTable; }
    vector<string> getErrors() { return errors; }
};

// ======================== TAC GENERATOR ========================
class TACGenerator {
private:
    vector<string> tac;
    int tempCounter;

    string newTemp() {
        return "t" + to_string(tempCounter++);
    }

    string generateExpr(ParseNode* node) {
        if (!node) return "";

        if (node->type == "Factor") {
            if (node->children[0]->type == "NUM") {
                return node->children[0]->value;
            } else if (node->children[0]->type == "ID") {
                return node->children[0]->value;
            } else {
                return generateExpr(node->children[0]);
            }
        }

        if (node->type == "Term" || node->type == "Expr") {
            if (node->children.size() == 1) {
                return generateExpr(node->children[0]);
            }

            string left = generateExpr(node->children[0]);

            for (size_t i = 1; i < node->children.size(); i += 2) {
                string op = node->children[i]->value;
                string right = generateExpr(node->children[i + 1]);
                string temp = newTemp();
                tac.push_back(temp + " = " + left + " " + op + " " + right);
                left = temp;
            }

            return left;
        }

        return "";
    }

    void generateStmt(ParseNode* node) {
        if (!node) return;

        if (node->type == "Assignment") {
            string varName = node->children[0]->value;
            string exprResult = generateExpr(node->children[1]);
            tac.push_back(varName + " = " + exprResult);
        }

        for (auto child : node->children) {
            if (child->type == "Assignment") {
                generateStmt(child);
            }
        }
    }

public:
    TACGenerator() : tempCounter(1) {}

    vector<string> generate(ParseNode* root) {
        tac.clear();
        tempCounter = 1;

        if (root) {
            for (auto child : root->children) {
                generateStmt(child);
            }
        }

        return tac;
    }
};

// ======================== OPTIMIZER ========================
class Optimizer {
private:
    vector<string> optimizations;

    bool isNumber(const string& str) {
        return !str.empty() && all_of(str.begin(), str.end(), ::isdigit);
    }

    int evaluate(int a, char op, int b) {
        switch (op) {
            case '+': return a + b;
            case '-': return a - b;
            case '*': return a * b;
            case '/': return b != 0 ? a / b : 0;
        }
        return 0;
    }

public:
    vector<string> optimize(const vector<string>& tac) {
        vector<string> optimized;
        optimizations.clear();
        map<string, string> constants;

        for (const string& line : tac) {
            size_t eqPos = line.find(" = ");
            if (eqPos == string::npos) {
                optimized.push_back(line);
                continue;
            }

            string lhs = line.substr(0, eqPos);
            string rhs = line.substr(eqPos + 3);

            size_t opPos = rhs.find_first_of("+-*/");
            if (opPos != string::npos && opPos > 0 && opPos < rhs.length() - 1) {
                string left = rhs.substr(0, opPos - 1);
                char op = rhs[opPos];
                string right = rhs.substr(opPos + 2);

                if (constants.find(left) != constants.end()) {
                    left = constants[left];
                }
                if (constants.find(right) != constants.end()) {
                    right = constants[right];
                }

                if (isNumber(left) && isNumber(right)) {
                    int result = evaluate(stoi(left), op, stoi(right));
                    string newLine = lhs + " = " + to_string(result);
                    optimized.push_back(newLine);
                    constants[lhs] = to_string(result);
                    optimizations.push_back("Constant folding: " + line + " -> " + newLine);
                    continue;
                }

                if ((op == '+' || op == '-') && right == "0") {
                    optimizations.push_back("Eliminated: " + line + " (x " + string(1, op) + " 0 = x)");
                    constants[lhs] = left;
                    continue;
                }

                if (op == '*' && (left == "1" || right == "1")) {
                    string other = (left == "1") ? right : left;
                    optimizations.push_back("Eliminated: " + line + " (x * 1 = x)");
                    constants[lhs] = other;
                    continue;
                }

                if (op == '*' && (left == "0" || right == "0")) {
                    string newLine = lhs + " = 0";
                    optimized.push_back(newLine);
                    constants[lhs] = "0";
                    optimizations.push_back("Simplified: " + line + " -> " + newLine);
                    continue;
                }
            }

            if (constants.find(rhs) != constants.end()) {
                string newLine = lhs + " = " + constants[rhs];
                optimized.push_back(newLine);
                constants[lhs] = constants[rhs];
                optimizations.push_back("Constant propagation: " + line + " -> " + newLine);
            } else {
                optimized.push_back(line);
                if (isNumber(rhs)) {
                    constants[lhs] = rhs;
                }
            }
        }

        return optimized;
    }

    vector<string> getOptimizations() { return optimizations; }
};

// ======================== JSON HELPERS ========================
string escapeJson(const string& str) {
    string escaped;
    for (char c : str) {
        if (c == '"') escaped += "\\\"";
        else if (c == '\\') escaped += "\\\\";
        else if (c == '\n') escaped += "\\n";
        else if (c == '\t') escaped += "\\t";
        else escaped += c;
    }
    return escaped;
}

void printTokensJson(const vector<Token>& tokens) {
    cout << "\"tokens\": [";
    for (size_t i = 0; i < tokens.size() - 1; i++) { // skip END
        if (i > 0) cout << ",";
        cout << "{\"type\":\"" << Lexer::tokenTypeToString(tokens[i].type)
             << "\",\"value\":\"" << escapeJson(tokens[i].value) << "\"}";
    }
    cout << "]";
}

void printTreeJson(ParseNode* node) {
    if (!node) return;

    cout << "{\"type\":\"" << node->type << "\"";
    if (!node->value.empty()) {
        cout << ",\"value\":\"" << escapeJson(node->value) << "\"";
    }
    if (!node->children.empty()) {
        cout << ",\"children\":[";
        for (size_t i = 0; i < node->children.size(); i++) {
            if (i > 0) cout << ",";
            printTreeJson(node->children[i]);
        }
        cout << "]";
    }
    cout << "}";
}

void printSymbolTableJson(const map<string, SymbolInfo>& symbolTable) {
    cout << "\"symbolTable\": {";
    bool first = true;
    for (const auto& entry : symbolTable) {
        if (!first) cout << ",";
        first = false;
        cout << "\"" << entry.first << "\":{\"type\":\"" << entry.second.type << "\"}";
    }
    cout << "}";
}

void printTACJson(const vector<string>& tac, const string& fieldName) {
    cout << "\"" << fieldName << "\":[";
    for (size_t i = 0; i < tac.size(); i++) {
        if (i > 0) cout << ",";
        cout << "\"" << escapeJson(tac[i]) << "\"";
    }
    cout << "]";
}

void printOptimizationsJson(const vector<string>& opts) {
    cout << "\"optimizations\":[";
    for (size_t i = 0; i < opts.size(); i++) {
        if (i > 0) cout << ",";
        cout << "\"" << escapeJson(opts[i]) << "\"";
    }
    cout << "]";
}

void printErrorsJson(const vector<string>& errors) {
    cout << "\"semanticErrors\":[";
    for (size_t i = 0; i < errors.size(); i++) {
        if (i > 0) cout << ",";
        cout << "\"" << escapeJson(errors[i]) << "\"";
    }
    cout << "]";
}

// ======================== MAIN ========================
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: compiler <mode> [source_code]" << endl;
        return 1;
    }

    string mode = argv[1];

    if (mode == "tokenize" && argc >= 3) {
        string source = argv[2];
        Lexer lexer(source);
        vector<Token> tokens = lexer.tokenize();

        cout << "{\"success\":true,";
        printTokensJson(tokens);
        cout << "}" << endl;

    } else if (mode == "compile" && argc >= 3) {
        string source = argv[2];

        Lexer lexer(source);
        vector<Token> tokens = lexer.tokenize();

        Parser parser(tokens);
        ParseNode* parseTree = parser.parse();

        SemanticAnalyzer semantic;
        semantic.analyze(parseTree);

        TACGenerator tacGen;
        vector<string> tac = tacGen.generate(parseTree);

        Optimizer optimizer;
        vector<string> optimizedTac = optimizer.optimize(tac);

        cout << "{\"success\":true,";
        printTokensJson(tokens);
        cout << ",\"parseTree\":";
        printTreeJson(parseTree);
        cout << ",";
        printSymbolTableJson(semantic.getSymbolTable());
        cout << ",";
        printErrorsJson(semantic.getErrors());
        cout << ",";
        printTACJson(tac, "tac");
        cout << ",";
        printTACJson(optimizedTac, "optimizedTac");
        cout << ",";
        printOptimizationsJson(optimizer.getOptimizations());
        cout << "}" << endl;

        delete parseTree;
    }

    return 0;
}
