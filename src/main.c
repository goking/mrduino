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

/* Private typedef -----------------------------------------------------------*/
GPIO_InitTypeDef  GPIO_InitStructure;

/* Private define ------------------------------------------------------------*/

#define USERBUTTON GPIOA, GPIO_Pin_0

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

int write_handler(char c);
void Delay(__IO uint32_t nCount);
extern void rbmain(mrb_state *mrb);
void initialize_basic_periph();

/* Private functions ---------------------------------------------------------*/

/**
  * Main program
  * @param  None
  * @retval None
  */
int main(void)
{
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

  if (monitor_mode == Bit_SET) {
    puts("monitor mode¥n");
    while (!USART_GetFlagStatus(USART1, USART_FLAG_RXNE));
    char data = USART_ReceiveData(USART2);
    write_handler(data);
  } else {
    puts("execute mode¥n");
    while (1) {
      GPIO_SetBits(GPIOD, GPIO_Pin_14);
      Delay(0xFFFFFF);          
      GPIO_SetBits(GPIOD, GPIO_Pin_15);
      Delay(0xFFFFFF);
      GPIO_ResetBits(GPIOD, GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);
      Delay(0xFFFFFF);
    }
  }
    
/* Insert delay */
/*  
    mrb_state* mrb = mrb_open();
    rbmain(mrb);
    mrb_close(mrb);
    printf("count: %3d\r\n", ++i);
*/  

}

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
  /* STDOUT & STDERR */

  //GPIOAとUSART2にクロック供給
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
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

  SYSCALL_Init_STDOUT_Handler(&write_handler);
  SYSCALL_Init_STDERR_Handler(&write_handler);
  //SYSCALL_Init_STDIN_Handler(SYSREADHANDLER handler);

  /* USERBUTTON */

  //USERBUTTON(GPIOAのPIN0の設定)
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

}



int
write_handler(char c) {
  while (!(USART2->SR & USART_FLAG_TXE)); // check that USART is ready
  USART_SendData(USART2, c);
  return 1;
}


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
