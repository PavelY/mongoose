// Copyright (c) 2022 Cesanta Software Limited
// All rights reserved

#include "mcu.h"

enum { LED = 25, TX = 0, RX = 1 };  // Pins
enum { BLINK_PERIOD_MS = 750 };     // Tunables

static volatile uint32_t s_ticks;
void SysTick_Handler(void) {  // SyStick IRQ handler, triggered every 1ms
  s_ticks++;
}

static inline void printhex(const volatile void *buf, size_t len) {
  const char *hex = "0123456789abcdef";
  for (size_t i = 0; i < len; i++) {
    uint8_t byte = ((uint8_t *) buf)[i];
    putchar(hex[byte >> 4]);
    putchar(hex[byte & 15]);
    putchar(i > 0 && (i & 15) == 0 ? '\n' : ' ');
  }
  putchar('\n');
}

static volatile uint32_t s_usb;
void USBCTRL_IRQHandler(void) {  // USB IRQ handler
  uint32_t status = USB_REGS->INTS, handled = 0;
  printf("USB IRQ %lu %lx %lx\n", s_usb, USB_REGS->SIE_STATUS, USB_REGS->INTS);
  s_usb++;
  if (status & BIT(16)) {  // Setup request
    handled |= BIT(16);
    USB_REGS_CLR->SIE_STATUS = BIT(17);
    volatile uint8_t *p = (uint8_t *) &USB_DPRAM->SETUP_PACKET[0];
    if (p[0] == 0 && p[1] == 5) {  // SET_ADDRESS
      USB_REGS->ADDR_ENDP[0] = p[2];
      printf("USB SETUP: "), printhex(p, 8);
    }
  }
  if (status & BIT(4)) {  // Buffer status
    handled |= BIT(4);
    // usb_handle_buff_status();
    printf("USB BUF status\n");
  }
  if (status & BIT(12)) {  // Bus reset
    handled |= BIT(12);
    USB_REGS_CLR->SIE_STATUS = BIT(19);
    //USB_REGS->ADDR_ENDP[0] = 0;
    // printf("USB RESET SIES:%lx INTS:%lx\n", USB_REGS->SIE_STATUS,
    //       USB_REGS->INTS);
  }
  if (handled ^ status) printf("UUUUU status: %lx\n", status);
}

// uint64_t mg_millis(void) {
//   return s_ticks;
// }

static void blink(void) {
  static bool on = false;
  gpio_write(LED, on), gpio_write(16, on), gpio_write(17, on);
  on = !on;
  // printf("ticks=%lu %lu %lx\n", s_ticks, s_usb, USB_REGS->SIE_STATUS);
}

int main(void) {
  clock_init();
  usb_init();
  uart_init(UART0, 115200, RX, TX);
  gpio_init(LED, GPIO_MODE_OUTPUT, 0);  // Set LED to output mode
  gpio_write(LED, true);

  gpio_init(17, GPIO_MODE_OUTPUT, 0);
  gpio_init(16, GPIO_MODE_OUTPUT, 0);

  // printf("F_SYS=%u, F_USB=%u %x\n", F_SYS, F_USB,
  //     offsetof(struct usb_regs, INTS));
#if 0
  printf("regs: muxing=%08lx\n", USB_REGS->MUXING);
  printf("regs: pwr=%08lx\n", USB_REGS->PWR);
  printf("regs: main_ctrl=%08lx\n", USB_REGS->MAIN_CTRL);
  printf("regs: sie_ctrl=%08lx\n", USB_REGS->SIE_CTRL);
  printf("regs: inte=%08lx\n", USB_REGS->INTE);
#endif

  uint32_t blink_timer = 0;
  for (;;) {
    if (timer_expired(&blink_timer, BLINK_PERIOD_MS, s_ticks)) blink();
  }

  return 0;
}
