#include "gpio.h"
#include "timer.h"

// You call assert on an expression that you expect to be true. If expr
// actually evaluates to false, then assert calls abort, which stops
// your program and flashes the red light of doom.
#define assert(expr) if(!(expr)) abort()

// infinite loop that flashes the red power LED (GPIO 35)
void abort(void) {
    volatile unsigned int *FSEL3 = (void *)0x2020000c;
    volatile unsigned int *SET1 = (void *)0x20200020;
    volatile unsigned int *CLR1 = (void *)0x2020002c;
    volatile int delay;  // volatile counter to prevent optimize out of delay loop
    int bit35 =  1 << 3;

    // Configure GPIO 35 function to be output.
    // This wipes functions for other gpios in this register (30-39)
    // but that's okay, because this is a dead-end routine.
    *FSEL3 = 1 << 15;
    while (1) { // infinite loop
        *SET1 = bit35;  // on
        for (delay = 0x100000; delay > 0; delay--) ;
        *CLR1 = bit35;  // off
        for (delay = 0x100000; delay > 0; delay--) ;
    }
}

void test_gpio_set_get_function(void) {
    gpio_init();

    // Test get pin function (pin 2 defaults to input)
    assert( gpio_get_function(GPIO_PIN2) == GPIO_FUNC_INPUT );

    // Set pin 2 to output, confirm get returns what was set
    gpio_set_output(GPIO_PIN2);
    assert( gpio_get_function(GPIO_PIN2) == GPIO_FUNC_OUTPUT );

    // Set pin 2 back to input, confirm get returns what was set
    gpio_set_input(GPIO_PIN2);
    assert( gpio_get_function(GPIO_PIN2) == GPIO_FUNC_INPUT );

}

void test_gpio_set_get_additional_tests(void) {
    gpio_init();

    // Test get and set pin function on pin 52 and ALT3
    gpio_set_function(GPIO_PIN52, GPIO_FUNC_ALT3);
    assert( gpio_get_function(GPIO_PIN52) == GPIO_FUNC_ALT3);

    // Reconfiguring pin 52 and checking get/set's correctness
    gpio_set_function(GPIO_PIN52, GPIO_FUNC_ALT5);
    assert( gpio_get_function(GPIO_PIN52) == GPIO_FUNC_ALT5 );

    // Configuring pin 53 and checking functionality
    gpio_set_function(GPIO_PIN53, GPIO_FUNC_ALT0);
    assert( gpio_get_function(GPIO_PIN53) == GPIO_FUNC_ALT0 );
    
    // Checking pin 52 again to ensure independence from pin 53
    assert( gpio_get_function(GPIO_PIN52) == GPIO_FUNC_ALT5 );

    // Configuring pin 25 and checking functionality
    gpio_set_function(GPIO_PIN25, GPIO_FUNC_ALT2);
    assert( gpio_get_function(GPIO_PIN25) == GPIO_FUNC_ALT2 );
    
    // Set pin 2  to output, confirm get returns what was set
    gpio_set_output(GPIO_PIN2);
    assert( gpio_get_function(GPIO_PIN2) == GPIO_FUNC_OUTPUT );
    
    // Set pin 2 back to input, confirm get returns what was set
    gpio_set_input(GPIO_PIN2);
    assert( gpio_get_function(GPIO_PIN2) == GPIO_FUNC_INPUT );
    
    //Testing that setting pin 2, 25, or 53 did not interfere with pin 52
    assert( gpio_get_function(GPIO_PIN2) == GPIO_FUNC_INPUT );
    assert( gpio_get_function(GPIO_PIN25) == GPIO_FUNC_ALT2 );
    assert( gpio_get_function(GPIO_PIN52) == GPIO_FUNC_ALT5 );
    assert( gpio_get_function(GPIO_PIN53) == GPIO_FUNC_ALT0 );
    
    //Testing safeguard against getting invalid GPIO pin
    assert( gpio_get_function(-4) == GPIO_INVALID_REQUEST );
    assert( gpio_get_function(90) == GPIO_INVALID_REQUEST );

    //Testing safeguard against setting invalid function to GPIO; should "do nothing"
    gpio_set_function(GPIO_PIN52, 15);

    //Testing safeguard against setting invalid GPIO; should "do nothing"
    gpio_set_function(70, 15);
    
}

void test_gpio_read_write(void) {
    gpio_init();
    // set pin 20 to output before gpio_write
    gpio_set_function(GPIO_PIN20, GPIO_FUNC_OUTPUT);

    // gpio_write low, confirm gpio_read reads what was written
    gpio_write(GPIO_PIN20, 0);
    assert( gpio_read(GPIO_PIN20) ==  0 );

   // gpio_write high, confirm gpio_read reads what was written
    gpio_write(GPIO_PIN20, 1);
    assert( gpio_read(GPIO_PIN20) ==  1 );

    // gpio_write low, confirm gpio_read reads what was written
    gpio_write(GPIO_PIN20, 0);
    assert( gpio_read(GPIO_PIN20) ==  0 );
}

void test_gpio_read_write_additional_tests(void) {
    gpio_init();
    // set pin 53, 50, 25, and 12 to output before gpio_write
    gpio_set_function(GPIO_PIN53, GPIO_FUNC_OUTPUT);
    gpio_set_function(GPIO_PIN50, GPIO_FUNC_OUTPUT);
    gpio_set_function(GPIO_PIN25, GPIO_FUNC_OUTPUT);
    gpio_set_function(GPIO_PIN12, GPIO_FUNC_OUTPUT);

    // gpio_write low, confirm gpio_read reads what was written
    gpio_write(GPIO_PIN53, 0);
    assert( gpio_read(GPIO_PIN53) ==  0 );

   // gpio_write high, confirm gpio_read reads what was written, reconfiguring works
    gpio_write(GPIO_PIN53, 1);
    assert( gpio_read(GPIO_PIN53) ==  1 );
    
    // ensure that all pins in range are covered and independent
    gpio_write(GPIO_PIN1, 1);
    assert( gpio_read(GPIO_PIN1) ==  1 );
    gpio_write(GPIO_PIN25, 0);
    assert( gpio_read(GPIO_PIN25) ==  0 );
    gpio_write(GPIO_PIN12, 1);
    assert( gpio_read(GPIO_PIN12) ==  1 );
    assert( gpio_read(GPIO_PIN25) ==  0 );

    // gpio_write low, confirm gpio_read reads what was written
    gpio_write(GPIO_PIN53, 0);
     assert( gpio_read(GPIO_PIN53) ==  0 );
    
    // write to two gpios in the same FSEL to check independence
    gpio_write(GPIO_PIN50, 1);
    assert( gpio_read(GPIO_PIN50) ==  1 );
    assert( gpio_read(GPIO_PIN53) ==  0 );
    
    // check error handling: invalid gpio, does nothing
    gpio_write(GPIO_PIN50, 5);
    
    // check error handling: invalid pin, returns -1
    assert( gpio_read(55) ==  GPIO_INVALID_REQUEST );

}

void test_timer(void) {
    timer_init();

    // Test timer tick count incrementing
    unsigned int start = timer_get_ticks();
    for( int i=0; i<10; i++ ) { /* Spin */ }
    unsigned int finish = timer_get_ticks();
    assert( finish > start );

    // Test timer delay
    int usecs = 100;
    start = timer_get_ticks();
    timer_delay_us(usecs);
    finish = timer_get_ticks();
    assert( finish >= start + usecs );
}

void test_timer_additional_tests(void) {
    timer_init();

    // Test timer delay in seconds
    int secs = 2;
    unsigned int start = timer_get_ticks();
    timer_delay(2);
    unsigned int finish = timer_get_ticks();
    assert( finish >= start + 1000000 * secs ); //converts to system default micro seconds
}

void test_breadboard(void) {
    unsigned int all[12] = {GPIO_PIN26, GPIO_PIN19, GPIO_PIN13, GPIO_PIN6,
                            GPIO_PIN5, GPIO_PIN11, GPIO_PIN9, GPIO_PIN10,
                            GPIO_PIN21, GPIO_PIN20, GPIO_PIN16, GPIO_PIN12};
    gpio_init();
    for (int i = 0; i < 12; i++) {
        gpio_set_output(all[i]);    // configure
        gpio_write(all[i], 1);      // turn on
    }
    gpio_set_input(GPIO_PIN2);      // configure
    while (gpio_read(GPIO_PIN2) == 1) ; // wait til button press
    for (int i = 0; i < 12; i++) {
        gpio_write(all[i], 0);      // turn off
    }
}

void main(void) {
    test_gpio_set_get_function();
    test_gpio_set_get_additional_tests();
    test_gpio_read_write();
    test_gpio_read_write_additional_tests();
    test_timer();
    test_timer_additional_tests();
    test_breadboard();
}
