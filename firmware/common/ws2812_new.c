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

#include <stdlib.h>
#include <string.h>

#include "ws2812_new.h"

enum
{
	STATE_CONTINUE,
	STATE_BUFFER_READ,
	STATE_SWAPPING,
};

/* wasteful, but this way we can reuse dma code */
uint16_t out_buf[N_BITS_PER_STRIP];
uint8_t residual[N_VALUES];

frame_t frame_a, frame_b;

int volatile bufstate;
frame_t * volatile cur;
frame_t * volatile next;

static void ws2812_apply_dither(uint16_t f[], uint32_t start, uint32_t end, uint8_t res[])
{
	uint16_t *out = &out_buf[start*8];
	uint32_t i;
	for (i=start; i<end; i++)
	{
		uint16_t v = f[i] + res[i];
		res[i] = v;
		/* data is inverted */
		out[0] = (v & 0x8000) ? 0 : MASK_LED_DATA;
		out[1] = (v & 0x4000) ? 0 : MASK_LED_DATA;
		out[2] = (v & 0x2000) ? 0 : MASK_LED_DATA;
		out[3] = (v & 0x1000) ? 0 : MASK_LED_DATA;
		out[4] = (v & 0x0800) ? 0 : MASK_LED_DATA;
		out[5] = (v & 0x0400) ? 0 : MASK_LED_DATA;
		out[6] = (v & 0x0200) ? 0 : MASK_LED_DATA;
		out[7] = (v & 0x0100) ? 0 : MASK_LED_DATA;
		out += 8;
	}
}

#define SAFE_HALF_TRANSFER ( (N_VALUES_PER_STRIP)/2 )

void ws2812_half_transfer(void)
{
	if (bufstate == STATE_BUFFER_READ)
	{
		bufstate = STATE_SWAPPING;
		frame_t *tmp = cur;
		cur = next;
		next = tmp;
	}
	ws2812_apply_dither(cur->data, 0, SAFE_HALF_TRANSFER, residual);
}

void ws2812_full_transfer(void)
{
	if (bufstate == STATE_SWAPPING)
		bufstate = STATE_CONTINUE;

	ws2812_apply_dither(cur->data, SAFE_HALF_TRANSFER, N_VALUES_PER_STRIP, residual);
}

static void clear_buf(frame_t *f)
{
	memset(f, 0, sizeof(*f));
}

void ws2812_init(void)
{
	int i;
	for (i=0; i<N_VALUES; i++)
		residual[i] = i*157;

	bufstate = STATE_CONTINUE;
	cur = &frame_a;
	next = &frame_b;
	clear_buf(cur);
	clear_buf(next);
	ws2812_half_transfer(); /* precompute first half */
	ws2812_full_transfer(); /* precompute second half */
	ws2812_dma_init(PORT_LED_DATA, MASK_LED_DATA, T0H, T1H, T_PULSE);
}

void ws2812_swap_frame(void)
{
	if (bufstate == STATE_CONTINUE)
		bufstate = STATE_BUFFER_READ;
}

frame_t *ws2812_get_frame(void)
{
	if (bufstate == STATE_CONTINUE)
		return next;
	else
		return NULL;
}

void ws2812_write(void)
{
	ws2812_dma_start(out_buf, N_BITS_PER_STRIP);
}

