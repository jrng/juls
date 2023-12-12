static void
type_check_expression(Ast *expr)
{
    switch (expr->kind)
    {
        case AST_KIND_QUERY_SIZE_OF:
        {
            // TODO: do the real deal

            expr->kind = AST_KIND_LITERAL_INTEGER;
            expr->flags = AST_FLAG_UNSIGNED;
            expr->size = 1;
            expr->_u64 = 0;
            expr->_u8 = 99;
        } break;

        default:
        {
            assert(!"expression not supported");
        } break;
    }
}

static void
type_check_statement(Ast *statement)
{
    switch (statement->kind)
    {
        case AST_KIND_VARIABLE_DECLARATION:
        {
        } break;

        case AST_KIND_FUNCTION_CALL:
        {
            For(argument, statement->children.first)
            {
                type_check_expression(argument);
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
    For(decl, parser->global_declarations.first)
    {
        print_ast(decl, 0);

        // TODO: check for redefinition

        switch (decl->kind)
        {
            case AST_KIND_FUNCTION_DECLARATION:
            {
                For(statement, decl->children.first)
                {
                    type_check_statement(statement);
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
