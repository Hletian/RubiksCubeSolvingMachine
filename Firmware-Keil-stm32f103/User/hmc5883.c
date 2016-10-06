#include "hmc5883.h"
#include <stdio.h>
#include <math.h>  
#include <stdlib.h>
#include "delay.h"


//HMC5983��IICд��ַ
#define   SlaveAddress    0x3c           
 
#define   IIC_SDA_1     GPIOB->BSRR = GPIO_Pin_14     
#define   IIC_SDA_0     GPIOB->BRR  = GPIO_Pin_14    
#define   IIC_SCL_1     GPIOB->BSRR = GPIO_Pin_13     
#define   IIC_SCL_0     GPIOB->BRR  = GPIO_Pin_13    
#define   READ_SDA	    GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14)   //��ȡSDA״̬
  

 
int Xmax,Xmin,Ymax,Ymin,Zmax,Zmin;     //X��Y ��Z ����Сֵ�����ֵ
int magOffsetX,magOffsetY,magOffsetZ;  //X��Y ��Z  ��ƫ����
u8 BUF[6]={0,0,0,0,0,0};               //���ڴ�Ŷ�ȡ��X��Y ��Z ֵ
float angle;                          //�Ƕ�ֵ
int calThreshold=1;                    //ƫ�����Ƚ�ֵ

int x=0,y=0,z=0;



//����IIC������Ϊ��� 
static void SDA_OUT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}      

//����IIC������Ϊ����
static void SDA_IN(void)       
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}   
  

//�����ƴ��������
static void HMC5883_GPIO_Config(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB ,ENABLE);	   

	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_13 | GPIO_Pin_14;			
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	IIC_SDA_1;	  	  
	IIC_SCL_1;
}



void IIC_Start(void)
{
	SDA_OUT();     //sda�����
	IIC_SDA_1;	  	  
	IIC_SCL_1;
	delay_us(5);
 	IIC_SDA_0;//START:when CLK is high,DATA change form high to low 
	delay_us(5);
	IIC_SCL_0;//ǯסI2C���ߣ�׼�����ͻ�������� 
}



void IIC_Stop(void)
{
	SDA_OUT();
	IIC_SCL_0;
	IIC_SDA_0;
	delay_us(5);
	IIC_SCL_1;
	IIC_SDA_1;
	delay_us(5);
}


u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime = 0;
	SDA_IN();
	IIC_SDA_1;
	delay_us(2);
	IIC_SCL_1;
	delay_us(2);
	while(READ_SDA)
	{
		ucErrTime++;
		if (ucErrTime > 250)
		{
			IIC_Stop();
			return 1;	  //Ӧ��ʧ��
		}
	}
	IIC_SCL_0;
	return 0;	//Ӧ��ɹ�
}



void IIC_Ack(void)                   
{
	IIC_SCL_0;
	SDA_OUT();
	IIC_SDA_0;
	delay_us(3);
	IIC_SCL_1;
	delay_us(3);
	IIC_SCL_0;
}

void IIC_NAck(void)                  
{
	IIC_SCL_0;
	SDA_OUT();
	IIC_SDA_1;
	delay_us(3);
	IIC_SCL_1;
	delay_us(3);
	IIC_SCL_0;
}

void IIC_Send_Byte(u8 txd)          //IIC����8λ����
{
	u8 t;

	SDA_OUT();      //SDA����Ϊ���
	IIC_SCL_0;    //SCLΪ��
	for (t = 0; t < 8; t++)
	{
// 		IIC_SDA = (txd&0x80) >> 7;
        if(txd&0x80)
        {
            IIC_SDA_1;
        }
        else
        {
            IIC_SDA_0;
        }
        
		txd <<= 1;
		delay_us(3);
		IIC_SCL_1;
		delay_us(3);
		IIC_SCL_0;
	}
}

//��һ���ֽڣ�ack = 1ʱ�� ����ACK�� ack = 0������nACK
u8 IIC_Read_Byte(u8 ack)          //IIC ����8���ֽ�
{
	u8 i, receive = 0;

	SDA_IN();
	for (i = 0; i < 8; i++)
	{
		IIC_SCL_0;
		delay_us(4);
		IIC_SCL_1;
		receive <<= 1;
		if(READ_SDA) receive++;
		delay_us(3);
	}
	if (!ack)	IIC_NAck();
	else IIC_Ack();
	return receive;
}
 

void Write_HMC5983(u8 add, u8 da)      //HMC5983д���� 
{
    IIC_Start();                  //��ʼ�ź�
    IIC_Send_Byte(SlaveAddress);   //�����豸��ַ+д�ź�
		IIC_Wait_Ack();

    IIC_Send_Byte(add);    //�ڲ��Ĵ�����ַ����ο�����pdf 
		IIC_Wait_Ack();

    IIC_Send_Byte(da);       //�ڲ��Ĵ������ݣ���ο�����pdf
		IIC_Wait_Ack();

    IIC_Stop();                   //����ֹͣ�ź�
}


u8 Read_HMC5983(u8 REG_Address)         //HMC5983 ������
{   
		u8 REG_data;
    IIC_Start();                          //��ʼ�ź�
    IIC_Send_Byte(SlaveAddress);          //�����豸��ַ+д�ź�
		IIC_Wait_Ack();

    IIC_Send_Byte(REG_Address);           //���ʹ洢��Ԫ��ַ����0��ʼ	
		IIC_Wait_Ack();

    IIC_Start();                          //��ʼ�ź�
    IIC_Send_Byte(SlaveAddress+1);        //�����豸��ַ+���ź�
		IIC_Wait_Ack();

    REG_data=IIC_Read_Byte(0);             //�����Ĵ�������
		IIC_Stop();                           //ֹͣ�ź�
    return REG_data; 
}


//******************************************************
//
//��������HMC5983�ڲ��Ƕ����ݣ���ַ��Χ0x3~0x5
//
//******************************************************
void Multiple_read_HMC5983(u8*BUF)         //HMC5983 ��ȡһ�����ݡ���ȡ��ֵΪ X��Y��Z ��ֵ��
{  
    u8 i;
    IIC_Start();                          //��ʼ�ź�
    IIC_Send_Byte(SlaveAddress);           //�����豸��ַ+д�ź�
		IIC_Wait_Ack();
    IIC_Send_Byte(0x03);                   //���ʹ洢��Ԫ��ַ����0x3��ʼ	
		IIC_Wait_Ack();
    IIC_Start();                          //��ʼ�ź�
    IIC_Send_Byte(SlaveAddress+1);         //�����豸��ַ+���ź�
		IIC_Wait_Ack();
	for (i=0; i<6; i++)                      //������ȡ6����ַ���ݣ��洢��BUF
    { 
        if (i == 5)
        {
           BUF[i] = IIC_Read_Byte(0);          //���һ��������Ҫ��NOACK
        }
        else
        {
           BUF[i] = IIC_Read_Byte(1);          //����ACK
        }
    }
    IIC_Stop();                          //ֹͣ�ź�
}




void CollectDataItem(int magX, int magY, int magZ)     //����У׼HMC5983ʱ�á� У׼�ķ�����ͨ���ѴŸ�Ӧ��ԭ��ˮƽת��2Ȧ��
{
	if(magX > Xmax)       //�õ� X�����ֵ ����Сֵ 
		Xmax = magX;
	if(magX < Xmin)
		Xmin = magX;
		
	if(magY > Ymax)       //�õ� Y�����ֵ ����Сֵ 
		Ymax = magY;
	if (magY < Ymin)
		Ymin = magY;
		
	if(magZ > Zmax)       //�õ� Z�����ֵ ����Сֵ 
		Zmax = magZ;
	if (magZ < Zmin)
		Zmin = magZ;
	
	if(abs(Xmax - Xmin) > calThreshold)            //�����ֵ-��Сֵ /2 �õ�ƫ����
			magOffsetX = ( Xmax + Xmin) / 2; 
	if(abs(Ymax - Ymin) > calThreshold)
			magOffsetY = ( Ymax + Ymin) / 2;
	if(abs(Zmax - Zmin) > calThreshold)
			magOffsetZ = ( Zmax + Zmin) / 2;          //ƫ�������÷���  �ڽǶ������м�ȥƫ������
}

  
//�����Ƴ�ʼ��
void HMC5983_Init(void)          //��ʼ��HMC5983
{
	HMC5883_GPIO_Config();
    
	Write_HMC5983(0x00, 0x78);	 //�������Ƶ��Ϊ75HZ
 
	Write_HMC5983(0x02, 0x00);		//��������ģʽ
 
}

 
 
//���ݴ���
void HMC5883_DataDeal(void)
{
    u8 status=0;
    
    status = Read_HMC5983(0x09);  //��ȡstatus�Ĵ���
 
    if(status & 0x01 == 0x01)   //���ݸ������
    {
        Multiple_read_HMC5983(BUF);
        
        x = BUF[0] << 8 | BUF[1]; //Combine MSB and LSB of X Data output register
        z = BUF[2] << 8 | BUF[3]; //Combine MSB and LSB of Z Data output register
        y = BUF[4] << 8 | BUF[5]; //Combine MSB and LSB of Y Data output register
  
        
        if(x>32768)                 //�ѵõ���XYZֵ���д���
            x = -(0xFFFF - x + 1);
        if(z>32768)
            z = -(0xFFFF - z + 1);
        if(y>32768)
            y = -(0xFFFF - y + 1);
 
//         CollectDataItem(x,y,z);
      	x = x + 84;   //У��
  		y = y + 169;		  

        
        if(x>0&&y>0)
        {
             angle= atan2((double)(y),(double)(x)) * (180 / 3.14159265);
        }
        else if(x>0&&y<0)
        {
             angle=360-atan2((double)(-y),(double)(x)) * (180 / 3.14159265);		
        }
        else if(x<0&&y<0)
        {
             angle=180+atan2((double)(-y),(double)(-x)) * (180 / 3.14159265);		
        }
        else if(x<0&&y>0)
        {
             angle=180-atan2((double)(y),(double)(-x)) * (180 / 3.14159265);		
        }
        else if(x==0&&y<0)
        {
             angle=270;		
        }
        else if(x==0&&y>0)
        {
             angle=90;		
        }		 
        else if(x>0&&y==0)
        {
             angle=0;		
        }
        else if(x<0&&y==0)
        {
             angle=180;		
        }
  
    }
}

