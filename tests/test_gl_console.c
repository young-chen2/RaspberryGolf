#include "assert.h"
#include "console.h"
#include "fb.h"
#include "gl.h"
#include "printf.h"
#include "strings.h"
#include "timer.h"
#include "uart.h"

void test_fb(void)
{
    fb_init(500, 500, 4, FB_SINGLEBUFFER);

    unsigned char *cptr = fb_get_draw_buffer();
    int nbytes = fb_get_pitch()*fb_get_height();
    memset(cptr, 0x99, nbytes); // fill entire framebuffer with light gray pixelsv
    timer_delay(3);
    
    fb_init(600, 400, 4, FB_DOUBLEBUFFER);
    cptr = fb_get_draw_buffer();
    nbytes = fb_get_pitch()*fb_get_height();
    memset(cptr, 0xff, nbytes); // fill one buffer with white pixels
    
    fb_swap_buffer();
    timer_delay(1);
    
    cptr = fb_get_draw_buffer();
    memset(cptr, 0x33, nbytes); // fill other buffer with dark gray pixels
    
    fb_swap_buffer();
    timer_delay(1);

    for (int i = 0; i < 5; i++) {
        fb_swap_buffer();
        timer_delay(1);
    }
}

void test_gl(void)
{
    const int WIDTH = 640;
    const int HEIGHT = 512;

    // Double buffer mode, make sure you test single buffer too!
    gl_init(WIDTH, HEIGHT, GL_DOUBLEBUFFER);

    // Background is purple
    printf("%x", gl_color(0x55, 0, 0x55));
    gl_clear(gl_color(0x55, 0, 0x55));
    
    //assert that the correct color is in the background
    assert(gl_read_pixel(0,0) == gl_color(0x55, 0, 0x55));

    // Draw green pixel at an arbitrary spot
    gl_draw_pixel(WIDTH/3, HEIGHT/3, GL_GREEN);
    assert(gl_read_pixel(WIDTH/3, HEIGHT/3) == GL_GREEN);

    // Blue rectangle in center of screen
    gl_draw_rect(WIDTH/2 - 50, HEIGHT/2 - 50, 100, 100, GL_BLUE);
    
    //assert the correct bounds of the blue rectangle
    //assert that the correct color is in the background
    assert(gl_read_pixel(WIDTH/2 - 30, HEIGHT/2 - 30) == GL_BLUE);
    assert(gl_read_pixel(WIDTH/2 + 40, HEIGHT/2 - 40) == GL_BLUE);
    assert(gl_read_pixel(WIDTH/2 - 20, HEIGHT/2 + 20) == GL_BLUE);
    assert(gl_read_pixel(WIDTH/2 + 48, HEIGHT/2 + 48) == GL_BLUE);

    // Single amber character
    gl_draw_char(60, 10, 'A', GL_AMBER);
    
    //Drawing a string!
    gl_draw_string(80, 30, "Hello, I like waffle fries. ARGHH!", GL_INDIGO);
    
    //Drawing a series of objects that fall off of the screen
    gl_draw_rect(WIDTH - 50, HEIGHT - 50, 100, 100, GL_BLUE);
    gl_draw_rect(WIDTH - 50, 0, 200, 200, GL_AMBER);
    gl_draw_string(WIDTH - 100, 100, "Hi hi hi hi hi hi hi hi hi hi hi", GL_MAGENTA);
    gl_draw_string(WIDTH - 400, 200, "TWICE STAYC BLACKPINK REDVELVET", GL_MAGENTA);

    // Show buffer with drawn contents
    gl_swap_buffer();
    timer_delay(3);
    
    //redraws the buffer and swaps it again
    gl_clear(gl_color(0x10, 0x40, 0x40));
    gl_draw_string(10, 20, "Just like TT, ah, just like TT, ah!", GL_AMBER);
    gl_draw_string(230, 90, "OOH AHH OOH AHH HEY!", GL_AMBER);
    gl_draw_rect(WIDTH/3 - 50, HEIGHT/3 - 50, 200, 340, GL_MAGENTA);
    gl_draw_char(578, 189, 'X', GL_AMBER);
    gl_draw_char(478, 189, 'Z', GL_AMBER);
    gl_draw_char(528, 189, 'N', GL_AMBER);
    
    //drawing lines!
    gl_draw_line(0, 0, WIDTH, HEIGHT, GL_GREEN);
    gl_draw_line(300, 220, 100, 0, GL_GREEN);
    
    //triangle test
    gl_draw_triangle(200, 400, 600, 400, 400, 100, GL_BLUE);
    gl_draw_triangle(200, 400, 600, 400, 400, 100, GL_AMBER);
    gl_draw_triangle(182, 600, 77, 600, 113, 100, GL_BLUE);
    
    //asserting correct pixel values
    assert(gl_read_pixel(WIDTH/3 - 40, HEIGHT/3) == GL_MAGENTA);
    
    //multiple swap buffers to ensure behavior
    for(int x = 0; x <= 10; x++) {
        gl_swap_buffer();
        timer_delay(2);
    }
}

void test_console(void)
{
    console_init(10, 30, GL_CYAN, GL_INDIGO);

    // Line 1: Hello, world!
    console_printf("Hello, worldd\b!\b!\n"); //backspace correctness
    timer_delay(2);

    // Add line 2: Happiness == CODING
    console_printf("Happiness");
    console_printf(" == ");
    console_printf("CODING\b\bNG\n");
    timer_delay(2);

    // Add 2 blank lines and line 5: I am Pi, hear me roar!
    console_printf("\n\nI am Pi, hear me v\b \broar!\n"); // typo, backspace, correction
    timer_delay(2);
    
    // Print some more!
    console_printf("What is love %c ", '?');
    console_printf("Square. %04x, %d. \n", 0x90, 390);
    timer_delay(2);
    
    //Line wrapping!
    console_printf("So I run to you! I run to you! I run to you! Saranghae. Run2U! STAYC Girls.");
    timer_delay(3);
    
    // Scrolling and wrapping in combination!
    console_printf("\n\n\n\n\nHey!");
    timer_delay(3);
    console_printf("Hey!! ABCDEFGHIJKMNOPQRSTUVWXYZ I like cupcakes.");
    timer_delay(3);
    console_printf("\nIt's a love story, Baby, just say yes!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    timer_delay(3);
    
    //Special Chars
    console_printf("\nByebye~/*");

    // Clear all lines
    console_printf("\f");

    // Fills up the new screen and scrolls
    console_printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nOOOH MMMM AAAAAAAAA YEA YEAH!");
    console_printf("Stanford Stanford Stanford Stanford Go Card?!?~!@^");
    timer_delay(3);
    
    // Clear all lines
    console_printf("\f");
    
    //New console: "Goodbye"
    console_printf("Goodbye!\n");
}

void test_autograder(void) {
    gl_init(32, 16, GL_DOUBLEBUFFER);

    gl_clear(gl_color(0x55, 0, 0x55));
    
    //assert that the correct color is in the background
    assert(gl_read_pixel(0,0) == gl_color(0x55, 0, 0x55));

    gl_draw_char(60, 10, 'A', GL_AMBER);
    
    gl_draw_rect(-1, -1, 5, 3, GL_RED);
    
    gl_draw_rect(27, 11, 20, 20, GL_BLUE);

    gl_draw_char(0, 8, 'f', GL_AMBER);

    // Show buffer with drawn contents
    gl_swap_buffer();
    timer_delay(5);
}

void main(void)
{
    uart_init();
    timer_init();
    printf("Executing main() in test_gl_console.c\n");

//    test_fb();
//    test_gl();
    test_autograder();
//    test_console();

    printf("Completed main() in test_gl_console.c\n");
    uart_putchar(EOT);
}
