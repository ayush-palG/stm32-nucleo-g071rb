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

// Simple software delay
void delay(volatile uint32_t count) {
    while (count--);
}

int main(void) {

    // Enable GPIOA clock
    RCC_IOPENR |= (1U << 0);

    // Set PA5 to General Purpose Output (MODER bits [11:10] = 01)
    GPIOA->MODER &= ~(3U << 10);   // clear bits 11:10
    GPIOA->MODER |=  (1U << 10);   // set bits 11:10 = 01

    // Push-pull output (OTYPER bit 5 = 0) - default, but explicit
    GPIOA->OTYPER &= ~(1U << 5);

    // Low speed (OSPEEDR bits [11:10] = 00) - default
    GPIOA->OSPEEDR &= ~(3U << 10);

    // No pull-up/pull-down (PUPDR bits [11:10] = 00) - default
    GPIOA->PUPDR &= ~(3U << 10);

    /*
    while (1) {
	GPIOA_ODR ^= (1U << 5);
	for (volatile int i = 0; i < 500000; ++i);
    }
    */
    
    while (1) {
        GPIOA->BSRR = (1U << 5);       // Set PA5 HIGH (LED ON)
        delay(500000);

        GPIOA->BSRR = (1U << 21);      // Reset PA5 LOW (LED OFF)
        delay(500000);
    }

    return 0;
}

