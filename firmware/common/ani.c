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

#include "ani.h"

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
static uint16_t wave[256] =
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

static int state;

void prepare_next_frame(frame_t *f)
{

	static uint16_t n = 0;
	n+=1;

	int i;
	uint32_t b = n&7;
	uint32_t a = 8-b;
	uint32_t m = n>>3;
	switch (state)
	{
		case PARTY:
			for (i=0; i<N_LEDS; i++)
			{
				f->data[i*3+0] = ( a*wave[ (i*16+m)&0xff ] + b*wave[ (i*16+m+1)&0xff ] )>>4;
				f->data[i*3+1] = ( a*wave[ (i*16+m+85)&0xff ] + b*wave[ (i*16+m+86)&0xff ] )>>3;
				f->data[i*3+2] = ( a*wave[ (i*16+m+170)&0xff ] + b*wave[ (i*16+m+171)&0xff ] )>>4;
			}
			break;
		case SLEEP:
			m >>= 1;
			b = (n>>1)&7;
			a = 8-b;
			for (i=0; i<N_LEDS; i++)
			{
				f->data[i*3+0] = 0;
				f->data[i*3+1] = ( a*wave[ m&0xff ] + b*wave[ (m+1)&0xff ] )>>5;
				f->data[i*3+2] = 0;
			}
			break;
		case RED:
			for (i=0; i<N_LEDS; i++)
			{
				f->data[i*3+0] = 0;
				f->data[i*3+1] = ( a*wave[ (i*16+m)&0xff ] + b*wave[ (i*16+m+1)&0xff ] )>>3;
				f->data[i*3+2] = 0;
			}
			break;
		case GREEN:
			for (i=0; i<N_LEDS; i++)
			{
				f->data[i*3+0] = ( a*wave[ (i*16+m)&0xff ] + b*wave[ (i*16+m+1)&0xff ] )>>3;;
				f->data[i*3+1] = 0;
				f->data[i*3+2] = 0;
			}
			break;
		case BLUE:
			for (i=0; i<N_LEDS; i++)
			{
				f->data[i*3+0] = 0;
				f->data[i*3+1] = 0;
				f->data[i*3+2] = ( a*wave[ (i*16+m)&0xff ] + b*wave[ (i*16+m+1)&0xff ] )>>3;;
			}
			break;
		default:
			for (i=0; i<N_VALUES; i++)
				f->data[i] = 0;
			break;
	}
}

void set_animation(int s)
{
	state = s;
}
