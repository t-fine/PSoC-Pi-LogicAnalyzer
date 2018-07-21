/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"

#define USBFS_DEVICE  (0)

/* Size of SRAM buffer to store endpoint data. */
#define DATASIZE 64

/* Defines for DMA */
#define DMA_BYTES_PER_BURST 1
#define DMA_REQUEST_PER_BURST 1
#define DMA_SRC_BASE (CYDEV_PERIPH_BASE)
#define DMA_DST_BASE (CYDEV_SRAM_BASE)

#if (USBFS_16BITS_EP_ACCESS_ENABLE)
    /* To use the 16-bit APIs, the buffer has to be:
    *  1. The buffer size must be multiple of 2 (when endpoint size is odd).
    *     For example: the endpoint size is 63, the buffer size must be 64.
    *  2. The buffer has to be aligned to 2 bytes boundary to not cause exception
    *     while 16-bit access.
    */
    #ifdef CY_ALIGN
        /* Compiler supports alignment attribute: __ARMCC_VERSION and __GNUC__ */
        CY_ALIGN(2) uint8 buffer[BUFFER_SIZE];
    #else
        /* Complier uses pragma for alignment: __ICCARM__ */
        #pragma data_alignment = 2
        uint8 buffer[BUFFER_SIZE];
    #endif /* (CY_ALIGN) */
#else
    /* There are no specific requirements to the buffer size and alignment for 
    * the 8-bit APIs usage.
    */
    uint8 buffer[DATASIZE];
#endif /* (USBFS_GEN_16BITS_EP_ACCESS) */

/* Variable declarations for DMA */
/* Move these variable declarations to the top of the function */
uint8 DMA_Chan;
uint8 DMA_TD[2];

uint8 statusFirstArray[DATASIZE];
uint8 statusSecondArray[DATASIZE];
int statusArrayWriteFlag = 1;

uint8 potVal[2];
int oldPot1Val;
int pot1Val;
int oldPot2Val;
int pot2Val;


CY_ISR(dma_Int){
    AMux_Select(0);
    pot1Val = ADC_Read16(); 
        if((pot1Val != oldPot1Val)){
            if((pot1Val < 1))
                pot1Val = 0; 
            if((pot1Val > 254))
                pot1Val = 255;
            potVal[0] = pot1Val;
            USBFS_LoadInEP(3, potVal, 2);
        }
    oldPot1Val = pot1Val;
    
    AMux_Select(1);
    pot2Val = ADC_Read16(); 
        if((pot2Val != oldPot2Val)){
            if((pot2Val < 1))
                pot2Val = 0; 
            if((pot2Val > 254))
                pot2Val = 255;
            potVal[1] = pot2Val;
            USBFS_LoadInEP(3, potVal, 2);
        }
    oldPot2Val = pot2Val;
    
    statusArrayWriteFlag = 1 - statusArrayWriteFlag;
    if (USBFS_IN_BUFFER_EMPTY == USBFS_GetEPState(1)){
        if(statusArrayWriteFlag == 0){ //transfer 64 bytes from adcFirstArray on USB
            USBFS_LoadInEP(1, statusFirstArray, DATASIZE);
        }
        else{
            USBFS_LoadInEP(1, statusSecondArray, DATASIZE);
        }
    }
    ISR_DMA_ClearPending();   
}

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    
    USBFS_Start(USBFS_DEVICE, USBFS_5V_OPERATION);

    /* Wait until device is enumerated by host. */
    while (0u == USBFS_GetConfiguration())
    {
    }
    
    Control_Reg_Write(2);
    
    ADC_Start();
    
    ISR_DMA_StartEx(dma_Int);
    
    AMux_Select(0);
    oldPot1Val = ADC_Read16();
    AMux_Select(1);
    oldPot2Val = ADC_Read16();

    /* DMA Configuration for DMA */
    DMA_Chan = DMA_DmaInitialize(DMA_BYTES_PER_BURST, DMA_REQUEST_PER_BURST, 
        HI16(DMA_SRC_BASE), HI16(DMA_DST_BASE));
    DMA_TD[0] = CyDmaTdAllocate();
    DMA_TD[1] = CyDmaTdAllocate();
    CyDmaTdSetConfiguration(DMA_TD[0], 64, DMA_TD[1], CY_DMA_TD_INC_DST_ADR | DMA__TD_TERMOUT_EN);
    CyDmaTdSetConfiguration(DMA_TD[1], 64, DMA_TD[0], CY_DMA_TD_INC_DST_ADR | DMA__TD_TERMOUT_EN);
    CyDmaTdSetAddress(DMA_TD[0], LO16((uint32)Status_Reg_Status_PTR), LO16((uint32)statusFirstArray));
    CyDmaTdSetAddress(DMA_TD[1], LO16((uint32)Status_Reg_Status_PTR), LO16((uint32)statusSecondArray));
    CyDmaChSetInitialTd(DMA_Chan, DMA_TD[0]);
    CyDmaChEnable(DMA_Chan, 1);

    for(;;)
    {
        
    }
}

/* [] END OF FILE */
