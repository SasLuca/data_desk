#include "data_desk.h"

static FILE *global_generation_file = 0;

void
DataDeskCustomInitCallback(void)
{
    global_generation_file = fopen("generated.c", "w");
}

void
DataDeskCustomFileCallback(char *filename)
{}

void
DataDeskCustomStructCallback(DataDeskStruct struct_info, char *filename)
{
    
    if(DataDeskStructHasTag(struct_info, "Printable"))
    {
        FILE *file = global_generation_file;
        
        DataDeskFWriteStructAsC(file, struct_info);
        
        DataDeskASTNode *root = struct_info.root;
        fprintf(file, "void\n");
        fprintf(file, "%sPrint(%s *object)\n", struct_info.name, struct_info.name);
        fprintf(file, "{\n");
        
        for(DataDeskASTNode *member = root->struct_declaration.first_member;
            member;
            member = member->next)
        {
            if(!DataDeskNodeHasTag(member, "NoPrint"))
            {
                if(DataDeskStructMemberIsType(member, "i32"))
                {
                    fprintf(file, "    printf(\"%%i\", object->%s);\n", member->string);
                }
                else if(DataDeskStructMemberIsType(member, "char"))
                {
                    fprintf(file, "    printf(\"%%c\", object->%s);\n", member->string);
                }
                else if(DataDeskStructMemberIsType(member, "f32"))
                {
                    fprintf(file, "    printf(\"%%f\", object->%s);\n", member->string);
                }
                else if(DataDeskStructMemberIsType(member, "*char"))
                {
                    fprintf(file, "    printf(\"%%s\", object->%s);\n", member->string);
                }
            }
        }
        
        fprintf(file, "}\n\n");
    }
    
}

void
DataDeskCustomDeclarationCallback(DataDeskDeclaration declaration_info, char *filename)
{
    if(DataDeskDeclarationHasTag(declaration_info, "Shader"))
    {
        FILE *file = global_generation_file;
        
        fprintf(file, "static const char *%s = \"\"\n", declaration_info.name);
        
        char *shader_string = declaration_info.root->declaration.initialization->string;
        int shader_string_length = declaration_info.root->declaration.initialization->string_length;
        if(shader_string && shader_string_length > 3)
        {
            fprintf(file, "\"");
            for(int i = 3; shader_string[i]; ++i)
            {
                if(shader_string[i] == '"' && shader_string[i+1] == '"' &&
                   shader_string[i+2] == '"')
                {
                    break;
                }
                else if(shader_string[i] == '\n')
                {
                    fprintf(file, "\\n\"\n\"");
                }
                else
                {
                    fprintf(file, "%c", shader_string[i]);
                }
            }
            fprintf(file, "\"");
        }
        
        fprintf(file, ";\n\n");
    }
}