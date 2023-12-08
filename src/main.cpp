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

class SoftwarePwm
{
  public:
    // WARNING ! Must be determined by running tests
    static constexpr unsigned UPDATE_TIME_US = 18;

    static constexpr unsigned DUTY_CYCLE_RESOULTION = 256;

    SoftwarePwm(const LedPin& led)
        : led(led)
        , counter(0)
        , dutyCycle(0xFF)

    {
        led.set(true);
    }

    void update() {
        if (counter == 0) {
            if (dutyCycle != 0)
                led.set(true);

        } else if (counter == dutyCycle) {
            if (dutyCycle != 0xFF)
                led.set(false);
        }

        counter++;
    }

    void setDutyCycle(uint8_t dutyCycle_) {
        dutyCycle = dutyCycle_;
    }

  private:
    const LedPin& led;
    uint8_t       counter;
    uint8_t       dutyCycle;
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

const auto& LAST_LED = LEDS[NUM_LEDS - 1];

void sleep_1s() {
    // steup WDT
    wdt_reset();
    WDTCSR = (1 << WDIE) | (0 << WDP3) | 0b110; // 1 s timeout
    WDTCSR |= (1 << WDE);

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    // sleep
    sleep_enable();
    sleep_bod_disable();
    sei();
    sleep_cpu();

    // wake
    sleep_disable();
    wdt_disable();
    cli();
}

// --------------------------------------------------------------------------------------------------------------------

// User defined
static constexpr uint32_t FALL_TIME_MS   = 700;
static constexpr uint32_t FADE_TIME_MS   = 700;
static constexpr uint32_t SILENCE_TIME_S = 3;

// Calculated
static constexpr uint32_t FALL_DELAY_MS    = FALL_TIME_MS / (NUM_LEDS - 1); // Last one is fading not falling
static constexpr uint32_t FADER_ITERATIONS = FADE_TIME_MS * 1000 / SoftwarePwm::UPDATE_TIME_US;

int main() {

    // make sure we start from darkness
    for (auto& led : LEDS)
        led.set(false);

    // Set everything to output expect "reset" (PB3)
    DDRA |= 0b11111111;
    DDRB |= 0b111;

    while (1) {
        // fall
        for (auto& led : LEDS) {
            if (&led == &LAST_LED)
                continue;

            led.set(true);
            _delay_ms(FALL_DELAY_MS);
            led.set(false);
        }

        // fade out
        uint8_t  dutyCycle = 255;
        uint32_t j         = 0;

        SoftwarePwm fader(LAST_LED);

        for (uint32_t i = 0; i < FADER_ITERATIONS; i++) {
            fader.update();

            if (++j == FADER_ITERATIONS / SoftwarePwm::DUTY_CYCLE_RESOULTION) {
                fader.setDutyCycle(dutyCycle);
                dutyCycle--;
                j = 0;
            }
        }

        // silence
        LAST_LED.set(false);
        for (uint8_t i = 0; i < SILENCE_TIME_S; i++)
            sleep_1s();
    }

    return 0;
}

ISR(WATCHDOG_vect) {}
