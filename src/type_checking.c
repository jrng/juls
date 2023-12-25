static bool
can_implicitly_cast_to(DatatypeTable *table, DatatypeId from_type_id, DatatypeId to_type_id)
{
    if (from_type_id == to_type_id)
    {
        return true;
    }

    Datatype *from_type = get_datatype(table, from_type_id);
    Datatype *to_type = get_datatype(table, to_type_id);

    if ((from_type->kind == DATATYPE_INTEGER) && (to_type->kind == DATATYPE_INTEGER))
    {
        if ((!(to_type->flags & DATATYPE_FLAG_UNSIGNED) || (from_type->flags & DATATYPE_FLAG_UNSIGNED)) &&
            (to_type->size > from_type->size))
        {
            return true;
        }
    }
    else
    {
        assert(!"not implemented");
    }

    return false;
}

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

        case AST_KIND_POINTER:
        {
            resolve_type(parser, type_def->left_expr);

            assert(type_def->left_expr->type_id);

            DatatypeId type_id = find_datatype_by_kind_and_reference(&parser->datatypes, DATATYPE_POINTER, type_def->left_expr->type_id);

            if (!type_id)
            {
                type_id = parser->datatypes.count;
                array_append(&parser->datatypes, ((Datatype) { .kind = DATATYPE_POINTER, .flags = 0, .ref = type_def->left_expr->type_id, .name = S("*"), .size = 8 }));
            }

            type_def->type_id = type_id;
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

            if (can_implicitly_cast_to(&parser->datatypes, expr->left_expr->type_id, expr->right_expr->type_id) ||
                can_implicitly_cast_to(&parser->datatypes, expr->right_expr->type_id, expr->left_expr->type_id))
            {
            }
            else
            {
                // TODO: error
                fprintf(stderr, "error: not compatible types\n");
            }

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

            if (can_implicitly_cast_to(&parser->datatypes, expr->left_expr->type_id, expr->right_expr->type_id) ||
                can_implicitly_cast_to(&parser->datatypes, expr->right_expr->type_id, expr->left_expr->type_id))
            {
                Datatype *left_type = get_datatype(&parser->datatypes, expr->left_expr->type_id);
                Datatype *right_type = get_datatype(&parser->datatypes, expr->right_expr->type_id);

                if (left_type->size > right_type->size)
                {
                    expr->type_id = expr->left_expr->type_id;
                }
                else
                {
                    expr->type_id = expr->right_expr->type_id;
                }
            }
            else
            {
                // TODO: error
                fprintf(stderr, "error: not compatible integer types\n");
            }
        } break;

        case AST_KIND_EXPRESSION_UNARY_MINUS:
        {
            DatatypeId type_id = parser->basetype_s64;

            if (preferred_type_id)
            {
                Datatype *datatype = get_datatype(&parser->datatypes, preferred_type_id);

                if ((datatype->kind == DATATYPE_INTEGER) &&
                    !(datatype->flags & DATATYPE_FLAG_UNSIGNED))
                {
                    type_id = preferred_type_id;
                }
            }

            type_check_expression(parser, expr->left_expr, type_id);

            if (expr->left_expr->kind == AST_KIND_LITERAL_INTEGER)
            {
                Datatype *datatype = get_datatype(&parser->datatypes, type_id);

                // TODO: does fit into type?

                expr->kind = AST_KIND_LITERAL_INTEGER;
                expr->_s64 = -expr->left_expr->_s64;
            }

            expr->type_id = type_id;
        } break;

        case AST_KIND_FUNCTION_CALL:
        {
            assert(expr->left_expr);

            Ast *left_expr = expr->left_expr;

            if (left_expr->kind == AST_KIND_IDENTIFIER)
            {
                if (strings_are_equal(left_expr->name, S("exit")))
                {
                    s32 argument_count = ast_list_count(&expr->children);

                    if (argument_count == 1)
                    {
                        Ast *argument = expr->children.first;

                        type_check_expression(parser, argument, parser->basetype_s64);

                        if (!can_implicitly_cast_to(&parser->datatypes, argument->type_id, parser->basetype_s64) &&
                            !can_implicitly_cast_to(&parser->datatypes, argument->type_id, parser->basetype_u64))
                        {
                            report_error(parser->lexer.input, left_expr->source_location,
                                         "function 'exit' expects type s64/u64 for parameter 1 ('return_code')");
                        }
                    }
                    else
                    {
                        report_error(parser->lexer.input, left_expr->source_location,
                                     "function 'exit' expects 1 argument, but was given %d",
                                     argument_count);
                    }

                    expr->type_id = parser->basetype_void;
                }
                else
                {
                    expr->decl = find_function_declaration_by_name(parser->global_declarations.children.first, left_expr->name);

                    if (expr->decl)
                    {
                        assert(expr->decl->type_id);

                        s32 parameter_count = ast_list_count(&expr->decl->parameters);
                        s32 argument_count = ast_list_count(&expr->children);

                        if (argument_count == parameter_count)
                        {
                            Ast *parameter = expr->decl->parameters.first;
                            Ast *argument = expr->children.first;

                            s32 parameter_position = 1;

                            while (parameter)
                            {
                                type_check_expression(parser, argument, parameter->type_id);

                                if (!can_implicitly_cast_to(&parser->datatypes, argument->type_id, parameter->type_id))
                                {
                                    Datatype *parameter_type = get_datatype(&parser->datatypes, parameter->type_id);

                                    report_error(parser->lexer.input, left_expr->source_location,
                                                 "function '%.*s' expects type %.*s for parameter %d ('%.*s')",
                                                 (int) left_expr->name.count, left_expr->name.data,
                                                 (int) parameter_type->name.count, parameter_type->name.data,
                                                 parameter_position,
                                                 (int) parameter->name.count, parameter->name.data);
                                }

                                parameter = parameter->next;
                                argument = argument->next;

                                parameter_position += 1;
                            }
                        }
                        else
                        {
                            report_error(parser->lexer.input, left_expr->source_location,
                                         "function '%.*s' expects %d arguments, but was given %d",
                                         (int) left_expr->name.count, left_expr->name.data,
                                         parameter_count, argument_count);
                        }

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

        case AST_KIND_ASSIGN:
        case AST_KIND_PLUS_ASSIGN:
        case AST_KIND_MINUS_ASSIGN:
        case AST_KIND_MUL_ASSIGN:
        case AST_KIND_DIV_ASSIGN:
        case AST_KIND_OR_ASSIGN:
        case AST_KIND_AND_ASSIGN:
        case AST_KIND_XOR_ASSIGN:
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

            type_check_expression(parser, expr->right_expr, expr->type_id);

            if (!can_implicitly_cast_to(&parser->datatypes, expr->right_expr->type_id, expr->type_id))
            {
                Datatype *left_datatype = get_datatype(&parser->datatypes, expr->type_id);
                Datatype *right_datatype = get_datatype(&parser->datatypes, expr->right_expr->type_id);

                report_error(parser->lexer.input, expr->source_location,
                             "can not assign type %.*s to type %.*s",
                             (int) right_datatype->name.count, right_datatype->name.data,
                             (int) left_datatype->name.count, left_datatype->name.data);
            }
        } break;

        default:
        {
            printf("kind %u\n", expr->kind);
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

        case AST_KIND_FOR:
        {
            type_check_statement(parser, statement->decl);
            type_check_expression(parser, statement->left_expr, 0);

            if (statement->left_expr->type_id != parser->basetype_bool)
            {
                report_error(parser->lexer.input, statement->left_expr->source_location, "expression in for statement has to be of type bool");
            }

            type_check_expression(parser, statement->right_expr, 0);

            For(stmt, statement->children.first)
            {
                type_check_statement(parser, stmt);
            }
        } break;

        case AST_KIND_BLOCK:
        {
            For(stmt, statement->children.first)
            {
                type_check_statement(parser, stmt);
            }
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
