#include <stdint.h>

// -----------------------------------------------------------------------------
// RCC
// -----------------------------------------------------------------------------
#define RCC_BASE     0x40021000UL
#define RCC_IOPENR   (*(volatile uint32_t *) (RCC_BASE + 0x34))
#define RCC_APBENR1  (*(volatile uint32_t *) (RCC_BASE + 0x3C))

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
// Timer
// -----------------------------------------------------------------------------

typedef struct {
    volatile uint32_t CR1;    // 0x00  Control 1
    volatile uint32_t CR2;    // 0x04  Control 2
    volatile uint32_t SMCR;   // 0x08  Slave Mode
    volatile uint32_t DIER;   // 0x0C  DMA/Interrupt Enable
    volatile uint32_t SR;     // 0x10  Status
    volatile uint32_t EGR;    // 0x14  Event Generation
    volatile uint32_t CCMR1;  // 0x18  Capture/Compare Mode 1 (CH1, CH2)
    volatile uint32_t CCMR2;  // 0x1C  Capture/Compare Mode 2 (CH3, CH4)
    volatile uint32_t CCER;   // 0x20  Capture/Compare Enable
    volatile uint32_t CNT;    // 0x24  Counter
    volatile uint32_t PSC;    // 0x28  Prescaler
    volatile uint32_t ARR;    // 0x2C  Auto-Reload (Period)
    volatile uint32_t _res1;  // 0x30  Reserved
    volatile uint32_t CCR1;   // 0x34  Compare CH1 -> Duty Cycle
    volatile uint32_t CCR2;   // 0x38  Compare CH2
    volatile uint32_t CCR3;   // 0x3C  Compare CH3
    volatile uint32_t CCR4;   // 0x40  Compare CH4
} TIM_t;

#define TIM2_BASE  0x40000000UL
#define TIM2       ((TIM_t *) TIM2_BASE)

// -----------------------------------------------------------------------------

void tim2_init(void)
{
    // Enable TIM2 clock on APB1
    RCC_APBENR1 |= (1U << 0);

    // Set prescaler : For 1MHz tick, divide by 16
    TIM2->PSC = 15U;

    // One Pulse Mode : counter stops after reaching ARR
    TIM2->CR1 |= (1U << 3);

    // Force PSC to load immediately via update event
    TIM2->EGR |= (1U << 0);

    // Clear the update flag that EGR just triggered
    TIM2->SR &= ~(1U << 0);
}

void delay_micro_sec(uint32_t micro_secs)
{
    // Reset Counter
    TIM2->CNT = 0;

    // Set the number of ticks to count
    TIM2->ARR = micro_secs;

    // Clear update flag before starting
    TIM2->SR &= ~(1U << 0);

    // Start counter
    TIM2->CR1 |= (1U << 0);

    // Wait until update Event flag is set (counter reached ARR)
    while (!(TIM2->SR & (1U << 0)));

    // Stop Timer (OPM[One-Pulse Mode] should handle this, but explicit is safe)
    TIM2->CR1 &= ~(1U << 0);
}

void delay_ms(uint32_t ms)
{
    while (ms-- > 0) delay_micro_sec(1000U);
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

    tim2_init();
    
    while (1) {
        GPIOA->BSRR = (1U << 5);       // Set PA5 HIGH (LED ON)
        delay_ms(500);
	
        GPIOA->BSRR = (1U << 21);      // Reset PA5 LOW (LED OFF)
        delay_ms(500);
    }

    return 0;
}
