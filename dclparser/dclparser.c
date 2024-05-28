#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/*
 * Sudo code:
 *      - first determine the datatype (type specifier and type qualifier) tokens
 *        and store it in the datatype array.
 *      - call dcl to check for pointers (incl. types such as const, volatile etc.) tokens
 *      - call dirdcl to determine other tokens i.e. name, brackets, parethesis, '(', ')' etc.
 */
#define MAXTOKEN 500
#define MAXOUTPUT 5000
                                
#define BUFSIZE MAXTOKEN * 2
#define NUMOFTYPEQUALIFIERS 4
#define NUMOFTYPESPECIFIERS 11
#define MAXTYPEQUALIFIERTOKENS 10
#define MAXTYPEQUALIFIERTOKENLENGTH 30

enum tokentype { VARIABLE = 0, BRACKETS, PARENS, TYPEQUALIFIER, TYPESPECIFIER };
enum returnstatus { OK = 10, ERROR = 11 };
enum boolean { FALSE, TRUE };

int processdeclaration(char *);
int dcl(char *, char *);
int dirdcl(char *, char *);
int gettoken(void);
int istype(char *[], char *, int);
int get_ch(void);
void unget_ch(int);
char *safestrcat(char *, const char *, size_t);
int error(char *);
char **allocmem(void);
void deallocmem(char **v, int);

int tokentype;          // type of last token
char token[MAXTOKEN];   // last token string
int buffer[BUFSIZE];    // shared by get_ch and unget_ch
int bufp = 0;           // next free position in buffer
char **qtoken;          // array of pointers to store type qualifiers after pointers
int qcount = 0;         // var to keep track of type qualifiers token
static char *type_qualifier[NUMOFTYPEQUALIFIERS] = {
    "_Atomic",
    "const",
    "restrict",
    "volatile"
};
static char *type_specifier[NUMOFTYPESPECIFIERS] = {
    "_Bool",
    "_Complex",
    "char",
    "double",
    "float",
    "int",
    "long",
    "short",
    "signed",
    "unsigned",
    "void"
};

void printtoken(void)
{
    char out[MAXOUTPUT];
    while (gettoken() != '~')
    {
        processdeclaration(out);
        for (int c = tokentype; c != '\n' && c != '~'; ) {
            if ((c = get_ch()) == '~')
                break;
        }
    }
}

char datatype[MAXTOKEN];
char typequalifier[MAXTOKEN];
char typespecifier[MAXTOKEN];
int processdeclaration(char *declaration)
{
    char varname[MAXTOKEN];     // stores variable/function name
    char output[MAXOUTPUT];     // stores final output after processing declaration
    typespecifier[0] = '\0';    // ensure null terminated
    typequalifier[0] = '\0';    // ensure null terminated
    datatype[0] = '\0';         // ensure null terminated
    
    // Process for datatype tokens
    if (!(tokentype == TYPESPECIFIER || tokentype == TYPEQUALIFIER))
        return error("Error: expected type");
    
    while (tokentype == TYPESPECIFIER || tokentype == TYPEQUALIFIER) {
        if (tokentype == TYPESPECIFIER) {
            if (safestrcat(typespecifier, " ", MAXTOKEN) == NULL)
                return error("Error: typespecifier buffer full");
            if (safestrcat(typespecifier, token, MAXTOKEN) == NULL)
                return error("Error: typespecifier buffer full");
        }
        if (tokentype == TYPEQUALIFIER) {
            if (safestrcat(typequalifier, " ", MAXTOKEN) == NULL)
                return error("Error: typequalifier buffer full");
            if (safestrcat(typequalifier, token, MAXTOKEN) == NULL)
                return error("Error: typequalifier buffer full");
        }
        gettoken(); // get tokens until token is no longer datatype
    }
    
    // Concatenate the types to build datatype
    if (safestrcat(datatype, typequalifier, MAXTOKEN) == NULL)
        return error("Error: datatype buffer full");
    if (safestrcat(datatype, typespecifier, MAXTOKEN) == NULL)
        return error("Error: datatype buffer full");
    
    // Push back the unneeded token fetched by while loop above
    for (long i = strlen(token) - 1; i >= 0; i--)
        unget_ch(token[i]);
    
    // Process tokens for variable, *, (), [] and other tokens, invoke dcl and dirdcl (recursive)
    output[0] = '\0';           // ensure null termination
    qtoken = allocmem();        // allocate array to store type qualifiers string tokens
    
    if (dcl(varname, output) == ERROR) {
        deallocmem(qtoken, MAXTYPEQUALIFIERTOKENS);
        return ERROR;
    }
    if (tokentype != '\n') {
        deallocmem(qtoken, MAXTYPEQUALIFIERTOKENS);
        return error("Error: Syntax error");
    }
    printf("%s:%s%s\n", varname, output, datatype);
    deallocmem(qtoken, MAXTYPEQUALIFIERTOKENS);
    return OK;
}

int dcl(char *name, char *out)
{
    int npointers = 0;              // number of '*'
    typequalifier[0] = '\0';        // ensure null terminated, reset array
    
    while (gettoken() == '*') {
        npointers++;
        
        if (gettoken() == TYPEQUALIFIER) {          // check for pointer with type qualifier
            do {
                if (safestrcat(typequalifier, " ", MAXTOKEN) == NULL)
                    return error("Error: typequalifier buffer in dcl full");
                if (safestrcat(typequalifier, token, MAXTOKEN) == NULL)
                    return error("Error: typequalifier buffer in dcl full");
            } while (gettoken() == TYPEQUALIFIER);  // if true then adjacent qualifiers
            strcpy(qtoken[qcount], typequalifier);
            typequalifier[0] = '\0';
        } else
            qtoken[qcount] = NULL;

        qcount++;       // to sync with number of '*'
        
        // Push back the unneeded token fetched by do-while loop above
        for (long i = strlen(token) - 1; i >= 0; i--)
            unget_ch(token[i]);
    }
    
    // Call dirdcl to check for name, (), [], name, '(' etc.
    if (dirdcl(name, out) == ERROR)
        return ERROR;
    
    // No. of pointers and no. of qtokens are synced
    // If qtoken[i] == NULL then normal pointer
    // If qtoken[i] != NULL then pointer with qualifier
    while (npointers-- > 0) {
        if(qtoken[--qcount] == NULL) {
            if (safestrcat(out, " pointer to", MAXOUTPUT) == NULL)
                return error("Error: out buffer in dcl full");
        } else {
            if (safestrcat(out, qtoken[qcount], MAXOUTPUT) == NULL)
                return error("Error: out buffer in dcl full");
            if (safestrcat(out, " pointer to", MAXOUTPUT) == NULL)
                return error("Error: out buffer in dcl full");
        }
    }
    return OK;
}

int dirdcl(char *name, char *out)
{
    int type;
    
    if (tokentype == '(') {
        if (dcl(name, out) == ERROR)        // call dcl to check for parenthesized tokens with '*'
            return ERROR;
        if (tokentype != ')')
            return error("Error: missing )");
    } else if (tokentype == VARIABLE)
        strcpy(name, token);
    else
        return error("Error: expected variable name or (dcl)");
         
    while ((type = gettoken()) == PARENS || type == BRACKETS) {
        if (type == PARENS) {
            if (safestrcat(out, " function", MAXOUTPUT) == NULL)
                return error("Error: out buffer in dirdcl full");
            if (safestrcat(out, token, MAXOUTPUT) == NULL)
                return error("Error: out buffer in dirdcl full");
            if (safestrcat(out, " returning", MAXOUTPUT) == NULL)
                return error("Error: out buffer in dirdcl full");
        }
        else {
            if (safestrcat(out, " array", MAXOUTPUT) == NULL)
                return error("Error: out buffer in dirdcl full");
            if (safestrcat(out, token, MAXOUTPUT) == NULL)
                return error("Error: out buffer in dirdcl full");
            if (safestrcat(out, " of", MAXOUTPUT) == NULL)
                return error("Error: out buffer in dirdcl full");
        }
    }
    if (type == ERROR)
        return ERROR;
    return OK;
}

// gettoken: check for token types: VARIABLE, BRACKETS, PARENS, TYPESPECIFIER, TYPEQUALIFIER
int gettoken(void)
{
    int c, temp;
    char *p = token;
    
    while ((c = get_ch()) == ' ' || c == '\t');     // skip white spaces and tabs
    
    if (c == '(') {                                 // check for PARENS
        int paren_start = c;
        if ((c = get_ch()) == ')') {
            strcpy(token, "()");
            return tokentype = PARENS;
        }
        else if (c != '*') {
            for (*p++ = paren_start, *p++ = c; (temp = *p++ = get_ch()) != ')'; ) {
                if (temp == '\n')
                    return error("Error: missing )");
            }
            *p = '\0';
            return tokentype = PARENS;
        }
        else {
            unget_ch(c);
            *(p + 1) = '\0'; // terminate token string
            return *p = tokentype = paren_start;
        }
    } else if (c == '[') {                          // check for BRACKETS
        for (*p++ = c; (temp = *p++ = get_ch()) != ']'; ) {
            if (temp == '\n')
                return error("Error: missing ]");
        }
        *p = '\0';
        return tokentype = BRACKETS;
    } else if (isalpha(c)) {                        // check for TYPES
        for (*p++ = c; isalnum(c = get_ch()) || c == '_'; )
            *p++ = c;
        *p = '\0';
        unget_ch(c); // push back unneeded extra char to process later
        
        if (istype(type_specifier, token, NUMOFTYPESPECIFIERS))
            return tokentype = TYPESPECIFIER;
        else if (istype(type_qualifier, token, NUMOFTYPEQUALIFIERS))
            return tokentype = TYPEQUALIFIER;
        else
            return tokentype = VARIABLE;
    }
    
    *(p + 1) = '\0';            // terminate token string
    return *p = tokentype = c;  // none of the types above, return char as type and store it in the token
}

// get_ch and unget_ch: Get char from input or incase of extra char
// that needs to be processed later, uses buffer to store and retrieve.
// the char. "buffer" is shared between get_ch and unget_ch.
int get_ch(void)
{
    return (bufp > 0) ? buffer[--bufp] : getchar();
}

void unget_ch(int c)
{
    if (bufp >= BUFSIZE)
        printf("unget_ch: buffer full, too many characters\n");
    else
        buffer[bufp++] = c;
}

// istype: Check if type is among the types specified
int istype(char *types[], char *type, int ntype)
{
    for (int i = 0; i < ntype; i++) {
        if (strcmp(types[i], type) == 0)
            return TRUE;
    }
    return FALSE;
}

// safestrcat: Concatenates str to end of dest.
// Requires str i.e. should be null-terminated string and dest buffer size
// Returns null if provided bad pointers for dest or str or buffer is too small
// other return pointer dest.
char *safestrcat(char *dest, const char *str, size_t destsize)
{
    if (dest == NULL || str == NULL)    // if either pointer is NULL, return NULL
        return NULL;
    char *dest_start = dest;            // track dest start address
    size_t dest_len = strlen(dest), str_len = strlen(str);
    if (dest_len + str_len >= destsize) // check space is available, strlen doesn't count '\0'
        return NULL;
    dest += dest_len;                   // move pointer to next free position i.e. '\0' at end of dest string value
    
    while ((*dest++ = *str++));         // copy str to dest until str reaches '\0'
    return dest_start;                  // return pointer to dest start
}

// error: Print error message and return ERROR.
int error(char *msg)
{
    printf("%s\n", msg);
    return ERROR;
}

char **allocmem(void)
{
    char **A = malloc(sizeof(char *) * MAXTYPEQUALIFIERTOKENS);
    for (int i = 0; i < 10; i++) {
        A[i] = malloc(sizeof(char) * MAXTYPEQUALIFIERTOKENLENGTH);
    }
    return A;
}

void deallocmem(char **v, int length)
{
     for (int i = 0; i < length; i++) {
         free(v[i]);
     }
     free(v);
}

