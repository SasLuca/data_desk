typedef struct Tokenizer
{
    char *at;
    char *filename;
    int line;
}
Tokenizer;

enum
{
    TOKEN_invalid,
    TOKEN_alphanumeric_block,
    TOKEN_numeric_constant,
    TOKEN_string_constant,
    TOKEN_char_constant,
    TOKEN_symbolic_block,
    TOKEN_tag,
};

typedef struct Token
{
    int type;
    char *string;
    int string_length;
}
Token;

static Token
GetNextTokenFromBuffer(Tokenizer *tokenizer)
{
    Token token = {0};
    
    int skip_mode = 0;
    enum
    {
        SKIP_MODE_none,
        SKIP_MODE_line_comment,
        SKIP_MODE_block_comment,
    };
    int block_comment_nest_level = 0;
    
    char *buffer = tokenizer->at;
    for(int i = 0; buffer[i]; ++i)
    {
        if(buffer[i] == '\n')
        {
            ++tokenizer->line;
        }
        
        if(skip_mode == SKIP_MODE_none)
        {
            if(buffer[i] == '/' && buffer[i+1] == '/')
            {
                skip_mode = SKIP_MODE_line_comment;
                ++i;
            }
            else if(buffer[i] == '/' && buffer[i+1] == '*')
            {
                block_comment_nest_level = 1;
                skip_mode = SKIP_MODE_block_comment;
                ++i;
            }
            else
            {
                
                int j = 0;
                
                // NOTE(rjf): Alphanumeric block
                if(CharIsAlpha(buffer[i]) || buffer[i] == '_')
                {
                    for(j = i+1; buffer[j]; ++j)
                    {
                        if(buffer[j] == '\n')
                        {
                            ++tokenizer->line;
                        }
                        
                        if(!CharIsAlpha(buffer[j]) && buffer[j] != '_' &&
                           !CharIsDigit(buffer[j]))
                        {
                            break;
                        }
                    }
                    token.type = TOKEN_alphanumeric_block;
                }
                
                // NOTE(rjf): Numeric block
                else if(CharIsDigit(buffer[i]))
                {
                    for(j = i+1; buffer[j]; ++j)
                    {
                        if(buffer[j] == '\n')
                        {
                            ++tokenizer->line;
                        }
                        
                        if(!CharIsAlpha(buffer[j]) && buffer[j] != '.' &&
                           !CharIsDigit(buffer[j]))
                        {
                            break;
                        }
                    }
                    token.type = TOKEN_numeric_constant;
                }
                
                // NOTE(rjf): String constant
                else if(buffer[i] == '"')
                {
                    
                    // NOTE(rjf): Multiline string constant
                    if(buffer[i+1] == '"' && buffer[i+2] == '"')
                    {
                        i += 3;
                        for(j = i+1; buffer[j]; ++j)
                        {
                            if(buffer[j] == '\n')
                            {
                                ++tokenizer->line;
                            }
                            
                            if(buffer[j] == '"' && buffer[j+1] == '"' && buffer[j+2] == '"')
                            {
                                break;
                            }
                        }
                    }
                    
                    // NOTE(rjf): Single line string constant
                    else
                    {
                        ++i;
                        for(j = i+1; buffer[j]; ++j)
                        {
                            if(buffer[j] == '\n')
                            {
                                ++tokenizer->line;
                            }
                            
                            if(buffer[j] == '"')
                            {
                                break;
                            }
                        }
                    }
                    token.type = TOKEN_string_constant;
                }
                
                // NOTE(rjf): Char constant
                else if(buffer[i] == '\'')
                {
                    ++i;
                    for(j = i+1; buffer[j]; ++j)
                    {
                        if(buffer[j] == '\'')
                        {
                            break;
                        }
                    }
                    token.type = TOKEN_char_constant;
                }
                
                // NOTE(rjf): Tag block
                else if(buffer[i] == '@')
                {
                    for(j = i+1; buffer[j]; ++j)
                    {
                        if(buffer[j] == '\n')
                        {
                            ++tokenizer->line;
                        }
                        
                        if(buffer[j] <= 32)
                        {
                            break;
                        }
                    }
                    token.type = TOKEN_tag;
                }
                
                // NOTE(rjf): Symbolic block
                else if(CharIsSymbol(buffer[i]))
                {
                    static char *symbolic_blocks_to_break_out[] = {
                        "(",
                        ")",
                        "[",
                        "]",
                        "{",
                        "}",
                        ";",
                        ":",
                        "*",
                        "+",
                        "-",
                        "/",
                    };
                    
                    for(j = i+1; buffer[j]; ++j)
                    {
                        if(buffer[j] == '\n')
                        {
                            ++tokenizer->line;
                        }
                        
                        for(int k = 0; k < ArrayCount(symbolic_blocks_to_break_out); ++k)
                        {
                            if(StringMatchCaseSensitiveN(symbolic_blocks_to_break_out[k],
                                                         buffer+i,
                                                         j-i))
                            {
                                goto end_symbolic_block;
                            }
                        }
                        
                        if(!CharIsSymbol(buffer[j]))
                        {
                            break;
                        }
                    }
                    end_symbolic_block:;
                    token.type = TOKEN_symbolic_block;
                }
                
                if(j)
                {
                    token.string = buffer+i;
                    token.string_length = j-i;
                    break;
                }
            }
        }
        else if(skip_mode == SKIP_MODE_line_comment)
        {
            if(buffer[i] == '\n')
            {
                skip_mode = 0;
            }
        }
        
        else if(skip_mode == SKIP_MODE_block_comment)
        {
            
            if(buffer[i] == '/' && buffer[i+1] == '*')
            {
                ++block_comment_nest_level;
                ++i;
            }
            else if(buffer[i] == '*' && buffer[i+1] == '/')
            {
                if(!--block_comment_nest_level)
                {
                    skip_mode = 0;
                }
                ++i;
            }
        }
        
        
    }
    
    return token;
}

static Token
PeekToken(Tokenizer *tokenizer)
{
    Token token = GetNextTokenFromBuffer(tokenizer);
    return token;
}

static Token
NextToken(Tokenizer *tokenizer)
{
    Token token = GetNextTokenFromBuffer(tokenizer);
    tokenizer->at = token.string + token.string_length;
    return token;
}

static int
TokenMatch(Token token, char *string)
{
    return (token.type != TOKEN_invalid &&
            StringMatchCaseSensitiveN(token.string, string, token.string_length) &&
            string[token.string_length] == 0);
}

static int
RequireToken(Tokenizer *tokenizer, char *string, Token *token_ptr)
{
    int match = 0;
    Token token = GetNextTokenFromBuffer(tokenizer);
    if(TokenMatch(token, string))
    {
        tokenizer->at = token.string + token.string_length;
        if(token_ptr)
        {
            *token_ptr = token;
        }
        match = 1;
    }
    return match;
}

static int
RequireTokenType(Tokenizer *tokenizer, int type, Token *token_ptr)
{
    int match = 0;
    Token token = GetNextTokenFromBuffer(tokenizer);
    if(type == token.type)
    {
        tokenizer->at = token.string + token.string_length;
        if(token_ptr)
        {
            *token_ptr = token;
        }
        match = 1;
    }
    return match;
}

static char *
GetBinaryOperatorStringFromType(int type)
{
    static char *binary_operator_strings[] = {
        "INVALIDBINARYOPERATOR",
        "+",
        "-",
        "*",
        "/",
    };
    
    return binary_operator_strings[type];
}

static int
GetBinaryOperatorTypeFromToken(Token token)
{
    int type = DATA_DESK_BINARY_OPERATOR_TYPE_invalid;
    
    for(int i = 1; i < DATA_DESK_BINARY_OPERATOR_TYPE_MAX; ++i)
    {
        if(TokenMatch(token, GetBinaryOperatorStringFromType(i)))
        {
            type = i;
            break;
        }
    }
    
    return type;
}