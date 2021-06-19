#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gmp.h>
#include <ctype.h>
#include <errno.h>

extern int errno;
#define BASE 29
static const char* base_a = "34567ABCDEFGHIJKMNOPQRSTUWXYZ";
//static const char* base_a = "0123456789ab";

void encode(FILE *f);
void decode(FILE *f);
char* read_string(FILE *fd, int* size, int mode);
int search(const char* arr, char el);

int main(int argc, char** argv)
{
    FILE *fd;
    int errnum;

    if(argv[1] == NULL)
    {
        fd = stdin;
        encode(fd);
        // close file and exit from program
        fclose(fd);
        exit(EXIT_SUCCESS);
    }    

    // Decode input
    if(!strncmp(argv[1], "-d", 2) || !strncmp(argv[1], "--decode", 8))
    {
        if(argv[2] == NULL)
            fd = stdin;
        else
            fd = fopen(argv[2], "rb");
        if(fd == NULL)
        {
            errnum = errno;
            fprintf(stderr, "\e[0;31m[-]\e[0m Can't open file: %s\n", strerror(errnum));
            exit(EXIT_FAILURE);
        } 
        decode(fd);
        // close file and exit from program
        fclose(fd);
        exit(EXIT_SUCCESS);
    }

    // check help message
    char help[] = {"\e[1;32mUsage:\e[0m base29 [OPTION] [FILE]\nBase29 encode or decode FILE, or standard input, to standard output.\n\nWith no FILE read standard input.\n\nOptional arguments:\n\t\e[1;32m-d, --decode\e[0m\n\t\tdecode data(output in HEX)\n\t\tFor decode HEX you can use pipe to '\e[1;33mxxd -r -p\e[0m'\n\n\t\e[1;32m-h, --help\e[0m\n\t\tprint this help and exit"};
    if(!strncmp(argv[1], "--help", 6) || !strncmp(argv[1], "-h", 2))
    {
        printf("%s", help);
        exit(0);
    }

    if(argv[1][0] == '-')
    {
        fprintf(stderr, "\e[1;31m[!] Unknow option\e[0m\n");
        printf("%s", help);
        exit(0);
    }

    // The last one it just encode
    fd = fopen(argv[1], "rb");
    if(fd == NULL)
    {
        errnum = errno;
        fprintf(stderr, "\e[0;31m[-]\e[0m Can't open file: %s\n", strerror(errnum));
        exit(EXIT_FAILURE);
    } 

    encode(fd);
    fclose(fd);
    return 0; 
}

// encode value to BASE29
void encode(FILE *f)
{
    // base settings
    int size = 1024;
    unsigned counter = 0;

    // read string from input
    char* str = read_string(f, &size, 0);
    // Create array of number for reverse output
    int* numbers = calloc(size * 2, sizeof(int));

    // use big numbers
    // init vars
    mpz_t n,i,b,r;
    mpz_init(n);
    mpz_init(i);
    mpz_init(b);
    mpz_init(r);
    // mpz_inits(n, i, b, r);
    mpz_set_ui(n,0);
    mpz_set_ui(i,0);
    mpz_set_ui(b, BASE);

    // convert from hex to 'int'
    mpz_set_str(n, str, 16);

    // calculate base29
    while(mpz_cmp(n,i) != 0)
    {
        // in r stored remain n % b, where b is BASE of alphabit
        mpz_mod(r, n, b);
        // in n stored quotient n % b
        // n = q*b + r
        mpz_div(n, n, b);
        numbers[counter++] = mpz_get_si(r);
    }

    // append to numbers special int -1 for next check
    for(int i = counter; i < size*2; i++)
        numbers[i] = -1;

    // output encoded data
    for(int j = size * 2 - 1; j >= 0; j--)
    {
        // skip empty field
        if(numbers[j] == -1)
            continue;
        printf("%c", base_a[numbers[j]]);
    }

    // make free's
    mpz_clear(r);
    mpz_clear(n);
    mpz_clear(i);
    mpz_clear(b);
    free(str);
    free(numbers);

    // delete links to var(use after free)
    str=NULL;
    numbers=NULL;
}

// decode value from base29
void decode(FILE *f)
{
    // base settings
    int size = 1024;
    // unsigned counter = 0;

    // read string from input
    char* str = read_string(f, &size, 1);
    
    // check that encoded value in base29
    for(int i = 0; i < strlen(str) - 1; i++)
    {
        if(search(base_a, str[i]) == -1)
        {
            printf("\e[0;31m\n[-] \e[0mPlease be sure, that value encoded in Base29\n");
            exit(EXIT_FAILURE);
        }
    }

    char* rev = calloc(strlen(str), sizeof(char));
    for(int a = strlen(str) - 1, b =0; a >= 0; a--, b++)
        rev[b] = str[a];

    // char* hex;
    mpz_t n, base, value;
    mpz_init(n);
    mpz_init(base);
    mpz_init(value);
    mpz_set_ui(base, BASE);
    mpz_set_ui(n, 0);

    for(unsigned j = 0; j < strlen(rev); j++)
    {
        mpz_pow_ui(value, base, j);
        mpz_mul_ui(value, value, search(base_a, rev[j]));
        mpz_add(n, n, value);
    }
    // output hex string
    puts(" ");
    mpz_out_str(stdout, 16, n);

    mpz_clear(n);
    mpz_clear(value);
    mpz_clear(base);
    free(str);
    // avoid use after free
    str = NULL;
}

// read STREAM and return hex output
// mode == 0 - hex output
// mode == 1 - ascii output
char* read_string(FILE *fd, int* size, int mode)
{
    char* string = realloc(NULL, sizeof(char)*(*size));
    char c;
    int len = 0;

    // Memory check, get or not realloc
    if(!string)
        return string;

    if(mode)
    { 
        while(!feof(fd))
        {
            fread(&c, sizeof(char), 1, fd);
            string[len++] = c;
            if(len == *size)
            {
                *size *= 2;
                string = realloc(string, sizeof(char)*(*size));

                // Memory check, get or not realloc
                if(!string)
                    return string;
            }
        }
        string[len - 1] = '\0';
    }
    else
    {
        int i = 0;
        while(!feof(fd))
        {
            fread(&c, sizeof(char), 1, fd);
            sprintf((char*)(string + 2*i), "%02x", c);
            i++;
            len=2*i;
            if(len == *size)
            {
                *size *= 2;
                string = realloc(string, sizeof(char)*(*size));
                if(!string)
                    return string;
            }
        }
        string[i*2 - 1] = '\0';
        string[i*2 - 2] = '\0';
    }
    return string;
}

// find index of element in array
int search(const char* arr, char el)
{
    for(int i = 0; i < strlen(arr); i++)
    {
        if(arr[i] == el)
            return i;
    }
    return -1;
}
