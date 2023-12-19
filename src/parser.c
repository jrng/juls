typedef struct
{
    bool has_error;

    Token previous;
    Token current;
    Lexer lexer;

    AstBucketArray ast_nodes;

    Ast global_declarations;

    DatatypeTable datatypes;

    DatatypeId basetype_void;
    DatatypeId basetype_bool;

    DatatypeId basetype_s8;
    DatatypeId basetype_s16;
    DatatypeId basetype_s32;
    DatatypeId basetype_s64;

    DatatypeId basetype_u8;
    DatatypeId basetype_u16;
    DatatypeId basetype_u32;
    DatatypeId basetype_u64;

    DatatypeId basetype_f32;
    DatatypeId basetype_f64;
    DatatypeId basetype_string;

    // TODO: this is code generation
    u64 current_stack_offset;
    u64 stack_allocated[64];
    s32 stack_allocated_index;
} Parser;

static inline void
push_scope(Parser *parser)
{
    assert((parser->stack_allocated_index + 1) < ArrayCount(parser->stack_allocated));
    parser->stack_allocated_index += 1;
    parser->stack_allocated[parser->stack_allocated_index] = 0;
}

static inline void
pop_scope(Parser *parser)
{
    assert(parser->stack_allocated_index >= 0);
    parser->stack_allocated_index -= 1;
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

static void
report_error_valist(String source, String location, const char *message, va_list args)
{
    s64 line_indices[3] = { 0, 0, 0 };

    s64 line = 1;
    s64 character = 1;
    s64 index = 0;

    s64 token_index = location.data - source.data;

    while (index < token_index)
    {
        if (source.data[index] == '\n')
        {
            line_indices[0] = line_indices[1];
            line_indices[1] = line_indices[2];
            line_indices[2] = index + 1;
            line += 1;
            character = 0;
        }

        character += 1;
        index += 1;
    }

    fprintf(stderr, "%" PRId64 ":%" PRId64 ": error: ", line, character);
    vfprintf(stderr, message, args);
    fprintf(stderr, "\n");

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

            if (source.data[index] == '\n')
            {
                break;
            }

            index += 1;
        }

        index += 1;

        fprintf(stderr, "  %.*s", len, source.data + start_index);
    }

    fprintf(stderr, "  ");

    index = line_indices[2];

    while (index < token_index)
    {
        if (source.data[index] == '\t')
        {
            fprintf(stderr, "\t");
        }
        else
        {
            fprintf(stderr, " ");
        }

        index += 1;
    }

    for (s64 i = 0; i < location.count; i += 1)
    {
        fprintf(stderr, "^");
    }

    fprintf(stderr, "\n");
}

static void
report_error(String source, String location, const char *message, ...)
{
    va_list args;
    va_start(args, message);
    report_error_valist(source, location, message, args);
    va_end(args);
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
        String source_location = parser->previous.lexeme;

        expect_token(parser, '(');

        if (type_def)
        {
            assert(current_def);

            ast_set_left_expr(current_def, append_ast(&parser->ast_nodes, AST_KIND_QUERY_TYPE_OF, source_location));
            current_def = current_def->left_expr;
            ast_set_left_expr(current_def, parse_expression(parser));
        }
        else
        {
            type_def = append_ast(&parser->ast_nodes, AST_KIND_QUERY_TYPE_OF, source_location);
            ast_set_left_expr(type_def, parse_expression(parser));
            current_def = type_def;
        }

        expect_token(parser, ')');
    }
    else
    {
        expect_token(parser, TOKEN_IDENTIFIER);

        if (type_def)
        {
            assert(current_def);

            ast_set_left_expr(current_def, append_ast(&parser->ast_nodes, AST_KIND_IDENTIFIER, parser->previous.lexeme));
            current_def = current_def->left_expr;
        }
        else
        {
            type_def = append_ast(&parser->ast_nodes, AST_KIND_IDENTIFIER, parser->previous.lexeme);
            current_def = type_def;
        }

        current_def->name = parser->previous.lexeme;
    }

    return type_def;
}

static s64
parse_integer(String str)
{
    s64 result = 0;

    for (s64 i = 0; i < str.count; i += 1)
    {
        // TODO: check for under- or overflow
        assert((str.data[i] >= '0') && (str.data[i] <= '9'));
        result = (10 * result) + (str.data[i] - '0');
    }

    return result;
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
            expr = append_ast(&parser->ast_nodes, AST_KIND_IDENTIFIER, parser->previous.lexeme);

            expr->name = parser->previous.lexeme;
        } break;

        case TOKEN_LITERAL_STRING:
        {
            expect_token(parser, TOKEN_LITERAL_STRING);
            expr = append_ast(&parser->ast_nodes, AST_KIND_LITERAL_STRING, parser->previous.lexeme);
            expr->type_id = parser->basetype_string;

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
            expr = append_ast(&parser->ast_nodes, AST_KIND_LITERAL_INTEGER, parser->previous.lexeme);
            expr->type_id = parser->basetype_s64;

            expr->_s64 = parse_integer(parser->previous.lexeme);
        } break;

        case TOKEN_KEYWORD_TRUE:
        {
            expect_token(parser, TOKEN_KEYWORD_TRUE);
            expr = append_ast(&parser->ast_nodes, AST_KIND_LITERAL_BOOLEAN, parser->previous.lexeme);
            expr->type_id = parser->basetype_bool;

            expr->_bool = true;
        } break;

        case TOKEN_KEYWORD_FALSE:
        {
            expect_token(parser, TOKEN_KEYWORD_FALSE);
            expr = append_ast(&parser->ast_nodes, AST_KIND_LITERAL_BOOLEAN, parser->previous.lexeme);
            expr->type_id = parser->basetype_bool;

            expr->_bool = false;
        } break;

        case TOKEN_LITERAL_FLOAT:
        {
            expect_token(parser, TOKEN_LITERAL_FLOAT);
            expr = append_ast(&parser->ast_nodes, AST_KIND_LITERAL_FLOAT, parser->previous.lexeme);
            // TODO: choose correct type
            expr->type_id = parser->basetype_f32;
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
            report_error(parser->lexer.input, parser->current.lexeme, "expected a primary expression");
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

            expr = append_ast(&parser->ast_nodes, AST_KIND_QUERY_SIZE_OF, parser->previous.lexeme);

            expect_token(parser, '(');

            ast_set_type_def(expr, parse_type_definition(parser));

            expect_token(parser, ')');
        } break;

        default:
        {
            expr = parse_primary(parser);

            while (match_token(parser, '('))
            {
                Ast *function_call = append_ast(&parser->ast_nodes, AST_KIND_FUNCTION_CALL, expr->source_location);

                ast_set_left_expr(function_call, expr);
                expr = function_call;

                for (;;)
                {
                    Ast *argument = parse_expression(parser);

                    if (!argument) return 0;

                    ast_list_append(&function_call->children, argument);
                    argument->parent = function_call;

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

        Ast *expr = append_ast(&parser->ast_nodes, ast_kind, parser->previous.lexeme);

        ast_set_left_expr(expr, parse_unary(parser));

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
        String source_location = parser->previous.lexeme;

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

        expr = append_ast(&parser->ast_nodes, ast_kind, source_location);

        ast_set_left_expr(expr, left_expr);
        ast_set_right_expr(expr, right_expr);
    }

    return expr;
}

static Ast *
parse_term(Parser *parser)
{
    Ast *expr = parse_factor(parser);

    while (match_token(parser, TOKEN_BINOP_PLUS) || match_token(parser, TOKEN_BINOP_MINUS))
    {
        String source_location = parser->previous.lexeme;

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

        expr = append_ast(&parser->ast_nodes, ast_kind, source_location);

        ast_set_left_expr(expr, left_expr);
        ast_set_right_expr(expr, right_expr);
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
        String source_location = parser->previous.lexeme;

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

        expr = append_ast(&parser->ast_nodes, ast_kind, source_location);

        ast_set_left_expr(expr, left_expr);
        ast_set_right_expr(expr, right_expr);
    }

    return expr;
}

static Ast *
parse_equality(Parser *parser)
{
    Ast *expr = parse_comparison(parser);

    while (match_token(parser, TOKEN_EQUAL) || match_token(parser, TOKEN_NOT_EQUAL))
    {
        String source_location = parser->previous.lexeme;

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

        expr = append_ast(&parser->ast_nodes, ast_kind, source_location);

        ast_set_left_expr(expr, left_expr);
        ast_set_right_expr(expr, right_expr);
    }

    return expr;
}

static Ast *
parse_logic_and(Parser *parser)
{
    Ast *expr = parse_equality(parser);

    while (match_token(parser, TOKEN_LOGICAL_AND))
    {
        String source_location = parser->previous.lexeme;

        Ast *left_expr = expr;
        Ast *right_expr = parse_equality(parser);

        expr = append_ast(&parser->ast_nodes, AST_KIND_EXPRESSION_LOGIC_AND, source_location);

        ast_set_left_expr(expr, left_expr);
        ast_set_right_expr(expr, right_expr);
    }

    return expr;
}

static Ast *
parse_logic_or(Parser *parser)
{
    Ast *expr = parse_logic_and(parser);

    while (match_token(parser, TOKEN_LOGICAL_OR))
    {
        String source_location = parser->previous.lexeme;

        Ast *left_expr = expr;
        Ast *right_expr = parse_logic_and(parser);

        expr = append_ast(&parser->ast_nodes, AST_KIND_EXPRESSION_LOGIC_OR, source_location);

        ast_set_left_expr(expr, left_expr);
        ast_set_right_expr(expr, right_expr);
    }

    return expr;
}

static Ast *
parse_assignment(Parser *parser)
{
    Ast *expr = 0;

    expect_token(parser, TOKEN_IDENTIFIER);

    switch (parser->current.type)
    {
        case TOKEN_PLUS_EQUAL:
        {
            expr = append_ast(&parser->ast_nodes, AST_KIND_PLUS_ASSIGN, parser->current.lexeme);
            expr->name = parser->previous.lexeme;

            expect_token(parser, TOKEN_PLUS_EQUAL);
        } break;

        case TOKEN_MINUS_EQUAL:
        {
            expr = append_ast(&parser->ast_nodes, AST_KIND_MINUS_ASSIGN, parser->current.lexeme);
            expr->name = parser->previous.lexeme;

            expect_token(parser, TOKEN_MINUS_EQUAL);
        } break;

        case TOKEN_MUL_EQUAL:
        {
            expr = append_ast(&parser->ast_nodes, AST_KIND_MUL_ASSIGN, parser->current.lexeme);
            expr->name = parser->previous.lexeme;

            expect_token(parser, TOKEN_MUL_EQUAL);
        } break;

        case TOKEN_DIV_EQUAL:
        {
            expr = append_ast(&parser->ast_nodes, AST_KIND_DIV_ASSIGN, parser->current.lexeme);
            expr->name = parser->previous.lexeme;

            expect_token(parser, TOKEN_DIV_EQUAL);
        } break;

        case TOKEN_OR_EQUAL:
        {
            expr = append_ast(&parser->ast_nodes, AST_KIND_OR_ASSIGN, parser->current.lexeme);
            expr->name = parser->previous.lexeme;

            expect_token(parser, TOKEN_OR_EQUAL);
        } break;

        case TOKEN_AND_EQUAL:
        {
            expr = append_ast(&parser->ast_nodes, AST_KIND_AND_ASSIGN, parser->current.lexeme);
            expr->name = parser->previous.lexeme;

            expect_token(parser, TOKEN_AND_EQUAL);
        } break;

        case TOKEN_XOR_EQUAL:
        {
            expr = append_ast(&parser->ast_nodes, AST_KIND_XOR_ASSIGN, parser->current.lexeme);
            expr->name = parser->previous.lexeme;

            expect_token(parser, TOKEN_XOR_EQUAL);
        } break;

        case TOKEN_ASSIGN:
        {
            expr = append_ast(&parser->ast_nodes, AST_KIND_ASSIGN, parser->current.lexeme);
            expr->name = parser->previous.lexeme;

            expect_token(parser, TOKEN_ASSIGN);
        } break;

        default:
        {
            report_error(parser->lexer.input, parser->current.lexeme, "expected an assignment operator after an identifier");
        } break;
    }

    if (expr)
    {
        ast_set_right_expr(expr, parse_logic_or(parser));

        if (!expr->right_expr) return 0;
    }

    return expr;
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

        if ((token_type == TOKEN_ASSIGN) || (token_type == TOKEN_PLUS_EQUAL) ||
            (token_type == TOKEN_MINUS_EQUAL) || (token_type == TOKEN_MUL_EQUAL) ||
            (token_type == TOKEN_DIV_EQUAL) || (token_type == TOKEN_OR_EQUAL) ||
            (token_type == TOKEN_AND_EQUAL) || (token_type == TOKEN_XOR_EQUAL))
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
    expect_token(parser, TOKEN_IDENTIFIER);

    Ast *ast = append_ast(&parser->ast_nodes, AST_KIND_VARIABLE_DECLARATION, parser->previous.lexeme);

    ast->name = parser->previous.lexeme;
    ast->right_expr = 0;

    if (match_token(parser, TOKEN_COLON_EQUAL))
    {
        ast_set_right_expr(ast, parse_expression(parser));
    }
    else
    {
        expect_token(parser, ':');

        ast_set_type_def(ast, parse_type_definition(parser));

        if (match_token(parser, '='))
        {
            ast_set_right_expr(ast, parse_expression(parser));
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
            expect_token(parser, TOKEN_KEYWORD_IF);

            ast = append_ast(&parser->ast_nodes, AST_KIND_IF, parser->previous.lexeme);

            expect_token(parser, '(');

            ast_set_left_expr(ast, parse_expression(parser));

            expect_token(parser, ')');

            Ast *statement = parse_statement(parser);

            if (!statement) return 0;

            ast_list_append(&ast->children, statement);
            statement->parent = ast;
        } break;

        case TOKEN_KEYWORD_FOR:
        {
            expect_token(parser, TOKEN_KEYWORD_FOR);

            ast = append_ast(&parser->ast_nodes, AST_KIND_FOR, parser->previous.lexeme);

            expect_token(parser, '(');

            ast_set_decl(ast, parse_variable_declaration(parser));

            expect_token(parser, ';');

            ast_set_left_expr(ast, parse_expression(parser));

            expect_token(parser, ';');

            ast_set_right_expr(ast, parse_expression(parser));

            expect_token(parser, ')');

            Ast *statement = parse_statement(parser);

            if (!statement) return 0;

            ast_list_append(&ast->children, statement);
            statement->parent = ast;
        } break;

        case TOKEN_KEYWORD_WHILE:
        {
            assert(!"not implemented");
        } break;

        case TOKEN_KEYWORD_RETURN:
        {
            expect_token(parser, TOKEN_KEYWORD_RETURN);

            ast = append_ast(&parser->ast_nodes, AST_KIND_RETURN, parser->previous.lexeme);

            ast_set_left_expr(ast, parse_expression(parser));

            expect_token(parser, ';');
        } break;

        case '{':
        {
            expect_token(parser, '{');

            ast = append_ast(&parser->ast_nodes, AST_KIND_BLOCK, parser->previous.lexeme);

            while (parser->current.type != '}')
            {
                Ast *statement = parse_statement(parser);

                if (!statement) return 0;

                ast_list_append(&ast->children, statement);
                statement->parent = ast;
            }

            expect_token(parser, '}');
        } break;

        default:
        {
            report_error(parser->lexer.input, parser->current.lexeme, "expected a statement");
        } break;
    }

    return ast;
}

static Ast *
parse_parameter(Parser *parser)
{
    expect_token(parser, TOKEN_IDENTIFIER);

    Ast *ast = append_ast(&parser->ast_nodes, AST_KIND_VARIABLE_DECLARATION, parser->previous.lexeme);

    ast->name = parser->previous.lexeme;

    expect_token(parser, ':');

    ast_set_type_def(ast, parse_type_definition(parser));

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
        declaration = append_ast(&parser->ast_nodes, AST_KIND_STRUCT_DECLARATION, name);

        declaration->name = name;
    }
    else if (match_token(parser, '('))
    {
        declaration = append_ast(&parser->ast_nodes, AST_KIND_FUNCTION_DECLARATION, name);

        declaration->name = name;
        declaration->address = S64MAX;

        if (!match_token(parser, ')'))
        {
            for (;;)
            {
                Ast *parameter = parse_parameter(parser);

                if (!parameter) return 0;

                ast_list_append(&declaration->parameters, parameter);
                parameter->parent = declaration;

                if (!match_token(parser, ','))
                {
                    break;
                }
            }

            expect_token(parser, ')');
        }

        if (match_token(parser, TOKEN_RIGHT_ARROW))
        {
            ast_set_type_def(declaration, parse_type_definition(parser));
        }

        expect_token(parser, '{');

        while (parser->current.type != '}')
        {
            Ast *statement = parse_statement(parser);

            if (!statement) return 0;

            ast_list_append(&declaration->children, statement);
            statement->parent = declaration;
        }

        expect_token(parser, '}');
    }
    else
    {
        report_error(parser->lexer.input, parser->current.lexeme, "expected struct or function declaration");
    }

    return declaration;
}

static void
parse(Parser *parser)
{
    advance_token(parser);

    while (!match_token(parser, TOKEN_END_OF_INPUT) && !parser->has_error)
    {
        Ast *decl = parse_declaration(parser);

        if (!decl) return;

        ast_list_append(&parser->global_declarations.children, decl);
        decl->parent = &parser->global_declarations;
    }
}
