/**
  ******************************************************************************
  * @file    main.c 
  * @author  Go Takahashi
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "syscall.h"
#include "stm32f4_discovery.h"
#include "stm32f4xx_conf.h" // again, added because ST didn't put it here ?
#include "mruby.h"
#include "mruby/dump.h"
#include "mruby/proc.h"
#include "mrduino.h"

#ifndef USE_UART_STDIO
  #include "usbd_usr.h"
  #include "usbd_desc.h"
  #include "usbd_cdc_vcp.h"
  #ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
    #if defined ( __ICCARM__ ) /*!< IAR Compiler */
      #pragma data_alignment = 4   
    #endif
  #endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
  __ALIGN_BEGIN USB_OTG_CORE_HANDLE  USB_OTG_dev __ALIGN_END;
#endif

/* Private typedef -----------------------------------------------------------*/
GPIO_InitTypeDef  GPIO_InitStructure;

extern const char mrbmain_irep[];

/* Private define ------------------------------------------------------------*/

#define USERBUTTON GPIOA, GPIO_Pin_0


#define FLASH_PAGE_SIZE    ((uint16_t)0x400)
#define RBBIN_START_ADDR  ((uint32_t)0x08040000)
#define RRBIN_END_ADDR    ((uint32_t)0x08080000)


#define BIN_BUFFER_SIZE ((unsigned int)256)


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
  
int write_handler(char c);
int read_handler();
void Delay(__IO uint32_t nCount);
extern void rbmain(mrb_state *mrb);
void initialize_basic_periph();

//uint8_t read_byte();
uint32_t read_uint32();
uint32_t read_bytes(uint8_t *buf, uint32_t len);
uint16_t addr2sector(uint8_t* addr);

#define FLASH_SECTOR_SIZE ((uint8_t)12)
#define FLASH_Sector_Unknown ((uint16_t)0x0FFF)

uint32_t flash_sector_base_addrs[] = {
  0x08000000, 0x08004000, 0x08008000, 0x0800C000, 
  0x08010000, 0x08020000, 0x08040000, 0x08060000,
  0x08080000, 0x080A0000, 0x080C0000, 0x080E0000
};

/* Private functions ---------------------------------------------------------*/

/**
  * Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  extern uint8_t _mrb_bin_base __asm__ ("_mrb_bin_base"); /* Defined by the linker.  */
  extern uint8_t _mrb_bin_limit __asm__ ("_mrb_bin_limit");
  uint8_t* mrb_bin_base = &_mrb_bin_base;
  uint8_t* mrb_bin_limit = &_mrb_bin_limit;
  uint32_t mrb_bin_size = mrb_bin_limit - mrb_bin_base;

  initialize_basic_periph();

  uint8_t monitor_mode = GPIO_ReadInputDataBit(USERBUTTON);

  /* GPIOD Periph clock enable */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

  /* Configure PD12, PD13, PD14 and PD15 in output pushpull mode */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13| GPIO_Pin_14| GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

  Delay(0xFFFFFF); // wait for initialize usb_vcp

  if (monitor_mode == Bit_SET) {
    puts("monitor mode");
    GPIO_SetBits(GPIOD, GPIO_Pin_13|GPIO_Pin_15);
    while (1) {
      int cmd = getchar();
      if (cmd == 0xF1) { // Write mrb byte code to Flash ROM
        puts("BEGIN:WBF");
        GPIO_SetBits(GPIOD, GPIO_Pin_12);
        uint32_t size = read_uint32();
        if (size == 0) {
          puts("ERR:WRONG_SIZE");
          continue;
        } else if (size > mrb_bin_size) {
          puts("ERR:SIZE_TOO_BIG");
          continue;
        }
        GPIO_SetBits(GPIOD, GPIO_Pin_14);

        FLASH_Status flstat;
        FLASH_Unlock();
        uint16_t sc = addr2sector(mrb_bin_base);
        if (sc == FLASH_Sector_Unknown) {
          puts("ERR:WRONG_SECTOR");
          continue;
        }
        while (sc <= FLASH_Sector_11) {
          FLASH_EraseSector(sc, VoltageRange_3);
          sc += FLASH_Sector_1;
        }
        uint8_t buf[BIN_BUFFER_SIZE];
        uint32_t total = 0;
        uint8_t* addr = mrb_bin_base;
        for (uint32_t i = 0; i < 0xFFFF && total < size; i++) {
          uint32_t remain = size - total;
          printf("OK:SIZE=%u,BUF=%u\n", (unsigned int)remain, BIN_BUFFER_SIZE);
          uint32_t len = read_bytes(buf, remain > BIN_BUFFER_SIZE ? BIN_BUFFER_SIZE : remain);
          for (uint32_t j = 0; j < len; j++) {
            flstat = FLASH_ProgramByte((uint32_t)addr++, buf[j]);
            if (flstat != FLASH_COMPLETE) {
              puts("ERR:FLASH_WRITE");
              goto EXIT;
            }
          }
          total += len;
        }
        if (total == size) {
          puts("END:WBF");
        } else {
          puts("ERR:WBF");
          GPIO_ResetBits(GPIOD, GPIO_Pin_12|GPIO_Pin_14);
        }
        EXIT:
        FLASH_Lock();
      }
    }
  } else {
    puts("execute mode");
    GPIO_SetBits(GPIOD, GPIO_Pin_12);
    while (1) {
      mrb_state* mrb = mrb_open();
      init_mrduino(mrb);
      mrb_irep* irep = mrb_read_irep(mrb, mrb_bin_base);
      if (irep == NULL) {
        while (1) {
          GPIO_SetBits(GPIOD, GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);
          Delay(0x0FFFFF);
          GPIO_ResetBits(GPIOD, GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);
          Delay(0x0FFFFF);
        }
      }
      GPIO_SetBits(GPIOD, GPIO_Pin_13);
      mrb_run(mrb, mrb_proc_new(mrb, irep), mrb_top_self(mrb));
//      rbmain(mrb);
      GPIO_SetBits(GPIOD, GPIO_Pin_14);
      mrb_close(mrb);
//      Delay(0xFFFFFF);          
//      Delay(0xFFFFFF);  
      GPIO_SetBits(GPIOD, GPIO_Pin_15);
      Delay(0xFFFFFF);
      GPIO_ResetBits(GPIOD, GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);
    }
  }
}

/*
void transfer_ruby_binary(uint32_t length) {
  FLASH_Unlock();
  FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGAERR |FLASH_FLAG_WRPERR);

  for (int i = 0; i < length; i++) {
    uint8_t data = read_byte();
  }
}
*/

/**
  * @brief  Delay Function.
  * @param  nCount:specifies the Delay time length.
  * @retval None
  */
void Delay(__IO uint32_t nCount)
{
  while(nCount--)
  {
  }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */ 

/**
  * @}
  */ 

void
initialize_basic_periph (void)
{
#ifndef USE_UART_STDIO
  USBD_Init(&USB_OTG_dev,     
            USB_OTG_FS_CORE_ID, 
            &USR_desc, 
            &USBD_CDC_cb, 
            &USR_cb);
#endif

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

  //USERBUTTON(GPIOAのPIN0の設定)
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; /* USERBUTTON */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

#ifdef USE_UART_STDIO
  /* STDOUT & STDERR */

  //GPIOAとUSART2にクロック供給
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

  //GPIOAのPIN2を出力に設定
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  //GPIOAのPIN2をオルタネィテブファンクションのUSART2に割り当て
  GPIO_PinAFConfig(GPIOA , GPIO_PinSource2 , GPIO_AF_USART2);

  //GPIOAのPIN3を出力に設定
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  //GPIOAのPIN3をオルタネィテブファンクションのUSART2に割り当て
  GPIO_PinAFConfig(GPIOA , GPIO_PinSource3 , GPIO_AF_USART2);

  //USART初期化用構造体を作る
  USART_InitTypeDef USART_InitStructure;

  //USART2を9600bps,8bit,ストップビット1,パリティなし,フロー制御なし,送受信有効に設定
  USART_InitStructure.USART_BaudRate = 9600*3;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART2, &USART_InitStructure);

  //USART2を有効化
  USART_Cmd(USART2, ENABLE);
#endif

  SYSCALL_Init_STDOUT_Handler(&write_handler);
  SYSCALL_Init_STDERR_Handler(&write_handler);
  SYSCALL_Init_STDIN_Handler(&read_handler);

}

/*
uint8_t read_byte() {
}
*/

uint16_t
addr2sector(uint8_t* addr) {
  for (uint8_t i = 0; i < FLASH_SECTOR_SIZE; i++) {
    if (addr == (uint8_t*)flash_sector_base_addrs[i]) {
      return FLASH_Sector_1 * i;
    }  
  }
  return FLASH_Sector_Unknown;
}

#define MAX_READ_RETRY 0xFFFFFF
#define UINT32_BYTES 4

uint32_t
read_uint32() {
  uint32_t result = 0;
  uint8_t data[UINT32_BYTES];
  uint32_t bytes = read_bytes(data, UINT32_BYTES);
  if (bytes == UINT32_BYTES) {
    for (uint32_t i = 0; i < UINT32_BYTES; i++) {
      result = (result << 8) | data[i];    
    }
  }
  return result;
}

uint32_t
read_bytes(uint8_t *buf, uint32_t len) {
  uint32_t c = 0;
  for (uint32_t i = 0; i < MAX_READ_RETRY && c < len; i++) {
    int data = getchar();
    if (data < 0) continue;
    buf[c++] = (uint8_t)data & 0x0FF;
    i = 0;
  }
  return c; 
}

int
read_handler() {
#ifdef USE_UART_STDIO
  while (!USART_GetFlagStatus(USART2, USART_FLAG_RXNE));
  return (int)(USART_ReceiveData(USART2) & 0x0FF);  
#else
  uint8_t buf;
  uint8_t len = VCP_get_char(&buf);
  return len > 0 ? (int)buf : -1;
#endif
}

int
write_handler(char c) {
#ifdef USE_UART_STDIO
  while (!(USART2->SR & USART_FLAG_TXE)); // check that USART is ready
  USART_SendData(USART2, c);
  return 1;
#else
  VCP_put_char(c);
  return 1;  
#endif
}


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
