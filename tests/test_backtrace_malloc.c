#include "assert.h"
#include "backtrace.h"
#include "malloc.h"
#include "printf.h"
#include "rand.h"
#include <stdint.h>
#include "strings.h"
#include "uart.h"

void main(void);
void heap_dump(const char *label);
int mystery(void);

static void test_name_of(void)
{
    const char *name;

    name = name_of((uintptr_t)main);
    assert(strcmp(name, "main") == 0);
    name = name_of((uintptr_t)uart_init);
    assert(strcmp(name, "uart_init") == 0);
    name = name_of((uintptr_t)mystery); // function compiled without embedded name
    assert(strcmp(name, "???") == 0);
}

static void test_backtrace_simple(void)
{
    frame_t f[2];
    int frames_filled = backtrace(f, 2);

    assert(frames_filled == 2);
    assert(strcmp(f[0].name, "test_backtrace_simple") == 0);
    assert(f[0].resume_addr == (uintptr_t)test_backtrace_simple + f[0].resume_offset);
    assert(strcmp(f[1].name, "main") == 0);
    assert(f[1].resume_addr == (uintptr_t)main + f[1].resume_offset);
    printf("Here is a simple backtrace:\n");
    print_frames(f, frames_filled);
    printf("\n");
}

static void test_backtrace_embedded(void)
{
    //tests that embedding callees in the same .c file
    //traces back to the caller -- 3 frames, cstart, main,
    //test_backtrace_embedded,
    frame_t f[3];
    int frames_filled = backtrace(f, 10); //uses max of 10 frames to check that program doesn't overflow even if given wrong constraints
    assert(frames_filled == 3);
    assert(strcmp(f[0].name, "test_backtrace_embedded") == 0);
    //ensures that the frame lies on the stack (i.e. starts with 0x8)
    assert((f[0].resume_addr & 0x8000) == 0x8000);
    assert(f[0].resume_addr == (uintptr_t)test_backtrace_embedded + f[0].resume_offset);
    assert(strcmp(f[1].name, "main") == 0);
    assert(f[1].resume_addr == (uintptr_t)main + f[1].resume_offset);
    assert(strcmp(f[2].name, "_cstart") == 0);
    printf("Here is an embedded backtrace:\n");
    print_frames(f, frames_filled);
    printf("\n");
}

static int recursion_fun(int n)
{
    if (n == 0)
        return mystery();   // look in nameless.c
    else
        return 1 + recursion_fun(n-1);
}

static int test_backtrace_complex(int n)
{
    return recursion_fun(n);
}

static void test_heap_dump(void)
{
    heap_dump("Empty heap");

    int *p = malloc(sizeof(int));
    *p = 0;
    heap_dump("After p = malloc(4)");

    char *q = malloc(16);
    memcpy(q, "aaaaaaaaaaaaaaa", 16);
    heap_dump("After q = malloc(16)");

    free(p);
    heap_dump("After free(p)");

    free(q);
    heap_dump("After free(q)");
}

static void test_recycle(void)
{
    //mallocs 24 bytes, while continuously freeing and then
    //mallocing the same size. tests using heap_dump that
    //malloc is recycling the same block
    size_t SIZE = 24;
    for (int i = 0; i < 5; i++) {
        int *ptr = malloc(SIZE);
        memset((int*)ptr, 12, SIZE);
        heap_dump("After setting heap size of 24 with a header and filling it with 12s.");

        free(ptr); //frees the just-allocated block
        heap_dump("After free(ptr) in recycle.");
    }
    heap_dump("Final state");
}

static void test_split(void)
{
    //allocates a large chunk of memory in the heap, frees it,
    //and ensures via heap_dump that the large chunk is being
    //correctly broken down into smaller chunks
    int SIZE = 240;
    int *ptr = malloc(SIZE);
    heap_dump("After allocating 240 bytes.");
    free(ptr);
    heap_dump("After freeing ptr.");
    //adds a smaller malloc chunk; should reuse the contiguous
    //memory that the larger malloc used
    ptr = malloc(SIZE/8);
    memset((int*)ptr, 12, SIZE/8);
    heap_dump("After allocating 30/32 bytes into 240 bytes.");
    free(ptr);
    heap_dump("Free ptr.");
    ptr = malloc(SIZE/4);
    memset((int*)ptr, 12, SIZE/4);
    heap_dump("After allocating 60/64 bytes into 240 bytes.");
    free(ptr);
    heap_dump("Free ptr.");
    ptr = malloc(SIZE/3);
    memset((int*)ptr, 12, SIZE/3);
    heap_dump("After allocating 80 bytes into 240 bytes.");
    free(ptr);
    heap_dump("Free ptr.");
}

static void test_heap_simple(void)
{
    // allocate a string and array of ints
    // assign to values, check, then free
    const char *alphabet = "abcdefghijklmnopqrstuvwxyz";
    int len = strlen(alphabet);

    char *str = malloc(len + 1);
    memcpy(str, alphabet, len + 1);
    
    heap_dump("After memcpy in str.");

    int n = 10;
    int *arr = malloc(n*sizeof(int));
    for (int i = 0; i < n; i++) {
        arr[i] = i;
        heap_dump("After setting a member in the array.");
    }

    assert(strcmp(str, alphabet) == 0);
    free(str);
    assert(arr[0] == 0 && arr[n-1] == n-1);
    free(arr);
}

static void test_heap_oddballs(void)
{
    // test oddball cases
    char *ptr;

    ptr = malloc(200000000); // request too large to fit
    assert(ptr == NULL); // should return NULL if cannot service request
    heap_dump("After reject too-large request");

    ptr = malloc(0); // legal request, though weird
    heap_dump("After malloc(0)");
    free(ptr);

    free(NULL); // legal request, should do nothing
    heap_dump("After free(NULL)");
}

static void test_heap_multiple(void)
{
    // array of dynamically-allocated strings, each
    // string filled with repeated char, e.g. "A" , "BB" , "CCC"
    // Examine each string, verify expected contents intact.

    int n = 8;
    char *arr[n];

    for (int i = 0; i < n; i++) {
        int num_repeats = i + 1;
        char *ptr = malloc(num_repeats + 1);
        memset(ptr, 'A' - 1 + num_repeats, num_repeats);
        ptr[num_repeats] = '\0';
        arr[i] = ptr;
    }
    heap_dump("After all allocations");
    for (int i = n-1; i >= 0; i--) {
        int len = strlen(arr[i]);
        char first = arr[i][0], last = arr[i][len -1];
        assert(first == 'A' - 1 + len);  // verify payload contents
        assert(first == last);
        free(arr[i]);
    }
    heap_dump("After all frees");
}

void test_coalesce(void)
{
    //allocates a sequence of mallocs before freeing each
    //in reverse order; uses heap_dump to ensure that
    //adjacent forward blocks are being coalesced
    int *ptr1 = malloc(5);
    heap_dump("Malloc 1");
    int *ptr2 = malloc(88);
    heap_dump("Malloc 2");
    int *ptr3 = malloc(27);
    heap_dump("Malloc 3");
    int *ptr4 = malloc(22);
    heap_dump("Malloc 4");
    int *ptr5 = malloc(9);
    heap_dump("Malloc 5");
    free(ptr5);
    heap_dump("Free 5");
    free(ptr4);
    heap_dump("Free 4");
    free(ptr3);
    heap_dump("Free 3");
    free(ptr2);
    heap_dump("Free 2");
    free(ptr1);
    heap_dump("Free 1");
    int *ptr_large = malloc(140);
    heap_dump("Large malloc");
}

void test_heap_redzones(void)
{
    // DO NOT ATTEMPT THIS TEST unless your heap has red zone protection!
    char *ptr;

    ptr = malloc(9);
    memset(ptr, 'a', 9); // write into payload
    free(ptr); // ptr is OK

    ptr = malloc(5);
    ptr[-1] = 0x45; // write before payload
    free(ptr);      // ptr is NOT ok


    ptr = malloc(12);
    ptr[13] = 0x45; // write after payload
    free(ptr);      // ptr is NOT ok
}

void test_autograder(void) {
    char *a = malloc(8);
    char *b = malloc(8);
    char *c = malloc(8);
    char *d = malloc(8);
    heap_dump("After mallocing a,b,c,d");
    free(c);
    heap_dump("Free c");
    free(b);
    heap_dump("Free b");
    char *e = malloc(8);
    heap_dump("Mallocing e");
    char *w = malloc(256);
    char *x = malloc(256);
    char *y = malloc(256);
    heap_dump("Mallocing w, x, y");
    free(x);
    heap_dump("Free x");
    free(y);
    heap_dump("Free y");
    free(w);
    heap_dump("Free w");
    char *z = malloc(768);
    heap_dump("Free z, size 768, should recycle");
}

void main(void)
{
    uart_init();
    uart_putstring("Start execute main() in tests/test_backtrace_malloc.c\n");

    test_name_of();

    test_backtrace_simple();
    test_backtrace_simple(); // Again so you can see the main offset change!
    test_backtrace_embedded();
    test_backtrace_complex(7);  // Slightly tricky backtrace

    test_heap_dump();
    test_recycle();
    test_split();

    test_heap_simple();
    test_heap_oddballs();
    test_heap_multiple();

    test_coalesce();
    
    test_autograder();
    
//    test_heap_redzones(); // DO NOT USE unless you implemented red zone protection
    
    uart_putstring("\nSuccessfully finished executing main() in tests/test_backtrace_malloc.c\n");
    uart_putchar(EOT);
}
