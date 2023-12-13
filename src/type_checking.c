static void type_check_expression(Parser *parser, Ast *expr);

static void
resolve_type(Parser *parser, Ast *type_def)
{
    switch (type_def->kind)
    {
        case AST_KIND_QUERY_TYPE_OF:
        {
            type_check_expression(parser, type_def->left_expr);
            type_def->type_id = type_def->left_expr->type_id;
        } break;

        case AST_KIND_IDENTIFIER:
        {
            DatatypeId type_id = find_datatype_by_name(&parser->datatypes, type_def->name);

            if (type_id)
            {
                Datatype *datatype = get_datatype(&parser->datatypes, type_id);

                type_def->type_id = type_id;
            }
            else
            {
                // TODO: search declaration and append new type
            }
        } break;

        default:
        {
            printf("kind = %u\n", type_def->kind);
            assert(!"type not supported");
        } break;
    }

    printf("kind = %u\n", type_def->kind);
    assert(type_def->type_id);
}

static void
type_check_expression(Parser *parser, Ast *expr)
{
    switch (expr->kind)
    {
        case AST_KIND_QUERY_SIZE_OF:
        {
            resolve_type(parser, expr->type_def);

            Datatype *type = get_datatype(&parser->datatypes, expr->type_def->type_id);

            expr->kind = AST_KIND_LITERAL_INTEGER;
            expr->type_id = parser->basetype_u64;
            expr->_u64 = type->size;
        } break;

        case AST_KIND_IDENTIFIER:
        {
            printf("type check identifier '%.*s'\n", (int) expr->name.count, expr->name.data);
            assert(!expr->type_def);

            expr->left_expr = find_declaration_by_name(expr, expr->name);

            if (expr->left_expr)
            {
                print_ast(expr->left_expr, 0);
                printf("type_id = %u\n", expr->left_expr->type_id);
                expr->type_id = expr->left_expr->type_id;
            }
            else
            {
                printf("not found\n");
            }
        } break;

        case AST_KIND_LITERAL_BOOLEAN:
        {
        } break;

        case AST_KIND_LITERAL_INTEGER:
        {
        } break;

        case AST_KIND_LITERAL_FLOAT:
        {
        } break;

        case AST_KIND_LITERAL_STRING:
        {
        } break;

        default:
        {
            assert(!"expression not supported");
        } break;
    }
}

static void
type_check_statement(Parser *parser, Ast *statement)
{
    switch (statement->kind)
    {
        case AST_KIND_VARIABLE_DECLARATION:
        {
            if (statement->right_expr)
            {
                type_check_expression(parser, statement->right_expr);
            }

            if (statement->type_def)
            {
                resolve_type(parser, statement->type_def);

                statement->type_id = statement->type_def->type_id;

                // TODO: declaration type and expression type compatible?
            }
            else
            {
                assert(statement->right_expr);
                assert(statement->right_expr->type_id);

                statement->type_id = statement->right_expr->type_id;
            }
        } break;

        case AST_KIND_FUNCTION_CALL:
        {
            For(argument, statement->children.first)
            {
                type_check_expression(parser, argument);
            }
        } break;

        default:
        {
            assert(!"statement not supported");
        } break;
    }
}

static void
type_checking(Parser *parser)
{
    For(decl, parser->global_declarations.children.first)
    {
        print_ast(decl, 0);

        // TODO: check for redefinition

        switch (decl->kind)
        {
            case AST_KIND_FUNCTION_DECLARATION:
            {
                For(statement, decl->children.first)
                {
                    type_check_statement(parser, statement);
                }
            } break;

            case AST_KIND_STRUCT_DECLARATION:
            {
            } break;

            default:
            {
                assert(!"not allowed");
            } break;
        }
    }
}
