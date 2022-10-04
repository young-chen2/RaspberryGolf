#include "assert.h"
#include "printf.h"
#include <stddef.h>
#include "strings.h"
#include "uart.h"

// Copied from printf.c
int unsigned_to_base(char *buf, size_t bufsize, unsigned int val, int base, size_t min_width);
int signed_to_base(char *buf, size_t bufsize, int val, int base, size_t min_width);

static void test_memset(void)
{
    char buf[12];
    size_t bufsize = sizeof(buf);

    memset(buf, 0x77, bufsize); // fill buffer with repeating value
    for (int i = 0; i < bufsize; i++)
        assert(buf[i] == 0x77); // confirm value
    
    char buf1[4];
    size_t buf1size = sizeof(buf1);

    memset(buf1, 'x', buf1size - 1);
    memset(buf1 + 3, 'p', 1);

    for (int i = 0; i < buf1size - 1; i++)
        assert(buf1[i] == 'x'); // confirm value
    assert(buf1[buf1size - 1] == 'p');
    
    char buf2[6700];
    size_t buf2size = sizeof(buf2);
    
    memset(buf2, 'v', buf2size/2);
    memset(buf2 + (buf2size/2), 'p', buf2size/2);
    
    for (int i = 0; i < buf2size/2; i++)
        assert(buf2[i] == 'v'); // confirm value
    for (int i = buf2size/2; i < buf2size; i++)
        assert(buf2[i] == 'p'); // confirm value
}

static void test_strcmp(void)
{
    assert(strcmp("apple", "apple") == 0);
    assert(strcmp("apple", "applesauce") == 0);
    assert(strcmp("pears", "apples") > 0);
    assert(strcmp("1290", "1289") > 0);
    // different lengths of cmp
    assert(strcmp("!", "$$$$") < 0);
    assert(strcmp("extravagant!23-", "extravagant!23-") == 0);
    // checking when first letter is different
    assert(strcmp("Emma", "emma") < 0);
    assert(strcmp("xylophone", "when") > 0);
    // checking for differences beyond first letter
    assert(strcmp("emPhatic", "emphatic") < 0);
    assert(strcmp("elePHaNt", "elePHant") < 0);
    assert(strcmp("emmA", "emma") < 0);
    assert(strcmp("sporRT", "sporRt") < 0);
    // different string lengths but same prefix
    assert(strcmp("emmA", "emm") == 0);
    assert(strcmp("similarly", "sim") == 0);
    assert(strcmp("simulat", "simulate") == 0);
    //edge cases
    assert(strcmp(" ", " ") == 0);
    assert(strcmp("", "") == 0);
    assert(strcmp("123", "") > 0);
}

static void test_strlcat(void)
{
    char buf[20];
    memset(buf, 0x77, sizeof(buf)); // init contents with known value

    buf[0] = '\0'; // start with empty string
    assert(strlen(buf) == 0);
    strlcat(buf, "CS", sizeof(buf));
    assert(strlen(buf) == 2);
    assert(strcmp(buf, "CS") == 0);
    strlcat(buf, "107e", sizeof(buf));
    assert(strlen(buf) == 6);
    assert(strcmp(buf, "CS107e") == 0);
    // appending more strings
    strlcat(buf, " is cool!", sizeof(buf));
    assert(strlen(buf) == 15);
    assert(strcmp(buf, "CS107e is cool!") == 0);
    // append a null terminator/string; should not affect code
    strlcat(buf, "", sizeof(buf));
    assert(strlen(buf) == 15);
    assert(strcmp(buf, "CS107e is cool!") == 0);
    
    // check error correction
    char queen[3];
    memset(queen, 0x77, sizeof(queen)); // init contents with known value
    
    queen[0] = '\0';
    assert(strlen(queen) == 0);
    strlcat(queen, "CSS", sizeof(queen));
    assert(strcmp(queen, "CS") == 0);
    assert(strlcat(queen, "CSS", sizeof(queen)) == 5); // CSCSS = 5 chars
    strlcat(queen, "Rupaul's Drag Race", sizeof(queen));
    assert(strlcat(queen, "Rupaul's Drag Race", sizeof(queen)) == 20); // 2 + 18
}

static void test_strtonum(void)
{
    int val = strtonum("013", NULL);
    assert(val == 13);

    const char *input = "107rocks", *rest = NULL;
    val = strtonum(input, &rest);
    assert(val == 107);
    assert(rest == &input[3]);
    
    //case with more zeroes out front
    val = strtonum("000000001232XI", NULL);
    assert(val == 1232);
    
    //handling hexadecimal numbers
    val = strtonum("0x001", NULL);
    assert(val == 1);
    input = "0xahello, hello!";
    val = strtonum(input, &rest);
    assert(val == 10);
    assert(rest == &input[3]); //testing that endptr works on invalid hex digits
    input = "0xaeee132",
    val = strtonum(input, &rest);
    assert(val == 183427378);
    assert(*rest == '\0'); //testing that endptr works for valid hex
    val = strtonum("0x000023", NULL);
    assert(val == 35);
    val = strtonum("0xBBB", NULL);
    assert(val == 0);
    val = strtonum("0baae", &rest);
    assert(val == 0);
    assert(*rest == 'b'); //testing that endptr works for invalid number

    //boundary testing of hexadecimal numbers
    val = strtonum("0xx", NULL);
    assert(val == 0);
    val = strtonum("0xgf", NULL);
    assert(val == 0);
    val = strtonum("0x@1829", NULL);
    assert(val == 0);
    val = strtonum("0xABCDE", NULL);
    assert(val == 0);
    val = strtonum("0x", NULL);
    assert(val == 0);
}

static void test_to_base(void)
{
    char buf[15];
    size_t bufsize = sizeof(buf);

    memset(buf, 0x77, bufsize); // init contents with known value

    int n = unsigned_to_base(buf, bufsize, 35, 10, 0);
    assert(strcmp(buf, "35") == 0)
    assert(n == 2);
    
    n = unsigned_to_base(buf, bufsize, 35, 16, 4);
    assert(strcmp(buf, "0023") == 0);
    assert(n == 4);
    
    n = unsigned_to_base(buf, bufsize, 890, 10, 15);
    assert(strcmp(buf, "000000000000890") == 0);
    assert(n == 15);
    
    n = unsigned_to_base(buf, bufsize, 0xaaeb9, 10, 30);
    assert(strcmp(buf, "0000000000000000000000000aaeb9") == 0);
    assert(n == 30);
    
    //handling a bufsize that's too small
    n = unsigned_to_base(buf, 5, 123456, 10, 0);
    assert(strcmp(buf, "1234") == 0);
    assert(n == 6);
    
    //support for signed int
    n = signed_to_base(buf, bufsize, 35, 16, 4);
    assert(strcmp(buf, "0023") == 0);
    assert(n == 4);

    n = signed_to_base(buf, bufsize, -9999, 10, 6);
    assert(strcmp(buf, "-09999") == 0)
    assert(n == 6);

    n = signed_to_base(buf, bufsize, -458000, 10, 10);
    assert(strcmp(buf, "-000458000") == 0)
    assert(n == 10);
    
    n = signed_to_base(buf, bufsize, -0x89ee, 16, 6);
    assert(strcmp(buf, "-089ee") == 0)
    assert(n == 6);
    
    n = signed_to_base(buf, bufsize, -0xfe9, 16, 0);
    assert(strcmp(buf, "-fe9") == 0);
    assert(n == 4);

    //signed handling a bufsize that's too small
    n = signed_to_base(buf, 5, 129000, 10, 0);
    assert(strcmp(buf, "1290") == 0);
    assert(n == 6);

    n = signed_to_base(buf, 5, -123456, 10, 0);
    assert(strcmp(buf, "-123") == 0);
    assert(n == 7);

    n = signed_to_base(buf, 5, -0xfe900, 16, 0);
    assert(strcmp(buf, "-fe9") == 0);
    assert(n == 6);

    //support for hex
    n = unsigned_to_base(buf, bufsize, 0xaae, 16, 0);
    assert(strcmp(buf, "aae") == 0)
    assert(n == 3);

    n = unsigned_to_base(buf, bufsize, 0x98b2f, 16, 8);
    assert(strcmp(buf, "00098b2f") == 0)
    assert(n == 8);

    n = signed_to_base(buf, bufsize, -0xbb, 16, 0);
    assert(strcmp(buf, "-bb") == 0)
    assert(n == 3);

    //reverse hex
    n = signed_to_base(buf, bufsize, 60159, 16, 0);
    assert(strcmp(buf, "eaff") == 0)
    assert(n == 4);

    //edge cases
    n = signed_to_base(buf, bufsize, -9999, 10, 6);
    assert(strcmp(buf, "-09999") == 0)
    assert(n == 6);
}

static void test_snprintf(void)
{
    char buf[500];
    size_t bufsize = sizeof(buf);

    memset(buf, 0x77, sizeof(buf)); // init contents with known value

    // Simple Case
    snprintf(buf, bufsize, "Hello, world!");
    assert(strcmp(buf, "Hello, world!") == 0);

    // Simple Cases
    snprintf(buf, bufsize, "123 Hey*");
    assert(strcmp(buf, "123 Hey*") == 0);

    snprintf(buf, bufsize, "780x0x@@.");
    assert(strcmp(buf, "780x0x@@.") == 0);

    // Decimal
    snprintf(buf, bufsize, "%d", 45);
    assert(strcmp(buf, "45") == 0);
    
    // Decimal works with pre-exsiting string
    snprintf(buf, bufsize, "I am %d years old on %d year, %d day, %d month.", 20, 2022, 5, 10);
    assert(strcmp(buf, "I am 20 years old on 2022 year, 5 day, 10 month.") == 0);
        
    snprintf(buf, bufsize, "I am %d years old on %d year, %d day, %05d month.", -20, -2022, 5, -10);
    assert(strcmp(buf, "I am -20 years old on -2022 year, 5 day, -0010 month.") == 0);

    // Hexadecimal
    snprintf(buf, bufsize, "%04x", 0xef);
    assert(strcmp(buf, "00ef") == 0);
    
    // Hexadecimal works with pre-existing string
    snprintf(buf, bufsize, "It's %04x.", 0xef);
    assert(strcmp(buf, "It's 00ef.") == 0);

    snprintf(buf, bufsize, "Hello %04x hi!", 0xef);
    assert(strcmp(buf, "Hello 00ef hi!") == 0);

    snprintf(buf, bufsize, "Hey %04x, Yeah %06x, %02x is good.", 0x1, 0x680, 0x12);
    assert(strcmp(buf, "Hey 0001, Yeah 000680, 12 is good.") == 0);

    // Pointer
    snprintf(buf, bufsize, "%p", (void *) 0x20200004);
    assert(strcmp(buf, "0x20200004") == 0);

    // Pointer works with pre-existing string
    snprintf(buf, bufsize, "Hey %p, Yeah %p, %p is so fun!", (void *) 0x90abf, (void *) 0x90abc, (void *) 0x90aba);
    assert(strcmp(buf, "Hey 0x90abf, Yeah 0x90abc, 0x90aba is so fun!") == 0);

    // Character
    snprintf(buf, bufsize, "%c", 'A');
    assert(strcmp(buf, "A") == 0);

    // Char works with pre-existing string
    snprintf(buf, bufsize, "Hey %c, Yeah %c, %%%%.", '$', 'b');
    assert(strcmp(buf, "Hey $, Yeah b, %%.") == 0);

    // String
    snprintf(buf, bufsize, "%s", "binky");
    assert(strcmp(buf, "binky") == 0);

    // String to pre-existing string
    snprintf(buf, bufsize, "Oh my gosh, %s", "I haven't seen you in forever!");
    assert(strcmp(buf, "Oh my gosh, I haven't seen you in forever!") == 0);
    snprintf(buf, bufsize, "Oh my gosh, %s I've missed you. %s", "I haven't seen you in forever!", "Let's catch up.");
    assert(strcmp(buf, "Oh my gosh, I haven't seen you in forever! I've missed you. Let's catch up.") == 0);

    // Format string with intermixed codes
    snprintf(buf, bufsize, "CS%d%c!", 107, 'e');
    assert(strcmp(buf, "CS107e!") == 0);

    // More mixed codes
    snprintf(buf, bufsize, "CS%x%s!", 0x6b, "e");
    assert(strcmp(buf, "CS6be!") == 0);

    // Test return value
    assert(snprintf(buf, bufsize, "Hello") == 5);
    assert(snprintf(buf, 2, "%s", "Hello") == 5);
}

static void test_printf(void)
{
    // Simple Cases
    printf("Hello world \n");
    
    // Mixed flags
    printf("CS%d%c!\n", 107, 'e');

    // String concatenation
    printf("Oh my gosh, %s I've missed you. %s\n", "I haven't seen you in forever!", "Let's catch up.");
    
    // Repeat String concat to check for memory bug
    printf("Oh my gosh, %s I've missed you. %s\n", "I haven't seen you in forever!", "Let's catch up.");
    
    // Decimals
    printf("I am %d years old on %d year, %d day, %d month.\n", 20, 2022, 5, 10);

    // Pointer check
    printf("Hey %p, Yeah %p, %p is so fun!\n", (void *) 0x90abf, (void *) 0x90abc, (void *) 0x90aba);
    
    // Hex check
    printf("Hey %04x, Yeah %06x, %02x is good. \n", 0x1, 0x680, 0x12);
    
    // Duplicating string calls to eradicate memory bugs
    printf("I love %s. I see %s, I miss %s. %s haha.\n", "You", "You", "U", "See you soon");
    
    printf("I love %s. I see %s, I miss %s. %s haha.\n", "You", "You", "U", "I'm funny");
    
    printf("I love %s. I see %s, I miss %s. %s haha.\n", "You", "You", "U", "I'm funny");
}

void test_autograder(void) {
    char buf_16[16];
    snprintf(buf_16, 16, "FSEL:%p!", (void *)0x20200000);
    assert(strcmp(buf_16, "FSEL:0x20200000") == 0);
    
    char buf_6[6];
    memcpy(buf_6, "cs", 6);
    strlcat(buf_6, "107e", 6);
    assert(strcmp(buf_6, "cs107") == 0);
    
    snprintf(buf_6, 6, "%06x", 255);
    assert(strcmp(buf_6, "0000f") == 0);
    printf("%s\n", buf_6);
    
    int x1 = snprintf(buf_16, 16, "%s" ,"I love CS107E and learning is awesome!");
    assert(strcmp(buf_16, "I love CS107E a") == 0);
    assert(x1 == 38);
    assert(snprintf(buf_16, 16, "%s", "I love CS107E and learning is awesome!") == 38); //needed?
    
    char buf_3[3];
    memcpy(buf_3, "cs", 3);
    assert(strlcat(buf_3, "107e", 3) == 6);
    
    void *ptr = NULL;
    strtonum("908", ptr);
    assert(ptr == NULL);
    
}

// This function just here as code to disassemble for extension
int sum(int n)
{
    int result = 6;
    for (int i = 0; i < n; i++) {
        result += i * 3;
    }
    return result + 729;
}

void test_disassemble(void)
{
    const unsigned int add = 0xe0843005;
    const unsigned int sub = 0xe24bd00c;
    const unsigned int mov = 0xe3a0006b;
    const unsigned int bne = 0x1afffffa;
    const unsigned int add1 = 0xe0843005;
    const unsigned int sub1 = 0xe2463063;
    const unsigned int mov1 = 0xe3a00007;
    const unsigned int bichi = 0x81c10001;
    const unsigned int rscgt = 0xc0e4d006;
    const unsigned int cmp = 0xe1570004;
    const unsigned int movle = 0xd3a01c02;

    // If you have not implemented the extension, core printf
    // will output address not disassembled followed by I
    // e.g.  "... disassembles to 0x07ffffd4I"
    printf("Encoded instruction %x disassembles to %pI\n", add, &add);
    printf("Encoded instruction %x disassembles to %pI\n", sub, &sub);
    printf("Encoded instruction %x disassembles to %pI\n", mov, &mov);
    printf("Encoded instruction %x disassembles to %pI\n", bne, &bne);
    printf("Encoded instruction %x disassembles to %pI\n", add1, &add1);
    printf("Encoded instruction %x disassembles to %pI\n", sub1, &sub1);
    printf("Encoded instruction %x disassembles to %pI\n", mov1, &mov1);
    printf("Encoded instruction %x disassembles to %pI\n", bichi, &bichi);
    printf("Encoded instruction %x disassembles to %pI\n", rscgt, &rscgt);
    printf("Encoded instruction %x disassembles to %pI\n", cmp, &cmp);
    printf("Encoded instruction %x disassembles to %pI\n", movle, &movle);

//    unsigned int *fn = (unsigned int *)sum; // diassemble instructions from sum function
//    for (int i = 0; i < 10; i++) {
//        printf("%p:  %x  %pI\n", &fn[i], fn[i], &fn[i]);
//    }
}

void main(void)
{
    uart_init();
    uart_putstring("Start execute main() in tests/test_strings_printf.c\n");

//    test_memset();
//    test_strcmp();
//    test_strlcat();
//    test_strtonum();
//    test_to_base();
//    test_snprintf();
//    test_printf();
    test_autograder();
    test_disassemble();

    uart_putstring("Successfully finished executing main() in tests/test_strings_printf.c\n");
    uart_putchar(EOT);
}
