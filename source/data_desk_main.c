// NOTE(rjf): Platform-Specific
#if BUILD_WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif BUILD_LINUX
#include <dlfcn.h>
#endif

// NOTE(rjf): C Runtime Library
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

// NOTE(rjf): Data Desk Code
#include "data_desk.h"
#include "data_desk_utilities.c"
#include "data_desk_debug.c"
#include "data_desk_tokenizer.c"
#include "data_desk_custom.c"
#include "data_desk_parse.c"
#include "data_desk_ast_traverse.c"

static void
ProcessFile(DataDeskCustom custom, char *file, char *filename)
{
    Tokenizer tokenizer = {0};
    {
        tokenizer.at = file;
        tokenizer.filename = filename;
        tokenizer.line = 1;
    }
    
    ParseContext context = {0};
    
    ASTNode *root = ParseCode(&tokenizer, &context);
    GenerateNullTerminatedStringsForAST(&context, root);
    TraverseASTAndCallCustomParseCallbacks(&context, root, custom, filename);
    
    for(int i = 0; i < context.error_stack_size; ++i)
    {
        fprintf(stderr,
                "ERROR (%s:%i): %s\n",
                context.error_stack[i].file,
                context.error_stack[i].line,
                context.error_stack[i].string);
    }
}

int
main(int argument_count, char **arguments)
{
    if(argument_count > 1)
    {
        Log("Data Desk v0.1");
        
        DataDeskCustom custom = {0};
        char *custom_layer_dll_path = 0;
        
        // NOTE(rjf): Load command line arguments and set all non-file arguments
        // to zero, so that we know the arguments to process in the file-processing
        // loop.
        {
            int argument_read_mode = 0;
            enum
            {
                ARGUMENT_READ_MODE_files,
                ARGUMENT_READ_MODE_custom_layer_dll,
            };
            
            for(int i = 1; i < argument_count; ++i)
            {
                if(argument_read_mode == ARGUMENT_READ_MODE_files)
                {
                    if(StringMatchCaseInsensitive(arguments[i], "-c") ||
                       StringMatchCaseInsensitive(arguments[i], "--custom"))
                    {
                        argument_read_mode = ARGUMENT_READ_MODE_custom_layer_dll;
                        arguments[i] = 0;
                    }
                    else if(StringMatchCaseInsensitive(arguments[i], "-l") ||
                            StringMatchCaseInsensitive(arguments[i], "--log"))
                    {
                        global_log_enabled = 1;
                        arguments[i] = 0;
                    }
                }
                else if(argument_read_mode == ARGUMENT_READ_MODE_custom_layer_dll)
                {
                    custom_layer_dll_path = arguments[i];
                    arguments[i] = 0;
                    argument_read_mode = ARGUMENT_READ_MODE_files;
                }
            }
        }
        
        // NOTE(rjf): Load custom code DLL if needed.
        if(custom_layer_dll_path)
        {
            Log("Loading custom layer from \"%s\"", custom_layer_dll_path);
            custom = DataDeskCustomLoad(custom_layer_dll_path);
        }
        else
        {
            Log("WARNING: No custom layer loaded");
        }
        
        if(custom.InitCallback)
        {
            custom.InitCallback();
        }
        
        for(int i = 1; i < argument_count; ++i)
        {
            if(arguments[i] != 0)
            {
                char *filename = arguments[i];
                Log("Processing file at \"%s\"", filename);
                char *file = LoadEntireFileAndNullTerminate(filename);
                if(file)
                {
                    if(custom.FileCallback)
                    {
                        custom.FileCallback(filename);
                    }
                    ProcessFile(custom, file, filename);
                }
                else
                {
                    Log("ERROR: Could not load \"%s\"", filename);
                }
            }
        }
        
        if(custom.CleanUpCallback)
        {
            custom.CleanUpCallback();
        }
        
        DataDeskCustomUnload(&custom);
    }
    else
    {
        fprintf(stderr, "USAGE: %s [-c|--custom <path to custom layer DLL>] [-l|--log] <files to process>\n",
                arguments[0]);
    }
    
    return 0;
}