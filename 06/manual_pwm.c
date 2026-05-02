// -----------------------------------------------------------------------------
// This is a manual way of doing something like PWM. But in this the CPU is
// fully involved and doing all the work and constantly busy. But this is a
// good example how we can do this. And this also helps us in understanding
// how PWM works. This example works without any involvement of any timers.
// -----------------------------------------------------------------------------

#include <stdint.h>

// -----------------------------------------------------------------------------
// RCC
// -----------------------------------------------------------------------------
#define RCC_BASE     0x40021000UL
#define RCC_IOPENR   (*(volatile uint32_t *) (RCC_BASE + 0x34))

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

// -----------------------------------------------------------------------------

int main(void)
{
    // Enable GPIOA clock
    RCC_IOPENR |= (1U << 0);

    // PA5 : OUTPUT (LD4)
    GPIOA->MODER  &= ~(3U << 10);
    GPIOA->MODER  |=  (1U << 10);
    GPIOA->OTYPER &= ~(1U << 5);
    GPIOA->PUPDR  &= ~(3U << 10);

    uint32_t duty = 100;
    while (1) {
	for (uint32_t i = 0; i < duty; ++i) {
	    GPIOA->BSRR = (1U << 5);
	}

	for (uint32_t i = 0; i < 1000-duty; ++i) {
	    GPIOA->BSRR = (1U << 21);
	}
    }

    return 0;
}
