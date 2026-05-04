#include <stdint.h>

// -----------------------------------------------------------------------------
// Software Division (Shift-based for uint32_t)
// -----------------------------------------------------------------------------

uint32_t sw_div(uint32_t dividend, uint32_t divisor, uint32_t *rem)
{
    uint32_t quotient = 0;
    for (int8_t i = 31; i >= 0; --i) {
        if ((dividend >> i) >= divisor) {
            quotient |= (1U << i);
            dividend -= (divisor << i);
        }
    }
    if (rem) *rem = dividend;
    return quotient;
}

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

#define GPIOC_BASE  0x50000800UL
#define GPIOC       ((GPIO_t *) GPIOC_BASE)

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
// NVIC
// -----------------------------------------------------------------------------
typedef struct {
    volatile uint32_t ISER;       // Interrupt Set-Enable Register
    volatile uint32_t _res1[31];  // Reserved
    volatile uint32_t ICER;       // Interrupt Clear Enable Register
    volatile uint32_t _res2[31];  // Reserved
    volatile uint32_t ISPR;       // Interrupt Set-Pending Register
    volatile uint32_t _res3[31];  // Reserved
    volatile uint32_t ICPR;       // Interrupt Clear-Pending Register
    volatile uint32_t _res4[31];  // Reserved
    volatile uint32_t IPR[8];     // Interrupt Priority Registers
} NVIC_t;

#define NVIC_BASE  (0xE000E100UL)
#define NVIC       ((NVIC_t *) NVIC_BASE)

// -----------------------------------------------------------------------------
// EXTI
// -----------------------------------------------------------------------------

typedef struct {
    volatile uint32_t RTSR1;      // 0x00 Rising Trigger Selection Register 1
    volatile uint32_t FTSR1;      // 0x04 Falling Trigger Selection Register 1
    volatile uint32_t SWIER1;     // 0x08 Software Interrupt Event Register 1
    volatile uint32_t RPR1;       // 0x0C Rising Edge Pending Register 1
    volatile uint32_t FPR1;       // 0x10 Falling Edge Pending Register 1
    volatile uint32_t _res1[5];   // 0x14-0x24 Reserved
    volatile uint32_t RTSR2;      // 0x28 Rising Trigger Selection Register 2
    volatile uint32_t FTSR2;      // 0x2C Falling Trigger Selection Register 2
    volatile uint32_t SWIER2;     // 0x30 Software Interrupt Event Register 2
    volatile uint32_t RPR2;       // 0x34 Rising Edge Pending Register 2
    volatile uint32_t FPR2;       // 0x38 Falling Edge Pending Register 2
    volatile uint32_t _res2[9];   // 0x3C-0x5C Reserved
    volatile uint32_t EXTICR1;    // 0x60 External Interrupt Selection Register 1 (lines 0-3)
    volatile uint32_t EXTICR2;    // 0x64 External Interrupt Selection Register 2 (lines 4-7)
    volatile uint32_t EXTICR3;    // 0x68 External Interrupt Selection Register 3 (lines 8-11)
    volatile uint32_t EXTICR4;    // 0x6C External Interrupt Selection Register 4 (lines 12-15)
    volatile uint32_t _res3[4];   // 0x70-0x7C Reserved
    volatile uint32_t IMR1;       // 0x80 CPU wake-up with Interrupt Mask Register 1
    volatile uint32_t EMR1;       // 0x84 CPU wake-up with Event Mask Register 1
    volatile uint32_t _res4[2];   // 0x88-0x8C Reserved
    volatile uint32_t IMR2;       // 0x90 CPU wake-up with Interrupt Mask Register 2
    volatile uint32_t EMR2;       // 0x94 CPU wake-up with Event Mask Register 2
} EXTI_t;

#define EXTI_BASE  (0x40021800UL)
#define EXTI       ((EXTI_t *) EXTI_BASE)

// -----------------------------------------------------------------------------

void exti_init(void)
{
    EXTI->EXTICR4 &= ~(0xFU << 8);
    EXTI->EXTICR4 |=  (0x2U << 8);

    EXTI->FTSR1 |= (1U << 13);
    EXTI->IMR1  |= (1U << 13);

    NVIC->ISER |= (1U << 7);
}

static volatile uint8_t button_flag = 0;
void EXTI4_15_IRQHandler(void)
{
    if (EXTI->FPR1 & (1U << 13)) {
        EXTI->FPR1 = (1U << 13);
	button_flag = 1;
    }
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

#define USART2_BRR_115200  139;

void usart2_init(void)
{
    RCC_APBENR1 |= (1U << 17);
    RCC_IOPENR  |= (1U << 0);

    // PA2 -> AF1 (USART2_TX) via AFRL[11:8]
    GPIOA->MODER &= ~(3U << 4);
    GPIOA->MODER |=  (2U << 4);
    GPIOA->AFRL  &= ~(0xFU << 8);
    GPIOA->AFRL  |=  (0x1U << 8);

    USART2->BRR = USART2_BRR_115200;

    USART2->CR1 |= (1U << 3);  // TE
    USART2->CR1 |= (1U << 0);  // UE
}

void usart2_send_byte(uint8_t byte)
{
    while (!(USART2->ISR & (1U << 7)));  // wait TXE
    USART2->TDR = byte;
}

void usart2_send_string(const char *str)
{
    while (*str) {
        usart2_send_byte(*str);
        ++str;
    }
    while (!(USART2->ISR & (1U << 6)));  // wait TC
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
    while (!(USART2->ISR & (1U << 6)));  // wait TC
}

// -----------------------------------------------------------------------------
// LFSR (Linear Feedback Shift Register) - PRNG (Pseudo-Random Number Generator)
// -----------------------------------------------------------------------------

static uint16_t lfsr_state = 0xACE1U;

uint16_t lfsr_next(void)
{
    uint8_t lsb = lfsr_state & 1U;
    lfsr_state >>= 1;
    if (lsb) lfsr_state ^= 0xB400U;  // FeedBack Polynomial
    return lfsr_state;
}

// Map LFSR Output to printable ASCII Range [33, 126], Range-size = 94
uint8_t lfsr_to_printable_ascii(uint16_t value)
{
    uint32_t rem;
    sw_div(value, 94U, &rem);
    return 33U + rem;
}

// -----------------------------------------------------------------------------

#define NEWLINE  "\r\n"

// minicom -b 115200 -D /dev/ttyACM0
int main(void)
{
    systick_init();
    exti_init();
    usart2_init();

    RCC_IOPENR |= (1U << 0) | (1U << 2);

    // PA5 -> Output (LD4)
    GPIOA->MODER  &= ~(3U << 10);
    GPIOA->MODER  |=  (1U << 10);
    GPIOA->OTYPER &= ~(1U << 5);
    GPIOA->PUPDR  &= ~(3U << 10);
    GPIOA->BSRR    =  (1U << 21);  // LED off

    // PC13 -> Input (B1) pull-up
    GPIOC->MODER &= ~(3U << 26);
    GPIOC->PUPDR &= ~(3U << 26);
    GPIOC->PUPDR |=  (1U << 26);

    uint8_t is_led_on = 0;
    uint32_t press_count = 0;
    
    usart2_send_string("Press button to send random character"NEWLINE);

    while (1) {
	if (button_flag) {
	    button_flag = 0;

	    // Debounce
	    uint32_t debounce = ms_ticks;
	    while ((ms_ticks - debounce) < 10U);

	    if (!(GPIOC->IDR & (1U << 13))) {
		uint16_t random_value = lfsr_next();
		uint8_t  random_char  = lfsr_to_printable_ascii(random_value);

		++press_count;

		usart2_send_string("Number of Button Presses: ");
		usart2_send_uint_hex(press_count);
		usart2_send_string(" -> '");
		usart2_send_byte(random_char);
		usart2_send_string("' (raw: ");
		usart2_send_uint_hex(random_value);
		usart2_send_string(")"NEWLINE);

		is_led_on = !is_led_on;
		GPIOA->BSRR = (is_led_on) ? (1U << 5) : (1U << 21);

		while (!(GPIOC->IDR & (1U << 13)));  // wait for button-release
		delay_ms(10);
	    }
	}
	
	__asm volatile ("wfi");
    }

    return 0;
}
