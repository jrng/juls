typedef enum
{
    TOKEN_END_OF_INPUT      =           0,
    TOKEN_ERROR             =           1,
    TOKEN_IDENTIFIER        =           2,
    TOKEN_LITERAL_STRING    =           3,
    TOKEN_LITERAL_INTEGER   =           4,
    TOKEN_LITERAL_BOOLEAN   =           5, // TODO: do we need this?
    TOKEN_LITERAL_FLOAT     =           6,
    TOKEN_KEYWORD_IF        =           7,
    TOKEN_KEYWORD_ELSE      =           8,
    TOKEN_KEYWORD_STRUCT    =           9,
    TOKEN_KEYWORD_FOR       =          10,
    TOKEN_KEYWORD_WHILE     =          11,
    TOKEN_KEYWORD_NULL      =          12,
    TOKEN_KEYWORD_TRUE      =          13,
    TOKEN_KEYWORD_FALSE     =          14,
    TOKEN_KEYWORD_RETURN    =          15,
    TOKEN_KEYWORD_SIZE_OF   =          16,
    TOKEN_KEYWORD_TYPE_OF   =          17,
    TOKEN_EQUAL             = /* == */ 18,
    TOKEN_NOT_EQUAL         = /* != */ 19,
    TOKEN_SHIFT_LEFT        = /* << */ 20,
    TOKEN_SHIFT_RIGHT       = /* >> */ 21,
    TOKEN_LESS_EQUAL        = /* <= */ 22,
    TOKEN_GREATER_EQUAL     = /* >= */ 23,
    TOKEN_LOGICAL_OR        = /* || */ 24,
    TOKEN_LOGICAL_AND       = /* && */ 25,
    TOKEN_COLON_EQUAL       = /* := */ 26,
    TOKEN_COLON_COLON       = /* :: */ 27,
    TOKEN_PLUS_EQUAL        = /* += */ 28,
    TOKEN_MINUS_EQUAL       = /* -= */ 29,
    TOKEN_MUL_EQUAL         = /* *= */ 30,
    TOKEN_DIV_EQUAL         = /* /= */ 31,
    TOKEN_OR_EQUAL          = /* |= */ 32,
    TOKEN_UNARY_NOT         = '!', //  33
    TOKEN_AND_EQUAL         = /* &= */ 34,
    TOKEN_XOR_EQUAL         = /* ^= */ 35,
    TOKEN_RIGHT_ARROW       = /* -> */ 36,
    TOKEN_BINOP_MOD         = '%', //  37
    TOKEN_BINOP_AND         = '&', //  38
    TOKEN_KEYWORD_CAST      =          39,
    TOKEN_LEFT_PAREN        = '(', //  40
    TOKEN_RIGHT_PAREN       = ')', //  41
    TOKEN_BINOP_MUL         = '*', //  42
    TOKEN_BINOP_PLUS        = '+', //  43
    TOKEN_COMMA             = ',', //  44
    TOKEN_BINOP_MINUS       = '-', //  45
    TOKEN_DOT               = '.', //  46
    TOKEN_BINOP_DIV         = '/', //  47
    TOKEN_DIRECTIVE_LOAD    =          48,
    TOKEN_DIRECTIVE_IMPORT  =          49,
    TOKEN_COLON             = ':', //  58
    TOKEN_SEMICOLON         = ';', //  59
    TOKEN_LESS              = '<', //  60
    TOKEN_ASSIGN            = '=', //  61
    TOKEN_GREATER           = '>', //  62
    TOKEN_QUESTIONMARK      = '?', //  63
    TOKEN_LEFT_BRACKET      = '[', //  91
    TOKEN_RIGHT_BRACKET     = ']', //  93
    TOKEN_BINOP_XOR         = '^', //  94
    TOKEN_LEFT_BRACE        = '{', // 123
    TOKEN_BINOP_OR          = '|', // 124
    TOKEN_RIGHT_BRACE       = '}', // 125
    TOKEN_UNARY_NEG         = '~', // 126
} TokenType;

typedef struct
{
    u8 type;
    u16 file_index;
    String lexeme;
} Token;

typedef struct
{
    s64 start;
    s64 current;

    u16 current_file_index;

    String input;
} Lexer;

static inline bool
is_at_end(Lexer lexer)
{
    return (lexer.current >= lexer.input.count);
}

static inline u8
eat_character(Lexer *lexer)
{
    return lexer->input.data[lexer->current++];
}

static inline u8
peek_character(Lexer lexer)
{
    return lexer.input.data[lexer.current];
}

static inline u8
peek_next_character(Lexer lexer)
{
    if ((lexer.current + 1) < lexer.input.count)
    {
        return lexer.input.data[lexer.current + 1];
    }

    return 0;
}

static inline bool
matches_character(Lexer *lexer, u8 c)
{
    if (is_at_end(*lexer) || (lexer->input.data[lexer->current] != c))
    {
        return false;
    }

    lexer->current += 1;

    return true;
}

static inline bool
is_whitespace(u8 c)
{
    return ((c == ' ') || (c == '\r') || (c == '\n') || (c == '\t')) ? true : false;
}

static inline bool
is_digit(u8 c)
{
    return ((c >= '0') && (c <= '9')) ? true : false;
}

static inline bool
is_alpha(u8 c)
{
    return (((c >= 'a') && (c <= 'z')) ||
            ((c >= 'A') && (c <= 'Z')) ||
            (c == '_')) ? true : false;
}

static inline void
skip_whitespace(Lexer *lexer)
{
    while (!is_at_end(*lexer))
    {
        u8 c = peek_character(*lexer);

        if (is_whitespace(c))
        {
        }
        else if (c == '/')
        {
            if (peek_next_character(*lexer) == '/')
            {
                while (!is_at_end(*lexer) && (peek_character(*lexer) != '\n'))
                {
                    eat_character(lexer);
                }
            }
            else if (peek_next_character(*lexer) == '*')
            {
                while (!is_at_end(*lexer))
                {
                    if ((peek_character(*lexer) == '*') &&
                        (peek_next_character(*lexer) == '/'))
                    {
                        eat_character(lexer);
                        eat_character(lexer);
                        break;
                    }

                    eat_character(lexer);
                }
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }

        eat_character(lexer);
    }
}

static inline Token
make_token(Lexer lexer, u8 token_type)
{
    Token token;

    token.type = token_type;
    token.file_index = lexer.current_file_index;
    token.lexeme = make_string(lexer.current - lexer.start, lexer.input.data + lexer.start);

    return token;
}

static Token
identifier(Lexer *lexer)
{
    while (!is_at_end(*lexer) && (is_alpha(peek_character(*lexer)) || is_digit(peek_character(*lexer))))
    {
        eat_character(lexer);
    }

    String ident = make_string(lexer->current - lexer->start, lexer->input.data + lexer->start);

    // TODO: group by count
    if (strings_are_equal(ident, S("if")))
    {
        return make_token(*lexer, TOKEN_KEYWORD_IF);
    }
    else if (strings_are_equal(ident, S("else")))
    {
        return make_token(*lexer, TOKEN_KEYWORD_ELSE);
    }
    else if (strings_are_equal(ident, S("struct")))
    {
        return make_token(*lexer, TOKEN_KEYWORD_STRUCT);
    }
    else if (strings_are_equal(ident, S("for")))
    {
        return make_token(*lexer, TOKEN_KEYWORD_FOR);
    }
    else if (strings_are_equal(ident, S("while")))
    {
        return make_token(*lexer, TOKEN_KEYWORD_WHILE);
    }
    else if (strings_are_equal(ident, S("null")))
    {
        return make_token(*lexer, TOKEN_KEYWORD_NULL);
    }
    else if (strings_are_equal(ident, S("true")))
    {
        return make_token(*lexer, TOKEN_KEYWORD_TRUE);
    }
    else if (strings_are_equal(ident, S("false")))
    {
        return make_token(*lexer, TOKEN_KEYWORD_FALSE);
    }
    else if (strings_are_equal(ident, S("return")))
    {
        return make_token(*lexer, TOKEN_KEYWORD_RETURN);
    }
    else if (strings_are_equal(ident, S("size_of")))
    {
        return make_token(*lexer, TOKEN_KEYWORD_SIZE_OF);
    }
    else if (strings_are_equal(ident, S("type_of")))
    {
        return make_token(*lexer, TOKEN_KEYWORD_TYPE_OF);
    }
    else if (strings_are_equal(ident, S("cast")))
    {
        return make_token(*lexer, TOKEN_KEYWORD_CAST);
    }

    return make_token(*lexer, TOKEN_IDENTIFIER);
}

static Token
number(Lexer *lexer)
{
    u8 token_type = TOKEN_LITERAL_INTEGER;

    while (!is_at_end(*lexer) && is_digit(peek_character(*lexer)))
    {
        eat_character(lexer);
    }

    if (!is_at_end(*lexer) && is_digit(peek_next_character(*lexer)))
    {
        eat_character(lexer);
        token_type = TOKEN_LITERAL_FLOAT;

        while (!is_at_end(*lexer) && is_digit(peek_character(*lexer)))
        {
            eat_character(lexer);
        }
    }

    return make_token(*lexer, token_type);
}

static Token
string(Lexer *lexer)
{
    while (!is_at_end(*lexer) && (peek_character(*lexer) != '"'))
    {
        eat_character(lexer);
    }

    if (is_at_end(*lexer))
    {
        // TODO: error token
        return make_token(*lexer, TOKEN_END_OF_INPUT);
    }

    eat_character(lexer);

    return make_token(*lexer, TOKEN_LITERAL_STRING);
}

static Token
directive(Lexer *lexer)
{
    while (!is_at_end(*lexer) && is_alpha(peek_character(*lexer)))
    {
        eat_character(lexer);
    }

    String ident = make_string(lexer->current - lexer->start, lexer->input.data + lexer->start);

    if (strings_are_equal(ident, S("#load")))
    {
        return make_token(*lexer, TOKEN_DIRECTIVE_LOAD);
    }
    else if (strings_are_equal(ident, S("#import")))
    {
        return make_token(*lexer, TOKEN_DIRECTIVE_IMPORT);
    }

    // TODO: error message
    return make_token(*lexer, TOKEN_ERROR);
}

static Token
get_next_token(Lexer *lexer)
{
    skip_whitespace(lexer);

    lexer->start = lexer->current;

    if (is_at_end(*lexer))
    {
        return make_token(*lexer, TOKEN_END_OF_INPUT);
    }

    u8 c = eat_character(lexer);

    if (is_alpha(c))
    {
        return identifier(lexer);
    }

    if (is_digit(c))
    {
        return number(lexer);
    }

    switch (c)
    {
        case '!': return make_token(*lexer, matches_character(lexer, '=') ? TOKEN_NOT_EQUAL : TOKEN_UNARY_NOT);
        case '"': return string(lexer);
        case '#': return directive(lexer);
        case '%': return make_token(*lexer, TOKEN_BINOP_MOD);
        case '&': return make_token(*lexer, matches_character(lexer, '=') ? TOKEN_AND_EQUAL : TOKEN_BINOP_AND);
        case '(': return make_token(*lexer, TOKEN_LEFT_PAREN);
        case ')': return make_token(*lexer, TOKEN_RIGHT_PAREN);
        case '*': return make_token(*lexer, matches_character(lexer, '=') ? TOKEN_MUL_EQUAL : TOKEN_BINOP_MUL);
        case '+': return make_token(*lexer, matches_character(lexer, '=') ? TOKEN_PLUS_EQUAL : TOKEN_BINOP_PLUS);
        case ',': return make_token(*lexer, TOKEN_COMMA);
        case '-': return make_token(*lexer, matches_character(lexer, '=') ? TOKEN_MINUS_EQUAL : matches_character(lexer, '>') ? TOKEN_RIGHT_ARROW : TOKEN_BINOP_MINUS);
        case '.': return make_token(*lexer, TOKEN_DOT);
        case '/': return make_token(*lexer, matches_character(lexer, '=') ? TOKEN_DIV_EQUAL : TOKEN_BINOP_DIV);
        case ':': return make_token(*lexer, matches_character(lexer, '=') ? TOKEN_COLON_EQUAL : matches_character(lexer, ':') ? TOKEN_COLON_COLON : TOKEN_COLON);
        case ';': return make_token(*lexer, TOKEN_SEMICOLON);
        case '<': return make_token(*lexer, matches_character(lexer, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '=': return make_token(*lexer, matches_character(lexer, '=') ? TOKEN_EQUAL : TOKEN_ASSIGN);
        case '>': return make_token(*lexer, matches_character(lexer, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
        case '?': return make_token(*lexer, TOKEN_QUESTIONMARK);
        case '[': return make_token(*lexer, TOKEN_LEFT_BRACKET);
        case ']': return make_token(*lexer, TOKEN_RIGHT_BRACKET);
        case '^': return make_token(*lexer, matches_character(lexer, '=') ? TOKEN_XOR_EQUAL : TOKEN_BINOP_XOR);
        case '{': return make_token(*lexer, TOKEN_LEFT_BRACE);
        case '|': return make_token(*lexer, matches_character(lexer, '=') ? TOKEN_OR_EQUAL : TOKEN_BINOP_OR);
        case '}': return make_token(*lexer, TOKEN_RIGHT_BRACE);
        case '~': return make_token(*lexer, TOKEN_UNARY_NEG);
    }

    // TODO: error message
    return make_token(*lexer, TOKEN_ERROR);
}
