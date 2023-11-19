
OPENCM3_DIR=../libopencm3
SHARED_DIR=../common
PROJECT=uartkeys
CFILES=main.c ws2812_new_dma.c ws2812_new.c keypad.c ani.c

# target chip
DEVICE=stm32f030c8t6

default: all

INCLUDES += $(patsubst %,-I%, . $(SHARED_DIR))
VPATH += $(SHARED_DIR)

include $(OPENCM3_DIR)/mk/genlink-config.mk
include $(OPENCM3_DIR)/mk/genlink-rules.mk
include ../rules.mk


