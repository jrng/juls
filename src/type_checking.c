static void
type_checking(Parser *parser)
{
    Ast *elem = parser->global_declarations.first;

    while (elem)
    {
        print_ast(elem, 0);
        elem = elem->next;
    }
}
