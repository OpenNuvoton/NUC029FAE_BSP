/******************************************************************************
 * @file     LDROM_main.c
 * @version  V1.00
 * $Revision: 2 $
 * $Date: 14/06/06 2:38p $
 * @brief    FMC LDROM IAP sample program for NUC029FAE MCU
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include "NUC029FAE.h"
#include "uart.h"
#include "fmc.h"

typedef void (FUNC_PTR)(void);


void SYS_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable External XTAL (4~24 MHz) */
    CLK->PWRCON &= ~CLK_PWRCON_XTLCLK_EN_Msk;
    CLK->PWRCON |= (0x1 << CLK_PWRCON_XTLCLK_EN_Pos); // XTAL12M (HXT) Enabled

    /* Waiting for 12MHz clock ready */
    while (!(CLK->CLKSTATUS & CLK_CLKSTATUS_XTL_STB_Msk));

    /* Switch HCLK clock source to XTAL */
    CLK->CLKSEL0 &= ~CLK_CLKSEL0_HCLK_S_Msk;
    CLK->CLKSEL0 |= CLK_CLKSEL0_HCLK_S_XTAL;

    /* Enable IP clock */
    CLK->APBCLK |= CLK_APBCLK_UART_EN_Msk; // UART Clock Enable

    /* Select IP clock source */
    CLK->CLKSEL1 &= ~CLK_CLKSEL1_UART_S_Msk;
    CLK->CLKSEL1 |= CLK_CLKSEL1_UART_S_XTAL;// Clock source from external 12 MHz or 32 KHz crystal clock

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate PllClock, SystemCoreClock and CycylesPerUs automatically. */
    SystemCoreClockUpdate();

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set P0.0,P0.1 multi-function pins for UART RXD and TXD  */
    SYS->P0_MFP = SYS_MFP_P00_TXD | SYS_MFP_P01_RXD;

    /* Lock protected registers */
    SYS_LockReg();
}

void UART_Init()
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init UART                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/
    outp32(0x40050024, 0x30000066);             // 12M Baud Rate:115200
    outp32(0x4005000C, 0x3);                    // UART0: Line Control
}


#ifdef __ARMCC_VERSION
__asm __set_SP(uint32_t _sp)
{
    MSR MSP, r0
    BX lr
}
#endif


static void SYS_UnlockReg(void)
{
    while(SYS->RegLockAddr != SYS_RegLockAddr_RegUnLock_Msk) {
        SYS->RegLockAddr = 0x59;
        SYS->RegLockAddr = 0x16;
        SYS->RegLockAddr = 0x88;
    }
}


int main()
{
    FUNC_PTR    *func;

    SYS_Init();
    UART_Init();

    printf("\n\n");
    printf("NUC029FAE FMC IAP Sample Code [LDROM code]\n");

    SYS_UnlockReg();

    /* Enable FMC ISP function */
    FMC_Open();

    printf("\n\nPress any key to branch to APROM...\n");
    getchar();

    printf("\n\nChange VECMAP and branch to LDROM...\n");
    while (!(UART->FSR & UART_FSR_TX_EMPTY_Msk));

    /*  NOTE!
     *     Before change VECMAP, user MUST disable all interrupts.
     */
    FMC_SetVectorPageAddr(FMC_APROM_BASE);
    SYS_LockReg();

    func = (FUNC_PTR *)*(uint32_t *)(FMC_APROM_BASE + 4);
    __set_SP(*(uint32_t *)FMC_APROM_BASE);
    func();

    while (1);
}

/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/
