#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
            }
        }
        
        fprintf(file, "}\n\n");
    }
}

void
DataDeskCustomDeclarationCallback(DataDeskDeclaration declaration_info, char *filename)
{
    
}