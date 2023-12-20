typedef enum
{
    DATATYPE_VOID       = 0,
    DATATYPE_BOOLEAN    = 1,
    DATATYPE_INTEGER    = 2,
    DATATYPE_FLOAT      = 3,
    DATATYPE_STRING     = 4,
} DatatypeKind;

typedef enum
{
    DATATYPE_FLAG_UNSIGNED = (1 << 0),
} DatatypeFlag;

typedef struct
{
    DatatypeKind kind;
    u32 flags;

    String name;
    u64 size;
} Datatype;

typedef u16 DatatypeId;

typedef struct
{
    s32 count;
    s32 allocated;
    Datatype *items;
} DatatypeTable;

typedef enum
{
    AST_KIND_GLOBAL_SCOPE                       =  0,
    AST_KIND_STRUCT_DECLARATION                 =  1,
    AST_KIND_FUNCTION_DECLARATION               =  2,
    AST_KIND_VARIABLE_DECLARATION               =  3,
    AST_KIND_EXPRESSION_LOGIC_OR                =  4,
    AST_KIND_EXPRESSION_LOGIC_AND               =  5,
    AST_KIND_EXPRESSION_EQUAL                   =  6,
    AST_KIND_EXPRESSION_NOT_EQUAL               =  7,
    AST_KIND_EXPRESSION_COMPARE_LESS            =  8,
    AST_KIND_EXPRESSION_COMPARE_GREATER         =  9,
    AST_KIND_EXPRESSION_COMPARE_LESS_EQUAL      = 10,
    AST_KIND_EXPRESSION_COMPARE_GREATER_EQUAL   = 11,
    AST_KIND_EXPRESSION_BINOP_ADD               = 12,
    AST_KIND_EXPRESSION_BINOP_MINUS             = 13,
    AST_KIND_EXPRESSION_BINOP_MUL               = 14,
    AST_KIND_EXPRESSION_BINOP_DIV               = 15,
    AST_KIND_EXPRESSION_UNARY_NOT               = 16,
    AST_KIND_EXPRESSION_UNARY_MINUS             = 17,
    AST_KIND_LITERAL_BOOLEAN                    = 18,
    AST_KIND_LITERAL_INTEGER                    = 19,
    AST_KIND_LITERAL_FLOAT                      = 20,
    AST_KIND_LITERAL_STRING                     = 21,
    AST_KIND_IDENTIFIER                         = 22,
    AST_KIND_QUERY_SIZE_OF                      = 23,
    AST_KIND_QUERY_TYPE_OF                      = 24,
    AST_KIND_FUNCTION_CALL                      = 25,
    AST_KIND_IF                                 = 26,
    AST_KIND_FOR                                = 27,
    AST_KIND_RETURN                             = 28,
    AST_KIND_ASSIGN                             = 29,
    AST_KIND_PLUS_ASSIGN                        = 30,
    AST_KIND_MINUS_ASSIGN                       = 31,
    AST_KIND_MUL_ASSIGN                         = 32,
    AST_KIND_DIV_ASSIGN                         = 33,
    AST_KIND_OR_ASSIGN                          = 34,
    AST_KIND_AND_ASSIGN                         = 35,
    AST_KIND_XOR_ASSIGN                         = 36,
    AST_KIND_BLOCK                              = 37,
} AstKind;

typedef struct Ast Ast;

typedef struct
{
    Ast *first;
    Ast *last;
} AstList;

struct Ast
{
    AstKind kind;
    DatatypeId type_id;

    Ast *next;
    Ast *prev;
    Ast *parent;

    String name;
    String source_location;

    Ast *decl;
    Ast *type_def;
    Ast *left_expr;
    Ast *right_expr;

    AstList children;
    AstList parameters;

    s64 size;
    u64 stack_offset;
    s64 address;

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
#define ForReversed(iter, last) for (Ast *iter = (last); iter; iter = iter->prev)

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

static inline Datatype *
get_datatype(DatatypeTable *table, DatatypeId id)
{
    Datatype *result = 0;

    if ((id > 0) && (id < table->count))
    {
        result = table->items + id;
    }

    return result;
}

static inline DatatypeId
find_datatype_by_name(DatatypeTable *table, String name)
{
    for (s32 i = 1; i < table->count; i += 1)
    {
        Datatype *datatype = table->items + i;

        if (strings_are_equal(datatype->name, name))
        {
            return i;
        }
    }

    return 0;
}

static inline void
ast_list_append(AstList *list, Ast *ast)
{
    if (list->last)
    {
        ast->prev = list->last;
        list->last->next = ast;
    }
    else
    {
        ast->prev = 0;
        list->first = ast;
    }

    list->last = ast;
    ast->next = 0;
}

static inline s32
ast_list_count(AstList *list)
{
    s32 count = 0;

    For (elem, list->first)
    {
        count += 1;
    }

    return count;
}

static Ast *
append_ast(AstBucketArray *array, AstKind ast_kind, String source_location)
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
    node->source_location = source_location;

    return node;
}

static inline void
ast_set_decl(Ast *ast, Ast *decl)
{
    decl->parent = ast;
    ast->decl = decl;
}

static inline void
ast_set_type_def(Ast *ast, Ast *type_def)
{
    type_def->parent = ast;
    ast->type_def = type_def;
}

static inline void
ast_set_left_expr(Ast *ast, Ast *expr)
{
    expr->parent = ast;
    ast->left_expr = expr;
}

static inline void
ast_set_right_expr(Ast *ast, Ast *expr)
{
    expr->parent = ast;
    ast->right_expr = expr;
}

static void
print_ast(Ast *ast, s32 indent)
{
    switch (ast->kind)
    {
        case AST_KIND_FUNCTION_DECLARATION:
        {
            fprintf(stderr, "%*sFunctionDeclaration '%.*s'\n", indent, "", (int) ast->name.count, ast->name.data);

            fprintf(stderr, "%*s(\n", indent, "");

            For(elem, ast->parameters.first)
            {
                print_ast(elem, indent + 2);
            }

            fprintf(stderr, "%*s)\n", indent, "");

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

        case AST_KIND_EXPRESSION_COMPARE_LESS:
        {
            fprintf(stderr, "%*sCompareLess\n", indent, "");

            print_ast(ast->left_expr, indent + 2);
            print_ast(ast->right_expr, indent + 2);
        } break;

        case AST_KIND_EXPRESSION_COMPARE_GREATER:
        {
            fprintf(stderr, "%*sCompareGreater\n", indent, "");

            print_ast(ast->left_expr, indent + 2);
            print_ast(ast->right_expr, indent + 2);
        } break;

        case AST_KIND_EXPRESSION_COMPARE_LESS_EQUAL:
        {
            fprintf(stderr, "%*sCompareLessEqual\n", indent, "");

            print_ast(ast->left_expr, indent + 2);
            print_ast(ast->right_expr, indent + 2);
        } break;

        case AST_KIND_EXPRESSION_COMPARE_GREATER_EQUAL:
        {
            fprintf(stderr, "%*sCompareGreaterEqual\n", indent, "");

            print_ast(ast->left_expr, indent + 2);
            print_ast(ast->right_expr, indent + 2);
        } break;

        case AST_KIND_EXPRESSION_BINOP_ADD:
        {
            fprintf(stderr, "%*sBinopAdd\n", indent, "");

            print_ast(ast->left_expr, indent + 2);
            print_ast(ast->right_expr, indent + 2);
        } break;

        case AST_KIND_EXPRESSION_BINOP_MINUS:
        {
            fprintf(stderr, "%*sBinopMinus\n", indent, "");

            print_ast(ast->left_expr, indent + 2);
            print_ast(ast->right_expr, indent + 2);
        } break;

        case AST_KIND_LITERAL_BOOLEAN:
        {
            fprintf(stderr, "%*sLiteralBoolean(type_id = %u): %s\n", indent, "", ast->type_id, ast->_bool ? "true" : "false");
        } break;

        case AST_KIND_LITERAL_INTEGER:
        {
            fprintf(stderr, "%*sLiteralInteger(type_id = %u) %" PRId64 "\n", indent, "", ast->type_id, ast->_s64);
        } break;

        case AST_KIND_LITERAL_FLOAT:
        {
            fprintf(stderr, "%*sLiteralFloat(type_id = %u)\n", indent, "", ast->type_id);
        } break;

        case AST_KIND_LITERAL_STRING:
        {
            fprintf(stderr, "%*sLiteralString(type_id = %u) '%.*s'\n", indent, "", ast->type_id, (int) ast->name.count, ast->name.data);
        } break;

        case AST_KIND_IDENTIFIER:
        {
            fprintf(stderr, "%*sIdentifier '%.*s'\n", indent, "", (int) ast->name.count, ast->name.data);
        } break;

        case AST_KIND_QUERY_SIZE_OF:
        {
            fprintf(stderr, "%*sSizeOf\n", indent, "");
            print_ast(ast->type_def, indent + 2);
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

        case AST_KIND_IF:
        {
            fprintf(stderr, "%*sIf\n", indent, "");

            fprintf(stderr, "%*s(\n", indent, "");

            print_ast(ast->left_expr, indent + 2);

            fprintf(stderr, "%*s)\n", indent, "");

            fprintf(stderr, "%*s{\n", indent, "");

            For(elem, ast->children.first)
            {
                print_ast(elem, indent + 2);
            }

            fprintf(stderr, "%*s}\n", indent, "");
        } break;

        case AST_KIND_RETURN:
        {
            fprintf(stderr, "%*sReturn\n", indent, "");
            print_ast(ast->left_expr, indent + 2);
        } break;

        case AST_KIND_FOR:
        {
            fprintf(stderr, "%*sFor\n", indent, "");

            fprintf(stderr, "%*s(\n", indent, "");

            print_ast(ast->decl, indent + 2);
            print_ast(ast->left_expr, indent + 2);
            print_ast(ast->right_expr, indent + 2);

            fprintf(stderr, "%*s)\n", indent, "");

            fprintf(stderr, "%*s{\n", indent, "");

            For(elem, ast->children.first)
            {
                print_ast(elem, indent + 2);
            }

            fprintf(stderr, "%*s}\n", indent, "");
        } break;

        case AST_KIND_ASSIGN:
        {
            fprintf(stderr, "%*sAssign '%.*s'\n", indent, "", (int) ast->name.count, ast->name.data);
            print_ast(ast->right_expr, indent + 2);
        } break;

        case AST_KIND_PLUS_ASSIGN:
        {
            fprintf(stderr, "%*sPlusAssign '%.*s'\n", indent, "", (int) ast->name.count, ast->name.data);
            print_ast(ast->right_expr, indent + 2);
        } break;

        case AST_KIND_BLOCK:
        {
            fprintf(stderr, "%*s{\n", indent, "");

            For(elem, ast->children.first)
            {
                print_ast(elem, indent + 2);
            }

            fprintf(stderr, "%*s}\n", indent, "");
        } break;

        default:
        {
            fprintf(stderr, "%*sunknown ast kind: %u\n", indent, "", ast->kind);
        } break;
    }
}

static Ast *
find_declaration_by_name(Ast *ast, String name)
{
    Ast *result = 0;

    while (ast->parent)
    {
        Ast *before = ast;
        ast = ast->parent;

        // TODO: do code blocks
        if (ast->kind == AST_KIND_FUNCTION_DECLARATION)
        {
            For(statement, ast->children.first)
            {
                if (statement == before) break;

                if ((statement->kind == AST_KIND_VARIABLE_DECLARATION) &&
                    strings_are_equal(statement->name, name))
                {
                    result = statement;
                    break;
                }
            }

            if (result) break;

            For(parameter, ast->parameters.first)
            {
                assert(parameter->kind == AST_KIND_VARIABLE_DECLARATION);

                if (strings_are_equal(parameter->name, name))
                {
                    result = parameter;
                    break;
                }
            }

            if (result) break;
        }
        else if (ast->kind == AST_KIND_GLOBAL_SCOPE)
        {
            For(statement, ast->children.first)
            {
                if (((statement->kind == AST_KIND_VARIABLE_DECLARATION) ||
                     (statement->kind == AST_KIND_VARIABLE_DECLARATION)) &&
                    strings_are_equal(statement->name, name))
                {
                    result = statement;
                    break;
                }
            }

            if (result) break;
        }
    }

    return result;
}

static Ast *
find_function_declaration_by_name(Ast *first, String name)
{
    Ast *result = 0;

    For(decl, first)
    {
        if ((decl->kind == AST_KIND_FUNCTION_DECLARATION) &&
            strings_are_equal(decl->name, name))
        {
            result = decl;
            break;
        }
    }

    return result;
}
