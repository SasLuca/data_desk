static void
GenerateNullTerminatedStringsForAST(ParseContext *context, ASTNode *root)
{
    if(root)
    {
        if(root->string)
        {
            char *new_string = ParseContextAllocateMemory(context, root->string_length+1);
            MemoryCopy(new_string, root->string, root->string_length);
            new_string[root->string_length] = 0;
            root->string = new_string;
        }
        
        if(root->tag)
        {
            char *new_tag = ParseContextAllocateMemory(context, root->tag_length+1);
            MemoryCopy(new_tag, root->tag, root->tag_length);
            new_tag[root->tag_length] = 0;
            root->tag = new_tag;
        }
        
        switch(root->type)
        {
            case DATA_DESK_AST_NODE_TYPE_binary_operator:
            {
                GenerateNullTerminatedStringsForAST(context, root->binary_operator.left);
                GenerateNullTerminatedStringsForAST(context, root->binary_operator.right);
                break;
            }
            case DATA_DESK_AST_NODE_TYPE_struct_declaration:
            {
                GenerateNullTerminatedStringsForAST(context, root->struct_declaration.first_member);
                break;
            }
            case DATA_DESK_AST_NODE_TYPE_declaration:
            {
                GenerateNullTerminatedStringsForAST(context, root->declaration.type);
                break;
            }
            case DATA_DESK_AST_NODE_TYPE_type_usage:
            {
                GenerateNullTerminatedStringsForAST(context, root->type_usage.first_array_size_expression);
                GenerateNullTerminatedStringsForAST(context, root->type_usage.struct_declaration);
                break;
            }
            
            default: break;
        }
        
        if(root->next)
        {
            GenerateNullTerminatedStringsForAST(context, root->next);
        }
    }
}

static void
PrintASTFromRoot(ASTNode *root, int follow_next)
{
    if(root)
    {
        if(root->tag)
        {
            printf("%.*s\n", root->tag_length, root->tag);
        }
        
        switch(root->type)
        {
            case DATA_DESK_AST_NODE_TYPE_identifier:
            case DATA_DESK_AST_NODE_TYPE_numeric_constant:
            case DATA_DESK_AST_NODE_TYPE_string_constant:
            case DATA_DESK_AST_NODE_TYPE_char_constant:
            {
                printf("%.*s", root->string_length, root->string);
                break;
            }
            
            case DATA_DESK_AST_NODE_TYPE_binary_operator:
            {
                printf("(");
                PrintASTFromRoot(root->binary_operator.left, 0);
                printf(" %s ", GetBinaryOperatorStringFromType(root->binary_operator.type));
                PrintASTFromRoot(root->binary_operator.right, 0);
                printf(")");
                break;
            }
            
            case DATA_DESK_AST_NODE_TYPE_struct_declaration:
            {
                printf("struct");
                
                if(root->string)
                {
                    printf(" %.*s", root->string_length, root->string);
                }
                
                printf("\n{\n");
                
                for(DataDeskASTNode *member = root->struct_declaration.first_member;
                    member;
                    member = member->next)
                {
                    PrintASTFromRoot(member, 0);
                    printf(";\n");
                }
                
                printf("}\n\n");
                break;
            }
            
            case DATA_DESK_AST_NODE_TYPE_declaration:
            {
                printf("%.*s : ", root->string_length, root->string);
                PrintASTFromRoot(root->declaration.type, 0);
                break;
            }
            
            case DATA_DESK_AST_NODE_TYPE_type_usage:
            {
                for(int i = 0; i < root->type_usage.pointer_count; ++i)
                {
                    printf("*");
                }
                
                if(root->type_usage.struct_declaration)
                {
                    PrintASTFromRoot(root->type_usage.struct_declaration, 0);
                }
                else
                {
                    printf("%.*s", root->string_length, root->string);
                }
                
                for(DataDeskASTNode *array = root->type_usage.first_array_size_expression;
                    array;
                    array = array->next)
                {
                    printf("[");
                    PrintASTFromRoot(array, 0);
                    printf("]");
                }
                
                break;
            }
            
            default: break;
        }
        
        if(follow_next)
        {
            PrintASTFromRoot(root->next, follow_next);
        }
    }
}

static void 
PrintAST(ASTNode *root)
{
    PrintASTFromRoot(root, 1);
}

static void
TraverseASTAndCallCustomParseCallbacks(ParseContext *context, ASTNode *root, DataDeskCustom custom, char *filename)
{
    if(root)
    {
        
        switch(root->type)
        {
            
            case DATA_DESK_AST_NODE_TYPE_struct_declaration:
            {
                if(custom.StructCallback)
                {
                    DataDeskStruct struct_info = {0};
                    {
                        struct_info.name = root->string;
                        struct_info.name_lowercase_with_underscores =
                            ParseContextAllocateStringCopyLowercaseWithUnderscores(context, struct_info.name);
                        struct_info.name_uppercase_with_underscores =
                            ParseContextAllocateStringCopyUppercaseWithUnderscores(context, struct_info.name);
                        struct_info.name_lower_camel_case =
                            ParseContextAllocateStringCopyLowerCamelCase(context, struct_info.name);
                        struct_info.name_upper_camel_case =
                            ParseContextAllocateStringCopyUpperCamelCase(context, struct_info.name);
                        struct_info.tag = root->tag;
                        struct_info.root = root;
                    }
                    custom.StructCallback(struct_info, filename);
                }
                
                TraverseASTAndCallCustomParseCallbacks(context, root->struct_declaration.first_member, custom, filename);
                break;
            }
            
            case DATA_DESK_AST_NODE_TYPE_declaration:
            {
                if(custom.DeclarationCallback)
                {
                    DataDeskDeclaration decl_info = {0};
                    {
                        decl_info.name = root->string;
                        decl_info.name_lowercase_with_underscores =
                            ParseContextAllocateStringCopyLowercaseWithUnderscores(context, decl_info.name);
                        decl_info.name_uppercase_with_underscores =
                            ParseContextAllocateStringCopyUppercaseWithUnderscores(context, decl_info.name);
                        decl_info.name_lower_camel_case =
                            ParseContextAllocateStringCopyLowerCamelCase(context, decl_info.name);
                        decl_info.name_upper_camel_case =
                            ParseContextAllocateStringCopyUpperCamelCase(context, decl_info.name);
                        decl_info.tag = root->tag;
                        decl_info.root = root;
                    }
                    custom.DeclarationCallback(decl_info, filename);
                }
                
                TraverseASTAndCallCustomParseCallbacks(context, root->declaration.type, custom, filename);
                break;
            }
            
            case DATA_DESK_AST_NODE_TYPE_type_usage:
            {
                if(root->type_usage.struct_declaration)
                {
                    TraverseASTAndCallCustomParseCallbacks(context, root->type_usage.struct_declaration, custom, filename);
                }
                break;
            }
            
            default: break;
        }
        
        if(root->next)
        {
            TraverseASTAndCallCustomParseCallbacks(context, root->next, custom, filename);
        }
    }
}