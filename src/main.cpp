#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>

class LedPin
{
  public:
    LedPin(volatile uint8_t& portRegister, uint8_t pinIndex)
        : portRegister(portRegister)
        , pinMask(1 << pinIndex)

    {}

    void toggle() const {
        portRegister ^= pinMask;
    }

    void set(bool isOn) const {
        if (!isOn) // Open drain
            portRegister |= pinMask;
        else
            portRegister &= ~pinMask;
    }

  private:
    volatile uint8_t& portRegister;
    const uint8_t     pinMask;
};

// --------------------------------------------------------------------------------------------------------------------

static constexpr uint8_t NUM_LEDS = 11;

const LedPin LEDS[NUM_LEDS] = {
    LedPin(PORTA, 0), //
    LedPin(PORTA, 1), //
    LedPin(PORTA, 2), //
    LedPin(PORTA, 3), //
    LedPin(PORTA, 4), //
    LedPin(PORTA, 5), //
    LedPin(PORTA, 6), //
    LedPin(PORTA, 7), //
    LedPin(PORTB, 2), //
    LedPin(PORTB, 1), //
    LedPin(PORTB, 0), //
};

// --------------------------------------------------------------------------------------------------------------------

int main() {

    // make sure we start from darkness
    for (auto& led : LEDS)
        led.set(false);

    // Set everything to output expect "reset" (PB3)
    DDRA |= 0b11111111;
    DDRB |= 0b111;

    while (1) {
        for (auto& led : LEDS) {
            led.set(true);
            _delay_ms(100);
            led.set(false);
        }
    }

    return 0;
}
