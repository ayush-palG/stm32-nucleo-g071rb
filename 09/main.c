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

#define PWM_ARR            999U
#define BLINK_INTERVAL_MS  500U
#define STEP_INTERVAL_MS   5U
#define STEP_SIZE          5U

typedef enum {
    LED_BLINK_MODE,
    LED_BREATHE_MODE,
} LED_MODE;

static volatile LED_MODE current_mode = LED_BLINK_MODE;
static volatile uint8_t  is_mode_changed = 0;

void pa5_set_gpio_output(void)
{
    // Ensure TIM2 channel is disconnected first
    TIM2->CCER &= ~(1U << 0);
    TIM2->CR1  &= ~(1U << 0);

    // PA5 -> GPIO output
    GPIOA->MODER  &= ~(3U << 10);
    GPIOA->MODER  |=  (1U << 10);
    GPIOA->AFRL   &= ~(0xFU << 20);  // Clear AF selection

    // Ensure LED start with off state
    GPIOA->BSRR = (1U << 21);
}

void pa5_set_af_pwm(void)
{
    // PA5 -> Alternate Function (AF2) (TIM2_CH1)
    GPIOA->MODER &= ~(3U << 10);
    GPIOA->MODER |=  (2U << 10);
    GPIOA->AFRL  &= ~(0xFU << 20);
    GPIOA->AFRL  |=  (0x2U << 20);

    // Reset duty to 0 before enabling
    TIM2->CCR1   =   0U;
    TIM2->CCER  |=  (1U << 0);

    TIM2->EGR |=  (1U << 0);
    TIM2->SR  &= ~(1U << 0);
    TIM2->CR1 |=  (1U << 0);
}

void tim2_pwm_base_init(void)
{
    RCC_APBENR1 |= (1U << 0);

    TIM2->PSC = 15U;
    TIM2->ARR = PWM_ARR;

    TIM2->CR1   |=  (1U << 7);
    TIM2->CCMR1 &= ~(7U << 4);
    TIM2->CCMR1 |=  (6U << 4);
    TIM2->CCMR1 |=  (1U << 3);
}

void exti_button_init(void)
{
    EXTI->EXTICR4 &= ~(0xFU << 8);
    EXTI->EXTICR4 |=  (0x2U << 8);

    EXTI->FTSR1 |= (1U << 13);
    EXTI->IMR1  |= (1U << 13);

    NVIC->ISER |= (1U << 7);
}

void EXTI4_15_IRQHandler(void)
{
    if (EXTI->FPR1 & (1U << 13)) {
        EXTI->FPR1 = (1U << 13);

        switch (current_mode) {
	case LED_BLINK_MODE: current_mode = LED_BREATHE_MODE; break;
	case LED_BREATHE_MODE: current_mode = LED_BLINK_MODE; break;
	}
	is_mode_changed = 1;
    }
}

// -----------------------------------------------------------------------------

int main(void)
{
    // Enable clocks for GPIOA and GPIOC
    RCC_IOPENR |= (1U << 0) | (1U << 2);

    // PA5 : OUTPUT (LD4) initially
    GPIOA->MODER  &= ~(3U << 10);
    GPIOA->MODER  |=  (1U << 10);
    GPIOA->OTYPER &= ~(1U << 5);
    GPIOA->PUPDR  &= ~(3U << 10);

    // PC13 : INPUT (B1) with pull-up
    GPIOC->MODER &= ~(3U << 26);
    GPIOC->PUPDR &= ~(3U << 26);
    GPIOC->PUPDR |=  (1U << 26);

    systick_init();
    tim2_pwm_base_init();
    exti_button_init();

    uint8_t is_led_on  = 0U;
    uint8_t is_ramp_up = 1U;

    uint32_t duty = 0U;
    uint32_t last = 0U;

    while (1) {
        uint32_t now = ms_ticks;

        if (is_mode_changed) {
            uint32_t debounce_start = ms_ticks;
            while ((ms_ticks - debounce_start) < 20U);
            while (!(GPIOC->IDR & (1U << 13)));

            is_mode_changed = !is_mode_changed;

            switch (current_mode) {
                case LED_BLINK_MODE: {
                    is_led_on = 0U;
                    pa5_set_gpio_output();
                } break;

                case LED_BREATHE_MODE: {
                    is_ramp_up = 1U;
                    duty = 0U;
                    pa5_set_af_pwm();
                } break;
            }
        }

        switch (current_mode) {
            case LED_BLINK_MODE: {
                if ((now - last) >= BLINK_INTERVAL_MS) {
                    last = now;

                    is_led_on = !is_led_on;
                    if (is_led_on) {
                        GPIOA->BSRR = (1U << 5);
                    } else {
                        GPIOA->BSRR = (1U << 21);
                    }
                }
            } break;

            case LED_BREATHE_MODE: {
                if ((now - last) >= STEP_INTERVAL_MS) {
                    last = now;

                    if (is_ramp_up) {
                        duty += STEP_SIZE;
                        if (duty >= PWM_ARR) {
                            duty = PWM_ARR;
                            is_ramp_up = !is_ramp_up;
                        }
                    } else {
                        if (duty <= STEP_SIZE) {
                            duty = 0U;
                            is_ramp_up = !is_ramp_up;
                        } else {
                            duty -= STEP_SIZE;
                        }
                    }
                    TIM2->CCR1 = duty;
                }
            } break;
        }
    }

    return 0;
}
