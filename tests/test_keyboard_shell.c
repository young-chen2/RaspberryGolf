#include "assert.h"
#include "keyboard.h"
#include "printf.h"
#include "shell.h"
#include "strings.h"
#include "uart.h"
#include "timer.h"
#include "interrupts.h"

#define ESC_SCANCODE 0x76

static void test_keyboard_scancodes(void)
{
    printf("\nNow reading single scancodes. Type ESC to finish this test.\n");
    while (1) {
        unsigned char scancode = keyboard_read_scancode();
        printf("[%02x]\n", scancode);
        if (scancode == ESC_SCANCODE) break;
    }
    printf("\nDone with scancode test.\n");
}

static void test_keyboard_sequences(void)
{
    printf("\nNow reading scancode sequences (key actions). Type ESC to finish this test.\n");
    while (1) {
        key_action_t action = keyboard_read_sequence();
        printf("%s [%02x]\n", action.what == KEY_PRESS ? "  Press" :"Release", action.keycode);
        if (action.keycode == ESC_SCANCODE) break;
    }
    printf("Done with scancode sequences test.\n");
}

static void test_keyboard_events(void)
{
    printf("\nNow reading key events. Type ESC to finish this test.\n");
    while (1) {
        key_event_t evt = keyboard_read_event();
        printf("%s PS2_key: {%c,%c} Modifiers: 0x%x\n", evt.action.what == KEY_PRESS? "  Press" : "Release", evt.key.ch, evt.key.other_ch, evt.modifiers);
        if (evt.action.keycode == ESC_SCANCODE) break;
    }
    printf("Done with key events test.\n");
}

static void test_keyboard_chars(void)
{
    printf("\nNow reading chars. Type ESC to finish this test.\n");
    while (1) {
        char c = keyboard_read_next();
        if (c >= '\t' && c <= 0x80)
            printf("%c", c);
        else
            printf("[%02x]", c);
        if (c == ps2_keys[ESC_SCANCODE].ch) break;
    }
    printf("\nDone with key chars test.\n");
}

static void test_keyboard_assert(void)
{
    char ch;
    printf("\nHold down Shift and type 'g'\n");
    ch = keyboard_read_next();
    printf("%c\n", ch);
    assert(ch == 'G');  // confirm user can follow directions and correct key char generated
    
    //checks that entering a sequence of modifiers work as expected
    printf("\nHold down a sequence of modifiers: CAP_LOCK, SHIFT(HOLD), and +\n"); //handles other ch (+)
    ch = keyboard_read_next();
    printf("%c\n", ch);
    assert(ch == '+');  //the caps lock should do nothing while the shift returns the other char
    
    printf("\nHold down a sequence of modifiers: SHIFT(HOLD), CAP_LOCK (RELEASE), and 9\n");
    ch = keyboard_read_next();
    printf("%c\n", ch);
    assert(ch == '(');  //the shift should write the other char for 9
    
    //checks that modifiers themselves don't do anything; correct output at the end
    printf("\nHold down a sequence of modifiers: CTRL, CTRL, ALT, ALT CAPS_LOCK(RELEASE), CAPS_LOCK(RELEASE), SHIFT (RELEASE), CTRL, 4\n");
    ch = keyboard_read_next();
    printf("%c\n", ch);
    assert(ch == '4');
    
    timer_delay(1); //delay 1 second for the next test
    
    //checking the correct behavior for keyboard_read_event
    printf("\nPress Shift once and release\n");
    key_event_t event = keyboard_read_event();
    
    timer_delay(1); //delay 1 second for the next test
    
    //checking the correct behavior for keyboard_read_sequence
    printf("\nPress Ctrl (left) and release\n");
    key_action_t action = keyboard_read_sequence();
}

static void test_shell_evaluate(void)
{
    shell_init(keyboard_read_next, printf);

    printf("\nTest shell_evaluate on fixed commands.\n");
    int ret = shell_evaluate("echo hello, world!");
    printf("Command result should be 0 (successful), is it? %d\n", ret);
    
    //whitespace in shell_evaluate
    ret = shell_evaluate("echo      hello world!");
    printf("Command result should be 0 (successful), is it? %d\n", ret);
    
    //invalid call: command DNE
    ret = shell_evaluate("heyheyhey ! xo xo");
    printf("Command result should be 0 (successful), is it? %d\n", ret);
    
    ret = shell_evaluate("~valentine haha yeah");
    printf("Command result should be nonzero (unsuccessful), is it? %d\n", ret);
    
    //testing help
    ret = shell_evaluate("help");
    printf("Command result should be 0 (successful), is it? %d\n", ret);

    ret = shell_evaluate("help help");
    printf("Command result should be 0 (successful), is it? %d\n", ret);

    ret = shell_evaluate("help           help");
    printf("Command result is zero if successful, is it? %d\n", ret);
    
    ret = shell_evaluate("help   disad");
    printf("Command result should be nonzero (unsuccessful), is it? %d\n", ret);

    ret = shell_evaluate("help reboot");
    printf("Command result should be 0 (successful), is it? %d\n", ret);

    ret = shell_evaluate("help echo");
    printf("Command result should be 0 (successful), is it? %d\n", ret);

    ret = shell_evaluate("help poke");
    printf("Command result should be 0 (successful), is it? %d\n", ret);

    ret = shell_evaluate("help peek");
    printf("Command result should be 0 (successful), is it? %d\n", ret);

    //testing poke, peek, and reboot
    ret = shell_evaluate("      reboot      ");
    printf("Command result should be 0 (successful), is it? %d\n", ret);
    
    ret = shell_evaluate("  peek         ");
    printf("Command result should be 0 (successful), is it? %d\n", ret);
    
    ret = shell_evaluate("poke          ");
    printf("Command result should be 0 (successful), is it? %d\n", ret);
}

// This is an example of a "fake" input. When asked to "read"
// next character, returns char from a fixed string, advances index
static unsigned char read_fixed(void)
{
    const char *input = "echo hello, world\nhelp\n";
    static int index;

    char next = input[index];
    index = (index + 1) % strlen(input);
    return next;
}

static void test_shell_readline_fixed_input(void)
{
    char buf[80];
    size_t bufsize = sizeof(buf);

    shell_init(read_fixed, printf); // input is fixed sequence of characters

    printf("\nTest shell_readline, feed chars from fixed string as input.\n");
    printf("readline> ");
    shell_readline(buf, bufsize);
    printf("readline> ");
    shell_readline(buf, bufsize);
}

static void test_shell_readline_keyboard(void)
{
    char buf[80];
    size_t bufsize = sizeof(buf);

    shell_init(keyboard_read_next, printf); // input from keybaord

    printf("\nTest shell_readline, type a line of input on ps2 keyboard.\n");
    printf("? ");
    shell_readline(buf, bufsize);
    
    //use asserts to ensure functionality
    printf("? ");
    printf("\nWrite down this exact sequence: 89EXD and enter\n");
    shell_readline(buf, bufsize);
    assert(strlen(buf) == 5);
    assert(strcmp(buf, "89EXD") == 0);
    
    //tests that backspace works properly
    printf("? ");
    printf("\nWrite down this exact sequence: 800[backspace]CS and enter\n");
    shell_readline(buf, bufsize);
    assert(strlen(buf) == 4);
    assert(strcmp(buf, "80CS") == 0);

}

void main(void)
{
    gpio_init();
    timer_init();
    uart_init();
    
    interrupts_init();

    keyboard_init(KEYBOARD_CLOCK, KEYBOARD_DATA);
    interrupts_global_enable();
    
    printf("Testing keyboard and shell.\n");

    test_keyboard_scancodes();
    timer_delay_ms(500);

    test_keyboard_sequences();
    timer_delay_ms(500);

    test_keyboard_events();
    timer_delay_ms(500);

    test_keyboard_chars();

    test_keyboard_assert();

    test_shell_evaluate();

    test_shell_readline_fixed_input();

    test_shell_readline_keyboard();

    printf("All done!\n");
    uart_putchar(EOT);
}
