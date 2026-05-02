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
    RCC_APBENR1 |= (1U << 0);
    RCC_IOPENR  |= (1U << 0);

    GPIOA->MODER &= ~(3U << 10);
    GPIOA->MODER |=  (2U << 10);
    GPIOA->AFRL  &= ~(0xFU << 20);
    GPIOA->AFRL  |=  (0x2U << 20);
    
    TIM2->PSC = 15U;
    TIM2->ARR = PWM_ARR;

    TIM2->CR1   |=  (1U << 7);
    TIM2->CCR1   =   0U;
    TIM2->CCMR1 &= ~(7U << 4);
    TIM2->CCMR1 |=  (6U << 4);
    TIM2->CCMR1 |=  (1U << 3);
    TIM2->CCER  |=  (1U << 0);
    
    TIM2->EGR |=  (1U << 0);
    TIM2->SR  &= ~(1U << 0);
    TIM2->CR1 |=  (1U << 0);
}

void pwm_set_duty(uint32_t duty)
{
    TIM2->CCR1 = (duty > PWM_ARR) ? PWM_ARR : duty;
}

// -----------------------------------------------------------------------------

#define STEP_INTERVAL_MS  5U   // Update duty every 5ms
#define STEP_SIZE         5U   // Duty steps per update

int main(void)
{
    systick_init();
    pwm_init();

    uint8_t ramp_up = 1U;  // 0 -> ramping down, 1 -> ramping up
    uint32_t duty = 0U;
    uint32_t last_step = 0U;

    while (1) {
	uint32_t now = ms_ticks;

	if ((now - last_step) >= STEP_INTERVAL_MS) {
	    last_step = now;

	    if (ramp_up) {
		duty += STEP_SIZE;
		if (duty >= PWM_ARR) {
		    duty = PWM_ARR;
		    ramp_up = !ramp_up;
		}
	    } else {
		if (duty <= STEP_SIZE) {
		    duty = 0U;
		    ramp_up = !ramp_up;
		} else {
		    duty -= STEP_SIZE;
		}
	    }
	}

	pwm_set_duty(duty);
    }

    return 0;
}
