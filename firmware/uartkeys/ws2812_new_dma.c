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
//#include "stm32f0xx.h"
#include <libopencmsis/core_cm3.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/gpio.h>


#include "ws2812_new.h"

/* each timer channel/compare register is mapped onto one DMA channel by hardware  */

#define DMA_WS2812 (DMA1)

#define DMA_CHANNEL_DATA  (DMA_CHANNEL2)
#define DMA_CHANNEL_SET   (DMA_CHANNEL3)
#define DMA_CHANNEL_CLEAR (DMA_CHANNEL4)

#define TIMER_WS2812 (TIM1)
#define TIMER_COMPARE_DATA  (TIM_CCR1(TIMER_WS2812))
#define TIMER_COMPARE_SET   (TIM_CCR2(TIMER_WS2812))
#define TIMER_COMPARE_CLEAR (TIM_CCR4(TIMER_WS2812))
#define TIMER_PULSELEN      (TIM_ARR(TIMER_WS2812))

	                                   /* v------- 16 bit ------v */             
#define DMA_CONFIG_MEM_TO_PERIPH_16BIT ( DMA_CCR_MSIZE_16BIT | DMA_CCR_PSIZE_16BIT | \
                                         (1*DMA_CCR_DIR) | DMA_CCR_PL_HIGH )

#define DMA_CONFIG_BUF_TO_PERIPH_16BIT ( DMA_CCR_MINC | DMA_CONFIG_MEM_TO_PERIPH_16BIT )

#define DMA_CONFIG_INTERRUPT_HALF_TRANSFER (DMA_CCR_HTIE)
#define DMA_CONFIG_INTERRUPT_FULL_TRANSFER (DMA_CCR_TCIE)

static uint16_t pin_mask;

static void ws2812_dma_setup(uintptr_t gpio, uint16_t mask)
{
	pin_mask = mask;
	RCC_AHBENR |= RCC_AHBENR_DMA1EN;
	DMA_CPAR(DMA_WS2812, DMA_CHANNEL_SET) = (uint32_t)&GPIO_BSRR(gpio);
	DMA_CMAR(DMA_WS2812, DMA_CHANNEL_SET) = (uint32_t)&pin_mask;
	DMA_CCR(DMA_WS2812, DMA_CHANNEL_SET)  = DMA_CONFIG_MEM_TO_PERIPH_16BIT;

	DMA_CPAR(DMA_WS2812, DMA_CHANNEL_DATA) = (uint32_t)&GPIO_BRR(gpio);
	//DMA_CHANNEL_DATA->CMAR = ...
	DMA_CCR(DMA_WS2812, DMA_CHANNEL_DATA) = DMA_CONFIG_BUF_TO_PERIPH_16BIT |
	                                        DMA_CONFIG_INTERRUPT_HALF_TRANSFER;

	DMA_CPAR(DMA_WS2812, DMA_CHANNEL_CLEAR) = (uint32_t)&GPIO_BRR(gpio);
	DMA_CMAR(DMA_WS2812, DMA_CHANNEL_CLEAR) = (uint32_t)&pin_mask;
	DMA_CCR(DMA_WS2812, DMA_CHANNEL_CLEAR)  = DMA_CONFIG_MEM_TO_PERIPH_16BIT |
	                          DMA_CONFIG_INTERRUPT_FULL_TRANSFER;

	NVIC_EnableIRQ(NVIC_DMA1_CHANNEL2_3_DMA2_CHANNEL1_2_IRQ);
	NVIC_EnableIRQ(NVIC_DMA1_CHANNEL4_7_DMA2_CHANNEL3_5_IRQ);
}

void ws2812_dma_start(volatile uint16_t buf[], uint32_t length)
{
	/* clear flags, ... */
	DMA_CMAR(DMA_WS2812, DMA_CHANNEL_DATA) = (uint32_t)buf;
	DMA_CNDTR(DMA_WS2812, DMA_CHANNEL_SET) = length;
	DMA_CNDTR(DMA_WS2812, DMA_CHANNEL_DATA) = length;
	DMA_CNDTR(DMA_WS2812, DMA_CHANNEL_CLEAR) = length;

	DMA_CCR(DMA_WS2812, DMA_CHANNEL_SET) |= DMA_CCR_EN;
	DMA_CCR(DMA_WS2812, DMA_CHANNEL_DATA) |= DMA_CCR_EN;
	DMA_CCR(DMA_WS2812, DMA_CHANNEL_CLEAR) |= DMA_CCR_EN;

	/* start timer */
	TIM_CR1(TIMER_WS2812) |= TIM_CR1_CEN;
}

static void ws2812_dma_stop(void)
{
	/* stop timer */
	TIM_CR1(TIMER_WS2812) &= ~TIM_CR1_CEN;
	TIM_DIER(TIMER_WS2812) &=~ ( TIM_DIER_CC1DE | TIM_DIER_CC2DE | TIM_DIER_CC4DE );

	/* disable dma */
	DMA_CCR(DMA_WS2812, DMA_CHANNEL_SET) &=~ DMA_CCR_EN;
	DMA_CCR(DMA_WS2812, DMA_CHANNEL_DATA) &=~ DMA_CCR_EN;
	DMA_CCR(DMA_WS2812, DMA_CHANNEL_CLEAR) &=~ DMA_CCR_EN;

	TIM_DIER(TIMER_WS2812) = ( TIM_DIER_CC1DE | TIM_DIER_CC2DE | TIM_DIER_CC4DE );

	TIM_CNT(TIMER_WS2812) = 0;
	TIM_SR(TIMER_WS2812)  = 0;
}

/* half-transfer */
void dma1_channel2_3_dma2_channel1_2_isr(void)
{
	cm_enable_interrupts();
	ws2812_half_transfer();

	DMA_IFCR(DMA_WS2812) = DMA_ISR_HTIF2;
}

/* full-transfer */
void dma1_channel4_7_dma2_channel3_5_isr(void)
{
	ws2812_dma_stop();
	cm_enable_interrupts();
	ws2812_full_transfer();

	DMA_IFCR(DMA_WS2812) = DMA_ISR_TCIF4;
}

static void ws2812_timer_setup(int t0h, int t1h, int t_pulse)
{
	RCC_APB2ENR |= RCC_APB2ENR_TIM1EN;

	TIM_CR1(TIMER_WS2812) = (TIM_CR1_CKD_CK_INT) | /* no clock division */
	            (TIM_CR1_CMS_EDGE) |
	            (TIM_CR1_DIR_UP)   |
	            (0*TIM_CR1_OPM)   |
	            (0*TIM_CR1_URS)   |
	            (0*TIM_CR1_UDIS)  |
	            (0*TIM_CR1_CEN)   ; /* don't start counting yet */

//	TIM1->PSC = 0;

	TIM_DIER(TIMER_WS2812) = TIM_DIER_CC1DE | TIM_DIER_CC2DE | TIM_DIER_CC4DE;

	/* default */
//	TIM1->CCMR1 = (0*TIM_CCMR1_CC1S_0) | /* channel 1 = output compare */
//	              (0*TIM_CCMR1_CC2S_0) ; /* channel 2 = output compare */
//	TIM1->CCMR2 = (0*TIM_CCMR2_CC4S_0) ; /* channel 4 = output compare */

	TIMER_PULSELEN      = t_pulse - 1;

	TIMER_COMPARE_SET   = t_pulse - 1 - t1h;
	TIMER_COMPARE_DATA  = t_pulse - 1 + t0h - t1h;
	TIMER_COMPARE_CLEAR = t_pulse - 1;
}


void ws2812_dma_init(uintptr_t gpio, uint16_t mask, int t0h, int t1h, int t_pulse)
{
	ws2812_dma_setup(gpio, mask);
	ws2812_timer_setup(t0h, t1h, t_pulse);
}

