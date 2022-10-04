#include "gpio.h"
#include "keyboard.h"
#include "printf.h"
#include "timer.h"
#include "uart.h"
#include "interrupts.h"

/*
 * This function tests the behavior of the assign5 ps2
 * implementation versus the new-improved assign7 version. If using the
 * assign5 implementation, a scancode that arrives while the main program
 * is waiting in delay is simply dropped. Once you upgrade your
 * ps2 module to be interrupt-driven, those scancodes should
 * be queued up and can be read after delay finishes.
 */
static void test_read_delay(void)
{
    while (1) {
        printf("Test program waiting for you to press a key (q to quit): ");
        uart_flush();
        char ch = keyboard_read_next();
        printf("\nRead: %c\n", ch);
        if (ch == 'q') break;
        printf("Test program will now pause for 1 second... ");
        uart_flush();
        timer_delay(1);
        printf("done.\n");
    }
    printf("\nGoodbye!\n");
}

/*
 Case 1:
 Typing 3 H's during the 10 second interrupts_received interval.
 Confirmed that after each key press, 32 +'s are displayed into the uart
 console, representing a full packet of press/release.
 (+++++++++++++++++++++++++++++++++)
 This represents a successful reading of clock and data lines, as well as a correct
 interrupts initialization & handler executability.
 */

/*
 Case 2:
 Typing 10 H's during the 10 second interrupts_received interval.
 '+'s are correctly displayed every time the keys are pressed.
 When executing test_read_display afterwards, 10 H's are correctly dequeued and
 displayed onto the screen.
 (Test program waiting for you to press a key (q to quit):
 Read: h
 Test program will now pause for 1 second... done.
 )
 This shows the functionality of the ringbuffer queue and scancode interpretation function
 of the handler.
 */

/*
 Case 3:
 Typing "okay" as many times as possible and testing if all of the keys are correctly caught by ps2.
 Keep typing "o"s after a load of "okay"s to see if they are registered correctly.
 Testing whether the scancodes are received in the correct sequence and dequeued in the correct
 sequence to be displayed by testing program.
 (Read: o
 Test program will now pause for 1 second... done.
 Test program waiting for you to press a key (q to quit):
 Read: k
 Test program will now pause for 1 second... done.
 Test program waiting for you to press a key (q to quit):
 Read: a
 Test program will now pause for 1 second... done.
 Test program waiting for you to press a key (q to quit):
 Read: y
 Test program will now pause for 1 second... done.
 )
 (Test program waiting for you to press a key (q to quit):
 Read: o
 Test program will now pause for 1 second... done.
 Test program waiting for you to press a key (q to quit):
 Read: o
 Test program will now pause for 1 second... done.
 Test program waiting for you to press a key (q to quit):
 Read: o
 Test program will now pause for 1 second... done.
 Test program waiting for you to press a key (q to quit):
 Read: o
 Test program will now pause for 1 second... done.
 )
 */

/*
 Case 4:
 Using key modifiers shift and capslock along with alt and ctrl to type special and alphanumeric keys.
 Testing the effect of modifiers on enqueued characters; correctly printed out ~, &, and / when shifting.
 Alphabetical characters are capitalized when capslock is active.
 Combination of shift and caps lock works correctly, even under duress testing with keyboard mashes and a
 barrage of inputs per second.
 ( Test program waiting for you to press a key (q to quit):
 Read: ~
 Test program will now pause for 1 second... done.
 Test program waiting for you to press a key (q to quit):
 Read: ~
 Test program will now pause for 1 second... done.
 Test program waiting for you to press a key (q to quit):
 Read: &
 Test program will now pause for 1 second... done.
 Test program waiting for you to press a key (q to quit):
 Read: &
 Test program will now pause for 1 second... done.
 Test program waiting for you to press a key (q to quit):
 Read: /
 Test program will now pause for 1 second... done.
 Test program waiting for you to press a key (q to quit):
 Read: A
 Test program will now pause for 1 second... done.
 Test program waiting for you to press a key (q to quit):
 Read: A
 Test program will now pause for 1 second... done.
 Test program waiting for you to press a key (q to quit):
 Read: A
 Test program will now pause for 1 second... done.
 Test program waiting for you to press a key (q to quit):
 Read: A
 Test program will now pause for 1 second... done.
 Test program waiting for you to press a key (q to quit):
 Read: g
 Test program will now pause for 1 second... done.
 Test program waiting for you to press a key (q to quit):
 Read: j
 Test program will now pause for 1 second... done.
 Test program waiting for you to press a key (q to quit):
 Read: h
 Test program will now pause for 1 second... done.
 Test program waiting for you to press a key (q to quit):
 Read: G
 Test program will now pause for 1 second... done.
 Test program waiting for you to press a key (q to quit):
 Read: J
 Test program will now pause for 1 second... done.
 )
 */

/*
 Case 5:
 Full console testing with make run.
 When typing hello with a moderately fast speed around 60 WPM, the console displays the whole sequence
 typed, and pressed enter to trigger shell evaluate evaluates the correct whitespace-delimited command
 "hello"
 Same process with "echo hello/bo/bo", "help/b/b/belp", etc. to check that backspaces operate as desired
 when executing commands and typing quickly with interrupts enabled. Newlines \n are registered correctly
 and in sequence of when it is typed while typing around 60 WPM.
 Console shows slowing performance as the buffer is scrolled and filled successively, but correctly
 registers typed characters nonetheless without dropping. GL_DRAW operates more slowly, but interrupts
 work as intended.
 Visually, observed that console works as intended with typing, scrolling, wrapping, and displaying the
 correct characters on screen.
 */

// This function doesn't test anything, it just waits in a delay
// loop, keeping the processor busy. This allows to observe whether your
// interrupt is being triggered to insert itself.
void check_interrupts_received(void)
{
    printf("Type on your PS/2 keyboard to generate events. You've got 10 seconds, go!\n");
    timer_delay(10);
    printf("Time's up!\n");
}

void main(void)
{
    gpio_init();
    timer_init();
    uart_init();
    
    //enabling & configuring global interrupts
    interrupts_init();

    keyboard_init(KEYBOARD_CLOCK, KEYBOARD_DATA);
    interrupts_global_enable();
    
    check_interrupts_received();

    test_read_delay();
    uart_putchar(EOT);
}
