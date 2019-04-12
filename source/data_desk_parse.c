typedef DataDeskASTNode ASTNode;

typedef struct ParseError
{
    char *string;
    char *file;
    int line;
}
ParseError;

typedef struct ParseContextMemoryBlock ParseContextMemoryBlock;
typedef struct ParseContextMemoryBlock
{
    char *memory;
    int memory_size;
    int memory_alloc_position;
    ParseContextMemoryBlock *next;
}
ParseContextMemoryBlock;

#define PARSE_CONTEXT_MEMORY_BLOCK_SIZE_DEFAULT 4096
typedef struct ParseContext
{
    ParseContextMemoryBlock *first_block;
    ParseContextMemoryBlock *active_block;
    int error_stack_size;
    int error_stack_max;
    ParseError *error_stack;
}
ParseContext;

static void *
ParseContextAllocateMemory(ParseContext *context, unsigned int size)
{
    if(!context->active_block ||
       context->active_block->memory_alloc_position + size > context->active_block->memory_size)
    {
        unsigned int needed_bytes = PARSE_CONTEXT_MEMORY_BLOCK_SIZE_DEFAULT;
        if(size > needed_bytes)
        {
            needed_bytes = size;
        }
        
        ParseContextMemoryBlock *new_block = 0;
        new_block = calloc(1, sizeof(ParseContextMemoryBlock) +
                           PARSE_CONTEXT_MEMORY_BLOCK_SIZE_DEFAULT);
        Assert(new_block != 0);
        new_block->memory = (char *)new_block + sizeof(ParseContextMemoryBlock);
        new_block->memory_size = needed_bytes;
        new_block->next = 0;
        
        if(context->active_block)
        {
            context->active_block->next = new_block;
            context->active_block = new_block;
        }
        else
        {
            context->first_block = new_block;
            context->active_block = new_block;
        }
    }
    
    Assert(context->active_block &&
           context->active_block->memory_alloc_position + size <=
           context->active_block->memory_size);
    
    void *memory = context->active_block->memory + context->active_block->memory_alloc_position;
    context->active_block->memory_alloc_position += size;
    return memory;
}

static ASTNode *
ParseContextAllocateASTNode(ParseContext *context)
{
    ASTNode *node = ParseContextAllocateMemory(context, sizeof(ASTNode));
    MemorySet(node, 0, sizeof(ASTNode));
    return node;
}

static char *
ParseContextAllocateStringCopyLowercaseWithUnderscores(ParseContext *context, char *string)
{
    char *new_string = 0;
    if(string)
    {
        int last_character_was_uppercase = 0;
        int bytes_needed = 0;
        
        for(int i = 0; string[i]; ++i)
        {
            if(string[i] >= 'A' && string[i] <= 'Z')
            {
                last_character_was_uppercase = 1;
            }
            else if(string[i] >= 'a' && string[i] <= 'z' &&
                    last_character_was_uppercase)
            {
                ++bytes_needed;
            }
            ++bytes_needed;
        }
        
        ++bytes_needed;
        
        new_string = ParseContextAllocateMemory(context, bytes_needed);
        int new_string_write_pos = 0;
        
        for(int i = 0; string[i]; ++i)
        {
            if(string[i] >= 'A' && string[i] <= 'Z')
            {
                last_character_was_uppercase = 1;
            }
            else if(string[i] >= 'a' && string[i] <= 'z' &&
                    last_character_was_uppercase)
            {
                new_string[new_string_write_pos++] = '_';
            }
            
            new_string[new_string_write_pos++] = CharToLower(string[i]);
        }
    }
    return new_string;
}

static char *
ParseContextAllocateStringCopyUppercaseWithUnderscores(ParseContext *context, char *string)
{
    char *new_string = 0;
    new_string = ParseContextAllocateStringCopyLowercaseWithUnderscores(context, string);
    for(int i = 0; new_string[i]; ++i)
    {
        new_string[i] = CharToUpper(new_string[i]);
    }
    return new_string;
}

static char *
ParseContextAllocateStringCopyLowerCamelCase(ParseContext *context, char *string)
{
    char *new_string = 0;
    if(string)
    {
        
    }
    return new_string;
}

static char *
ParseContextAllocateStringCopyUpperCamelCase(ParseContext *context, char *string)
{
    char *new_string = 0;
    if(string)
    {
        
    }
    return new_string;
}

static void
ParseContextPushError(ParseContext *context, Tokenizer *tokenizer, char *msg, ...)
{
    if(!context->error_stack)
    {
        context->error_stack_max = 16;
        context->error_stack = ParseContextAllocateMemory(context,
                                                          sizeof(ParseError) * context->error_stack_max);
    }
    
    if(context->error_stack_size < context->error_stack_max)
    {
        va_list args;
        va_start(args, msg);
        unsigned int msg_bytes = vsnprintf(0, 0, msg, args)+1;
        va_end(args);
        va_start(args, msg);
        char *stored_msg = ParseContextAllocateMemory(context, msg_bytes);
        Assert(stored_msg != 0);
        vsnprintf(stored_msg, msg_bytes, msg, args);
        va_end(args);
        
        ParseError error = {
            stored_msg,
            tokenizer->filename,
            tokenizer->line,
        };
        context->error_stack[context->error_stack_size++] = error;
    }
}

static ASTNode *
ParseStruct(Tokenizer *tokenizer, ParseContext *context);

static ASTNode *
ParseExpression(Tokenizer *tokenizer, ParseContext *context)
{
    ASTNode *expression = 0;
    
    Token token = PeekToken(tokenizer);
    
    // TODO(rjf): Check for prefix unary operators
    
    if(TokenMatch(token, "("))
    {
        NextToken(tokenizer);
        expression = ParseExpression(tokenizer, context);
        if(RequireToken(tokenizer, ")", 0));
    }
    else if(token.type == TOKEN_alphanumeric_block)
    {
        NextToken(tokenizer);
        ASTNode *identifier = ParseContextAllocateASTNode(context);
        identifier->type = DATA_DESK_AST_NODE_TYPE_identifier;
        identifier->string = token.string;
        identifier->string_length = token.string_length;
        expression = identifier;
    }
    else if(token.type == TOKEN_numeric_constant)
    {
        NextToken(tokenizer);
        ASTNode *numeric_constant = ParseContextAllocateASTNode(context);
        numeric_constant->type = DATA_DESK_AST_NODE_TYPE_numeric_constant;
        numeric_constant->string = token.string;
        numeric_constant->string_length = token.string_length;
        expression = numeric_constant;
    }
    else if(token.type == TOKEN_string_constant)
    {
        NextToken(tokenizer);
        ASTNode *string_constant = ParseContextAllocateASTNode(context);
        string_constant->type = DATA_DESK_AST_NODE_TYPE_string_constant;
        string_constant->string = token.string;
        string_constant->string_length = token.string_length;
        expression = string_constant;
    }
    else if(token.type == TOKEN_char_constant)
    {
        NextToken(tokenizer);
        ASTNode *char_constant = ParseContextAllocateASTNode(context);
        char_constant->type = DATA_DESK_AST_NODE_TYPE_char_constant;
        char_constant->string = token.string;
        char_constant->string_length = token.string_length;
        expression = char_constant;
    }
    else
    {
        ParseContextPushError(context, tokenizer, "Unexpected token");
    }
    
    token = PeekToken(tokenizer);
    
    if(TokenMatch(token, ")"))
    {
        goto end_parse;
    }
    else if(TokenMatch(token, "]"))
    {
        goto end_parse;
    }
    else if(token.type == TOKEN_symbolic_block)
    {
        int binary_operator_type = GetBinaryOperatorTypeFromToken(token);
        NextToken(tokenizer);
        
        if(binary_operator_type != DATA_DESK_BINARY_OPERATOR_TYPE_invalid)
        {
            ASTNode *binary_operator = ParseContextAllocateASTNode(context);
            binary_operator->type = DATA_DESK_AST_NODE_TYPE_binary_operator;
            binary_operator->binary_operator.type = binary_operator_type;
            binary_operator->binary_operator.left = expression;
            
            int is_guarded_expression = TokenMatch(PeekToken(tokenizer), "(");
            
            binary_operator->binary_operator.right = ParseExpression(tokenizer, context);
            
            // NOTE(rjf): Solve precedence problems
            if(binary_operator->binary_operator.right->type == DATA_DESK_AST_NODE_TYPE_binary_operator)
            {
                ASTNode *tree_1 = binary_operator;
                ASTNode *tree_2 = binary_operator->binary_operator.right;
                
                if(!is_guarded_expression &&
                   tree_1->binary_operator.type > tree_2->binary_operator.type)
                {
                    
                    // NOTE(rjf): Flip operators
                    {
                        int swap = tree_1->binary_operator.type;
                        tree_1->binary_operator.type = tree_2->binary_operator.type;
                        tree_2->binary_operator.type = swap;
                    }
                    
                    // NOTE(rjf): Rotate nodes
                    {
                        ASTNode *original_left_most_node = tree_1->binary_operator.left;
                        ASTNode *original_middle_node = tree_2->binary_operator.left;
                        ASTNode *original_right_most_node = tree_2->binary_operator.right;
                        
                        tree_1->binary_operator.left = original_right_most_node;
                        tree_2->binary_operator.left = original_left_most_node;
                        tree_2->binary_operator.right = original_middle_node;
                    }
                    
                    // NOTE(rjf): Flip top tree left/right
                    {
                        ASTNode *swap = tree_1->binary_operator.left;
                        tree_1->binary_operator.left = tree_1->binary_operator.right;
                        tree_1->binary_operator.right = swap;
                    }
                }
            }
            
            expression = binary_operator;
        }
        else
        {
            ParseContextPushError(context, tokenizer, "Unexpected token %.*s", token.string_length, token.string);
        }
    }
    else
    {
        ParseContextPushError(context, tokenizer, "Unexpected token %.*s", token.string_length, token.string);
    }
    
    // TODO(rjf): Check for postfix unary operators
    
    end_parse:;
    return expression;
}

static ASTNode *
ParseTypeUsage(Tokenizer *tokenizer, ParseContext *context)
{
    ASTNode *type = 0;
    
    int pointer_count = 0;
    while(1)
    {
        if(RequireToken(tokenizer, "*", 0))
        {
            ++pointer_count;
        }
        else
        {
            break;
        }
    }
    
    ASTNode *struct_declaration_type = 0;
    char *type_name_string = 0;
    int type_name_string_length = 0;
    
    if(TokenMatch(PeekToken(tokenizer), "struct"))
    {
        type_name_string = "ANONYMOUSSTRUCT";
        type_name_string_length = CalculateCStringLength(type_name_string);
        struct_declaration_type = ParseStruct(tokenizer, context);
    }
    else
    {
        Token type_name = {0};
        if(!RequireTokenType(tokenizer, TOKEN_alphanumeric_block, &type_name))
        {
            ParseContextPushError(context, tokenizer, "Missing type name in declaration");
            goto end_parse;
        }
        type_name_string = type_name.string;
        type_name_string_length = type_name.string_length;
    }
    
    type = ParseContextAllocateASTNode(context);
    type->type = DATA_DESK_AST_NODE_TYPE_type_usage;
    type->type_usage.pointer_count = pointer_count;
    type->type_usage.struct_declaration = struct_declaration_type;
    type->string = type_name_string;
    type->string_length = type_name_string_length;
    
    ASTNode **array_size_target = &type->type_usage.first_array_size_expression;
    
    while(1)
    {
        if(RequireToken(tokenizer, "[", 0))
        {
            *array_size_target = ParseExpression(tokenizer, context);
            array_size_target = &(*array_size_target)->next;
            
            if(!RequireToken(tokenizer, "]", 0))
            {
                ParseContextPushError(context, tokenizer, "Missing ]");
                goto end_parse;
            }
        }
        else
        {
            break;
        }
    }
    
    end_parse:;
    return type;
}

static ASTNode *
ParseDeclaration(Tokenizer *tokenizer, ParseContext *context)
{
    ASTNode *declaration = 0;
    
    Token declaration_name = {0};
    
    if(!RequireTokenType(tokenizer, TOKEN_alphanumeric_block, &declaration_name))
    {
        ParseContextPushError(context, tokenizer, "Missing identifier for declaration");
        goto end_parse;
    }
    
    if(!RequireToken(tokenizer, ":", 0))
    {
        ParseContextPushError(context, tokenizer, "Missing : for declaration");
        goto end_parse;
    }
    
    declaration = ParseContextAllocateASTNode(context);
    declaration->type = DATA_DESK_AST_NODE_TYPE_declaration;
    declaration->string = declaration_name.string;
    declaration->string_length = declaration_name.string_length;
    declaration->declaration.type = ParseTypeUsage(tokenizer, context);
    
    end_parse:;
    return declaration;
}

static ASTNode *
ParseStruct(Tokenizer *tokenizer, ParseContext *context)
{
    ASTNode *struct_declaration = 0;
    
    if(!RequireToken(tokenizer, "struct", 0))
    {
        ParseContextPushError(context, tokenizer, "Struct keyword not found");
        goto end_parse;
    }
    
    Token struct_name = {0};
    
    if(!RequireTokenType(tokenizer, TOKEN_alphanumeric_block, &struct_name));
    
    if(!RequireToken(tokenizer, "{", 0))
    {
        ParseContextPushError(context, tokenizer, "Missing { after struct name");
        goto end_parse;
    }
    
    struct_declaration = ParseContextAllocateASTNode(context);
    struct_declaration->type = DATA_DESK_AST_NODE_TYPE_struct_declaration;
    struct_declaration->string = struct_name.string;
    struct_declaration->string_length = struct_name.string_length;
    
    ASTNode **member_store_target = &struct_declaration->struct_declaration.first_member;
    
    while(1)
    {
        Token tag = {0};
        if(RequireTokenType(tokenizer, TOKEN_tag, &tag));
        
        Tokenizer reset_tokenizer = *tokenizer;
        
        if(TokenMatch(PeekToken(tokenizer), "struct"))
        {
            ASTNode *sub_struct_declaration = ParseStruct(tokenizer, context);
            sub_struct_declaration->tag = tag.string;
            sub_struct_declaration->tag_length = tag.string_length;
            *member_store_target = sub_struct_declaration;
            member_store_target = &(*member_store_target)->next;
        }
        else
        {
            if(PeekToken(tokenizer).type != TOKEN_alphanumeric_block)
            {
                break;
            }
            
            NextToken(tokenizer);
            
            if(!TokenMatch(PeekToken(tokenizer), ":"))
            {
                break;
            }
            
            *tokenizer = reset_tokenizer;
            ASTNode *declaration = ParseDeclaration(tokenizer, context);
            
            declaration->tag = tag.string;
            declaration->tag_length = tag.string_length;
            
            if(!RequireToken(tokenizer, ";", 0))
            {
                ParseContextPushError(context, tokenizer, "Missing ;");
                goto end_parse;
            }
            
            *member_store_target = declaration;
            member_store_target = &declaration->next;
        }
    }
    
    if(!RequireToken(tokenizer, "}", 0))
    {
        ParseContextPushError(context, tokenizer, "Missing } after struct declaration");
        goto end_parse;
    }
    
    end_parse:;
    return struct_declaration;
}

static ASTNode *
ParseCode(Tokenizer *tokenizer, ParseContext *context)
{
    ASTNode *root = 0;
    ASTNode **node_store_target = &root;
    Token token = {0};
    
    do
    {
        Token tag = {0};
        if(RequireTokenType(tokenizer, TOKEN_tag, &tag));
        
        token = PeekToken(tokenizer);
        
        if(token.type != TOKEN_invalid)
        {
            if(TokenMatch(token, "struct"))
            {
                ASTNode *struct_declaration = ParseStruct(tokenizer, context);
                struct_declaration->tag = tag.string;
                struct_declaration->tag_length = tag.string_length;
                *node_store_target = struct_declaration;
                node_store_target = &(*node_store_target)->next;
            }
            else if(PeekToken(tokenizer).type == TOKEN_alphanumeric_block)
            {
                Tokenizer reset_tokenizer = *tokenizer;
                
                if(RequireTokenType(tokenizer, TOKEN_alphanumeric_block, 0) &&
                   RequireToken(tokenizer, ":", 0))
                {
                    *tokenizer = reset_tokenizer;
                    ASTNode *declaration = ParseDeclaration(tokenizer, context);
                    declaration->tag = tag.string;
                    declaration->tag_length = tag.string_length;
                    *node_store_target = declaration;
                    node_store_target = &(*node_store_target)->next;
                    
                    if(!RequireToken(tokenizer, ";", 0))
                    {
                        ParseContextPushError(context, tokenizer, "Missing ;");
                    }
                }
                else
                {
                    ParseContextPushError(context, tokenizer, "Unexpected token %.*s",
                                          token.string_length, token.string);
                    NextToken(tokenizer);
                }
            }
            else
            {
                ParseContextPushError(context, tokenizer, "Unexpected token %.*s",
                                      token.string_length, token.string);
                NextToken(tokenizer);
            }
        }
        
        if(context->error_stack_size)
        {
            break;
        }
    }
    while(token.type != TOKEN_invalid);
    
    return root;
}