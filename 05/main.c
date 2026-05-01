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

#define INTERVALS_SIZE  5

int main(void) {

    // Enable clocks for GPIOA and GPIOC 
    RCC_IOPENR |= (1U << 0) | (1U << 2);

    // PA5 -> Output (LD4)
    GPIOA->MODER  &= ~(3U << 10);
    GPIOA->MODER  |=  (1U << 10);
    GPIOA->OTYPER &= ~(1U << 5);
    GPIOA->PUPDR  &= ~(3U << 10);

    // PC13 -> Input (B1) with pull-up
    GPIOC->MODER &= ~(3U << 26);
    GPIOC->PUPDR &= ~(3U << 26);
    GPIOC->PUPDR |=  (1U << 26);
    
    systick_init();
    
    const uint32_t intervals[INTERVALS_SIZE] = {100, 250, 500, 1000, 2000};
    uint8_t interval_index = 0;

    uint8_t  led_state         = 0;  // 0 -> off, 1 -> on
    uint8_t  button_pressed    = 0;  // 0 -> not pressed, 1 -> pressed

    uint32_t button_press_time = 0;
    uint32_t last_toggle_time  = 0;  // last toggle time

    while (1) {
	uint32_t now = ms_ticks;

	// Non-Blocking LED Blink
	if ((now - last_toggle_time) >= intervals[interval_index]) {
	    last_toggle_time = now;
	    if (led_state) {
		GPIOA->BSRR = (1U << 21);  // LED Off
	    } else {
		GPIOA->BSRR = (1U << 5);   // LED On
	    }
	    led_state = !led_state;
	}

	// Non-Blocking Button Debounce + Press detection
	uint8_t button_current = !(GPIOC->IDR & (1U << 13));

	if (!button_pressed && button_current) {
	    button_pressed = 1;
	    button_press_time = now;
	}

	if (button_pressed && button_current && (now - button_press_time) >= 10U) {
	    interval_index = (interval_index >= INTERVALS_SIZE - 1) ? 0 : interval_index + 1;
	    button_pressed = 0;

	    // wait for release (blocking, but only for brief)
	    while (!(GPIOC->IDR & (1U << 13)));
	}

	if (!button_current) {
	    button_pressed = 0;
	}
    }

    return 0;
}
