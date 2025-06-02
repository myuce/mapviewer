#pragma once

#include <string>

// Token types emitted by the lexer
enum class TokenType
{
    LBRACE,        // '{'
    RBRACE,        // '}'
    LPAREN,        // '('
    RPAREN,        // ')'
    LBRACKET,      // '['
    RBRACKET,      // ']'
    QUOTED_STRING, // "example"
    WORD,          // identifiers, numbers, texture names
    END,           // end of input
    UNKNOWN        // any unrecognized token
};

// Single token representation
struct Token
{
    TokenType type;
    std::string text;
};

// Lexer class responsible for tokenizing the input source
class Lexer
{
public:
    explicit Lexer(const char *source);

    // Fetch the next token, skipping whitespace and comments
    Token next();

    // Push a token back onto the lexer (supports one level pushback)
    void pushBack(const Token &tok);

    int getLine() const;

private:
    const char *_src;
    int _line = 1;
    size_t _pos;
    bool _hasPushback;
    Token _pushbackTok;

    void skipWhitespaceAndComments();
    Token readQuotedString();
    Token readWord();
};
