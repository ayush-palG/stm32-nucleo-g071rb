#include <stdint.h>

extern uint32_t _estack;
extern uint32_t _etext;
extern uint32_t _sidata;         // where .data is stored in Flash
extern uint32_t _sdata, _edata;  // .data section boundaries
extern uint32_t _sbss, _ebss;

void Reset_Handler(void);
int main(void);

void NMI_Handler                 (void) __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler           (void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler                 (void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler              (void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler             (void) __attribute__((weak, alias("Default_Handler")));
void WWDG_IRQHandler             (void) __attribute__((weak, alias("Default_Handler")));
void PVD_PVM_IRQHandler          (void) __attribute__((weak, alias("Default_Handler")));
void RTC_TAMP_IRQHandler         (void) __attribute__((weak, alias("Default_Handler")));
void FLASH_IRQHandler            (void) __attribute__((weak, alias("Default_Handler")));
void RCC_CRS_IRQHandler          (void) __attribute__((weak, alias("Default_Handler")));
void EXTI0_1_IRQHandler          (void) __attribute__((weak, alias("Default_Handler")));
void EXTI2_3_IRQHandler          (void) __attribute__((weak, alias("Default_Handler")));
void EXTI4_15_IRQHandler         (void) __attribute__((weak, alias("Default_Handler")));

__attribute__((section(".isr_vector"))) uint32_t vectors[] = {
    (uint32_t) &_estack,
    (uint32_t) &Reset_Handler,
    (uint32_t) &NMI_Handler,
    (uint32_t) &HardFault_Handler,
    0, 0, 0, 0, 0, 0, 0,
    (uint32_t) &SVC_Handler,
    0, 0,
    (uint32_t) &PendSV_Handler,
    (uint32_t) &SysTick_Handler,

    (uint32_t) &WWDG_IRQHandler,
    (uint32_t) &PVD_PVM_IRQHandler,
    (uint32_t) &RTC_TAMP_IRQHandler,
    (uint32_t) &FLASH_IRQHandler,
    (uint32_t) &RCC_CRS_IRQHandler,
    (uint32_t) &EXTI0_1_IRQHandler,
    (uint32_t) &EXTI2_3_IRQHandler,
    (uint32_t) &EXTI4_15_IRQHandler,
};

void Default_Handler(void)
{
    while (1);
}

void Reset_Handler(void)
{
    // 1. Copy .data from Flash to SRAM
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;
    while (dst < &_edata) {
	*dst++ = *src++;
    }

    // 2. Zero out .bss in SRAM
    dst = &_sbss;
    while (dst < &_ebss) {
	*dst++ = 0;
    }

    main();

    // If we got out of main function, should never reach here, trap if it does
    while (1);
}
