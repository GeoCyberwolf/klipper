// samd21 serial port
//
// Copyright (C) 2018-2019  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "autoconf.h" // CONFIG_*
#include "board/armcm_boot.h" // armcm_enable_irq
#include "board/t5uid1/serial_irq.h" // t5uid1_rx_data
#include "command.h" // DECL_CONSTANT_STR
#include "serial_pins.h" // GPIO_Rx
#include "../internal.h" // enable_pclock

DECL_CONSTANT_STR("RESERVE_PINS_t5uid1", GPIO_Rx_NAME "," GPIO_Tx_NAME);

#if CONFIG_ATSAMD_T5UID1_SERIAL_SERCOM0
  #if CONFIG_ATSAMD_SERIAL_SERCOM0
    #error "The serial interface selected for the T5UID1 screen is already used"
  #endif

  #define SERCOMx                 SERCOM0
  #define SERCOMx_GCLK_ID_CORE    SERCOM0_GCLK_ID_CORE
  #define ID_SERCOMx              ID_SERCOM0
  #define SERCOMx_IRQn            SERCOM0_IRQn
#elif CONFIG_ATSAMD_T5UID1_SERIAL_SERCOM1
  #if CONFIG_ATSAMD_SERIAL_SERCOM1
    #error "The serial interface selected for the T5UID1 screen is already used"
  #endif

  #define SERCOMx                 SERCOM1
  #define SERCOMx_GCLK_ID_CORE    SERCOM1_GCLK_ID_CORE
  #define ID_SERCOMx              ID_SERCOM1
  #define SERCOMx_IRQn            SERCOM1_IRQn
#elif CONFIG_ATSAMD_T5UID1_SERIAL_SERCOM2
  #if CONFIG_ATSAMD_SERIAL_SERCOM2
    #error "The serial interface selected for the T5UID1 screen is already used"
  #endif

  #define SERCOMx                 SERCOM2
  #define SERCOMx_GCLK_ID_CORE    SERCOM2_GCLK_ID_CORE
  #define ID_SERCOMx              ID_SERCOM2
  #define SERCOMx_IRQn            SERCOM2_IRQn
#elif CONFIG_ATSAMD_T5UID1_SERIAL_SERCOM3
  #if CONFIG_ATSAMD_SERIAL_SERCOM3
    #error "The serial interface selected for the T5UID1 screen is already used"
  #endif

  #define SERCOMx                 SERCOM3
  #define SERCOMx_GCLK_ID_CORE    SERCOM3_GCLK_ID_CORE
  #define ID_SERCOMx              ID_SERCOM3
  #define SERCOMx_IRQn            SERCOM3_IRQn
#elif CONFIG_ATSAMD_T5UID1_SERIAL_SERCOM4
  #if CONFIG_ATSAMD_SERIAL_SERCOM4
    #error "The serial interface selected for the T5UID1 screen is already used"
  #endif

  #define SERCOMx                 SERCOM4
  #define SERCOMx_GCLK_ID_CORE    SERCOM4_GCLK_ID_CORE
  #define ID_SERCOMx              ID_SERCOM4
  #define SERCOMx_IRQn            SERCOM4_IRQn
#elif CONFIG_ATSAMD_T5UID1_SERIAL_SERCOM5
  #if CONFIG_ATSAMD_SERIAL_SERCOM5
    #error "The serial interface selected for the T5UID1 screen is already used"
  #endif

  #define SERCOMx                 SERCOM5
  #define SERCOMx_GCLK_ID_CORE    SERCOM5_GCLK_ID_CORE
  #define ID_SERCOMx              ID_SERCOM5
  #define SERCOMx_IRQn            SERCOM5_IRQn
#endif

void
t5uid1_enable_tx_irq(void)
{
    SERCOMx->USART.INTENSET.reg = SERCOM_USART_INTENSET_DRE;
}

void
t5uid1_SERCOMx_Handler(void)
{
    uint32_t status = SERCOMx->USART.INTFLAG.reg;
    if (status & SERCOM_USART_INTFLAG_RXC)
        t5uid1_rx_byte(SERCOMx->USART.DATA.reg);
    if (status & SERCOM_USART_INTFLAG_DRE) {
        uint8_t data;
        int ret = t5uid1_get_tx_byte(&data);
        if (ret)
            SERCOMx->USART.INTENCLR.reg = SERCOM_USART_INTENSET_DRE;
        else
            SERCOMx->USART.DATA.reg = data;
    }
}

void
t5uid1_init(uint32_t baud)
{
    // Enable serial clock
    enable_pclock(SERCOMx_GCLK_ID_CORE, ID_SERCOMx);
    // Enable pins
    gpio_peripheral(GPIO_Rx, GPIO_Rx_FUNC, 0);
    gpio_peripheral(GPIO_Tx, GPIO_Tx_FUNC, 0);
    // Configure serial
    SercomUsart *su = &SERCOMx->USART;
    su->CTRLA.reg = 0;
    uint32_t areg = (SERCOM_USART_CTRLA_MODE(1)
                     | SERCOM_USART_CTRLA_DORD
                     | SERCOM_USART_CTRLA_SAMPR(1)
                     | SERCOM_USART_CTRLA_RXPO(GPIO_Rx_PINOUT)
                     | SERCOM_USART_CTRLA_TXPO(GPIO_Tx_PINOUT));
    su->CTRLA.reg = areg;
    su->CTRLB.reg = SERCOM_USART_CTRLB_RXEN | SERCOM_USART_CTRLB_TXEN;
    uint32_t freq = get_pclock_frequency(SERCOMx_GCLK_ID_CORE);
    uint32_t baud8 = freq / (2 * baud);
    su->BAUD.reg = (SERCOM_USART_BAUD_FRAC_BAUD(baud8 / 8)
                    | SERCOM_USART_BAUD_FRAC_FP(baud8 % 8));
    // enable irqs
    su->INTENSET.reg = SERCOM_USART_INTENSET_RXC;
    su->CTRLA.reg = areg | SERCOM_USART_CTRLA_ENABLE;
    armcm_enable_irq(t5uid1_SERCOMx_Handler, SERCOMx_IRQn, 0);
}
