typedef enum
{
    AST_KIND_STRUCT_DECLARATION                 =  0,
    AST_KIND_FUNCTION_DECLARATION               =  1,
    AST_KIND_VARIABLE_DECLARATION               =  2,
    AST_KIND_EXPRESSION_LOGIC_OR                =  3,
    AST_KIND_EXPRESSION_LOGIC_AND               =  4,
    AST_KIND_EXPRESSION_EQUAL                   =  5,
    AST_KIND_EXPRESSION_NOT_EQUAL               =  6,
    AST_KIND_EXPRESSION_COMPARE_LESS            =  7,
    AST_KIND_EXPRESSION_COMPARE_GREATER         =  8,
    AST_KIND_EXPRESSION_COMPARE_LESS_EQUAL      =  9,
    AST_KIND_EXPRESSION_COMPARE_GREATER_EQUAL   = 10,
    AST_KIND_EXPRESSION_BINOP_ADD               = 11,
    AST_KIND_EXPRESSION_BINOP_MINUS             = 12,
    AST_KIND_EXPRESSION_BINOP_MUL               = 13,
    AST_KIND_EXPRESSION_BINOP_DIV               = 14,
    AST_KIND_EXPRESSION_UNARY_NOT               = 15,
    AST_KIND_EXPRESSION_UNARY_MINUS             = 16,
    AST_KIND_LITERAL_BOOLEAN                    = 17,
    AST_KIND_LITERAL_INTEGER                    = 18,
    AST_KIND_LITERAL_FLOAT                      = 19,
    AST_KIND_LITERAL_STRING                     = 20,
    AST_KIND_IDENTIFIER                         = 21,
    AST_KIND_QUERY_SIZE_OF                      = 22,
    AST_KIND_QUERY_TYPE_OF                      = 23,
    AST_KIND_FUNCTION_CALL                      = 24,
} AstKind;

typedef enum
{
    AST_FLAG_UNSIGNED = (1 << 0),
} AstFlag;

typedef struct Ast Ast;

typedef struct
{
    Ast *first;
    Ast *last;
} AstList;

struct Ast
{
    AstKind kind;
    u32 flags;

    Ast *next;

    String name;

    Ast *type;
    Ast *left_expr;
    Ast *right_expr;

    AstList children;

    s64 size;

    union
    {
        bool _bool;

        u8  _u8;
        u16 _u16;
        u32 _u32;
        u64 _u64;

        s8  _s8;
        s16 _s16;
        s32 _s32;
        s64 _s64;
    };
};

#define For(iter, first) for (Ast *iter = (first); iter; iter = iter->next)

typedef struct AstBucket AstBucket;

struct AstBucket
{
    s32 count;
    AstBucket *next;
    Ast nodes[64];
};

typedef struct
{
    AstBucket *first_bucket;
    AstBucket *last_bucket;
} AstBucketArray;

typedef struct
{
    bool has_error;

    Token previous;
    Token current;
    Lexer lexer;

    AstBucketArray ast_nodes;

    AstList global_declarations;
} Parser;

static inline void
ast_list_append(AstList *list, Ast *ast)
{
    if (list->last)
    {
        list->last->next = ast;
    }
    else
    {
        list->first = ast;
    }

    list->last = ast;
    ast->next = 0;
}

static Ast *
append_ast(AstBucketArray *array, AstKind ast_kind)
{
    if (!array->last_bucket || (array->last_bucket->count >= ArrayCount(array->last_bucket->nodes)))
    {
        AstBucket *new_bucket = (AstBucket *) allocate(sizeof(AstBucket));

        new_bucket->count = 0;
        new_bucket->next = 0;

        if (array->last_bucket)
        {
            array->last_bucket->next = new_bucket;
        }
        else
        {
            array->first_bucket = new_bucket;
        }

        array->last_bucket = new_bucket;
    }

    Ast *node = array->last_bucket->nodes + array->last_bucket->count;
    array->last_bucket->count += 1;

    {
        u8 *ptr = (u8 *) node;
        s64 size = sizeof(*node);
        while (size--) *ptr++ = 0;
    }

    node->kind = ast_kind;

    return node;
}

static inline void
advance_token(Parser *parser)
{
    parser->previous = parser->current;
    parser->current = get_next_token(&parser->lexer);
}

static inline bool
match_token(Parser *parser, TokenType token_type)
{
    if (parser->current.type == token_type)
    {
        advance_token(parser);
        return true;
    }

    return false;
}

static s64
get_token_line(Lexer lexer, Token token)
{
    s64 line = 1;
    s64 index = 0;

    s64 token_index = token.lexeme.data - lexer.input.data;

    while (index < token_index)
    {
        if (lexer.input.data[index] == '\n')
        {
            line += 1;
        }

        index += 1;
    }

    return line;
}

static bool
expect_token(Parser *parser, TokenType token_type)
{
    if (parser->current.type == token_type)
    {
        advance_token(parser);
        return true;
    }

    s64 line_indices[3] = { 0, 0, 0 };

    s64 line = 1;
    s64 index = 0;

    s64 token_index = parser->current.lexeme.data - parser->lexer.input.data;

    while (index < token_index)
    {
        if (parser->lexer.input.data[index] == '\n')
        {
            line_indices[0] = line_indices[1];
            line_indices[1] = line_indices[2];
            line_indices[2] = index + 1;
            line += 1;
        }

        index += 1;
    }

    fprintf(stderr, "error: expected %u at line %" PRId64 ", got '%.*s' %u\n", token_type, line, (int) parser->current.lexeme.count, parser->current.lexeme.data, parser->current.type);

    s64 lines_to_print = 3;

    if (line < lines_to_print)
    {
        lines_to_print = line;
    }

    index = line_indices[3 - lines_to_print];

    for (s64 i = 0; i < lines_to_print; i += 1)
    {
        int len = 0;
        s64 start_index = index;

        for (;;)
        {
            len += 1;

            if (parser->lexer.input.data[index] == '\n')
            {
                break;
            }

            index += 1;
        }

        index += 1;

        fprintf(stderr, "  %.*s", len, parser->lexer.input.data + start_index);
    }

    fprintf(stderr, "  ");

    index = line_indices[2];

    while (index < token_index)
    {
        if (parser->lexer.input.data[index] == '\t')
        {
            fprintf(stderr, "\t");
        }
        else
        {
            fprintf(stderr, " ");
        }

        index += 1;
    }

    for (s64 i = 0; i < parser->current.lexeme.count; i += 1)
    {
        fprintf(stderr, "^");
    }

    fprintf(stderr, "\n");

    parser->has_error = true;

    return false;
}

static Ast *parse_expression(Parser *parser);

static Ast *
parse_type_definition(Parser *parser)
{
    Ast *type_def = 0;
    Ast *current_def = 0;

    for (;;)
    {
        if (match_token(parser, '*'))
        {
            assert(!"not implemented");
        }
        else if (match_token(parser, '['))
        {
            assert(!"not implemented");
        }
        else
        {
            break;
        }
    }

    if (match_token(parser, TOKEN_KEYWORD_TYPE_OF))
    {
        expect_token(parser, '(');

        if (type_def)
        {
            assert(current_def);

            current_def->left_expr = append_ast(&parser->ast_nodes, AST_KIND_QUERY_TYPE_OF);
            current_def = current_def->left_expr;
            current_def->left_expr = parse_expression(parser);
        }
        else
        {
            type_def = append_ast(&parser->ast_nodes, AST_KIND_QUERY_TYPE_OF);
            type_def->left_expr = parse_expression(parser);
            current_def = type_def;
        }

        expect_token(parser, ')');
    }
    else
    {
        expect_token(parser, TOKEN_IDENTIFIER);
    }

    return type_def;
}

static Ast *
parse_primary(Parser *parser)
{
    Ast *expr = 0;

    switch (parser->current.type)
    {
        case TOKEN_IDENTIFIER:
        {
            expect_token(parser, TOKEN_IDENTIFIER);
            expr = append_ast(&parser->ast_nodes, AST_KIND_IDENTIFIER);

            expr->name = parser->previous.lexeme;
        } break;

        case TOKEN_LITERAL_STRING:
        {
            expect_token(parser, TOKEN_LITERAL_STRING);
            expr = append_ast(&parser->ast_nodes, AST_KIND_LITERAL_STRING);

            String value = parser->previous.lexeme;

            assert(value.count >= 2);

            value.data += 1;
            value.count -= 2;

            // TODO: escape string value

            expr->name = value;
        } break;

        case TOKEN_LITERAL_INTEGER:
        {
            expect_token(parser, TOKEN_LITERAL_INTEGER);
            expr = append_ast(&parser->ast_nodes, AST_KIND_LITERAL_INTEGER);
            // TODO: store value
        } break;

        case TOKEN_KEYWORD_TRUE:
        {
            expect_token(parser, TOKEN_KEYWORD_TRUE);
            expr = append_ast(&parser->ast_nodes, AST_KIND_LITERAL_BOOLEAN);

            expr->_bool = true;
        } break;

        case TOKEN_KEYWORD_FALSE:
        {
            expect_token(parser, TOKEN_KEYWORD_FALSE);
            expr = append_ast(&parser->ast_nodes, AST_KIND_LITERAL_BOOLEAN);

            expr->_bool = false;
        } break;

        case TOKEN_LITERAL_FLOAT:
        {
            expect_token(parser, TOKEN_LITERAL_FLOAT);
            expr = append_ast(&parser->ast_nodes, AST_KIND_LITERAL_FLOAT);
            // TODO: store value
        } break;

        case TOKEN_KEYWORD_NULL:
        {
            assert(!"not implemented");
        } break;

        case '(':
        {
            expect_token(parser, '(');
            // TODO:
            expr = parse_expression(parser);
            expect_token(parser, ')');
        } break;

        default:
        {
            fprintf(stderr, "error: expected a primary expression at line %" PRId64 ", got '%.*s' %u\n",
                    get_token_line(parser->lexer, parser->current), (int) parser->current.lexeme.count, parser->current.lexeme.data, parser->current.type);
            parser->has_error = true;
        } break;
    }

    return expr;
}

static Ast *
parse_postfix_expression(Parser *parser)
{
    Ast *expr = 0;

    switch (parser->current.type)
    {
        case TOKEN_KEYWORD_SIZE_OF:
        {
            expect_token(parser, TOKEN_KEYWORD_SIZE_OF);
            expect_token(parser, '(');

            expr = append_ast(&parser->ast_nodes, AST_KIND_QUERY_SIZE_OF);
            expr->type = parse_type_definition(parser);

            expect_token(parser, ')');
        } break;

        default:
        {
            expr = parse_primary(parser);

            while (match_token(parser, '('))
            {
                Ast *function_call = append_ast(&parser->ast_nodes, AST_KIND_FUNCTION_CALL);

                function_call->left_expr = expr;
                expr = function_call;

                for (;;)
                {
                    Ast *argument = parse_expression(parser);

                    if (!argument) return 0;

                    ast_list_append(&function_call->children, argument);

                    if (!match_token(parser, ','))
                    {
                        break;
                    }
                }

                expect_token(parser, ')');
            }
        } break;
    }

    return expr;
}

static Ast *
parse_unary(Parser *parser)
{
    if (match_token(parser, TOKEN_UNARY_NOT) || match_token(parser, TOKEN_BINOP_MINUS))
    {
        AstKind ast_kind;

        if (parser->previous.type == TOKEN_UNARY_NOT)
        {
            ast_kind = AST_KIND_EXPRESSION_UNARY_NOT;
        }
        else
        {
            assert(parser->previous.type == TOKEN_BINOP_MINUS);
            ast_kind = AST_KIND_EXPRESSION_UNARY_MINUS;
        }

        Ast *expr = append_ast(&parser->ast_nodes, ast_kind);

        expr->left_expr = parse_unary(parser);

        return expr;
    }
    else
    {
        return parse_postfix_expression(parser);
    }
}

static Ast *
parse_factor(Parser *parser)
{
    Ast *expr = parse_unary(parser);

    while (match_token(parser, TOKEN_BINOP_MUL) || match_token(parser, TOKEN_BINOP_DIV))
    {
        AstKind ast_kind;

        if (parser->previous.type == TOKEN_BINOP_MUL)
        {
            ast_kind = AST_KIND_EXPRESSION_BINOP_MUL;
        }
        else
        {
            assert(parser->previous.type == TOKEN_BINOP_DIV);
            ast_kind = AST_KIND_EXPRESSION_BINOP_DIV;
        }

        Ast *left_expr = expr;
        Ast *right_expr = parse_unary(parser);

        expr = append_ast(&parser->ast_nodes, ast_kind);

        expr->left_expr = left_expr;
        expr->right_expr = right_expr;
    }

    return expr;
}

static Ast *
parse_term(Parser *parser)
{
    Ast *expr = parse_factor(parser);

    while (match_token(parser, TOKEN_BINOP_PLUS) || match_token(parser, TOKEN_BINOP_MINUS))
    {
        AstKind ast_kind;

        if (parser->previous.type == TOKEN_BINOP_PLUS)
        {
            ast_kind = AST_KIND_EXPRESSION_BINOP_ADD;
        }
        else
        {
            assert(parser->previous.type == TOKEN_BINOP_MINUS);
            ast_kind = AST_KIND_EXPRESSION_BINOP_MINUS;
        }

        Ast *left_expr = expr;
        Ast *right_expr = parse_factor(parser);

        expr = append_ast(&parser->ast_nodes, ast_kind);

        expr->left_expr = left_expr;
        expr->right_expr = right_expr;
    }

    return expr;
}

static Ast *
parse_comparison(Parser *parser)
{
    Ast *expr = parse_term(parser);

    while (match_token(parser, TOKEN_LESS) || match_token(parser, TOKEN_GREATER) ||
           match_token(parser, TOKEN_LESS_EQUAL) || match_token(parser, TOKEN_GREATER_EQUAL))
    {
        AstKind ast_kind;

        switch (parser->previous.type)
        {
            case TOKEN_LESS:            ast_kind = AST_KIND_EXPRESSION_COMPARE_LESS;            break;
            case TOKEN_GREATER:         ast_kind = AST_KIND_EXPRESSION_COMPARE_GREATER;         break;
            case TOKEN_LESS_EQUAL:      ast_kind = AST_KIND_EXPRESSION_COMPARE_LESS_EQUAL;      break;
            case TOKEN_GREATER_EQUAL:   ast_kind = AST_KIND_EXPRESSION_COMPARE_GREATER_EQUAL;   break;
            default: assert(!"");
        }

        Ast *left_expr = expr;
        Ast *right_expr = parse_term(parser);

        expr = append_ast(&parser->ast_nodes, ast_kind);

        expr->left_expr = left_expr;
        expr->right_expr = right_expr;
    }

    return expr;
}

static Ast *
parse_equality(Parser *parser)
{
    Ast *expr = parse_comparison(parser);

    while (match_token(parser, TOKEN_EQUAL) || match_token(parser, TOKEN_NOT_EQUAL))
    {
        AstKind ast_kind;

        if (parser->previous.type == TOKEN_EQUAL)
        {
            ast_kind = AST_KIND_EXPRESSION_EQUAL;
        }
        else
        {
            assert(parser->previous.type == TOKEN_NOT_EQUAL);
            ast_kind = AST_KIND_EXPRESSION_NOT_EQUAL;
        }

        Ast *left_expr = expr;
        Ast *right_expr = parse_comparison(parser);

        expr = append_ast(&parser->ast_nodes, ast_kind);

        expr->left_expr = left_expr;
        expr->right_expr = right_expr;
    }

    return expr;
}

static Ast *
parse_logic_and(Parser *parser)
{
    Ast *expr = parse_equality(parser);

    while (match_token(parser, TOKEN_LOGICAL_AND))
    {
        Ast *left_expr = expr;
        Ast *right_expr = parse_equality(parser);

        expr = append_ast(&parser->ast_nodes, AST_KIND_EXPRESSION_LOGIC_AND);

        expr->left_expr = left_expr;
        expr->right_expr = right_expr;
    }

    return expr;
}

static Ast *
parse_logic_or(Parser *parser)
{
    Ast *expr = parse_logic_and(parser);

    while (match_token(parser, TOKEN_LOGICAL_OR))
    {
        Ast *left_expr = expr;
        Ast *right_expr = parse_logic_and(parser);

        expr = append_ast(&parser->ast_nodes, AST_KIND_EXPRESSION_LOGIC_OR);

        expr->left_expr = left_expr;
        expr->right_expr = right_expr;
    }

    return expr;
}

static Ast *
parse_assignment(Parser *parser)
{
    assert(!"not implemented");
    return 0;
}

static inline void
rollback_one_token(Parser *parser)
{
    parser->lexer.current = parser->current.lexeme.data - parser->lexer.input.data;
    parser->current = parser->previous;
}

static Ast *
parse_expression(Parser *parser)
{
    Ast *ast = 0;

    if (parser->current.type == TOKEN_IDENTIFIER)
    {
        expect_token(parser, TOKEN_IDENTIFIER);

        TokenType token_type = parser->current.type;

        rollback_one_token(parser);

        if (token_type == TOKEN_ASSIGN)
        {
            ast = parse_assignment(parser);
        }
        else
        {
            ast = parse_logic_or(parser);
        }
    }
    else
    {
        ast = parse_logic_or(parser);
    }

    return ast;
}

static Ast *
parse_variable_declaration(Parser *parser)
{
    Ast *ast = append_ast(&parser->ast_nodes, AST_KIND_VARIABLE_DECLARATION);

    expect_token(parser, TOKEN_IDENTIFIER);

    ast->name = parser->previous.lexeme;
    ast->right_expr = 0;

    if (match_token(parser, TOKEN_COLON_EQUAL))
    {
        ast->right_expr = parse_expression(parser);
    }
    else
    {
        expect_token(parser, ':');

        ast->type = parse_type_definition(parser);

        if (match_token(parser, '='))
        {
            ast->right_expr = parse_expression(parser);
        }
    }

    return ast;
}

static Ast *
parse_statement(Parser *parser)
{
    Ast *ast = 0;

    switch (parser->current.type)
    {
        case TOKEN_IDENTIFIER:
        {
            expect_token(parser, TOKEN_IDENTIFIER);

            TokenType token_type = parser->current.type;

            rollback_one_token(parser);

            if ((token_type == TOKEN_COLON) || (token_type == TOKEN_COLON_EQUAL))
            {
                ast = parse_variable_declaration(parser);
            }
            else
            {
                ast = parse_expression(parser);
            }

            expect_token(parser, ';');
        } break;

        case TOKEN_KEYWORD_IF:
        {
            assert(!"not implemented");
        } break;

        case TOKEN_KEYWORD_FOR:
        {
            assert(!"not implemented");
        } break;

        case TOKEN_KEYWORD_WHILE:
        {
            assert(!"not implemented");
        } break;

#if 0
        case TOKEN_KEYWORD_RETURN:
        {
            assert(!"not implemented");
        } break;
#endif

        case '{':
        {
            assert(!"not implemented");
        } break;

        default:
        {
            fprintf(stderr, "error: expected a statement at line %" PRId64 ", got '%.*s' %u\n",
                    get_token_line(parser->lexer, parser->current), (int) parser->current.lexeme.count, parser->current.lexeme.data, parser->current.type);
            parser->has_error = true;
        } break;
    }

    return ast;
}

static Ast *
parse_declaration(Parser *parser)
{
    expect_token(parser, TOKEN_IDENTIFIER);

    String name = parser->previous.lexeme;

    expect_token(parser, TOKEN_COLON_COLON);

    Ast *declaration = 0;

    if (match_token(parser, TOKEN_KEYWORD_STRUCT))
    {
        declaration = append_ast(&parser->ast_nodes, AST_KIND_STRUCT_DECLARATION);

        declaration->name = name;
    }
    else if (match_token(parser, '('))
    {
        declaration = append_ast(&parser->ast_nodes, AST_KIND_FUNCTION_DECLARATION);

        declaration->name = name;

        expect_token(parser, ')');

        // TODO: parse return type

        expect_token(parser, '{');

        while (parser->current.type != '}')
        {
            Ast *statement = parse_statement(parser);

            if (!statement) return 0;

            ast_list_append(&declaration->children, statement);
        }

        expect_token(parser, '}');
    }
    else
    {
        fprintf(stderr, "error: expected struct or function declaration at line %" PRId64 ", got '%.*s' %u\n",
                get_token_line(parser->lexer, parser->current), (int) parser->current.lexeme.count, parser->current.lexeme.data, parser->current.type);
        parser->has_error = true;
    }

    return declaration;
}

static void
print_ast(Ast *ast, s32 indent)
{
    switch (ast->kind)
    {
        case AST_KIND_FUNCTION_DECLARATION:
        {
            fprintf(stderr, "%*sFunctionDeclaration '%.*s'\n", indent, "", (int) ast->name.count, ast->name.data);
            fprintf(stderr, "%*s{\n", indent, "");

            For(elem, ast->children.first)
            {
                print_ast(elem, indent + 2);
            }

            fprintf(stderr, "%*s}\n", indent, "");
        } break;

        case AST_KIND_VARIABLE_DECLARATION:
        {
            fprintf(stderr, "%*sVariableDeclaration '%.*s'\n", indent, "", (int) ast->name.count, ast->name.data);

            if (ast->right_expr)
            {
                print_ast(ast->right_expr, indent + 2);
            }
        } break;

        case AST_KIND_LITERAL_BOOLEAN:
        {
            fprintf(stderr, "%*sLiteralBoolean: %s\n", indent, "", ast->_bool ? "true" : "false");
        } break;

        case AST_KIND_LITERAL_INTEGER:
        {
            fprintf(stderr, "%*sLiteralInteger\n", indent, "");
        } break;

        case AST_KIND_LITERAL_STRING:
        {
            fprintf(stderr, "%*sLiteralString '%.*s'\n", indent, "", (int) ast->name.count, ast->name.data);
        } break;

        case AST_KIND_IDENTIFIER:
        {
            fprintf(stderr, "%*sIdentifier '%.*s'\n", indent, "", (int) ast->name.count, ast->name.data);
        } break;

        case AST_KIND_QUERY_SIZE_OF:
        {
            fprintf(stderr, "%*sSizeOf\n", indent, "");
            print_ast(ast->type, indent + 2);
        } break;

        case AST_KIND_QUERY_TYPE_OF:
        {
            fprintf(stderr, "%*sTypeOf\n", indent, "");
            print_ast(ast->left_expr, indent + 2);
        } break;

        case AST_KIND_FUNCTION_CALL:
        {
            fprintf(stderr, "%*sFunctionCall\n", indent, "");

            print_ast(ast->left_expr, indent + 2);

            fprintf(stderr, "%*s(\n", indent, "");

            For(elem, ast->children.first)
            {
                print_ast(elem, indent + 2);
            }

            fprintf(stderr, "%*s)\n", indent, "");
        } break;

        default:
        {
            fprintf(stderr, "%*sunknown ast kind: %u\n", indent, "", ast->kind);
        } break;
    }
}

static void
parse(Parser *parser)
{
    advance_token(parser);

    while (!match_token(parser, TOKEN_END_OF_INPUT) && !parser->has_error)
    {
        Ast *decl = parse_declaration(parser);

        if (!decl) return;

        ast_list_append(&parser->global_declarations, decl);
    }
}
