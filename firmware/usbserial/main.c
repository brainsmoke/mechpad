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
#include "keypad.h"
#include "usb_serial.h"
#include "ani.h"

static void enable_sys_tick(uint32_t ticks)
{
    STK_RVR = ticks;
    STK_CVR = 0;
    STK_CSR = STK_CSR_ENABLE|STK_CSR_TICKINT;
}

volatile uint32_t tick=0;
void SysTick_Handler(void)
{
	ws2812_write();
	tick+=1;
}

static void init(void)
{
	rcc_clock_setup_in_hsi_out_48mhz();
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);

	gpio_mode_setup(PORT_LED_DATA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, MASK_LED_DATA);

	ws2812_init();
	keypad_init();
	set_animation(OFF);

	usb_serial_init();
	enable_sys_tick(F_SYS_TICK_CLK/1600);
}

static const uint8_t keymap[N_KEYS] = KEY_MAPPING;

static volatile uint8_t key_event[N_KEYS];

void keypad_down(int key)
{
	key_event[key] = 1;
}

void keypad_up(int key)
{
	(void)key;
}

int main(void)
{
	init();
	uint32_t t_last=tick;

	frame_t *f = NULL;

	for(;;)
	{
		keypad_poll();
		usb_serial_poll();
		f = ws2812_get_frame();

		if (f != NULL && (tick-t_last > 4) )
		{
			prepare_next_frame(f);
			ws2812_swap_frame();
			t_last += 4;
		}

		int key, n_written, off=0;
		uint8_t outbuf[N_KEYS];
		uint8_t ixbuf[N_KEYS];

		for (key=0; key<N_KEYS; key++)
			if (key_event[key])
			{
				ixbuf[off] = key;
				outbuf[off] = keymap[key];
				off += 1;
			}

		if (off > 0)
		{
			n_written = usb_serial_write_noblock(outbuf, off);

			for (off=0; off<n_written; off++)
				key_event[ixbuf[off]] = 0;
		}

		switch (usb_serial_getchar())
		{
			case 'O': case 'o': set_animation(OFF);   break;
			case 'P': case 'p': set_animation(PARTY); break;
			case 'S': case 's': set_animation(SLEEP); break;
			case 'R': case 'r': set_animation(RED);   break;
			case 'G': case 'g': set_animation(GREEN); break;
			case 'B': case 'b': set_animation(BLUE);  break;
			default : break;
		}
	}
}

