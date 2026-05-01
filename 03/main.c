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

// -----------------------------------------------------------------------------
// SysTick
// -----------------------------------------------------------------------------

typedef struct {
    volatile uint32_t CTRL;
    volatile uint32_t LOAD;
    volatile uint32_t VAL;
    volatile uint32_t CALIB;
} SysTick_t;

// SysTick lives in the ARM core, not STM32 peripheral space
#define SYSTICK_BASE  (0xE000E010UL)
#define SYSTICK       ((SysTick_t *) SYSTICK_BASE)

// -----------------------------------------------------------------------------

static volatile uint32_t ms_ticks = 0;

// SysTick ISR - called every 1ms automatically by hardware
void SysTick_Handler(void)
{
    ++ms_ticks;
}

// Have to define the SYSTICK_RELOAD_1MS as there is no division hardware in M0+
#define SYSCLK_HZ  16000000UL
#define SYSTICK_RELOAD_1MS ((SYSCLK_HZ / 1000U) - 1U)

void systick_init(void)
{
    // Reload value for 1ms tick
    SYSTICK->LOAD = SYSTICK_RELOAD_1MS;

    // Clear Current Value
    SYSTICK->VAL = 0;

    // Enable SysTick | Enable Interrupt | Use Processor Clock
    SYSTICK->CTRL = (1U << 0) | (1U << 1) | (1U << 2);
}

void delay_ms(uint32_t ms)
{
    uint32_t start = ms_ticks;
    while ((ms_ticks - start) < ms);
}

// -----------------------------------------------------------------------------

int main(void) {

    // Enable GPIOA clock
    RCC_IOPENR |= (1U << 0);

    // PA5 -> Output (LD4)
    GPIOA->MODER  &= ~(3U << 10);
    GPIOA->MODER  |=  (1U << 10);
    GPIOA->OTYPER &= ~(1U << 5);
    GPIOA->PUPDR  &= ~(3U << 10);

    // start SysTick
    systick_init();
    
    while (1) {
        GPIOA->BSRR = (1U << 5);       // Set PA5 HIGH (LED ON)
        delay_ms(500);
	
        GPIOA->BSRR = (1U << 21);      // Reset PA5 LOW (LED OFF)
        delay_ms(500);
    }

    return 0;
}
