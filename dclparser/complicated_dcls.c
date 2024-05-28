#include <stdio.h>

void complicated_declarations(void);
void declarators(void);
void typequalifier_rule(void);
void test_complicated_decls(void)
{
    complicated_declarations();
}

void test1(void);
void test2(void);
void test3(void);
struct IMAGE;
void complicated_declarations(void)
{
    // argv: pointer to pointer to char
    char **argv;
    char *s = "hello";
    argv = &s;
    
    // daytab: pointer to array of int
    int (*daytab)[7];
    int days[7] = { 1, 2, 3, 4, 5, 6, 7 };
    daytab = &days;
   
    // ip: array of pointers to int
    int *ip[3];
    int x, y, z;
    *ip = &x;
    *(ip + 1) = &y;
    *(ip + 2) = &z;
    
    // comp: function returning pointer to void
    void *comp(void);
    
    // func: pointer to function returning void
    void (*func)(void);
    func = test1;
    
    // calc: array[3] of pointer to function returning void
    void (*calc[3])(void);
    calc[0] = test1;
    calc[1] = test2;
    calc[2] = test3;
    
    // foo: function returning pointer to array of char
    char (*foo(void))[];
    
    // fun: function returning pointer to array[] of pointers to function returning char
    char (*(*fun(void))[])(void);
    
    // A: array[3] of pointer to function returning pointer to array[5] of char
    char (*(*A[3])(void))[5];
    
    
    // Type specifier with type qualifiers
    // num: const int
    int const num;
    
    // nbr: const int
    const int nbr;
    
    // nbr1: pointer to const volatile int
    const volatile int *nbr1;
    
    // pointer to const int
    int const *nbr3;
    
    // num1: pointer to const int
    int const (*num1)(void);
    
    // num2: const pointer to int
    int *const num2;
    
    // num3: pointer to volatile const int
    int const volatile *num3;
    
    // num4: pointer to volatile const pointer to int
    int *const volatile *num4;
    
    // num5: volatile pointer to const pointer to int
    int *const *volatile num5;
    
    // num6: pointer to volatile pointer to const pointer to int
    int *const *volatile *num6;
    
    // xp: volatile pointer to const pointer to pointer to char
    char **const *volatile xp;
    
    // More examples of complicated declarations
    int (*cmp)(const void *, const void *);
    struct IMAGE *(*(*(*fp)[5]))(const char *, int);
    void (*jmp)(int *a, int *b);
    void *(*imp(int a, int b))(void);
}

void declarators(void)
{
    int x = 10;           // dirdcl = x
    int A[10];            // dirdcl = A[10]
    int *V[5];            // dirdcl = V[5]     dcl = *
    int *funct(void);     // dirdcl = funct()  dcl = *
    void (*foo)(void);    // dirdcl = foo      dcl = *
    void (*bar[5])(void); // dirdcl = bar[5]   dcl = *
}

void typequalifier_rule(void)
{
    int const *x; // type qualifier next to type specifier, so applies to type
    const int *y; // type qualifier next to type specifier, so applies to type
    
    x = y;        // OK
    y = x;        // OK
    //*x = 100;   // error: read-only variable i.e. const variable
    //*y = 200;   // error: read-only variable i.e. const variable
    
    int *const z; // type qualifier is not next to type specifier, so applies to pointer
    
    *z = 100;     // OK
    *z = 200;     // OK
    //z = x       // error: const-qualifier type i.e. const pointer
}

void test1(void) { printf("test1\n"); }
void test2(void) { printf("test2\n"); }
void test3(void) { printf("test3\n"); }
