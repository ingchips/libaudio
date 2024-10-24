check_env: CMSIS_INC="${CMSIS_BASE}/Include"
ifeq ($(CMSIS_BASE),)
	$(error "CMSIS_BASE is empty")
endif

CMSIS_INC="${CMSIS_BASE}/Include"
ING_REL=../keil/sdk
ING_BUNDLE=${ING_REL}/bundles/mini/inc
SYMDEFS=-Wl,--just-symbols=${ING_REL}/bundles/mini/ING9168xx/symdefs.g

ING_SRC_INC = -I ../../../../include
LFLAGS      =    ../../../../GCC/ing916_libaudio.lib

APPNAME=demo

STARTUP=src/gstartup_ing91600.s

APPDEFS=-D USE_STDPERIPH_DRIVER

PROJECT_FILES  = ../keil/src/main.c
PROJECT_FILES += ../keil/src/profile.c
PROJECT_FILES += ${ING_REL}/src/FWlib/peripheral_adc.c
PROJECT_FILES += ${ING_REL}/src/FWlib/peripheral_pinctrl.c
PROJECT_FILES += ${ING_REL}/src/FWlib/peripheral_ssp.c
PROJECT_FILES += ${ING_REL}/src/FWlib/peripheral_sysctrl.c
PROJECT_FILES += ${ING_REL}/src/FWlib/peripheral_uart.c
PROJECT_FILES += ${ING_REL}/src/FWlib/peripheral_i2s.c
PROJECT_FILES += ${ING_REL}/src/FWlib/peripheral_dma.c
PROJECT_FILES += ${ING_REL}/src/FWlib/peripheral_pwm.c

include makefile.conf
