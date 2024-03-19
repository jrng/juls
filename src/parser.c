typedef struct
{
    bool has_error;

    Token previous;
    Token current;
    Lexer lexer;
} Parser;

typedef struct
{
    Parser parser;

    String current_directory;
    SourceFileArray source_files;

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
} Compiler;

static inline SourceLocation
make_source_location(Parser *parser, String location)
{
    String source = parser->lexer.input;

    assert(location.data >= source.data);
    assert((location.data + location.count) <= (source.data + source.count));

    SourceLocation source_location;

    source_location.file_index = parser->lexer.current_file_index;
    source_location.index = location.data - source.data;
    source_location.count = location.count;

    return source_location;
}

static inline void
push_scope(Compiler *compiler)
{
    assert((compiler->stack_allocated_index + 1) < ArrayCount(compiler->stack_allocated));
    compiler->stack_allocated_index += 1;
    compiler->stack_allocated[compiler->stack_allocated_index] = 0;
}

static inline void
pop_scope(Compiler *compiler)
{
    assert(compiler->stack_allocated_index >= 0);
    compiler->stack_allocated_index -= 1;
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
report_error_valist(Compiler compiler, SourceLocation location, const char *message, va_list args)
{
    s64 line_indices[3] = { 0, 0, 0 };

    s64 line = 1;
    s64 character = 1;
    s64 index = 0;

    assert(location.file_index < compiler.source_files.count);

    SourceFile source_file = compiler.source_files.items[location.file_index];

    String source = source_file.content;

    while (index < location.index)
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

    fprintf(stderr, "%.*s:%" PRId64 ":%" PRId64 ": error: ",
            (int) source_file.full_path.count, source_file.full_path.data, line, character);
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

    while (index < location.index)
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
report_error(Compiler compiler, SourceLocation location, const char *message, ...)
{
    va_list args;
    va_start(args, message);
    report_error_valist(compiler, location, message, args);
    va_end(args);
}

static bool
expect_token(Compiler *compiler, TokenType token_type)
{
    if (compiler->parser.current.type == token_type)
    {
        advance_token(&compiler->parser);
        return true;
    }

    String lexeme = compiler->parser.current.lexeme;

    report_error(*compiler, make_source_location(&compiler->parser, lexeme),
                 "expected %u, got '%.*s' %u", token_type, (int) lexeme.count, lexeme.data,
                 compiler->parser.current.type);

    compiler->parser.has_error = true;

    return false;
}

static Ast *parse_expression(Compiler *compiler);

static Ast *
parse_type_definition(Compiler *compiler)
{
    Ast *type_def = 0;
    Ast *current_def = 0;

    for (;;)
    {
        if (match_token(&compiler->parser, '*'))
        {
            if (type_def)
            {
                assert(current_def);

                ast_set_left_expr(current_def, append_ast(&compiler->ast_nodes, AST_KIND_POINTER, make_source_location(&compiler->parser, compiler->parser.previous.lexeme)));
                current_def = current_def->left_expr;
            }
            else
            {
                type_def = append_ast(&compiler->ast_nodes, AST_KIND_POINTER, make_source_location(&compiler->parser, compiler->parser.previous.lexeme));
                current_def = type_def;
            }
        }
        else if (match_token(&compiler->parser, '['))
        {
            assert(!"not implemented");
        }
        else
        {
            break;
        }
    }

    if (match_token(&compiler->parser, TOKEN_KEYWORD_TYPE_OF))
    {
        SourceLocation source_location = make_source_location(&compiler->parser, compiler->parser.previous.lexeme);

        expect_token(compiler, '(');

        if (type_def)
        {
            assert(current_def);

            ast_set_left_expr(current_def, append_ast(&compiler->ast_nodes, AST_KIND_QUERY_TYPE_OF, source_location));
            current_def = current_def->left_expr;
            ast_set_left_expr(current_def, parse_expression(compiler));
        }
        else
        {
            type_def = append_ast(&compiler->ast_nodes, AST_KIND_QUERY_TYPE_OF, source_location);
            ast_set_left_expr(type_def, parse_expression(compiler));
            current_def = type_def;
        }

        expect_token(compiler, ')');
    }
    else
    {
        expect_token(compiler, TOKEN_IDENTIFIER);

        if (type_def)
        {
            assert(current_def);

            ast_set_left_expr(current_def, append_ast(&compiler->ast_nodes, AST_KIND_IDENTIFIER, make_source_location(&compiler->parser, compiler->parser.previous.lexeme)));
            current_def = current_def->left_expr;
        }
        else
        {
            type_def = append_ast(&compiler->ast_nodes, AST_KIND_IDENTIFIER, make_source_location(&compiler->parser, compiler->parser.previous.lexeme));
            current_def = type_def;
        }

        current_def->name = compiler->parser.previous.lexeme;
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
parse_primary(Compiler *compiler)
{
    Ast *expr = 0;

    switch (compiler->parser.current.type)
    {
        case TOKEN_IDENTIFIER:
        {
            expect_token(compiler, TOKEN_IDENTIFIER);
            expr = append_ast(&compiler->ast_nodes, AST_KIND_IDENTIFIER, make_source_location(&compiler->parser, compiler->parser.previous.lexeme));

            expr->name = compiler->parser.previous.lexeme;
        } break;

        case TOKEN_LITERAL_STRING:
        {
            expect_token(compiler, TOKEN_LITERAL_STRING);
            expr = append_ast(&compiler->ast_nodes, AST_KIND_LITERAL_STRING, make_source_location(&compiler->parser, compiler->parser.previous.lexeme));
            expr->type_id = compiler->basetype_string;

            String value = compiler->parser.previous.lexeme;

            assert(value.count >= 2);

            value.data += 1;
            value.count -= 2;

            expr->name.count = 0;
            expr->name.data = alloc(&default_allocator, value.count, 8, false);

            bool escaped = false;

            for (s64 i = 0; i < value.count; i += 1)
            {
                u8 c = value.data[i];

                if (escaped)
                {
                    switch (c)
                    {
                        case 'n':
                        {
                            expr->name.data[expr->name.count] = '\n';
                            expr->name.count += 1;
                        } break;
                    }

                    escaped = false;
                }
                else
                {
                    if (c == '\\')
                    {
                        escaped = true;
                    }
                    else
                    {
                        expr->name.data[expr->name.count] = value.data[i];
                        expr->name.count += 1;
                    }
                }
            }
        } break;

        case TOKEN_LITERAL_INTEGER:
        {
            expect_token(compiler, TOKEN_LITERAL_INTEGER);
            expr = append_ast(&compiler->ast_nodes, AST_KIND_LITERAL_INTEGER, make_source_location(&compiler->parser, compiler->parser.previous.lexeme));
            expr->type_id = compiler->basetype_s64;

            expr->_s64 = parse_integer(compiler->parser.previous.lexeme);
        } break;

        case TOKEN_KEYWORD_TRUE:
        {
            expect_token(compiler, TOKEN_KEYWORD_TRUE);
            expr = append_ast(&compiler->ast_nodes, AST_KIND_LITERAL_BOOLEAN, make_source_location(&compiler->parser, compiler->parser.previous.lexeme));
            expr->type_id = compiler->basetype_bool;

            expr->_bool = true;
        } break;

        case TOKEN_KEYWORD_FALSE:
        {
            expect_token(compiler, TOKEN_KEYWORD_FALSE);
            expr = append_ast(&compiler->ast_nodes, AST_KIND_LITERAL_BOOLEAN, make_source_location(&compiler->parser, compiler->parser.previous.lexeme));
            expr->type_id = compiler->basetype_bool;

            expr->_bool = false;
        } break;

        case TOKEN_LITERAL_FLOAT:
        {
            expect_token(compiler, TOKEN_LITERAL_FLOAT);
            expr = append_ast(&compiler->ast_nodes, AST_KIND_LITERAL_FLOAT, make_source_location(&compiler->parser, compiler->parser.previous.lexeme));
            // TODO: choose correct type
            expr->type_id = compiler->basetype_f32;
            // TODO: store value
        } break;

        case TOKEN_KEYWORD_NULL:
        {
            assert(!"not implemented");
        } break;

        case '(':
        {
            expect_token(compiler, '(');
            // TODO:
            expr = parse_expression(compiler);
            expect_token(compiler, ')');
        } break;

        default:
        {
            report_error(*compiler, make_source_location(&compiler->parser, compiler->parser.current.lexeme), "expected a primary expression");
            compiler->parser.has_error = true;
        } break;
    }

    return expr;
}

static Ast *
parse_postfix_expression(Compiler *compiler)
{
    Ast *expr = parse_primary(compiler);

    for (;;)
    {
        if (match_token(&compiler->parser, '('))
        {
            Ast *function_call = append_ast(&compiler->ast_nodes, AST_KIND_FUNCTION_CALL, expr->source_location);

            ast_set_left_expr(function_call, expr);
            expr = function_call;

            for (;;)
            {
                Ast *argument = parse_expression(compiler);

                if (!argument) return 0;

                ast_list_append(&function_call->children, argument);
                argument->parent = function_call;

                if (!match_token(&compiler->parser, ','))
                {
                    break;
                }
            }

            expect_token(compiler, ')');
        }
        else if (match_token(&compiler->parser, '.'))
        {
            expect_token(compiler, TOKEN_IDENTIFIER);

            Ast *member = append_ast(&compiler->ast_nodes, AST_KIND_MEMBER, make_source_location(&compiler->parser, compiler->parser.previous.lexeme));

            member->name = compiler->parser.previous.lexeme;

            ast_set_left_expr(member, expr);
            expr = member;
        }
        else
        {
            break;
        }
    }

    return expr;
}

static Ast *
parse_unary(Compiler *compiler)
{
    if (match_token(&compiler->parser, TOKEN_UNARY_NOT) || match_token(&compiler->parser, TOKEN_BINOP_MINUS))
    {
        AstKind ast_kind;

        if (compiler->parser.previous.type == TOKEN_UNARY_NOT)
        {
            ast_kind = AST_KIND_EXPRESSION_UNARY_NOT;
        }
        else
        {
            assert(compiler->parser.previous.type == TOKEN_BINOP_MINUS);
            ast_kind = AST_KIND_EXPRESSION_UNARY_MINUS;
        }

        Ast *expr = append_ast(&compiler->ast_nodes, ast_kind, make_source_location(&compiler->parser, compiler->parser.previous.lexeme));

        ast_set_left_expr(expr, parse_unary(compiler));

        return expr;
    }
    else if (match_token(&compiler->parser, TOKEN_KEYWORD_CAST))
    {
        Ast *expr = append_ast(&compiler->ast_nodes, AST_KIND_CAST, make_source_location(&compiler->parser, compiler->parser.previous.lexeme));

        expect_token(compiler, '(');

        ast_set_type_def(expr, parse_type_definition(compiler));

        expect_token(compiler, ')');

        ast_set_left_expr(expr, parse_unary(compiler));

        return expr;
    }
    else if (match_token(&compiler->parser, TOKEN_KEYWORD_SIZE_OF))
    {
        Ast *expr = append_ast(&compiler->ast_nodes, AST_KIND_QUERY_SIZE_OF, make_source_location(&compiler->parser, compiler->parser.previous.lexeme));

        expect_token(compiler, '(');

        ast_set_type_def(expr, parse_type_definition(compiler));

        expect_token(compiler, ')');

        return expr;
    }
    else
    {
        return parse_postfix_expression(compiler);
    }
}

static Ast *
parse_factor(Compiler *compiler)
{
    Ast *expr = parse_unary(compiler);

    while (match_token(&compiler->parser, TOKEN_BINOP_MUL) || match_token(&compiler->parser, TOKEN_BINOP_DIV))
    {
        SourceLocation source_location = make_source_location(&compiler->parser, compiler->parser.previous.lexeme);

        AstKind ast_kind;

        if (compiler->parser.previous.type == TOKEN_BINOP_MUL)
        {
            ast_kind = AST_KIND_EXPRESSION_BINOP_MUL;
        }
        else
        {
            assert(compiler->parser.previous.type == TOKEN_BINOP_DIV);
            ast_kind = AST_KIND_EXPRESSION_BINOP_DIV;
        }

        Ast *left_expr = expr;
        Ast *right_expr = parse_unary(compiler);

        expr = append_ast(&compiler->ast_nodes, ast_kind, source_location);

        ast_set_left_expr(expr, left_expr);
        ast_set_right_expr(expr, right_expr);
    }

    return expr;
}

static Ast *
parse_term(Compiler *compiler)
{
    Ast *expr = parse_factor(compiler);

    while (match_token(&compiler->parser, TOKEN_BINOP_PLUS) || match_token(&compiler->parser, TOKEN_BINOP_MINUS))
    {
        SourceLocation source_location = make_source_location(&compiler->parser, compiler->parser.previous.lexeme);

        AstKind ast_kind;

        if (compiler->parser.previous.type == TOKEN_BINOP_PLUS)
        {
            ast_kind = AST_KIND_EXPRESSION_BINOP_ADD;
        }
        else
        {
            assert(compiler->parser.previous.type == TOKEN_BINOP_MINUS);
            ast_kind = AST_KIND_EXPRESSION_BINOP_MINUS;
        }

        Ast *left_expr = expr;
        Ast *right_expr = parse_factor(compiler);

        expr = append_ast(&compiler->ast_nodes, ast_kind, source_location);

        ast_set_left_expr(expr, left_expr);
        ast_set_right_expr(expr, right_expr);
    }

    return expr;
}

static Ast *
parse_comparison(Compiler *compiler)
{
    Ast *expr = parse_term(compiler);

    while (match_token(&compiler->parser, TOKEN_LESS) || match_token(&compiler->parser, TOKEN_GREATER) ||
           match_token(&compiler->parser, TOKEN_LESS_EQUAL) || match_token(&compiler->parser, TOKEN_GREATER_EQUAL))
    {
        SourceLocation source_location = make_source_location(&compiler->parser, compiler->parser.previous.lexeme);

        AstKind ast_kind;

        switch (compiler->parser.previous.type)
        {
            case TOKEN_LESS:            ast_kind = AST_KIND_EXPRESSION_COMPARE_LESS;            break;
            case TOKEN_GREATER:         ast_kind = AST_KIND_EXPRESSION_COMPARE_GREATER;         break;
            case TOKEN_LESS_EQUAL:      ast_kind = AST_KIND_EXPRESSION_COMPARE_LESS_EQUAL;      break;
            case TOKEN_GREATER_EQUAL:   ast_kind = AST_KIND_EXPRESSION_COMPARE_GREATER_EQUAL;   break;
            default: assert(!"");
        }

        Ast *left_expr = expr;
        Ast *right_expr = parse_term(compiler);

        expr = append_ast(&compiler->ast_nodes, ast_kind, source_location);

        ast_set_left_expr(expr, left_expr);
        ast_set_right_expr(expr, right_expr);
    }

    return expr;
}

static Ast *
parse_equality(Compiler *compiler)
{
    Ast *expr = parse_comparison(compiler);

    while (match_token(&compiler->parser, TOKEN_EQUAL) || match_token(&compiler->parser, TOKEN_NOT_EQUAL))
    {
        SourceLocation source_location = make_source_location(&compiler->parser, compiler->parser.previous.lexeme);

        AstKind ast_kind;

        if (compiler->parser.previous.type == TOKEN_EQUAL)
        {
            ast_kind = AST_KIND_EXPRESSION_EQUAL;
        }
        else
        {
            assert(compiler->parser.previous.type == TOKEN_NOT_EQUAL);
            ast_kind = AST_KIND_EXPRESSION_NOT_EQUAL;
        }

        Ast *left_expr = expr;
        Ast *right_expr = parse_comparison(compiler);

        expr = append_ast(&compiler->ast_nodes, ast_kind, source_location);

        ast_set_left_expr(expr, left_expr);
        ast_set_right_expr(expr, right_expr);
    }

    return expr;
}

static Ast *
parse_logic_and(Compiler *compiler)
{
    Ast *expr = parse_equality(compiler);

    while (match_token(&compiler->parser, TOKEN_LOGICAL_AND))
    {
        SourceLocation source_location = make_source_location(&compiler->parser, compiler->parser.previous.lexeme);

        Ast *left_expr = expr;
        Ast *right_expr = parse_equality(compiler);

        expr = append_ast(&compiler->ast_nodes, AST_KIND_EXPRESSION_LOGIC_AND, source_location);

        ast_set_left_expr(expr, left_expr);
        ast_set_right_expr(expr, right_expr);
    }

    return expr;
}

static Ast *
parse_logic_or(Compiler *compiler)
{
    Ast *expr = parse_logic_and(compiler);

    while (match_token(&compiler->parser, TOKEN_LOGICAL_OR))
    {
        SourceLocation source_location = make_source_location(&compiler->parser, compiler->parser.previous.lexeme);

        Ast *left_expr = expr;
        Ast *right_expr = parse_logic_and(compiler);

        expr = append_ast(&compiler->ast_nodes, AST_KIND_EXPRESSION_LOGIC_OR, source_location);

        ast_set_left_expr(expr, left_expr);
        ast_set_right_expr(expr, right_expr);
    }

    return expr;
}

static Ast *
parse_assignment(Compiler *compiler)
{
    Ast *expr = 0;

    expect_token(compiler, TOKEN_IDENTIFIER);

    switch (compiler->parser.current.type)
    {
        case TOKEN_PLUS_EQUAL:
        {
            expr = append_ast(&compiler->ast_nodes, AST_KIND_PLUS_ASSIGN, make_source_location(&compiler->parser, compiler->parser.current.lexeme));
            expr->name = compiler->parser.previous.lexeme;

            expect_token(compiler, TOKEN_PLUS_EQUAL);
        } break;

        case TOKEN_MINUS_EQUAL:
        {
            expr = append_ast(&compiler->ast_nodes, AST_KIND_MINUS_ASSIGN, make_source_location(&compiler->parser, compiler->parser.current.lexeme));
            expr->name = compiler->parser.previous.lexeme;

            expect_token(compiler, TOKEN_MINUS_EQUAL);
        } break;

        case TOKEN_MUL_EQUAL:
        {
            expr = append_ast(&compiler->ast_nodes, AST_KIND_MUL_ASSIGN, make_source_location(&compiler->parser, compiler->parser.current.lexeme));
            expr->name = compiler->parser.previous.lexeme;

            expect_token(compiler, TOKEN_MUL_EQUAL);
        } break;

        case TOKEN_DIV_EQUAL:
        {
            expr = append_ast(&compiler->ast_nodes, AST_KIND_DIV_ASSIGN, make_source_location(&compiler->parser, compiler->parser.current.lexeme));
            expr->name = compiler->parser.previous.lexeme;

            expect_token(compiler, TOKEN_DIV_EQUAL);
        } break;

        case TOKEN_OR_EQUAL:
        {
            expr = append_ast(&compiler->ast_nodes, AST_KIND_OR_ASSIGN, make_source_location(&compiler->parser, compiler->parser.current.lexeme));
            expr->name = compiler->parser.previous.lexeme;

            expect_token(compiler, TOKEN_OR_EQUAL);
        } break;

        case TOKEN_AND_EQUAL:
        {
            expr = append_ast(&compiler->ast_nodes, AST_KIND_AND_ASSIGN, make_source_location(&compiler->parser, compiler->parser.current.lexeme));
            expr->name = compiler->parser.previous.lexeme;

            expect_token(compiler, TOKEN_AND_EQUAL);
        } break;

        case TOKEN_XOR_EQUAL:
        {
            expr = append_ast(&compiler->ast_nodes, AST_KIND_XOR_ASSIGN, make_source_location(&compiler->parser, compiler->parser.current.lexeme));
            expr->name = compiler->parser.previous.lexeme;

            expect_token(compiler, TOKEN_XOR_EQUAL);
        } break;

        case TOKEN_ASSIGN:
        {
            expr = append_ast(&compiler->ast_nodes, AST_KIND_ASSIGN, make_source_location(&compiler->parser, compiler->parser.current.lexeme));
            expr->name = compiler->parser.previous.lexeme;

            expect_token(compiler, TOKEN_ASSIGN);
        } break;

        default:
        {
            report_error(*compiler, make_source_location(&compiler->parser, compiler->parser.current.lexeme), "expected an assignment operator after an identifier");
        } break;
    }

    if (expr)
    {
        ast_set_right_expr(expr, parse_logic_or(compiler));

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
parse_expression(Compiler *compiler)
{
    Ast *ast = 0;

    if (compiler->parser.current.type == TOKEN_IDENTIFIER)
    {
        expect_token(compiler, TOKEN_IDENTIFIER);

        TokenType token_type = compiler->parser.current.type;

        rollback_one_token(&compiler->parser);

        if ((token_type == TOKEN_ASSIGN) || (token_type == TOKEN_PLUS_EQUAL) ||
            (token_type == TOKEN_MINUS_EQUAL) || (token_type == TOKEN_MUL_EQUAL) ||
            (token_type == TOKEN_DIV_EQUAL) || (token_type == TOKEN_OR_EQUAL) ||
            (token_type == TOKEN_AND_EQUAL) || (token_type == TOKEN_XOR_EQUAL))
        {
            ast = parse_assignment(compiler);
        }
        else
        {
            ast = parse_logic_or(compiler);
        }
    }
    else
    {
        ast = parse_logic_or(compiler);
    }

    return ast;
}

static Ast *
parse_variable_declaration(Compiler *compiler)
{
    expect_token(compiler, TOKEN_IDENTIFIER);

    Ast *ast = append_ast(&compiler->ast_nodes, AST_KIND_VARIABLE_DECLARATION, make_source_location(&compiler->parser, compiler->parser.previous.lexeme));

    ast->name = compiler->parser.previous.lexeme;
    ast->right_expr = 0;

    if (match_token(&compiler->parser, TOKEN_COLON_EQUAL))
    {
        ast_set_right_expr(ast, parse_expression(compiler));
    }
    else
    {
        expect_token(compiler, ':');

        ast_set_type_def(ast, parse_type_definition(compiler));

        if (match_token(&compiler->parser, '='))
        {
            ast_set_right_expr(ast, parse_expression(compiler));
        }
    }

    return ast;
}

static Ast *
parse_statement(Compiler *compiler)
{
    Ast *ast = 0;

    switch (compiler->parser.current.type)
    {
        case TOKEN_IDENTIFIER:
        {
            expect_token(compiler, TOKEN_IDENTIFIER);

            TokenType token_type = compiler->parser.current.type;

            rollback_one_token(&compiler->parser);

            if ((token_type == TOKEN_COLON) || (token_type == TOKEN_COLON_EQUAL))
            {
                ast = parse_variable_declaration(compiler);
            }
            else
            {
                ast = parse_expression(compiler);
            }

            expect_token(compiler, ';');
        } break;

        case TOKEN_KEYWORD_IF:
        {
            expect_token(compiler, TOKEN_KEYWORD_IF);

            ast = append_ast(&compiler->ast_nodes, AST_KIND_IF, make_source_location(&compiler->parser, compiler->parser.previous.lexeme));

            expect_token(compiler, '(');

            ast_set_left_expr(ast, parse_expression(compiler));

            expect_token(compiler, ')');

            Ast *statement = parse_statement(compiler);

            if (!statement) return 0;

            ast_list_append(&ast->children, statement);
            statement->parent = ast;
        } break;

        case TOKEN_KEYWORD_FOR:
        {
            expect_token(compiler, TOKEN_KEYWORD_FOR);

            ast = append_ast(&compiler->ast_nodes, AST_KIND_FOR, make_source_location(&compiler->parser, compiler->parser.previous.lexeme));

            expect_token(compiler, '(');

            ast_set_decl(ast, parse_variable_declaration(compiler));

            expect_token(compiler, ';');

            ast_set_left_expr(ast, parse_expression(compiler));

            expect_token(compiler, ';');

            ast_set_right_expr(ast, parse_expression(compiler));

            expect_token(compiler, ')');

            Ast *statement = parse_statement(compiler);

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
            expect_token(compiler, TOKEN_KEYWORD_RETURN);

            ast = append_ast(&compiler->ast_nodes, AST_KIND_RETURN, make_source_location(&compiler->parser, compiler->parser.previous.lexeme));

            ast_set_left_expr(ast, parse_expression(compiler));

            expect_token(compiler, ';');
        } break;

        case '{':
        {
            expect_token(compiler, '{');

            ast = append_ast(&compiler->ast_nodes, AST_KIND_BLOCK, make_source_location(&compiler->parser, compiler->parser.previous.lexeme));

            while (compiler->parser.current.type != '}')
            {
                Ast *statement = parse_statement(compiler);

                if (!statement) return 0;

                ast_list_append(&ast->children, statement);
                statement->parent = ast;
            }

            expect_token(compiler, '}');
        } break;

        default:
        {
            report_error(*compiler, make_source_location(&compiler->parser, compiler->parser.current.lexeme), "expected a statement");
        } break;
    }

    return ast;
}

static Ast *
parse_parameter(Compiler *compiler)
{
    expect_token(compiler, TOKEN_IDENTIFIER);

    Ast *ast = append_ast(&compiler->ast_nodes, AST_KIND_VARIABLE_DECLARATION, make_source_location(&compiler->parser, compiler->parser.previous.lexeme));

    ast->name = compiler->parser.previous.lexeme;

    expect_token(compiler, ':');

    ast_set_type_def(ast, parse_type_definition(compiler));

    return ast;
}

static Ast *
parse_declaration(Compiler *compiler)
{
    expect_token(compiler, TOKEN_IDENTIFIER);

    String name = compiler->parser.previous.lexeme;

    expect_token(compiler, TOKEN_COLON_COLON);

    Ast *declaration = 0;

    if (match_token(&compiler->parser, TOKEN_KEYWORD_STRUCT))
    {
        declaration = append_ast(&compiler->ast_nodes, AST_KIND_STRUCT_DECLARATION, make_source_location(&compiler->parser, name));

        declaration->name = name;
    }
    else if (match_token(&compiler->parser, '('))
    {
        declaration = append_ast(&compiler->ast_nodes, AST_KIND_FUNCTION_DECLARATION, make_source_location(&compiler->parser, name));

        declaration->name = name;
        declaration->address = S64MAX;

        if (!match_token(&compiler->parser, ')'))
        {
            for (;;)
            {
                Ast *parameter = parse_parameter(compiler);

                if (!parameter) return 0;

                ast_list_append(&declaration->parameters, parameter);
                parameter->parent = declaration;

                if (!match_token(&compiler->parser, ','))
                {
                    break;
                }
            }

            expect_token(compiler, ')');
        }

        if (match_token(&compiler->parser, TOKEN_RIGHT_ARROW))
        {
            ast_set_type_def(declaration, parse_type_definition(compiler));
        }

        expect_token(compiler, '{');

        while (compiler->parser.current.type != '}')
        {
            Ast *statement = parse_statement(compiler);

            if (!statement) return 0;

            ast_list_append(&declaration->children, statement);
            statement->parent = declaration;
        }

        expect_token(compiler, '}');
    }
    else
    {
        report_error(*compiler, make_source_location(&compiler->parser, compiler->parser.current.lexeme), "expected struct or function declaration");
    }

    return declaration;
}

static bool
parse(Compiler *compiler, StringArray *files_to_load)
{
    advance_token(&compiler->parser);

    while (!match_token(&compiler->parser, TOKEN_END_OF_INPUT) && !compiler->parser.has_error)
    {
        if (match_token(&compiler->parser, TOKEN_DIRECTIVE_IMPORT))
        {
            expect_token(compiler, TOKEN_LITERAL_STRING);
            String filename = compiler->parser.previous.lexeme;
            expect_token(compiler, ';');

            assert(filename.count >= 2);

            filename.data += 1;
            filename.count -= 2;

            filename = path_concat(&default_allocator, S("libraries"), concat(&default_allocator, filename, S(".juls")));

            add_file_to_load(files_to_load, filename);
        }
        else if (match_token(&compiler->parser, TOKEN_DIRECTIVE_LOAD))
        {
            expect_token(compiler, TOKEN_LITERAL_STRING);
            String filename = compiler->parser.previous.lexeme;
            expect_token(compiler, ';');

            assert(filename.count >= 2);

            filename.data += 1;
            filename.count -= 2;

            if (compiler->current_directory.count)
            {
                filename = path_concat(&default_allocator, compiler->current_directory, filename);
            }

            add_file_to_load(files_to_load, filename);
        }
        else
        {
            Ast *decl = parse_declaration(compiler);

            if (!decl) return false;

            ast_list_append(&compiler->global_declarations.children, decl);
            decl->parent = &compiler->global_declarations;
        }
    }

    return !compiler->parser.has_error;
}
