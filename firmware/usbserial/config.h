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
#define PIN_LED_DATA  (GPIO9)
#define N_STRIPS      (1)

#define N_COLUMNS (4)
#define N_ROWS (4)
#define N_KEYS (N_ROWS*N_COLUMNS)

#define KEY_MAPPING { '1', '2', '3', 'X', \
                      '4', '5', '6', 'Y', \
                      '7', '8', '9', 'Z', \
                      'C', '0', 'B', 'W', }


#define PORT_KEY_ROWS (GPIOB)
#define PIN_KEY_ROW_0 (GPIO0)
#define PIN_KEY_ROW_1 (GPIO1)
#define PIN_KEY_ROW_2 (GPIO2)
#define PIN_KEY_ROW_3 (GPIO3)
#define MASK_KEY_ROWS (PIN_KEY_ROW_0|PIN_KEY_ROW_1|PIN_KEY_ROW_2|PIN_KEY_ROW_3)

#define SELECT_ROW(n) ( PIN_KEY_ROW_##n | (MASK_KEY_ROWS^PIN_KEY_ROW_##n)<<16 )

#define SELECT_ROWS { SELECT_ROW(0), SELECT_ROW(1), SELECT_ROW(2), SELECT_ROW(3) }

#define PORT_KEY_COLUMNS (GPIOB)
#define PIN_KEY_COLUMN_0 (GPIO4)
#define PIN_KEY_COLUMN_1 (GPIO5)
#define PIN_KEY_COLUMN_2 (GPIO6)
#define PIN_KEY_COLUMN_3 (GPIO7)
#define MASK_KEY_COLUMNS (PIN_KEY_COLUMN_0|PIN_KEY_COLUMN_1|PIN_KEY_COLUMN_2|PIN_KEY_COLUMN_3)

#define COLUMN_LOOKUP { PIN_KEY_COLUMN_0, PIN_KEY_COLUMN_1, PIN_KEY_COLUMN_2, PIN_KEY_COLUMN_3 }

#define DEBOUNCE_COUNTDOWN (20*2) // 20ms


#endif // CONFIG_H
