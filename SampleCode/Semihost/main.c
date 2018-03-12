/****************************************************************************
* @file     main.c
* @version  V2.00
* $Revision: 2 $
* $Date: 14/06/11 10:13a $
* @brief    Show how to print and get character with IDE console window
*
* @note
* Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
*
******************************************************************************/

#include <stdio.h>
#include "NUC029FAE.h"



/*---------------------------------------------------------------------------------------------------------*/
/* Main Function                                                                                            */
/*---------------------------------------------------------------------------------------------------------*/

int32_t main()
{
    int8_t item;

    printf("\n Start SEMIHOST test: \n");

    while(1)
    {
        item = getchar();
        printf("%c\n",item);
    }

}





/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/



