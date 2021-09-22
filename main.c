#include <stdio.h>
#include <stdlib.h>

FILE *input = NULL;
FILE *output = NULL;

void error_handler(int error, int critical) 
{
    if (critical == 0) 
    {
        printf("Error");
    } 
    else 
    {
        printf("Critical error");
    }

    printf(" number %d: ", error);

    switch (error) 
    {
    case 1: 
    {
        printf("Invalid UTF-8 byte");
        break;
    }
    case 2: 
    {
        printf("UTF-8 text breaks off in the middle of the character");
        break;
    }
    case 3: 
    {
        printf("Missing the required number of continuation bytes for UTF-8");
        break;
    }
    case 4: 
    {
        printf("UTF-16 surrogate pair without initial word");
        break;
    }
    case 5: 
    {
        printf("It is not possible to write a character that is not in the UTF-32 range");
        break;
    }
    case 6: 
    {
        printf("UTF-16 text breaks off in the middle of the character");
        break;
    }
    case 7: 
    {
        printf("It is not possible to write a character that is not in the UTF-16 range");
        break;
    }
    case 8: 
    {
        printf("Invalid word continuation for UTF-16 surrogate pair");
        break;
    }
    case 9: 
    {
        printf("The program is started with an incorrect value of the <output file format> field");
        break;
    }
    case 10: 
    {
        printf("Invalid input file format value passed");
        break;
    }
    case 11: 
    {
        printf("Invalid output file format value passed");
        break;
    }
    case 12:
    {
        printf("It is not possible to write a character that is not in the UTF-8 range");
        break;
    }
    case 13:
    {
        printf("UTF-16 text breaks off in the middle of the character");
        break;
    }
    case 14: 
    {
        printf("Missing null command line argument");
        break;
    }
    case 15: 
    {
        printf("The program parameters are completely missing");
        break;
    }
    case 16: 
    {
        printf("The name of the output file and the output format are missing");
        break;
    }
    case 17: 
    {
        printf("The name of the output format is missing");
        break;
    }
    case 18: 
    {
        printf("Failed to open input file");
        break;
    }
    case 19: 
    {
        printf("Failed to open output file");
        break;
    }
    case 20:
    {
        printf("Uneconomical coding");
        break;
    }
    case 21:
    {
        printf("Invalid value of the number of UTF-8 bytes of the character passed to the function");
        break;
    }
    case 22:
    {
        printf("The names of the input and output files are the same");
        break;
    }
    default: 
    {
        printf("Unknown error");
        critical = 1;
    }
    }

    if (critical == 0) {
        printf(";\n");
    } 
    else 
    {
        printf(".");
        fclose(input);
    	fclose(output);
        exit(critical);
    }
}

unsigned int ARFD()
{ // Automatic Read Format Detection

    unsigned char BOM[4];

    fscanf(input, "%c", &BOM[0]);
    fscanf(input, "%c", &BOM[1]);
    fscanf(input, "%c", &BOM[2]);
    fscanf(input, "%c", &BOM[3]);

    unsigned int format;

    if (BOM[0] == 0xEF && BOM[1] == 0xBB && BOM[2] == 0xBF) 
    {
        fseek(input, 3, SEEK_SET);
        format = 1; // UTF-8 with BOM
    }
    else if (BOM[0] == 0xFF && BOM[1] == 0xFE && BOM[2] == 0x00 && BOM[3] == 0x00)
    {
        format = 4; // UTF-32 LE
    }
    else if (BOM[0] == 0xFF && BOM[1] == 0xFE) 
    {
        fseek(input, 2, SEEK_SET);
        format = 2; // UTF-16 LE
    } 
    else if (BOM[0] == 0xFE && BOM[1] == 0xFF) 
    {
        fseek(input, 2, SEEK_SET);
        format = 3; // UTF-16 BE
    }
    else if (BOM[0] == 0x00 && BOM[1] == 0x00 && BOM[2] == 0xFE && BOM[3] == 0xFF) 
    {
        format = 5; // UTF-32 BE
    } 
    else 
    {
        fseek(input, 0, SEEK_SET);
        format = 0; // UTF-8 without BOM
    }

    return format;
}

unsigned int AWFD(const char *argument) 
{ // Automatic Write Format Detection

    unsigned int format = 6;

    if ((argument[0] >= '0') && (argument[0] <= '5') && (argument[1] == '\0')) 
    {
        format = argument[0] - '0';
    } 
    else 
    {
        error_handler(9, 1);
    }
    return format;
}

struct Symbol 
{
    int countBytes;
    unsigned long code;
};

void modify_UTF8_byte(struct Symbol *symbol) 
{
    if (0xC0 <= symbol->code && symbol->code <= 0xDF) 
    {
        symbol->code -= 0xC0;
        symbol->countBytes = 2;
    } 
    else if (0xE0 <= symbol->code && symbol->code <= 0xEF) 
    {
        symbol->code -= 0xE0;
        symbol->countBytes = 3;
    } 
    else if (0xF0 <= symbol->code && symbol->code <= 0xF7) 
    {
        symbol->code -= 0xF0;
        symbol->countBytes = 4;
    } 
    else if (symbol->code <= 0x7F)
    {
        symbol->countBytes = 1;
    } 
    else 
    {
        symbol->code += 0xDC00;
        symbol->countBytes = 1;
    }
}

void check_code (struct Symbol *symbol)
{ // Checking for uneconomical coding
    switch (symbol->countBytes)
    {
    case 1:
    {
        break;
    }
    case 2:
    {
        if (0x80 >= symbol->code || symbol->code >= 0x7FF)
        {
            //error_handler(20, 0);
            unsigned long byte = 0;
            fseek(input, -(symbol->countBytes), SEEK_CUR);
            fscanf(input, "%c", &byte);
            symbol->code = 0;
            symbol->code = byte + 0xDC00;
        }
        break;
    }
    case 3:
    {
        if (0x800 >= symbol->code || symbol->code >= 0xFFFF || (0xDC80 <= symbol->code && symbol->code <= 0xDCFF))
        {
            //error_handler(20, 0);
            unsigned long byte = 0;
            fseek(input, -(symbol->countBytes), SEEK_CUR);
            fscanf(input, "%c", &byte);
            symbol->code = byte + 0xDC00;
        }
        break;
    }
    case 4:
    {
        if (0x10000 >= symbol->code || symbol->code >= 0x1FFFFF) //10FFFF
        {
            //error_handler(20, 0);
            unsigned long byte = 0;
            fseek(input, -(symbol->countBytes), SEEK_CUR);
            fscanf(input, "%c", &byte);
            symbol->code = byte + 0xDC00;
        }
        break;
    }
    default:
    {
        error_handler(21, 2);
        break;
    }
    }
}

unsigned long read_UTF8_symbol(unsigned long byte)
{ 
    struct Symbol symbol = {0, byte};

    modify_UTF8_byte(&symbol);

    for (int add = 1; add < symbol.countBytes; add++)
    {
        symbol.code *= 64;
        if (fscanf(input, "%c", &byte) == EOF) 
        {
            //error_handler(2, 0);
            fseek(input, -add, SEEK_CUR);
            byte = 0;
            fscanf(input, "%c", &byte);
            symbol.code = byte + 0xDC00;
            return symbol.code;
        }
        if (0x80 <= byte && byte <= 0xBF) 
        {
            symbol.code += byte - 0x80;
        } 
        else 
        {
            //error_handler(3, 0);
            fseek(input, -add-1, SEEK_CUR);
            byte = 0;
            fscanf(input, "%c", &byte);
            symbol.code = byte + 0xDC00;
            return symbol.code;
        }
    }
    check_code(&symbol);
    return symbol.code;
}

void modify_UTF16_byte(struct Symbol *oneSymbol) 
{
    if ((0x0000 <= oneSymbol->code && oneSymbol->code <= 0xD7FF)
            || 0xE000 <= oneSymbol->code && oneSymbol->code <= 0xFFFF
                || 0xDC80 <= oneSymbol->code && oneSymbol->code <= 0xDCFF)
    {
        oneSymbol->countBytes = 2;
    } 
    else if (0xD800 <= oneSymbol->code && oneSymbol->code <= 0xDBFF) 
    {
        oneSymbol->code -= 0xD800;
        oneSymbol->countBytes = 4;
    }
    else 
    {
        //error_handler(4, 0);
        oneSymbol->countBytes = 4;
    }
}

unsigned long read_UTF16_BE_symbol(unsigned long byte)
{
    struct Symbol symbol = {0, byte * 256};

    if (fscanf(input, "%c", &byte) == EOF) 
    {
        //error_handler(13, 0);
        return symbol.code;
    }
    symbol.code += byte;

    modify_UTF16_byte(&symbol);

    if (symbol.countBytes == 4) 
    {
        unsigned long byte_two;
        symbol.code *= 1024;
        symbol.code += 0x10000;

        if (fscanf(input, "%c", &byte) == EOF || fscanf(input, "%c", &byte_two) == EOF) 
        {
            //error_handler(13, 0);
            return symbol.code;
        }

        byte *= 256;
        byte += byte_two;

        if (0xDC00 <= byte && byte <= 0xDFFF) 
        {
            symbol.code += (byte - 0xDC00);
        } 
        else 
        {
            //error_handler(8, 0);
            fseek(input, -2, SEEK_CUR);
            return symbol.code;
        }
    }
    return symbol.code;
}

unsigned long read_UTF16_LE_symbol(unsigned long byte)
{
    struct Symbol symbol = {0, byte};

    if (fscanf(input, "%c", &byte) == EOF) 
    {
        //error_handler(13, 0);
        return symbol.code;
    }

    symbol.code += (byte * 256);

    modify_UTF16_byte(&symbol);

    if (symbol.countBytes == 4) 
    {
        unsigned long byte_two = 0;

        symbol.code *= 1024;
        symbol.code += 0x10000;

        if (fscanf(input, "%c", &byte) == EOF || fscanf(input, "%c", &byte_two) == EOF) 
        {
            //error_handler(13, 0);
            return symbol.code;
        }

        byte += (byte_two * 256);

        if (0xDC00 <= byte && byte <= 0xDFFF) 
        {
            symbol.code += (byte - 0xDC00);
        } 
        else 
        {
            //error_handler(8, 0);
            fseek(input, -2, SEEK_CUR);
            return symbol.code;
        }
    }
    return symbol.code;
}

unsigned long read_UTF32_BE_symbol(unsigned long byte)
{
    struct Symbol symbol = {0, byte * 256};

    if (fscanf(input, "%c", &byte) == EOF) 
    {
        //error_handler(13, 0);
        return symbol.code;
    }

    symbol.code += byte;
    symbol.code *= 256;

    if (fscanf(input, "%c", &byte) == EOF) 
    {
        //error_handler(13, 0);
        return symbol.code;
    }

    symbol.code += byte;
    symbol.code *= 256;

    if (fscanf(input, "%c", &byte) == EOF) 
    {
        //error_handler(13, 0);
        return symbol.code;
    }

    symbol.code += byte;
    return symbol.code;
}

unsigned long read_UTF32_LE_symbol(unsigned long byte)
{
    struct Symbol symbol = {0, byte};

    if (fscanf(input, "%c", &byte) == EOF) 
    {
        //error_handler(13, 0);
        return symbol.code;
    }

    symbol.code += (byte * 256);

    if (fscanf(input, "%c", &byte) == EOF) 
    {
        //error_handler(13, 0);
        return symbol.code;
    }

    symbol.code += (byte * 256 * 256);

    if (fscanf(input, "%c", &byte) == EOF) 
    {
        //error_handler(13, 0);
        return symbol.code;
    }

    symbol.code += (byte * 256 * 256 * 256);

    return symbol.code;
}

void write_BOM(unsigned int write_format)
{
    switch (write_format) 
    {
    case 0:
        return;
    case 1: 
    {
        fprintf(output, "%c%c%c", 0xEF, 0xBB, 0xBF);
        return;
    }
    case 2: 
    {
        fprintf(output, "%c%c", 0xFF, 0xFE);
        return;
    }
    case 3: 
    {
        fprintf(output, "%c%c", 0xFE, 0xFF);
        return;
    }
    case 4: 
    {
        fprintf(output, "%c%c%c%c", 0xFF, 0xFE, 0x00, 0x00);
        return;
    }
    case 5: 
    {
        fprintf(output, "%c%c%c%c", 0x00, 0x00, 0xFE, 0xFF);
        return;
    }
    default: 
    {
        error_handler(11, 2);
    }
    }
}

void write_UTF8_symbol(unsigned long symbol)
{
    if (0x00 <= symbol && symbol <= 0x7F) 
    {
        fprintf(output, "%c", symbol);
    } 
    else if (0x80 <= symbol && symbol <= 0x7FF)
    {
        fprintf(output, "%c", ((symbol / 64) + 0xC0));
        fprintf(output, "%c", symbol - ((symbol / 64) * 64) + 0x80);
    }
    else if (0xDC80 <= symbol && symbol <= 0xDCFF)
    {
        symbol -= 0xDC00;
        fprintf(output, "%c", symbol);
    }
    else if (0x800 <= symbol && symbol <= 0xFFFF) 
    {
        fprintf(output, "%c", ((symbol / 64 / 64) + 0xE0));
        fprintf(output, "%c", (symbol - ((symbol / 64 / 64) * 64 * 64)) / 64 + 0x80);
        fprintf(output, "%c", symbol - ((symbol / 64) * 64) + 0x80);
    } 
    else if (0x10000 <= symbol && symbol <= 0x1FFFFF) //10FFFF
    {
        fprintf(output, "%c", ((symbol / 64 / 64 / 64) + 0xF0));
        fprintf(output, "%c", (symbol - ((symbol / 64 / 64 / 64) * 64 * 64 * 64)) / 64 / 64 + 0x80);
        fprintf(output, "%c", (symbol - ((symbol / 64 / 64) * 64 * 64)) / 64 + 0x80);
        fprintf(output, "%c", symbol - ((symbol / 64) * 64) + 0x80);
    }
    else 
    {
        printf("%X ", symbol);
        error_handler(12, 2);
    }
}

void write_UTF16_BE_symbol(unsigned long symbol)
{
    if (0x00 <= symbol && symbol <= 0xFF) 
    {
        fprintf(output, "%c", 0x00);
        fprintf(output, "%c", symbol);
    } 
    else if ((0x100 <= symbol && symbol <= 0xD7FF) || (0xE000 <= symbol && symbol <= 0xFFFF) || (0xDC80 <= symbol && symbol <= 0xDCFF))
    {
        fprintf(output, "%c", symbol / 256);
        fprintf(output, "%c", symbol - ((symbol / 256) * 256));
    } 
    else if (0x10000 <= symbol && symbol <= 0x10FFFF) // 1FFFFF
    {
        symbol -= 0x10000;
        unsigned long first_ten_bit = symbol / 1024;
        unsigned long second_ten_bit = symbol - (first_ten_bit * 1024);

        first_ten_bit += 0xD800;
        second_ten_bit += 0xDC00;

        fprintf(output, "%c", first_ten_bit / 256);
        fprintf(output, "%c", first_ten_bit - ((first_ten_bit / 256) * 256));
        fprintf(output, "%c", second_ten_bit / 256);
        fprintf(output, "%c", second_ten_bit - ((second_ten_bit / 256) * 256));
    }
    else 
    {
        printf ("%X ", symbol);
        //error_handler(7, 0);
    }
}

void write_UTF16_LE_symbol(unsigned long symbol)
{
    if (0x00 <= symbol && symbol <= 0xFF) 
    {
        fprintf(output, "%c", symbol);
        fprintf(output, "%c", 0x00);

    } 
    else if ((0x100 <= symbol && symbol <= 0xD7FF) || (0xE000 <= symbol && symbol <= 0xFFFF) || (0xDC80 <= symbol && symbol <= 0xDCFF))
    {
        fprintf(output, "%c", symbol - ((symbol / 256) * 256));
        fprintf(output, "%c", symbol / 256);
    } 
    else if (0x10000 <= symbol && symbol <= 0x10FFFF) //1FFFFF
    {
        symbol -= 0x10000;
        unsigned long first_ten_bit = symbol / 1024;
        unsigned long second_ten_bit = symbol - (first_ten_bit * 1024);

        first_ten_bit += 0xD800;
        second_ten_bit += 0xDC00;

        fprintf(output, "%c", first_ten_bit - ((first_ten_bit / 256) * 256));
        fprintf(output, "%c", first_ten_bit / 256);

        fprintf(output, "%c", second_ten_bit - ((second_ten_bit / 256) * 256));
        fprintf(output, "%c", second_ten_bit / 256);
    }
    else 
    {
        //error_handler(7, 0);
    }
}

void write_UTF32_BE_symbol(unsigned long symbol)
{
    if (0x00 <= symbol && symbol <= 0xFF) 
    {
        fprintf(output, "%c", 0x00);
        fprintf(output, "%c", 0x00);
        fprintf(output, "%c", 0x00);
        fprintf(output, "%c", symbol);
    } 
    else if (0x100 <= symbol && symbol <= 0xFFFF) 
    {
        fprintf(output, "%c", 0x00);
        fprintf(output, "%c", 0x00);
        fprintf(output, "%c", symbol / 256);
        fprintf(output, "%c", symbol - ((symbol / 256) * 256));
    } 
    else if (0x10000 <= symbol && symbol <= 0xFFFFFF) 
    {
        fprintf(output, "%c", 0x00);
        fprintf(output, "%c", (symbol / 256 / 256));
        fprintf(output, "%c", (symbol - ((symbol / 256 / 256) * 256 * 256)) / 256);
        fprintf(output, "%c", (symbol - ((symbol / 256 ) * 256 )));
    } 
    else if (0x1000000 <= symbol && symbol <= 0xFFFFFFFF) 
    {
        fprintf(output, "%c", (symbol / 256 / 256 / 256));
        fprintf(output, "%c", (symbol - ((symbol / 256 / 256 / 256) * 256 * 256 * 256)) / 256 / 256);
        fprintf(output, "%c", (symbol - ((symbol / 256 / 256) * 256 * 256)) / 256);
        fprintf(output, "%c", (symbol - ((symbol / 256 ) * 256 )));
    } 
    else 
    {
        //error_handler(5, 0);
    }
}

void write_UTF32_LE_symbol(unsigned long symbol)
{
    if (0x00 <= symbol && symbol <= 0xFF) 
    {
        fprintf(output, "%c", symbol);
        fprintf(output, "%c", 0x00);
        fprintf(output, "%c", 0x00);
        fprintf(output, "%c", 0x00);
    } 
    else if (0x100 <= symbol && symbol <= 0xFFFF) 
    {
        fprintf(output, "%c", symbol - ((symbol / 256) * 256));
        fprintf(output, "%c", symbol / 256);
        fprintf(output, "%c", 0x00);
        fprintf(output, "%c", 0x00);
    } 
    else if (0x10000 <= symbol && symbol <= 0xFFFFFF) 
    {
        fprintf(output, "%c", (symbol - ((symbol / 256 ) * 256 )));
        fprintf(output, "%c", (symbol - ((symbol / 256 / 256) * 256 * 256)) / 256);
        fprintf(output, "%c", (symbol / 256 / 256));
        fprintf(output, "%c", 0x00);
    } 
    else if (0x1000000 <= symbol && symbol <= 0xFFFFFFFF) 
    {
        fprintf(output, "%c", (symbol - ((symbol / 256 ) * 256 )));
        fprintf(output, "%c", (symbol - ((symbol / 256 / 256) * 256 * 256)) / 256);
        fprintf(output, "%c", (symbol - ((symbol / 256 / 256 / 256) * 256 * 256 * 256)) / 256 / 256);
        fprintf(output, "%c", (symbol / 256 / 256 / 256));
    } 
    else 
    {
        //error_handler(5, 0);
    }
}

void convert(unsigned int read_format, unsigned int write_format)
{
    unsigned long buffer;

    write_BOM(write_format);

    while (fscanf(input, "%c", &buffer) != EOF) 
    {
        switch (read_format) 
        {
        case 0:
        case 1: 
        {
            buffer = read_UTF8_symbol(buffer);
            break;
        }
        case 2: 
        {
            buffer = read_UTF16_LE_symbol(buffer);
            break;
        }
        case 3: 
        {
            buffer = read_UTF16_BE_symbol(buffer);
            break;
        }
        case 4: 
        {
            buffer = read_UTF32_LE_symbol(buffer);
            break;
        }
        case 5: 
        {
            buffer = read_UTF32_BE_symbol(buffer);
            break;
        }
        default: 
        {
            error_handler(10, 2);
        }
        }

        switch (write_format) 
        {
        case 0:
        case 1: 
        {
            write_UTF8_symbol(buffer);
            break;
        }
        case 2: 
        {
            write_UTF16_LE_symbol(buffer);
            break;
        }
        case 3: 
        {
            write_UTF16_BE_symbol(buffer);
            break;
        }
        case 4: 
        {
            write_UTF32_LE_symbol(buffer);
            break;
        }
        case 5: 
        {
            write_UTF32_BE_symbol(buffer);
            break;
        }
        default: 
        {
            error_handler(11, 2);
        }
        }
        buffer = 0;
    }
}

void copy ()
{
    unsigned long buffer;

    while (fscanf(input, "%c", &buffer) != EOF)
    {
        fprintf(output, "%c", buffer);
        buffer = 0;
    }
}

void test_name (const char * first_name, const char * last_name)
{
    int first_length = 0;
    for(; first_name[first_length] != '\0'; first_length++);
    int last_length = 0;
    for(; last_name[last_length] != '\0'; last_length++);

    if (first_length == last_length) {
        for (first_length = 0; first_length < last_length; first_length++)
        {
            if(first_name[first_length] != last_name[first_length])
            {
                return;
            }
        }
        error_handler(22, 1);
    }
}

int main(int argc, char *argv[]) 
{
    if (argc < 1) 
    {
        error_handler(14, 1);
    }

    switch (argc) 
    {
    case 1:
        error_handler(15, 1);
    case 2:
        error_handler(16, 1);
    case 3:
        error_handler(17, 1);
    default:
        break;
    }

    test_name(argv[1], argv[2]);

    if ((input = fopen(argv[1], "rb")) == NULL) 
    {
        error_handler(18, 1);
    }

    unsigned int read_format = ARFD();
    unsigned int write_format = AWFD(argv[3]);

    if ((output = fopen(argv[2], "wb")) == NULL) 
    {
        error_handler(19, 1);
    }

    if ((read_format == 1 && write_format == 0) || (read_format == 0 && write_format == 1)) {
        read_format = write_format;
    }

    if (read_format == write_format) {
        write_BOM(write_format);
        copy();
    } else {
        convert(read_format, write_format);
    }

    fclose(input);
    fclose(output);

    return 0;
}