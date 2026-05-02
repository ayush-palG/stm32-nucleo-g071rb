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

#define PWM_ARR  999U

void pwm_init(void)
{
    // Enable TIM2 clock
    RCC_APBENR1 |= (1U << 0);

    // Enable GPIOA clock
    RCC_IOPENR |= (1U << 0);

    // PA5 -> Alternate Function Mode (MODER[11:10] = 10)
    GPIOA->MODER &= ~(3U << 10);
    GPIOA->MODER |=  (2U << 10);

    // Select AF2 (TIM2_CH1_ETR) on PA5 via AFRL[23:20]
    GPIOA->AFRL &= ~(0xFU << 20);
    GPIOA->AFRL |=  (0x2U << 20);
    
    // Set prescaler : For 1MHz tick, divide by 16
    TIM2->PSC = 15U;

    // ARR = 999 -> Period = 1000 micro-seconds = 1ms -> 1KHz PWM
    TIM2->ARR = PWM_ARR;
    
    // Enable ARR Prelaod (ARPE)
    TIM2->CR1 |= (1U << 7);

    // Initially (0% duty)
    TIM2->CCR1 = 0U;

    // PWM Mode 1 on CH1 (OC1M = 110) + enable CCR1 preload (OC1PE)
    TIM2->CCMR1 &= ~(7U << 4);
    TIM2->CCMR1 |=  (6U << 4);  // OC1M  = 110 = PWM Mode 1
    TIM2->CCMR1 |=  (1U << 3);  // OC1PE = 1

    // Enable CH1 Output (CC1E)
    TIM2->CCER |= (1U << 0);
    
    // Force update to load PSC and ARR into shadow registers
    TIM2->EGR |= (1U << 0);
    TIM2->SR &= ~(1U << 0);

    // Enable Counter
    TIM2->CR1 |= (1U << 0);
}

void pwm_set_duty(uint32_t duty)
{
    TIM2->CCR1 = (duty > PWM_ARR) ? PWM_ARR : duty;
}

// -----------------------------------------------------------------------------

int main(void)
{
    systick_init();
    pwm_init();

    while (1) {
	pwm_set_duty(250);  // 25% -> dim
	delay_ms(1000);

	pwm_set_duty(500);  // 50% -> medium
	delay_ms(1000);

	pwm_set_duty(950);  // 95% -> bright
	delay_ms(1000);
    }

    return 0;
}
