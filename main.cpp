#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype>
#include <memory>

// ==========================================
// 1. LEXER (SÖZCÜKSEL ANALİZ) KISMI
// ==========================================
enum class TokenType {
    KEYWORD_INT, IDENTIFIER, OPERATOR_ASSIGN, NUMBER, SYMBOL_SEMI, INDENT, UNKNOWN,
    SYMBOL_LBRACE, SYMBOL_RBRACE, // Süslü parantezler
    KEYWORD_CLASS, SYMBOL_LPAREN, SYMBOL_RPAREN, SYMBOL_COLON // Sınıf yapıları
};

struct Token {
    TokenType type;
    std::string text;
    int line;
};

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::KEYWORD_INT: return "KEYWORD_INT";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::OPERATOR_ASSIGN: return "OPERATOR_ASSIGN";
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::SYMBOL_SEMI: return "SYMBOL_SEMI";
        case TokenType::INDENT: return "INDENT";
        case TokenType::SYMBOL_LBRACE: return "SYMBOL_LBRACE";
        case TokenType::SYMBOL_RBRACE: return "SYMBOL_RBRACE";
        case TokenType::KEYWORD_CLASS: return "KEYWORD_CLASS";
        case TokenType::SYMBOL_LPAREN: return "SYMBOL_LPAREN";
        case TokenType::SYMBOL_RPAREN: return "SYMBOL_RPAREN";
        case TokenType::SYMBOL_COLON: return "SYMBOL_COLON";
        default: return "UNKNOWN";
    }
}

std::vector<Token> tokenizeLine(const std::string& line, int lineNumber) {
    std::vector<Token> tokens;
    int i = 0;
    int length = line.length();

    // Girinti (Indent) Kontrolü
    int spaceCount = 0;
    while (i < length && line[i] == ' ') {
        spaceCount++;
        i++;
    }
    
    if (spaceCount > 0) {
        tokens.push_back({TokenType::INDENT, std::to_string(spaceCount), lineNumber});
    }

    // Karakterleri Okuma
    while (i < length) {
        char c = line[i];
        if (std::isspace(c)) { i++; continue; }

        // Harfler (Kelime veya Değişken)
        if (std::isalpha(c)) {
            std::string word = "";
            while (i < length && (std::isalpha(line[i]) || std::isdigit(line[i]) || line[i] == '_')) {
                word += line[i]; i++;
            }
            if (word == "int") tokens.push_back({TokenType::KEYWORD_INT, word, lineNumber});
            else if (word == "class") tokens.push_back({TokenType::KEYWORD_CLASS, word, lineNumber});
            else tokens.push_back({TokenType::IDENTIFIER, word, lineNumber});
            continue;
        }

        // Sayılar
        if (std::isdigit(c)) {
            std::string num = "";
            while (i < length && std::isdigit(line[i])) {
                num += line[i]; i++;
            }
            tokens.push_back({TokenType::NUMBER, num, lineNumber});
            continue;
        }

        // Semboller
        if (c == '=') { tokens.push_back({TokenType::OPERATOR_ASSIGN, "=", lineNumber}); i++; continue; }
        if (c == ';') { tokens.push_back({TokenType::SYMBOL_SEMI, ";", lineNumber}); i++; continue; }
        if (c == '{') { tokens.push_back({TokenType::SYMBOL_LBRACE, "{", lineNumber}); i++; continue; }
        if (c == '}') { tokens.push_back({TokenType::SYMBOL_RBRACE, "}", lineNumber}); i++; continue; }
        if (c == '(') { tokens.push_back({TokenType::SYMBOL_LPAREN, "(", lineNumber}); i++; continue; }
        if (c == ')') { tokens.push_back({TokenType::SYMBOL_RPAREN, ")", lineNumber}); i++; continue; }
        if (c == ':') { tokens.push_back({TokenType::SYMBOL_COLON, ":", lineNumber}); i++; continue; }

        tokens.push_back({TokenType::UNKNOWN, std::string(1, c), lineNumber}); i++;
    }
    return tokens;
}

// ==========================================
// 2. AST (SOYUT SÖZDİZİMİ AĞACI) KISMI
// ==========================================
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void print() const = 0;
};

class VarDeclNode : public ASTNode {
public:
    std::string varType, varName, value;
    VarDeclNode(std::string t, std::string n, std::string v) : varType(t), varName(n), value(v) {}
    void print() const override {
        std::cout << "      L__ [Degisken] Tur: " << varType << " | Isim: " << varName << " | Deger: " << value << "\n";
    }
};

class ClassDeclNode : public ASTNode {
public:
    std::string className;
    std::string parentName;
    std::vector<std::unique_ptr<VarDeclNode>> variables;

    ClassDeclNode(std::string cName, std::string pName) : className(cName), parentName(pName) {}

    void addVariable(std::unique_ptr<VarDeclNode> var) {
        variables.push_back(std::move(var));
    }

    void print() const override {
        std::cout << "  --> [SINIF DUGUMU] Isim: " << className << " (Kalitim: " << parentName << ")\n";
        for (const auto& var : variables) {
            var->print();
        }
    }

    // 1. HEADER (.h) ÜRETİCİSİ (Güncellendi)
    void generateHeader(const std::string& outputFilePath) {
        std::ofstream outFile(outputFilePath);
        if (!outFile.is_open()) return;

        std::string prefix = (parentName == "Actor") ? "A" : "U";
        std::string ueParent = (parentName == "Actor") ? "AActor" : "UObject";

        outFile << "#pragma once\n\n";
        outFile << "#include \"CoreMinimal.h\"\n";
        if (parentName == "Actor") outFile << "#include \"GameFramework/Actor.h\"\n";
        outFile << "#include \"" << className << ".generated.h\"\n\n";

        outFile << "UCLASS()\n";
        outFile << "class " << prefix << className << " : public " << ueParent << "\n{\n";
        outFile << "\tGENERATED_BODY()\n\n";
        
        outFile << "public:\n";
        // Kurucu Fonksiyon (Constructor) Tanımı
        outFile << "\t" << prefix << className << "();\n\n";

        // Değişken Tanımları
        for (const auto& var : variables) {
            outFile << "\tUPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"KokGenerated\")\n";
            outFile << "\tint32 " << var->varName << " = " << var->value << ";\n\n";
        }

        // Actor ise BeginPlay ve Tick fonksiyon tanımları
        if (parentName == "Actor") {
            outFile << "protected:\n";
            outFile << "\tvirtual void BeginPlay() override;\n\n";
            outFile << "public:\n";
            outFile << "\tvirtual void Tick(float DeltaTime) override;\n\n";
        }

        outFile << "};\n";
        outFile.close();
        std::cout << "[KOK GENERATOR] Unreal Engine Header dosyasi uretildi: " << outputFilePath << "\n";
    }

    // 2. SOURCE (.cpp) ÜRETİCİSİ (YENİ!)
    void generateSource(const std::string& outputFilePath) {
        std::ofstream outFile(outputFilePath);
        if (!outFile.is_open()) return;

        std::string prefix = (parentName == "Actor") ? "A" : "U";

        // 1. Kendi Header dosyasını include et
        outFile << "#include \"" << className << ".h\"\n\n";

        // 2. Kurucu Fonksiyon (Constructor) Gövdesi
        outFile << prefix << className << "::" << prefix << className << "()\n{\n";
        
        if (parentName == "Actor") {
            outFile << "\tPrimaryActorTick.bCanEverTick = true;\n\n";
        }

        // Kök dilinde atanan değerleri Constructor içinde de güvenceye al (Unreal Standardı)
        for (const auto& var : variables) {
            outFile << "\t" << var->varName << " = " << var->value << ";\n";
        }
        outFile << "}\n\n";

        // 3. Actor ise BeginPlay ve Tick Gövdeleri
        if (parentName == "Actor") {
            outFile << "void " << prefix << className << "::BeginPlay()\n{\n";
            outFile << "\tSuper::BeginPlay();\n";
            outFile << "}\n\n";

            outFile << "void " << prefix << className << "::Tick(float DeltaTime)\n{\n";
            outFile << "\tSuper::Tick(DeltaTime);\n";
            outFile << "}\n\n";
        }

        outFile.close();
        std::cout << "[KOK GENERATOR] Unreal Engine Source dosyasi uretildi: " << outputFilePath << "\n";
    }
};

// ==========================================
// 3. PARSER (AYRIŞTIRICI) KISMI
// ==========================================
class Parser {
private:
    std::vector<Token> tokens;
    size_t pos = 0;

    Token peek() { return (pos < tokens.size()) ? tokens[pos] : Token{TokenType::UNKNOWN, "", 0}; }
    Token consume(TokenType expected, const std::string& errorMsg) {
        Token current = peek();
        if (current.type == expected) { pos++; return current; }
        std::cerr << "Sintaks Hatasi (Satir " << current.line << "): " << errorMsg << std::endl;
        exit(1); 
    }

public:
    Parser(std::vector<Token> t) : tokens(t) {}
    
    std::unique_ptr<ASTNode> parse() {
        if (tokens.empty()) return nullptr;
        if (peek().type == TokenType::INDENT) pos++; 

        if (peek().type == TokenType::KEYWORD_CLASS) return parseClassDeclaration();
        if (peek().type == TokenType::KEYWORD_INT) return parseVariableDeclaration();
        
        return nullptr;
    }

    std::unique_ptr<ASTNode> parseClassDeclaration() {
        consume(TokenType::KEYWORD_CLASS, "Sinif tanimi 'class' ile baslamali.");
        Token nameToken = consume(TokenType::IDENTIFIER, "Sinif ismi bekleniyor.");
        consume(TokenType::SYMBOL_LPAREN, "Kalitim icin '(' bekleniyor.");
        Token parentToken = consume(TokenType::IDENTIFIER, "Miras alinacak sinif (ornegin Actor) bekleniyor.");
        consume(TokenType::SYMBOL_RPAREN, "Kalitim parantezini kapatmak icin ')' bekleniyor.");
        consume(TokenType::SYMBOL_COLON, "Sinif tanimini bitirmek icin ':' bekleniyor.");

        return std::make_unique<ClassDeclNode>(nameToken.text, parentToken.text);
    }

    std::unique_ptr<ASTNode> parseVariableDeclaration() {
        consume(TokenType::KEYWORD_INT, "Degisken tanimi 'int' ile baslamali.");
        Token nameToken = consume(TokenType::IDENTIFIER, "Degisken ismi bekleniyor.");
        consume(TokenType::OPERATOR_ASSIGN, "Atama icin '=' isareti bekleniyor.");
        Token valueToken = consume(TokenType::NUMBER, "Atanacak sayi bekleniyor.");
        consume(TokenType::SYMBOL_SEMI, "Satir sonuna ';' bekleniyor.");
        
        return std::make_unique<VarDeclNode>("int", nameToken.text, valueToken.text);
    }
};

// ==========================================
// 4. ANA PROGRAM VE KOD ÜRETİCİ YÖNETİMİ
// ==========================================
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Hata: Derlenecek dosya belirtilmedi!\n";
        return 1;
    }

    std::string inputFilePath = argv[1];
    std::ifstream inputFile(inputFilePath);
    if (!inputFile.is_open()) return 1;

    std::cout << "[KOK TRANSPILER] Derleme baslatildi...\n";
    std::cout << "--------------------------------------------------\n";

    std::string line;
    int lineNumber = 1;
    
    int scopeDepth = 0;
    const int SPACES_PER_INDENT = 4; 
    
    // Geçerli sınıfı hafızada tutmak için işaretçi
    std::unique_ptr<ClassDeclNode> currentClass = nullptr;
    
    while (std::getline(inputFile, line)) {
        std::vector<Token> tokens = tokenizeLine(line, lineNumber);
        
        if (!tokens.empty()) {
            
            // --- LINTER KONTROLÜ ---
            bool isClosingBrace = false;
            for(const auto& t : tokens) {
                if (t.type == TokenType::SYMBOL_RBRACE) isClosingBrace = true;
            }
            if (isClosingBrace) scopeDepth--;

            int expectedSpaces = scopeDepth * SPACES_PER_INDENT;
            int actualSpaces = (tokens[0].type == TokenType::INDENT) ? std::stoi(tokens[0].text) : 0;

            bool hasCode = false;
            for(const auto& t : tokens) if (t.type != TokenType::INDENT) hasCode = true;

            if (hasCode && actualSpaces != expectedSpaces) {
                std::cerr << "Linter Hatasi (Satir " << lineNumber << "): Zorunlu girinti uyusmazligi!\n"
                          << "  Beklenen: " << expectedSpaces << " bosluk\n"
                          << "  Bulunan:  " << actualSpaces << " bosluk\n";
                return 1; 
            }

            if (!isClosingBrace) {
                for(const auto& t : tokens) {
                    // Sınıf tanımlayan iki nokta (:) veya süslü parantez ({) derinliği artırır
                    if (t.type == TokenType::SYMBOL_LBRACE || t.type == TokenType::SYMBOL_COLON) scopeDepth++;
                }
            }
            // --- LINTER SONU ---

            // Parser'ı çalıştır
            Parser parser(tokens);
            std::unique_ptr<ASTNode> astNode = parser.parse();
            
            if (astNode) {
                // Eğer dönen düğüm bir Sınıf ise, onu "aktif sınıf" olarak kaydet
                ASTNode* rawNode = astNode.release();
                
                if (ClassDeclNode* cNode = dynamic_cast<ClassDeclNode*>(rawNode)) {
                    currentClass.reset(cNode);
                } 
                // Eğer dönen düğüm bir Değişken ise, onu "aktif sınıfın" içine ekle
                else if (VarDeclNode* vNode = dynamic_cast<VarDeclNode*>(rawNode)) {
                    if (currentClass) {
                        currentClass->addVariable(std::unique_ptr<VarDeclNode>(vNode));
                    } else {
                        delete vNode; // Global değişkenleri şimdilik yoksayıyoruz
                    }
                } else {
                    delete rawNode;
                }
            }
        }
        lineNumber++;
    }
    inputFile.close();

    // DOSYA OKUMA BİTTİ. EĞER HAFIZADA BİR SINIF VARSA .H DOSYASINI ÜRET
    // DOSYA OKUMA BİTTİ. EĞER HAFIZADA BİR SINIF VARSA .H VE .CPP DOSYALARINI ÜRET
    if (currentClass) {
        currentClass->print();
        
        // Header (.h) dosyasını üret
        std::string headerName = currentClass->className + ".h";
        currentClass->generateHeader(headerName);

        // Source (.cpp) dosyasını üret
        std::string sourceName = currentClass->className + ".cpp";
        currentClass->generateSource(sourceName);
    }

    std::cout << "--------------------------------------------------\n";
    std::cout << "[KOK TRANSPILER] Basarili! Tum Unreal Engine dosyalari uretildi.\n";

    return 0;

}