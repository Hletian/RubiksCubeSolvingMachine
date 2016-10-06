#include "delay.h"

u8 fac_us=0;//us��ʱ������
 
 
void Delay(u32 time)
{
    while(time--);
  
}


//�����׼��ʱ1ms
void Delay1ms(void)
{
    u32 a=8000;
    while(a--);
    
}

void delay_ms(u32 n)
{
    u32 i=0;
    
    for(i=0;i<n;i++)    
    { 
        Delay1ms();
    }
}


 
//IICר����ʱ����
void delay_init(void)	 
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);	//ѡ���ⲿʱ��  HCLK/8
	fac_us = SystemCoreClock/8000000;	//Ϊϵͳʱ�ӵ�1/8  
}	


//�ǳ�׼ȷ��us����ʱ
void delay_us(u32 nus)
{		
	u32 temp;	    	 
	SysTick->LOAD=nus*fac_us; //ʱ�����	  		 
	SysTick->VAL=0x00;        //��ռ�����
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;          //��ʼ����	 
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));//�ȴ�ʱ�䵽��   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;       //�رռ�����
	SysTick->VAL =0X00;       //��ռ�����	 

}
 


