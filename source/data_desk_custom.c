typedef struct DataDeskCustom
{
    DataDeskInitCallback *InitCallback;
    DataDeskFileCallback *FileCallback;
    DataDeskStructCallback *StructCallback;
    DataDeskDeclarationCallback *DeclarationCallback;
    DataDeskCleanUpCallback *CleanUpCallback;
    
#if BUILD_WIN32
    HANDLE custom_dll;
#elif BUILD_LINUX
    void *custom_dll;
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
        custom.InitCallback = (void *)GetProcAddress(custom.custom_dll, "DataDeskCustomInitCallback");
        custom.FileCallback = (void *)GetProcAddress(custom.custom_dll, "DataDeskCustomFileCallback");
        custom.StructCallback = (void *)GetProcAddress(custom.custom_dll, "DataDeskCustomStructCallback");
        custom.DeclarationCallback = (void *)GetProcAddress(custom.custom_dll, "DataDeskCustomDeclarationCallback");
        custom.CleanUpCallback = (void *)GetProcAddress(custom.custom_dll, "DataDeskCustomCleanUpCallback");
    }
#elif BUILD_LINUX
    custom.custom_dll = dlopen(custom_dll_path, RTLD_NOW);
    if(custom.custom_dll)
    {
        custom.InitCallback = dlsym(custom.custom_dll, "DataDeskCustomInitCallback");
        custom.FileCallback = dlsym(custom.custom_dll, "DataDeskCustomFileCallback");
        custom.StructCallback = dlsym(custom.custom_dll, "DataDeskCustomStructCallback");
        custom.DeclarationCallback = dlsym(custom.custom_dll, "DataDeskCustomDeclarationCallback");
        custom.CleanUpCallback = dlsym(custom.custom_dll, "DataDeskCustomCleanUpCallback");
    }
#endif
    
    if(!custom.InitCallback && !custom.FileCallback &&
       !custom.StructCallback && !custom.DeclarationCallback &&
       !custom.CleanUpCallback)
    {
        fprintf(stdout, "WARNING: No callbacks successfully loaded in custom layer\n");
    }
    
    return custom;
}

static void
DataDeskCustomUnload(DataDeskCustom *custom)
{
    
#if BUILD_WIN32
    FreeLibrary(custom->custom_dll);
#elif BUILD_LINUX
    dlclose(custom->custom_dll);
#endif
    
    custom->InitCallback = 0;
    custom->FileCallback = 0;
    custom->StructCallback = 0;
    custom->DeclarationCallback = 0;
    custom->CleanUpCallback = 0;
    custom->custom_dll = 0;
}
