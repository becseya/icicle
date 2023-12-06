#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>

class Pin
{};

int main()
{
    DDRA |= (1 << 0);

    while (1) {
        _delay_ms(500);
        PORTA ^= (1 << 0);
    }

    return 0;
}
