#include <stdint.h>

// -----------------------------------------------------------------------------
// RCC
// -----------------------------------------------------------------------------
#define RCC_BASE     0x40021000UL
#define RCC_IOPENR   (*(volatile uint32_t *) (RCC_BASE + 0x34))
#define RCC_APBENR1  (*(volatile uint32_t *) (RCC_BASE + 0x3C))

// -----------------------------------------------------------------------------
// GPIO
// -----------------------------------------------------------------------------
typedef struct {
    volatile uint32_t MODER;    // 0x00 Mode Register
    volatile uint32_t OTYPER;   // 0x04 Output Type Register
    volatile uint32_t OSPEEDR;  // 0x08 Output Speed Register
    volatile uint32_t PUPDR;    // 0x0C Pull-Up/Pull-Down Register
    volatile uint32_t IDR;      // 0x10 Input Data Register
    volatile uint32_t ODR;      // 0x14 Output Data Regsiter
    volatile uint32_t BSRR;     // 0x18 Bit Set/Reset Register
    volatile uint32_t LCKR;     // 0x1C Lock Configuration Register
    volatile uint32_t AFRL;     // 0x20 Alternate Function Low Register
    volatile uint32_t AFRH;     // 0x24 Alternate Function High Register
    volatile uint32_t BRR;      // 0x28 Bit Reset Register
} GPIO_t;

#define GPIOA_BASE  0x50000000UL
#define GPIOA       ((GPIO_t *) GPIOA_BASE)

// -----------------------------------------------------------------------------
// SysTick
// -----------------------------------------------------------------------------

typedef struct {
    volatile uint32_t CTRL;
    volatile uint32_t LOAD;
    volatile uint32_t VAL;
    volatile uint32_t CALIB;
} SysTick_t;

#define SYSTICK_BASE  (0xE000E010UL)
#define SYSTICK       ((SysTick_t *) SYSTICK_BASE)

// -----------------------------------------------------------------------------

static volatile uint32_t ms_ticks = 0;

void SysTick_Handler(void)
{
    ++ms_ticks;
}

#define SYSCLK_HZ  16000000UL
#define SYSTICK_RELOAD_1MS ((SYSCLK_HZ / 1000U) - 1U)

void systick_init(void)
{
    SYSTICK->LOAD = SYSTICK_RELOAD_1MS;
    SYSTICK->VAL = 0;
    SYSTICK->CTRL = (1U << 0) | (1U << 1) | (1U << 2);
}

void delay_ms(uint32_t ms)
{
    uint32_t start = ms_ticks;
    while ((ms_ticks - start) < ms);
}

// -----------------------------------------------------------------------------
// USART
// -----------------------------------------------------------------------------
typedef struct {
  volatile uint32_t CR1;    // 0x00  Control Register 1
  volatile uint32_t CR2;    // 0x04  Control Register 2
  volatile uint32_t CR3;    // 0x08  Control Register 3
  volatile uint32_t BRR;    // 0x0C  Baud Rate Register
  volatile uint32_t GTPR;   // 0x10  Guard Time and Prescaler Register
  volatile uint32_t RTOR;   // 0x14  Receiver Timeout Register
  volatile uint32_t RQR;    // 0x18  Request Register
  volatile uint32_t ISR;    // 0x1C  Interrupt and Status Register
  volatile uint32_t ICR;    // 0x20  Interrupt Flag Clear Register
  volatile uint32_t RDR;    // 0x24  Receive Data Register
  volatile uint32_t TDR;    // 0x28  Transmit Data Register
  volatile uint32_t PRESC;  // 0x2C  Prescaler Register
} USART_t;

#define USART2_BASE  0x40004400UL
#define USART2       ((USART_t *) USART2_BASE)

// -----------------------------------------------------------------------------

// BRR = 16000000 / 115200 = 138.88 -> Rounded to 139
// Actual Baud = 16000000/139 = 115107 => 0.08% error, well-within tolerance
#define USART2_BRR_115200  139;

void usart2_init(void)
{
    // Enable USART2 clock
    RCC_APBENR1 |= (1U << 17);

    // Enable GPIOA clock
    RCC_IOPENR |= (1U << 0);

    // PA2 -> Alternate Function Mode
    GPIOA->MODER &= ~(3U << 4);
    GPIOA->MODER |=  (2U << 4);

    // PA2 -> AF1 (USART2_TX) via AFRL[11:8]
    GPIOA->AFRL &= ~(0xFU << 8);
    GPIOA->AFRL |=  (0x1U << 8);

    // Set Baud Rate
    USART2->BRR = USART2_BRR_115200;

    // Enable Transmitter
    USART2->CR1 |= (1U << 3);

    // Enable USART2 (UE [USART Enable] must come after configuration)
    USART2->CR1 |= (1U << 0);
}

void usart2_send_byte(uint8_t byte)
{
    // Wait until TDR is empty
    while (!(USART2->ISR & (1U << 7)));
    USART2->TDR = byte;
}

// Send null-terminated string
void usart2_send_string(const char *str)
{
    while (*str) {
        usart2_send_byte(*str);
        ++str;
    }
    // wait for last byte to fully shift out
    while (!(USART2->ISR & (1U << 6)));
}

void usart2_send_uint_hex(uint32_t value)
{
    for (uint8_t i = 0; i < 8; ++i) {
	uint8_t nibble = value >> (32 - 4*(i+1)) & 0xF;
	if (nibble >= 10) {
	    usart2_send_byte(nibble-10 + 'A');
	} else {
	    usart2_send_byte(nibble + '0');
	}
    }
    // wait for last byte to fully shift out
    while (!(USART2->ISR & (1U << 6)));
}

// -----------------------------------------------------------------------------

// minicom -b 115200 -D /dev/ttyACM0
int main(void)
{
    systick_init();
    usart2_init();

    uint32_t counter = 0;

    while (1) {
        usart2_send_string("Hello STM32. Counter = 0x");
        usart2_send_uint_hex(counter);
        usart2_send_string("\r\n");

        ++counter;
        delay_ms(500);
    }

    return 0;
}
