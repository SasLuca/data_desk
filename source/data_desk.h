#ifndef DATA_DESK_H_INCLUDED_
#define DATA_DESK_H_INCLUDED_

#ifndef DATA_DESK_NO_CRT
#include <stdio.h>
#endif

typedef struct DataDeskASTNode DataDeskASTNode;

typedef struct DataDeskStruct
{
    char *name;
    char *name_lowercase_with_underscores;
    char *name_uppercase_with_underscores;
    char *name_lower_camel_case;
    char *name_upper_camel_case;
    char *tag;
    DataDeskASTNode *root;
}
DataDeskStruct;

typedef struct DataDeskDeclaration
{
    char *name;
    char *name_lowercase_with_underscores;
    char *name_uppercase_with_underscores;
    char *name_lower_camel_case;
    char *name_upper_camel_case;
    char *tag;
    DataDeskASTNode *root;
}
DataDeskDeclaration;

typedef void DataDeskInitCallback(void);
typedef void DataDeskFileCallback(char *filename);
typedef void DataDeskStructCallback(DataDeskStruct parsed_struct, char *filename);
typedef void DataDeskDeclarationCallback(DataDeskDeclaration declaration, char *filename);

enum
{
    DATA_DESK_AST_NODE_TYPE_invalid,
    DATA_DESK_AST_NODE_TYPE_identifier,
    DATA_DESK_AST_NODE_TYPE_numeric_constant,
    DATA_DESK_AST_NODE_TYPE_string_constant,
    DATA_DESK_AST_NODE_TYPE_char_constant,
    DATA_DESK_AST_NODE_TYPE_binary_operator,
    DATA_DESK_AST_NODE_TYPE_struct_declaration,
    DATA_DESK_AST_NODE_TYPE_declaration,
    DATA_DESK_AST_NODE_TYPE_type_usage,
};

// NOTE(rjf): These must be in order of precedence
enum
{
    DATA_DESK_BINARY_OPERATOR_TYPE_invalid,
    DATA_DESK_BINARY_OPERATOR_TYPE_add,
    DATA_DESK_BINARY_OPERATOR_TYPE_subtract,
    DATA_DESK_BINARY_OPERATOR_TYPE_multiply,
    DATA_DESK_BINARY_OPERATOR_TYPE_divide,
    DATA_DESK_BINARY_OPERATOR_TYPE_MAX
};

typedef struct DataDeskASTNode
{
    int type;
    DataDeskASTNode *next;
    
    char *string;
    char *tag;
    int string_length;
    int tag_length;
    
    union
    {
        struct BinaryOperator
        {
            int type;
            DataDeskASTNode *left;
            DataDeskASTNode *right;
        }
        binary_operator;
        
        struct StructDeclaration
        {
            DataDeskASTNode *first_member;
        }
        struct_declaration;
        
        struct Declaration
        {
            DataDeskASTNode *type;
        }
        declaration;
        
        struct TypeUsage
        {
            int pointer_count;
            DataDeskASTNode *first_array_size_expression;
            DataDeskASTNode *struct_declaration;
        }
        type_usage;
    };
}
DataDeskASTNode;

int DataDeskTagHasSubString(char *tag, char *substring);
int DataDeskStructHasTag(DataDeskStruct struct_info, char *tag);
int DataDeskNodeHasTag(DataDeskASTNode *root, char *tag);
int DataDeskNodeHasTag(DataDeskASTNode *root, char *tag);
int DataDeskDeclarationHasTag(DataDeskDeclaration declaration_info, char *tag);
int DataDeskDeclarationIsType(DataDeskASTNode *root, char *type);
int DataDeskStructMemberIsType(DataDeskASTNode *root, char *type);

#ifndef DATA_DESK_NO_CRT
void DataDeskFWriteStructAsC(FILE *file, DataDeskStruct struct_info);
#endif

int
DataDeskTagHasSubString(char *tag, char *substring)
{
    int matches = 0;
    
    if(tag)
    {
        for(int i = 0; tag[i]; ++i)
        {
            if(tag[i] == substring[0])
            {
                matches = 1;
                
                int tag_i = i+1;
                int substring_i = 1;
                for(;;)
                {
                    if(!substring[substring_i])
                    {
                        break;
                    }
                    
                    if(tag[tag_i] != substring[substring_i])
                    {
                        matches = 0;
                        break;
                    }
                    
                    ++tag_i;
                    ++substring_i;
                }
            }
        }
    }
    
    return matches;
}

int
DataDeskStructHasTag(DataDeskStruct struct_info, char *tag)
{
    return DataDeskTagHasSubString(struct_info.tag, tag);
}

int
DataDeskDeclarationHasTag(DataDeskDeclaration declaration_info, char *tag)
{
    return DataDeskTagHasSubString(declaration_info.tag, tag);
}

int
DataDeskNodeHasTag(DataDeskASTNode *root, char *tag)
{
    return DataDeskTagHasSubString(root->tag, tag);
}

int
DataDeskDeclarationIsType(DataDeskASTNode *root, char *type)
{
    int pointer_count = 0;
    char *type_name = type;
    
    for(int i = 0; type[i]; ++i)
    {
        if(type[i] == '*')
        {
            ++pointer_count;
        }
        else
        {
            type_name = type+i;
            break;
        }
    }
    
    int matches = pointer_count == root->declaration.type->type_usage.pointer_count;
    if(matches)
    {
        for(int i = 0; type[i] && root->declaration.type->string[i]; ++i)
        {
            if(type[i] != root->declaration.type->string[i])
            {
                matches = 0;
                break;
            }
        }
    }
    
    return matches;
}

int
DataDeskStructMemberIsType(DataDeskASTNode *root, char *type)
{
    return DataDeskDeclarationIsType(root, type);
}

#ifndef DATA_DESK_NO_CRT
void
DataDeskFWriteASTFromRootAsC(FILE *file, DataDeskASTNode *root, int follow_next)
{
    if(root)
    {
        switch(root->type)
        {
            case DATA_DESK_AST_NODE_TYPE_identifier:
            case DATA_DESK_AST_NODE_TYPE_numeric_constant:
            case DATA_DESK_AST_NODE_TYPE_string_constant:
            case DATA_DESK_AST_NODE_TYPE_char_constant:
            {
                fprintf(file, "%s", root->string);
                break;
            }
            
            case DATA_DESK_AST_NODE_TYPE_binary_operator:
            {
                DataDeskFWriteASTFromRootAsC(file, root->binary_operator.left, 0);
                char *binary_operator_string = "";
                
                switch(root->binary_operator.type)
                {
                    case DATA_DESK_BINARY_OPERATOR_TYPE_add: { binary_operator_string = "+"; break; }
                    case DATA_DESK_BINARY_OPERATOR_TYPE_subtract: { binary_operator_string = "-"; break; }
                    case DATA_DESK_BINARY_OPERATOR_TYPE_multiply: { binary_operator_string = "*"; break; }
                    case DATA_DESK_BINARY_OPERATOR_TYPE_divide: { binary_operator_string = "/"; break; }
                    default: break;
                }
                
                fprintf(file, "%s", binary_operator_string);
                DataDeskFWriteASTFromRootAsC(file, root->binary_operator.right, 0);
                
                break;
            }
            
            case DATA_DESK_AST_NODE_TYPE_struct_declaration:
            {
                fprintf(file, "typedef struct %s\n{\n", root->string);
                for(DataDeskASTNode *member = root->struct_declaration.first_member;
                    member;
                    member = member->next)
                {
                    DataDeskFWriteASTFromRootAsC(file, member, 0);
                    fprintf(file, ";\n");
                }
                fprintf(file, "}\n%s;\n\n", root->string);
                break;
            }
            
            case DATA_DESK_AST_NODE_TYPE_declaration:
            {
                DataDeskFWriteASTFromRootAsC(file, root->declaration.type, 0);
                fprintf(file, "%s", root->string);
                
                for(DataDeskASTNode *array = root->declaration.type->type_usage.first_array_size_expression;
                    array;
                    array = array->next)
                {
                    fprintf(file, "[");
                    DataDeskFWriteASTFromRootAsC(file, array, 0);
                    fprintf(file, "]");
                }
                
                break;
            }
            
            case DATA_DESK_AST_NODE_TYPE_type_usage:
            {
                fprintf(file, "%s ", root->string);
                for(int i = 0; i < root->type_usage.pointer_count; ++i)
                {
                    fprintf(file, "*");
                }
                
                break;
            }
            
            default: break;
        }
        
        if(root->next && follow_next)
        {
            DataDeskFWriteASTFromRootAsC(file, root->next, follow_next);
        }
    }
}

void
DataDeskFWriteStructAsC(FILE *file, DataDeskStruct struct_info)
{
    DataDeskFWriteASTFromRootAsC(file, struct_info.root, 0);
}
#endif

#endif // DATA_DESK_H_INCLUDED_