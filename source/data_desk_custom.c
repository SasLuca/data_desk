typedef struct DataDeskCustom
{
    DataDeskInitCallback *init_callback;
    DataDeskFileCallback *file_callback;
    DataDeskStructCallback *struct_callback;
    DataDeskDeclarationCallback *declaration_callback;
    
#if BUILD_WIN32
    HANDLE custom_dll;
#endif
    
}
DataDeskCustom;

static DataDeskCustom
DataDeskCustomLoad(char *custom_dll_path)
{
    DataDeskCustom custom = {0};
    
#if BUILD_WIN32
    custom.custom_dll = LoadLibraryA(custom_dll_path);
    if(custom.custom_dll)
    {
        custom.init_callback = (void *)GetProcAddress(custom.custom_dll, "DataDeskCustomInitCallback");
        custom.file_callback = (void *)GetProcAddress(custom.custom_dll, "DataDeskCustomFileCallback");
        custom.struct_callback = (void *)GetProcAddress(custom.custom_dll, "DataDeskCustomStructCallback");
        custom.declaration_callback = (void *)GetProcAddress(custom.custom_dll, "DataDeskCustomDeclarationCallback");
    }
#endif
    
    if(!custom.init_callback)
    {
        fprintf(stdout, "WARNING: No initialization callback found in custom layer\n");
    }
    
    if(!custom.file_callback)
    {
        fprintf(stdout, "WARNING: No file callback found in custom layer\n");
    }
    
    if(!custom.struct_callback)
    {
        fprintf(stdout, "WARNING: No struct callback found in custom layer\n");
    }
    
    if(!custom.declaration_callback)
    {
        fprintf(stdout, "WARNING: No declaration callback found in custom layer\n");
    }
    
    return custom;
}

static void
DataDeskCustomUnload(DataDeskCustom *custom)
{
    
#if BUILD_WIN32
    FreeLibrary(custom->custom_dll);
#endif
    
    custom->struct_callback = 0;
    custom->declaration_callback = 0;
    custom->custom_dll = 0;
}
