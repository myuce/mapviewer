#include "Lexer.hpp"
#include <cctype>
#include <cstring>

Lexer::Lexer(const char *source)
    : _src(source), _pos(0), _hasPushback(false)
{
}

Token Lexer::next()
{
    if (_hasPushback)
    {
        _hasPushback = false;
        return _pushbackTok;
    }

    skipWhitespaceAndComments();

    if (_src[_pos] == '\0')
    {
        return Token{TokenType::END, ""};
    }

    char c = _src[_pos];

    switch (c)
    {
    case '{':
        _pos++;
        return Token{TokenType::LBRACE, "{"};
    case '}':
        _pos++;
        return Token{TokenType::RBRACE, "}"};
    case '(':
        _pos++;
        return Token{TokenType::LPAREN, "("};
    case ')':
        _pos++;
        return Token{TokenType::RPAREN, ")"};
    case '[':
        _pos++;
        return Token{TokenType::LBRACKET, "["};
    case ']':
        _pos++;
        return Token{TokenType::RBRACKET, "]"};
    case '"':
        return readQuotedString();
    default:
        return readWord();
    }
}

void Lexer::pushBack(const Token &tok)
{
    _hasPushback = true;
    _pushbackTok = tok;
}

void Lexer::skipWhitespaceAndComments()
{
    while (_src[_pos]) {
        if (_src[_pos] == '\n') {
            _line++;
            _pos++;
        }
        else if (std::isspace(_src[_pos])) {
            _pos++;
        }
        else if (_src[_pos] == '/' && _src[_pos + 1] == '/') {
            _pos += 2;
            while (_src[_pos] && _src[_pos] != '\n') _pos++;
        }
        else {
            break;
        }
    }
}

int Lexer::getLine() const {
    return _line;
}

Token Lexer::readQuotedString()
{
    _pos++; // skip opening quote
    size_t start = _pos;

    while (_src[_pos] && _src[_pos] != '"')
    {
        _pos++;
    }

    std::string text(&_src[start], _pos - start);
    if (_src[_pos] == '"')
        _pos++;

    return Token{TokenType::QUOTED_STRING, text};
}

Token Lexer::readWord()
{
    size_t start = _pos;

    while (_src[_pos] && !std::isspace(_src[_pos]) && !strchr("{}()[]\"", _src[_pos]))
    {
        _pos++;
    }

    std::string text(&_src[start], _pos - start);

    return Token{TokenType::WORD, text};
}
