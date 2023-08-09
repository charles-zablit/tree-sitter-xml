#pragma once

#include <ctype.h>
#include <tree_sitter/parser.h>

enum TokenType {
    PITarget,
    PIContent,
    CharData,
};

/// Advance the lexer to the next token
static inline void advance(TSLexer *lexer) { lexer->advance(lexer, false); }

/// Scan for the target of a PI node
static bool scan_pi_target(TSLexer *lexer) {
    bool advanced_once = false, found_x_first = false;

    if (isalpha(lexer->lookahead) || lexer->lookahead == '_') {
        if (lexer->lookahead == 'x' || lexer->lookahead == 'X') {
            found_x_first = true;
            lexer->mark_end(lexer);
        }
        advanced_once = true;
        advance(lexer);
    }

    if (advanced_once) {
        while (isalnum(lexer->lookahead) || lexer->lookahead == '_' ||
               lexer->lookahead == ':' || lexer->lookahead == '.' ||
               lexer->lookahead == L'·' || lexer->lookahead == '-') {
            if (lexer->lookahead == 'x' || lexer->lookahead == 'X') {
                lexer->mark_end(lexer);
                advance(lexer);
                if (lexer->lookahead == 'm' || lexer->lookahead == 'M') {
                    advance(lexer);
                    if (lexer->lookahead == 'l' || lexer->lookahead == 'L') {
                        advance(lexer);
                        return false;
                    }
                }
            }

            if (found_x_first &&
                (lexer->lookahead == 'm' || lexer->lookahead == 'M')) {
                advance(lexer);
                if (lexer->lookahead == 'l' || lexer->lookahead == 'L') {
                    advance(lexer);
                    return false;
                }
            }

            found_x_first = false;
            advance(lexer);
        }

        lexer->mark_end(lexer);
        lexer->result_symbol = PITarget;
        return true;
    }

    return false;
}

/// Scan for the content of a PI node
static bool scan_pi_content(TSLexer *lexer) {
    bool advanced_once = false;

    while (!lexer->eof(lexer) &&
           lexer->lookahead != '\n' &&
           lexer->lookahead != '?') {
        advanced_once = true;
        advance(lexer);
    }

    if (lexer->lookahead == '?') {
        lexer->mark_end(lexer);
        advance(lexer);
        if (lexer->lookahead == '>') {
            advance(lexer);
            return false;
        }
    }

    if (advanced_once) {
        lexer->mark_end(lexer);
        lexer->result_symbol = PIContent;
        return true;
    }

    return false;
}

/// Scan for a CharData node
static bool scan_char_data(TSLexer *lexer) {
    bool advanced_once = false;

    while (!lexer->eof(lexer) &&
           lexer->lookahead != '<' &&
           lexer->lookahead != '&') {
        if (lexer->lookahead == ']') {
            lexer->mark_end(lexer);
            advance(lexer);
            if (lexer->lookahead == ']') {
                advance(lexer);
                if (lexer->lookahead == '>') {
                    advance(lexer);
                    if (advanced_once) {
                        lexer->result_symbol = CharData;
                        return false;
                    }
                }
            }
        }
        advanced_once = true;
        advance(lexer);
    }

    if (advanced_once) {
        lexer->mark_end(lexer);
        lexer->result_symbol = CharData;
        return true;
    }
    return false;
}

/// Scan for the common symbols
#define SCAN_COMMON(lexer, valid_symbols) \
    if (in_error_recovery(valid_symbols)) return false; \
    \
    if (valid_symbols[PITarget]) return scan_pi_target(lexer); \
    \
    if (valid_symbols[PIContent]) return scan_pi_content(lexer);

/// Define the boilerplate functions of the scanner
/// @param name the name of the language
#define SCANNER_BOILERPLATE(name) \
    void *tree_sitter_##name##_external_scanner_create() { return NULL; } \
    \
    void tree_sitter_##name##_external_scanner_destroy(void *payload) {} \
    \
    void tree_sitter_##name##_external_scanner_reset(void *payload) {} \
    \
    unsigned tree_sitter_##name##_external_scanner_serialize(void *payload, char *buffer) { return 0; } \
    \
    void tree_sitter_##name##_external_scanner_deserialize(void *payload, const char *buffer, unsigned length) {}