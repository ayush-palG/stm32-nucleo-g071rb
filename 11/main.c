#include <stdint.h>

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

#define PWM_ARR            999U

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

    // PA3 -> AF1 (USART2_RX) via AFRL[15:12]
    GPIOA->MODER &= ~(3U << 6);
    GPIOA->MODER |=  (2U << 6);
    GPIOA->AFRL  &= ~(0xFU << 12);
    GPIOA->AFRL  |=  (0x1U << 12);

    USART2->BRR = USART2_BRR_115200;

    USART2->CR1 |= (1U << 3);  // TE
    USART2->CR1 |= (1U << 2);  // RE
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

uint8_t usart2_receive_byte(uint8_t *byte)
{
    // clear overrun if it occurred - otherwise RXNE never sets again
    if (USART2->ISR & (1U << 3)) {
        USART2->ICR |= (1U << 3);
    }

    if (USART2->ISR & (1U << 5)) {      // check the RXNE
        *byte = (uint8_t) USART2->RDR;  // Read and clears RXNE implicitly
        return 1;
    }
    return 0;
}

// -----------------------------------------------------------------------------

#define BLINK_INTERVALS_SIZE  5U
#define STEP_INTERVAL_MS      5U
#define STEP_SIZE             5U
#define NEWLINE               "\r\n"

typedef enum {
    LED_BLINK_MODE,
    LED_BREATHE_MODE,
} LED_MODE;

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

// -----------------------------------------------------------------------------

// minicom -b 115200 -D /dev/ttyACM0
int main(void)
{
    systick_init();
    usart2_init();
    tim2_pwm_base_init();

    RCC_IOPENR    |=  (1U << 0);
    GPIOA->MODER  &= ~(3U << 10);
    GPIOA->MODER  |=  (1U << 10);
    GPIOA->OTYPER &= ~(1U << 5);
    GPIOA->PUPDR  &= ~(3U << 10);

    usart2_send_string("[STM32-G071RB] UART Command Interface"NEWLINE);
    usart2_send_string("Commands: b=blink  p=breathe  f=faster  s=slower  ?=help"NEWLINE);
    usart2_send_string("> ");

    const uint32_t blink_intervals[BLINK_INTERVALS_SIZE] = {100, 250, 500, 1000, 2000};

    LED_MODE mode = LED_BLINK_MODE;
    uint8_t blink_interval_index = 0;

    uint8_t is_led_on  = 0U;
    uint8_t is_ramp_up = 1U;

    uint32_t duty = 0U;
    uint32_t last = 0U;
    uint8_t  rx;

    while (1) {
        uint32_t now = ms_ticks;

        if (usart2_receive_byte(&rx)) {
            usart2_send_byte(rx);
            usart2_send_string(NEWLINE);

            switch (rx) {
                case 'b': {
                    if (mode != LED_BLINK_MODE) {
                        mode = LED_BLINK_MODE;
                        is_led_on = 0U;
                        pa5_set_gpio_output();
                        last = ms_ticks;
                    }
                    usart2_send_string("Mode: BLINK"NEWLINE);
                } break;

                case 'p': {
                    if (mode != LED_BREATHE_MODE) {
                        mode = LED_BREATHE_MODE;
                        is_ramp_up = 1U;
                        duty = 0U;
                        pa5_set_af_pwm();
                        last = ms_ticks;
                    }
                    usart2_send_string("Mode: BREATHE"NEWLINE);
                } break;

                case 'f': {
                    blink_interval_index -= (blink_interval_index > 0U) ? 1 : 0;
                    usart2_send_string("Faster"NEWLINE);
                } break;

                case 's': {
                    blink_interval_index += (blink_interval_index >= BLINK_INTERVALS_SIZE - 1) ? 0 : 1;
                    usart2_send_string("Slower"NEWLINE);
                } break;

                case '?': {
                    usart2_send_string("--- Help ---"NEWLINE
                                       "b : LED blink mode"NEWLINE
                                       "p : LED breathe mode"NEWLINE
                                       "f : increase blink speed"NEWLINE
                                       "s : decrease blink speed"NEWLINE
                                       "? : show this help"NEWLINE);
                } break;

                default: {
                    usart2_send_string("Unknown Command"NEWLINE);
                }
            }

            usart2_send_string("> ");
        }

        switch (mode) {
            case LED_BLINK_MODE: {
                if ((now - last) >= blink_intervals[blink_interval_index]) {
                    last = now;

                    is_led_on = !is_led_on;
                    GPIOA->BSRR = (is_led_on) ? (1U << 5) : (1U << 21);
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
