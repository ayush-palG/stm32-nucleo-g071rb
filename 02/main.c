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

void delay(volatile uint32_t count) {
    while (count--);
}

int main(void) {

    // Enable clocks for GPIOA and GPIOC
    RCC_IOPENR |= (1U << 0) | (1U << 2);

    // PA5 : OUTPUT (LD4)
    GPIOA->MODER  &= ~(3U << 10);
    GPIOA->MODER  |=  (1U << 10);
    GPIOA->OTYPER &= ~(1U << 5);
    GPIOA->PUPDR  &= ~(3U << 10);

    // PC13 : INPUT (B1) with pull-up
    GPIOC->MODER &= ~(3U << 26);  // bits[27:26] = 00, input mode
    GPIOC->PUPDR &= ~(3U << 26);
    GPIOC->PUPDR |=  (1U << 26);  // bits[27:26] = 01, pull-up

    /*
    // Without debounce, we get unexpected blinking
    while (1) {
	if (!(GPIOC->IDR & (1U << 13))) {
	    GPIOA->ODR ^= (1U << 5);
	}
    }
    */
    
    while (1) {
	// Detect Initial Press (PC13 goes LOW)
	if (!(GPIOC->IDR & (1U << 13))) {
	    // Debounce - wait and re-check
	    delay(100000);

	    if (!(GPIOC->IDR & (1U << 13))) {
		// Toggle LED using ODR (xor operation)
		GPIOA->ODR ^= (1U << 5);

		// wait for button release before re-acting
		while (!(GPIOC->IDR & (1U << 13)));

		// small delay after release (release debounce)
		delay (100000);
	    }
	}
    }

    return 0;
}
