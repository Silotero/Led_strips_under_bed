#ifndef F_CPU
#define F_CPU 20000000UL
#endif
#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <FastLED.h>
#include <UART_w_c.h>
#include <stdio.h>

#define NUM_LEDS 60 // number of used leds
#define DATA_PIN_STRIP_1 2 // pin for the led strip control
#define DATA_PIN_STRIP_2 3 // pin for the led strip control
#define NUM_SENSORS 8 // number of used distance sensors
#define MAX_DIST 200  // cm
#define CHIPSET WS2813
#define COLOR_ORDER BRG

#define CPU_FREQUENCY 20000000UL
#define BAUDRATE 9600

#define DEBUG 0

#if DEBUG
    #define DEBUG_uart_putchar (x) uart_putchar(x)
#endif

CRGB leds1[NUM_LEDS]; // array for holding values for the leds
CRGB leds2[NUM_LEDS]; // array for holding values for the leds

typedef struct {
    volatile uint8_t *ddr;
    volatile uint8_t *port;
    volatile uint8_t *pinx;
    uint8_t pin;
} Sensor_Pin;

const Sensor_Pin sensor_pins[] = {
    { &DDRD, &PORTD, &PIND, PD5 },
    { &DDRD, &PORTD, &PIND, PD6 },
    { &DDRD, &PORTD, &PIND, PD7 },
    { &DDRB, &PORTB, &PINB, PB0 },
    { &DDRC, &PORTC, &PINC, PC0 },
    { &DDRC, &PORTC, &PINC, PC1 },
    { &DDRC, &PORTC, &PINC, PC2 },
    { &DDRC, &PORTC, &PINC, PC3 },
};

long distances[NUM_SENSORS];
uint8_t counter = 0;

void timer1_init(void) {
    TCCR1A = 0;               // Normal mode
    TCCR1B = (1 << CS11);     // Prescaler 8 → 1 tick = 0.5 us @ 16 MHz, 1 us @ 8 MHz, 0,4 us @ 20 MHz
    TCNT1 = 0;                // Reset counter
}

long readDistance(volatile uint8_t *ddr, volatile uint8_t *port_out, volatile uint8_t *pinx_in, uint8_t pin) {
    // Set pin as output to send TRIG
    *ddr |= (1 << pin);       // Output
    *port_out &= ~(1 << pin);     // LOW
    _delay_us(2);
    *port_out |= (1 << pin);      // HIGH for 10us
    _delay_us(10);
    *port_out &= ~(1 << pin);     // LOW

    // Set pin as input to receive ECHO
    *ddr &= ~(1 << pin);      // Input
    *port_out &= ~(1 << pin);

    uint32_t counter = 0;
    while(!(*pinx_in & (1 << pin))) {   // Wait for pin to be high, meaning echo has started
        if(counter++ > 10000) return 0; 
    }
    TCNT1=0; // reset counter
    while(*pinx_in & (1<< pin)) {  // wait while pin is high meaning echo is still sending
        if(TCNT1 > 40000) break; // Max distance timeout
    }
    long duration= TCNT1; // read counter after echo has ended

    long distance = duration / 146; // = 0.034 speed of sound / 2 divide by 2 cause travel forth and back * 0.4 timing with the prescaler to get 1 us Convert echo duration to cm
    return distance;
}
void setup() {
    timer1_init();
    uart_init(CPU_FREQUENCY, BAUDRATE);
    FastLED.addLeds<CHIPSET, DATA_PIN_STRIP_1, COLOR_ORDER>(leds1, NUM_LEDS);
    FastLED.addLeds<CHIPSET, DATA_PIN_STRIP_2, COLOR_ORDER>(leds2, NUM_LEDS);
    FastLED.setBrightness(100);
}

void loop() {
    counter++;
    if (counter % 10 == 0) {
        FastLED.clear();
    }
    for (int i = 0; i < NUM_SENSORS; i++) {
        long combined_distance = 0;
        for (int j = 0; j < 3; j++) {
            combined_distance += readDistance(sensor_pins[i].ddr, sensor_pins[i].port, sensor_pins[i].pinx, sensor_pins[i].pin);
            _delay_ms(80);
        }
        distances[i] = combined_distance / 2;
        distances[i] = constrain(distances[i], 0, MAX_DIST);
        #if DEBUG
            char buffer[16];
            snprintf(buffer, sizeof(buffer), "%ld\n", distances[i]);
            for (char *p = buffer; *p; p++) {
                uart_putchar(*p);
            }
            uart_putchar('c');
            uart_putchar('m');
        #endif
        if (i < 4) {
            if (distances[i] < 55 && distances[i] > 22) {
                for (int j= 0; j < 15; j++) {
                leds1[i*15+j] =  CRGB::Red;
                }
            }
            else if (distances[i] < 85) {
                for (int j = 0; j < 15; j++) {
                leds1[i*15+j] =  CRGB::Green;
                }
            }
            else if (distances[i] < 115) {
                for (int j = 0; j < 15; j++) {
                leds1[i*15+j] =  CRGB::Blue;
                }
            }
        }
        else {
            if (distances[i] < 55 && distances[i] > 22) {
                for (int j = 0; j < 15; j++) {
                leds2[(i-4)*15+j] =  CRGB::Red;
                }
            }
            else if (distances[i] < 85) {
                for (int j = 0; j < 15; j++) {
                leds2[(i-4)*15+j] =  CRGB::Green;
                }
            }
            else if (distances[i] < 115) {
                for (int j = 0; j < 15; j++) {
                leds2[(i-4)*15+j] =  CRGB::Blue;
                }
            }
        }
        FastLED.show();
    }
}