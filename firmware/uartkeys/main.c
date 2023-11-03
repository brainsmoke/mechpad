/*
 * Copyright (c) 2023 Erik Bosman <erik@minemu.org>
 *
 * Permission  is  hereby  granted,  free  of  charge,  to  any  person
 * obtaining  a copy  of  this  software  and  associated documentation
 * files (the "Software"),  to deal in the Software without restriction,
 * including  without  limitation  the  rights  to  use,  copy,  modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the
 * Software,  and to permit persons to whom the Software is furnished to
 * do so, subject to the following conditions:
 *
 * The  above  copyright  notice  and this  permission  notice  shall be
 * included  in  all  copies  or  substantial portions  of the Software.
 *
 * THE SOFTWARE  IS  PROVIDED  "AS IS", WITHOUT WARRANTY  OF ANY KIND,
 * EXPRESS OR IMPLIED,  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY,  FITNESS  FOR  A  PARTICULAR  PURPOSE  AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT,  TORT OR OTHERWISE,  ARISING FROM, OUT OF OR IN
 * CONNECTION  WITH THE SOFTWARE  OR THE USE  OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * (http://opensource.org/licenses/mit-license.html)
 *
 */

#include <stdint.h>
#include <string.h>

#include <libopencmsis/core_cm3.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>

#include "ws2812_new.h"


static const uint32_t select_row[N_ROWS] = SELECT_ROWS;
static const uint16_t column_lookup[N_COLUMNS] = COLUMN_LOOKUP;
static const uint16_t keys[N_KEYS] = KEY_MAPPING;

static uint16_t debounce[N_KEYS];

enum
{
	KEY_UP = 0,
	KEY_DOWN,
	KEY_PRESSED,
};

static uint16_t status[N_KEYS];

static void keystate_init(void)
{
	memset(debounce, 0, sizeof(debounce));
	memset(status, 0, sizeof(status));
	gpio_mode_setup(PORT_KEY_ROWS, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, MASK_KEY_ROWS);
	gpio_mode_setup(PORT_KEY_COLUMNS, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, MASK_KEY_COLUMNS);
}

static void enable_sys_tick(uint32_t ticks)
{
    STK_RVR = ticks;
    STK_CVR = 0;
    STK_CSR = STK_CSR_ENABLE|STK_CSR_TICKINT;
}

volatile uint32_t tick=0;
void SysTick_Handler(void)
{
	if ( (tick & 3) == 0)
		ws2812_write();

	uint32_t row = (tick>>1)%N_ROWS;
	if ( !(tick & 1) )
	{
		GPIO_BSRR(PORT_KEY_ROWS) = select_row[row];
	}
	else
	{
		int col;
		uint16_t port = gpio_get(PORT_KEY_COLUMNS, MASK_KEY_COLUMNS);
		for (col=0; col<N_COLUMNS; col++)
		{
			int key = col*N_ROWS+row;
			if (debounce[key] > 0)
			{
				debounce[key]--;
				continue;
			}

			if (status[key] == KEY_DOWN) /* key press not registered yet */
				continue;
			
			int pressed = (port & column_lookup[col]);

			if (pressed && status[key] == KEY_UP)
			{
				status[key] = KEY_DOWN;
				debounce[key] = DEBOUNCE_COUNTDOWN;
			}
			if (!pressed && status[key] != KEY_UP)
			{
				status[key] = KEY_UP;
				debounce[key] = DEBOUNCE_COUNTDOWN;
			}
		}
	}

	tick+=1;
}

/*
from math import tau, cos
max_val=0xff00
gamma=2.6

size = 256
wave = [ ( 1 - cos(x*tau/size) )/2 for x in range(size) ]

def clamp(x):
   return min(max_val, max(0, x))

table = [ clamp(int(max_val * x**gamma)) for x in wave ]

for i in range(0, size, 8):
    print ( '\t'+' '.join('0x{:04x},'.format(x) for x in table[i:i+8]) )

*/
uint16_t wave[256] =
{
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0001, 0x0001, 0x0003, 0x0004, 0x0006, 0x0009,
	0x000d, 0x0012, 0x0018, 0x0020, 0x0029, 0x0035, 0x0043, 0x0054,
	0x0069, 0x0081, 0x009d, 0x00bd, 0x00e3, 0x010f, 0x0141, 0x0179,
	0x01ba, 0x0202, 0x0253, 0x02ae, 0x0312, 0x0382, 0x03fe, 0x0486,
	0x051b, 0x05be, 0x0670, 0x0732, 0x0804, 0x08e7, 0x09dc, 0x0ae4,
	0x0c00, 0x0d2f, 0x0e74, 0x0fce, 0x113e, 0x12c6, 0x1465, 0x161c,
	0x17ec, 0x19d4, 0x1bd7, 0x1df3, 0x202a, 0x227b, 0x24e6, 0x276d,
	0x2a0f, 0x2ccb, 0x2fa3, 0x3295, 0x35a2, 0x38c9, 0x3c0a, 0x3f65,
	0x42d9, 0x4666, 0x4a0a, 0x4dc5, 0x5197, 0x557e, 0x597a, 0x5d89,
	0x61aa, 0x65dc, 0x6a1f, 0x6e6f, 0x72ce, 0x7737, 0x7bab, 0x8028,
	0x84ab, 0x8934, 0x8dc0, 0x924f, 0x96dd, 0x9b69, 0x9ff2, 0xa475,
	0xa8f1, 0xad63, 0xb1ca, 0xb623, 0xba6d, 0xbea6, 0xc2cb, 0xc6dc,
	0xcad5, 0xceb5, 0xd27b, 0xd624, 0xd9af, 0xdd19, 0xe062, 0xe387,
	0xe687, 0xe960, 0xec12, 0xee9a, 0xf0f8, 0xf32a, 0xf52f, 0xf705,
	0xf8ad, 0xfa25, 0xfb6d, 0xfc83, 0xfd68, 0xfe1a, 0xfe99, 0xfee6,
	0xff00, 0xfee6, 0xfe99, 0xfe1a, 0xfd68, 0xfc83, 0xfb6d, 0xfa25,
	0xf8ad, 0xf705, 0xf52f, 0xf32a, 0xf0f8, 0xee9a, 0xec12, 0xe960,
	0xe687, 0xe387, 0xe062, 0xdd19, 0xd9af, 0xd624, 0xd27b, 0xceb5,
	0xcad5, 0xc6dc, 0xc2cb, 0xbea6, 0xba6d, 0xb623, 0xb1ca, 0xad63,
	0xa8f1, 0xa475, 0x9ff2, 0x9b69, 0x96dd, 0x924f, 0x8dc0, 0x8934,
	0x84ab, 0x8028, 0x7bab, 0x7737, 0x72ce, 0x6e6f, 0x6a1f, 0x65dc,
	0x61aa, 0x5d89, 0x597a, 0x557e, 0x5197, 0x4dc5, 0x4a0a, 0x4666,
	0x42d9, 0x3f65, 0x3c0a, 0x38c9, 0x35a2, 0x3295, 0x2fa3, 0x2ccb,
	0x2a0f, 0x276d, 0x24e6, 0x227b, 0x202a, 0x1df3, 0x1bd7, 0x19d4,
	0x17ec, 0x161c, 0x1465, 0x12c6, 0x113e, 0x0fce, 0x0e74, 0x0d2f,
	0x0c00, 0x0ae4, 0x09dc, 0x08e7, 0x0804, 0x0732, 0x0670, 0x05be,
	0x051b, 0x0486, 0x03fe, 0x0382, 0x0312, 0x02ae, 0x0253, 0x0202,
	0x01ba, 0x0179, 0x0141, 0x010f, 0x00e3, 0x00bd, 0x009d, 0x0081,
	0x0069, 0x0054, 0x0043, 0x0035, 0x0029, 0x0020, 0x0018, 0x0012,
	0x000d, 0x0009, 0x0006, 0x0004, 0x0003, 0x0001, 0x0001, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};

enum
{
	OFF,
	PARTY,
	SLEEP,
} state;

static void prepare_next_frame(frame_t *f)
{

static uint8_t n = 0;
n+=1;

	int i;
	switch (state)
	{
		case PARTY:
			for (i=0; i<N_VALUES; i+=3)
			{
				f->data[i+0] = wave[ (i*16+n)&0xff ]>>3;
				f->data[i+1] = wave[ (i*16+n+85)&0xff ]>>3;
				f->data[i+2] = wave[ (i*16+n+170)&0xff ]>>3;
			}
			break;
		case SLEEP:
			for (i=0; i<N_VALUES; i+=3)
			{
				f->data[i+0] = 0;
				f->data[i+1] = wave[ n&0xff]>>4;
				f->data[i+2] = 0;
			}
			break;
		default:
			for (i=0; i<N_VALUES; i++)
				f->data[i] = 0;
			break;
	}
}

static void init(void)
{
	rcc_clock_setup_in_hsi_out_48mhz();
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_USART2);

	gpio_mode_setup(PORT_LED_DATA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, MASK_LED_DATA);
	gpio_mode_setup(PORT_UART_TX, GPIO_MODE_AF, GPIO_PUPD_NONE, PIN_UART_TX);
	gpio_mode_setup(PORT_UART_RX, GPIO_MODE_AF, GPIO_PUPD_NONE, PIN_UART_RX);
	gpio_set_af(PORT_UART_TX, GPIO_AF1, PIN_UART_TX);
	gpio_set_af(PORT_UART_RX, GPIO_AF1, PIN_UART_RX);

    USART_CR1(UART) = 0;
    USART_BRR(UART) = UART_BAUDRATE_PRESCALE;
    USART_CR1(UART) = USART_CR1_RE | USART_CR1_TE | USART_CR1_UE;


	ws2812_init();
	keystate_init();

	enable_sys_tick(F_SYS_TICK_CLK/4000);
}

static void uart_putchar(int c)
{
	while ( (USART_ISR(UART) & USART_ISR_TXE) == 0 );
	USART_TDR(UART) = (uint8_t)c;
}


static int uart_getchar(void)
{
	if ( (USART_ISR(UART) & USART_ISR_RXNE) != 0 )
		return (uint8_t)USART_RDR(UART);
	else
		return -1;
}

int main(void)
{
	init();
	uint32_t t_last=tick;

	frame_t *f = NULL;

	for(;;)
	{
		f = ws2812_get_frame();

		if (f != NULL && (tick-t_last > 50) )
		{
			prepare_next_frame(f);
			ws2812_swap_frame();
			t_last += 50;
		}

		int key;
		for (key=0; key<N_KEYS; key++)
		{
			if (status[key] == KEY_DOWN)
			{
				uart_putchar(keys[key]);
				status[key] = KEY_PRESSED;
			}
		}

		switch (uart_getchar())
		{
			case 'P': state = PARTY; break;
			case 'O': state = OFF; break;
			case 'S': state = SLEEP; break;
			default : break;
		}
	}
}

