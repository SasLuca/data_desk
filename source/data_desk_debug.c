#define _Assert(c, crash) if(!(c)) { _AssertFailure(#c, __LINE__, __FILE__, crash); }
#define Assert(c) _Assert(c, 1)
#define SoftAssert(c) _Assert(c, 0)

static void
_AssertFailure(char *condition, int line, char *file, int crash)
{
    fprintf(stderr, "ASSERTION FAILURE: %s at %s:%i\n", condition, file, line);
    if(crash)
    {
        *(int *)0 = 0;
    }
}

#define Log(...) fprintf(stderr, __VA_ARGS__)