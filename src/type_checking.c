static void type_check_expression(Parser *parser, Ast *expr, DatatypeId preferred_type_id);

static void
resolve_type(Parser *parser, Ast *type_def)
{
    switch (type_def->kind)
    {
        case AST_KIND_QUERY_TYPE_OF:
        {
            type_check_expression(parser, type_def->left_expr, 0);
            type_def->type_id = type_def->left_expr->type_id;
        } break;

        case AST_KIND_IDENTIFIER:
        {
            DatatypeId type_id = find_datatype_by_name(&parser->datatypes, type_def->name);

            if (type_id)
            {
                type_def->type_id = type_id;
            }
            else
            {
                // TODO: search declaration and append new type
                assert(!"not implemented");
            }
        } break;

        default:
        {
            assert(!"type not supported");
        } break;
    }

    assert(type_def->type_id);
}

static void
type_check_expression(Parser *parser, Ast *expr, DatatypeId preferred_type_id)
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
            expr->decl = find_declaration_by_name(expr, expr->name);

            if (expr->decl)
            {
                expr->type_id = expr->decl->type_id;
            }
            else
            {
                report_error(parser->lexer.input, expr->source_location, "undeclared identifier '%.*s'", (int) expr->name.count, expr->name.data);
            }
        } break;

        case AST_KIND_LITERAL_BOOLEAN:
        {
        } break;

        case AST_KIND_LITERAL_INTEGER:
        {
            if (preferred_type_id)
            {
                Datatype *datatype = get_datatype(&parser->datatypes, preferred_type_id);

                assert(datatype->kind == DATATYPE_INTEGER);

                if (datatype->flags & DATATYPE_FLAG_UNSIGNED)
                {
                    switch (datatype->size)
                    {
                        case 1: expr->type_id = parser->basetype_u8;  break;
                        case 2: expr->type_id = parser->basetype_u16; break;
                        case 4: expr->type_id = parser->basetype_u32; break;
                        case 8: expr->type_id = parser->basetype_u64; break;
                        default: assert(!"not allowed"); break;
                    }
                }
                else
                {
                    switch (datatype->size)
                    {
                        case 1: expr->type_id = parser->basetype_s8;  break;
                        case 2: expr->type_id = parser->basetype_s16; break;
                        case 4: expr->type_id = parser->basetype_s32; break;
                        case 8: expr->type_id = parser->basetype_s64; break;
                        default: assert(!"not allowed"); break;
                    }
                }

                // TODO: check if the value fits in the type
            }
        } break;

        case AST_KIND_LITERAL_FLOAT:
        {
        } break;

        case AST_KIND_LITERAL_STRING:
        {
        } break;

        case AST_KIND_EXPRESSION_EQUAL:
        case AST_KIND_EXPRESSION_NOT_EQUAL:
        case AST_KIND_EXPRESSION_COMPARE_LESS:
        case AST_KIND_EXPRESSION_COMPARE_GREATER:
        case AST_KIND_EXPRESSION_COMPARE_LESS_EQUAL:
        case AST_KIND_EXPRESSION_COMPARE_GREATER_EQUAL:
        {
            if (expr->left_expr->kind == AST_KIND_LITERAL_INTEGER)
            {
                type_check_expression(parser, expr->right_expr, 0);
                type_check_expression(parser, expr->left_expr, expr->right_expr->type_id);
            }
            else
            {
                type_check_expression(parser, expr->left_expr, 0);
                type_check_expression(parser, expr->right_expr, expr->left_expr->type_id);
            }

            // TODO: are types comparable

            expr->type_id = parser->basetype_bool;
        } break;

        case AST_KIND_EXPRESSION_BINOP_ADD:
        case AST_KIND_EXPRESSION_BINOP_MINUS:
        {
            if (expr->left_expr->kind == AST_KIND_LITERAL_INTEGER)
            {
                type_check_expression(parser, expr->right_expr, 0);
                type_check_expression(parser, expr->left_expr, expr->right_expr->type_id);
            }
            else
            {
                type_check_expression(parser, expr->left_expr, 0);
                type_check_expression(parser, expr->right_expr, expr->left_expr->type_id);
            }

            Datatype *left_type = get_datatype(&parser->datatypes, expr->left_expr->type_id);
            Datatype *right_type = get_datatype(&parser->datatypes, expr->right_expr->type_id);

            u64 max_size = left_type->size;

            if (right_type->size > max_size)
            {
                max_size = right_type->size;
            }

            DatatypeId type_id = 0;

            if ((left_type->flags & DATATYPE_FLAG_UNSIGNED) && (right_type->flags & DATATYPE_FLAG_UNSIGNED))
            {
                switch (max_size)
                {
                    case 1: type_id = parser->basetype_u8;  break;
                    case 2: type_id = parser->basetype_u16; break;
                    case 4: type_id = parser->basetype_u32; break;
                    case 8: type_id = parser->basetype_u64; break;
                    default: assert(!"not allowed"); break;
                }
            }
            else if (!(left_type->flags & DATATYPE_FLAG_UNSIGNED) && !(right_type->flags & DATATYPE_FLAG_UNSIGNED))
            {
                switch (max_size)
                {
                    case 1: type_id = parser->basetype_s8;  break;
                    case 2: type_id = parser->basetype_s16; break;
                    case 4: type_id = parser->basetype_s32; break;
                    case 8: type_id = parser->basetype_s64; break;
                    default: assert(!"not allowed"); break;
                }
            }
            else
            {
                // TODO: error
                fprintf(stderr, "error: not compatible integer types\n");
            }

            expr->type_id = type_id;
        } break;

        case AST_KIND_FUNCTION_CALL:
        {
            For(argument, expr->children.first)
            {
                type_check_expression(parser, argument, 0);
            }

            assert(expr->left_expr);

            Ast *left_expr = expr->left_expr;

            if (left_expr->kind == AST_KIND_IDENTIFIER)
            {
                if (strings_are_equal(left_expr->name, S("exit")))
                {
                    expr->type_id = parser->basetype_void;
                }
                else
                {
                    expr->decl = find_function_declaration_by_name(parser->global_declarations.children.first, left_expr->name);

                    if (expr->decl)
                    {
                        assert(expr->decl->type_id);
                        expr->type_id = expr->decl->type_id;
                    }
                    else
                    {
                        report_error(parser->lexer.input, left_expr->source_location, "undeclared identifier '%.*s'", (int) left_expr->name.count, left_expr->name.data);
                    }
                }
            }
            else
            {
                fprintf(stderr, "error: function pointers are not supported yet\n");
            }
        } break;

        default:
        {
            assert(!"expression not supported");
        } break;
    }

    assert(expr->type_id);
}

static void
type_check_statement(Parser *parser, Ast *statement)
{
    switch (statement->kind)
    {
        case AST_KIND_VARIABLE_DECLARATION:
        {
            if (statement->type_def)
            {
                resolve_type(parser, statement->type_def);
                statement->type_id = statement->type_def->type_id;

                if (statement->right_expr)
                {
                    type_check_expression(parser, statement->right_expr, statement->type_id);
                }

                // TODO: declaration type and expression type compatible?
            }
            else
            {
                assert(statement->right_expr);

                type_check_expression(parser, statement->right_expr, 0);

                assert(statement->right_expr->type_id);

                statement->type_id = statement->right_expr->type_id;
            }
        } break;

        case AST_KIND_IF:
        {
            type_check_expression(parser, statement->left_expr, 0);

            if (statement->left_expr->type_id != parser->basetype_bool)
            {
                report_error(parser->lexer.input, statement->left_expr->source_location, "expression in if statement has to be of type bool");
            }

            assert(statement->children.first && statement->children.last);
            assert((statement->children.first == statement->children.last) ||
                   ((statement->children.first->next == statement->children.last) &&
                    (statement->children.first == statement->children.last->prev)));

            For(stmt, statement->children.first)
            {
                type_check_statement(parser, stmt);
            }
        } break;

        case AST_KIND_RETURN:
        {
            // TODO: pass the return type of the function as a hint
            type_check_expression(parser, statement->left_expr, 0);
            statement->type_id = statement->left_expr->type_id;

            // TODO: compare to return type of the function
        } break;

        default:
        {
            type_check_expression(parser, statement, 0);
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
                if (decl->type_def)
                {
                    resolve_type(parser, decl->type_def);
                    decl->type_id = decl->type_def->type_id;
                }
                else
                {
                    decl->type_id = parser->basetype_void;
                }

                For(parameter, decl->parameters.first)
                {
                    type_check_statement(parser, parameter);
                }

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
