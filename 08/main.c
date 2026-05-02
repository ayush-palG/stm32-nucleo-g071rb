#include <stdint.h>

// -----------------------------------------------------------------------------
// RCC
// -----------------------------------------------------------------------------
#define RCC_BASE    0x40021000UL
#define RCC_IOPENR  (*(volatile uint32_t *) (RCC_BASE + 0x34))

// -----------------------------------------------------------------------------
// GPIOA
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

static volatile uint8_t led_state = 0;

void debounce_delay(void)
{
    volatile uint32_t i = 20000U;
    while (i--);
}

void EXTI4_15_IRQHandler(void)
{
    // check falling edge pending flag for line-13
    if (EXTI->FPR1 & (1U << 13)) {
	// Clear pending flag by writing 1 (atomic operation)
	EXTI->FPR1 = (1U << 13);

	// Debounce: wait and verify that button is still held
	debounce_delay();
	if (!(GPIOC->IDR & (1U << 13))) {
	    led_state = !led_state;

	    if (led_state) {
		GPIOA->BSRR = (1U << 5);   // LED On
	    } else {
		GPIOA->BSRR = (1U << 21);  // LED Off
	    }
	}
    }
}

// -----------------------------------------------------------------------------

int main(void)
{
    // Enable clocks for GPIOA and GPIOC
    RCC_IOPENR |= (1U << 0) | (1U << 2);

    // PA5 : OUTPUT (LD4)
    GPIOA->MODER  &= ~(3U << 10);
    GPIOA->MODER  |=  (1U << 10);
    GPIOA->OTYPER &= ~(1U << 5);
    GPIOA->PUPDR  &= ~(3U << 10);

    // PC13 : INPUT (B1) with pull-up
    GPIOC->MODER &= ~(3U << 26);
    GPIOC->PUPDR &= ~(3U << 26);
    GPIOC->PUPDR |=  (1U << 26);

    // Route PC13 -> EXTI line 13
    EXTI->EXTICR4 &= ~(0xFU << 8);
    EXTI->EXTICR4 |=  (0x2U << 8);  // bits[15:8] = 0x02 for Port C

    // Falling Edge Trigger on line 13
    EXTI->FTSR1 |= (1U << 13);

    // Unmask line 13 interrupt
    EXTI->IMR1 |= (1U << 13);

    // Enable EXTI4_15 IRQ (IRQ7) in NVIC
    NVIC->ISER |= (1U << 7);

    while (1) {
	// WFI -> sleep until next interrupt fires
	__asm volatile ("wfi");
    }

    return 0;
}
