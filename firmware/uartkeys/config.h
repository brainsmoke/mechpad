#ifndef CONFIG_H
#define CONFIG_H

#include <libopencm3/stm32/gpio.h>

#define F_CPU (48000000)
#define F_SYS_TICK_CLK (F_CPU/8)

#ifndef N_VALUES_PER_LED
#define N_VALUES_PER_LED  (3)
#endif

#ifndef N_LEDS_PER_STRIP
#define N_LEDS_PER_STRIP (16)
#endif

#define PORT_LED_DATA (GPIOA)
#define PIN_LED_DATA  (9)
#define N_STRIPS      (1)

#endif // CONFIG_H
