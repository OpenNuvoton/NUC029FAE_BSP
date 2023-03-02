/**************************************************************************//**
 * @file     main.c
 * @version  V2.10
 * $Date: 14/06/11 10:13a $
 * @brief    Demonstrate how to minimize power consumption when entering power down mode.
 *
 * @note
 * Copyright (C) 2023 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include "NUC029FAE.h"

#define GPIO_P0_TO_P15              0xFFFF

/**
 * @brief       Port0/Port1 IRQ
 *
 * @param       None
 *
 * @return      None
 *
 * @details     The Port0/Port1 default IRQ, declared in startup_NUC029FAE.s.
 */
void GPIO01_IRQHandler(void)
{
    uint32_t reg;
    /* To check if P0.7 interrupt occurred */
    if (P0->ISRC & BIT7)
    {
        P0->ISRC = BIT7;
        printf("P0.7 INT occurred. \n");
    }
    else
    {
        /* Un-expected interrupt. Just clear all PORT0, PORT1 interrupts */
        reg = P0->ISRC;
        P0->ISRC = reg;
        reg = P1->ISRC;
        P1->ISRC = reg;
        printf("Un-expected interrupts. \n");
    }
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Function for System Entry to Power Down Mode                                                           */
/*---------------------------------------------------------------------------------------------------------*/
void PowerDownFunction(void)
{
    /* To check if all the debug messages are finished */
    UART_WAIT_TX_EMPTY(UART);

    /* Enter to Power-down mode */
    CLK_PowerDown();
}

void SYS_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Enable internal 22.1184MHz */
    CLK->PWRCON |= CLK_PWRCON_IRC22M_EN_Msk;

    /* Waiting for clock ready */
    CLK_WaitClockReady(CLK_CLKSTATUS_IRC22M_STB_Msk);

    /* Switch HCLK clock source to HIRC */
    CLK_SetHCLK(CLK_CLKSEL0_HCLK_S_XTAL,CLK_CLKDIV_HCLK(1));

    /* STCLK to HIRC/2 STCLK to HIRC/2 */
    CLK_SetSysTickClockSrc(CLK_CLKSEL0_STCLK_S_IRC22M_DIV2);

    /* Enable IP clock */
    CLK_EnableModuleClock(UART_MODULE);

    /* Select IP clock source */
    CLK_SetModuleClock(UART_MODULE,CLK_CLKSEL1_UART_S_IRC22M,CLK_CLKDIV_UART(1));

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set P1 multi-function pins for UART RXD and TXD */
    SYS->P1_MFP = SYS_MFP_P12_RXD | SYS_MFP_P13_TXD;

    /* To update the variable SystemCoreClock */
    SystemCoreClockUpdate();

    /* Lock protected registers */
    SYS_LockReg();
}

void UART_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init UART                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Reset IP */
    SYS_ResetModule(UART_RST);

    /* Configure UART and set UART Baudrate */
    UART_Open(UART, 115200);

}

/*---------------------------------------------------------------------------------------------------------*/
/* MAIN function                                                                                           */
/*---------------------------------------------------------------------------------------------------------*/
int main (void)
{
    /* Init System, IP clock and multi-function I/O */
    SYS_Init(); //In the end of SYS_Init() will issue SYS_LockReg() to lock protected register. If user want to write protected register, please issue SYS_UnlockReg() to unlock protected register.

    /* Init UART for printf */
    UART_Init();

    printf("\n\nCPU @ %dHz\n", SystemCoreClock);

    printf("+-------------------------------------------------------------+\n");
    printf("|  SYS_PowerDown_MinCurrent and Wake-up by P0.7 Sample Code   |\n");
    printf("+-------------------------------------------------------------+\n\n");

    printf("+-------------------------------------------------------------------------+\n");
    printf("+ Operating sequence                                                      |\n");
    printf("|  1. Remove all continuous load, e.g. LED.                               |\n");
    printf("|  2. Configure all GPIO as Quasi-bidirectional Mode                      |\n");	
    printf("|  3. Enter to Power-Down                                                 |\n");
    printf("|  4. Wait for P0.7 falling-edge interrupt event to wakeup the MCU        |\n");
    printf("+-------------------------------------------------------------------------+\n\n");

    /* Check if all the debug messages are finished */
    UART_WAIT_TX_EMPTY(UART);

    /* Configure all GPIO as Quasi-bidirectional Mode*/
    GPIO_SetMode(P0, GPIO_P0_TO_P15, GPIO_PMD_QUASI);
    GPIO_SetMode(P1, GPIO_P0_TO_P15, GPIO_PMD_QUASI);
    GPIO_SetMode(P2, GPIO_P0_TO_P15, GPIO_PMD_QUASI);
    GPIO_SetMode(P3, GPIO_P0_TO_P15, GPIO_PMD_QUASI);
    GPIO_SetMode(P4, GPIO_P0_TO_P15, GPIO_PMD_QUASI);
    GPIO_SetMode(P5, GPIO_P0_TO_P15, GPIO_PMD_QUASI);

    /* Configure P0.7 as Quasi-bidirection mode and enable interrupt by falling edge trigger */
    GPIO_SetMode(P0, BIT7, GPIO_PMD_QUASI);
    GPIO_EnableInt(P0, 7, GPIO_INT_FALLING);
    NVIC_EnableIRQ(GPIO01_IRQn);

    /* Enable interrupt de-bounce function and select de-bounce sampling cycle time */
    GPIO_SET_DEBOUNCE_TIME(GPIO_DBNCECON_DBCLKSRC_HCLK, GPIO_DBNCECON_DBCLKSEL_1);
    GPIO_ENABLE_DEBOUNCE(P0, BIT7);

    /* Unlock protected registers before entering Power-down mode */
    SYS_UnlockReg();

    printf("Enter to Power-Down ......\n");
    /* Enter to Power-down mode */
    PowerDownFunction();

    /* Waiting for P0.7 falling-edge interrupt event */
    printf("System waken-up done.\n\n");

    while(1);

}


