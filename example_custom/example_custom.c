#include "data_desk.h"

static FILE *global_generation_file = 0;

DATA_DESK_FUNC void
DataDeskCustomInitCallback(void)
{
    global_generation_file = fopen("generated.c", "w");
}

DATA_DESK_FUNC void
DataDeskCustomFileCallback(char *filename)
{
}

DATA_DESK_FUNC void
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
                if(DataDeskStructMemberIsType(member, "int"))
                {
                    fprintf(file, "    printf(\"%%i\", object->%s);\n", member->string);
                }
                else if(DataDeskStructMemberIsType(member, "char"))
                {
                    fprintf(file, "    printf(\"%%c\", object->%s);\n", member->string);
                }
                else if(DataDeskStructMemberIsType(member, "float"))
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

DATA_DESK_FUNC void
DataDeskCustomDeclarationCallback(DataDeskDeclaration declaration_info, char *filename)
{
}

DATA_DESK_FUNC void
DataDeskCustomCleanUpCallback(void)
{
}
