#include "stm32f4xx.h"
#include "stm32f4_discovery.h"

uint8_t change_frequency = 0;
uint8_t character_counter = 0;
char receiving_buffer[6];
double ADC_result;
double current_frequency = 0.0;
double p = 0.0;

void send(char a)
{
	USART_ClearITPendingBit(USART3, USART_IT_RXNE);
	while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
	USART_SendData(USART3, a);
	while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
}

void USART3_IRQHandler(void)
{
	if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
		switch (character_counter) {
			case 0:
				receiving_buffer[0] = USART3->DR;
				break;
			case 1:
				receiving_buffer[1] = USART3->DR;
				break;
			case 2:
				receiving_buffer[2] = USART3->DR;
				break;
			case 3:
				receiving_buffer[3] = USART3->DR;
				break;
			case 4:
				receiving_buffer[4] = USART3->DR;
				break;
			case 5:
				receiving_buffer[5] = USART3->DR;
				break;
		}

		character_counter++;

		if (receiving_buffer[0] == 's' && receiving_buffer[4] == '.' && character_counter == 6) {
			double frequency = ((receiving_buffer[1] - '0') * 1000 + (receiving_buffer[2] - '0') * 100 + (receiving_buffer[3] - '0') * 10 + (receiving_buffer[5] - '0')) / 10.0;
			if (frequency >= 87.5 && frequency <= 108.0) {
				send('\n');
				current_frequency = frequency;
				change_frequency = 1;
			}
			else {
				send('\n');
				char* text = "Invalid frequency!";
				for (uint8_t i = 0; i < 18; i++) {
					send(text[i]);
				}
				send('\n');
			}
			character_counter = 0;
		}
		else if (receiving_buffer[0] == 's' && receiving_buffer[3] == '.' && character_counter == 5) {
			double frequency = ((receiving_buffer[1] - '0') * 100 + (receiving_buffer[2] - '0') * 10 + (receiving_buffer[4] - '0')) / 10.0;
			if (frequency >= 87.5 && frequency <= 108.0) {
				send('\n');
				current_frequency = frequency;
				change_frequency = 1;
			}
			else {
				send('\n');
				char* text = "Invalid frequency!";
				for (uint8_t i = 0; i < 18; i++) {
					send(text[i]);
				}
				send('\n');
			}
			character_counter = 0;
		}
		else if (receiving_buffer[0] == 'f') {
			send('\n');
			char* text = "Current frequency: ";
			for (uint8_t i = 0; i < 19; i++) {
				send(text[i]);
			}
			if (current_frequency >= 100.0) {
				send(((int)(current_frequency / 100.0) % 10) + '0');
				send(((int)(current_frequency / 10.0) % 10) + '0');
				send(((int)current_frequency % 10) + '0');
				send('.');
				send(((int)(current_frequency * 10.0) % 10) + '0');
			}
			else {
				send(((int)(current_frequency / 10.0) % 10) + '0');
				send(((int)current_frequency % 10) + '0');
				send('.');
				send(((int)(current_frequency * 10.0) % 10) + '0');
			}
			send('\n');
			character_counter = 0;
		}

		if (receiving_buffer[0] != 's' && receiving_buffer[0] != 'f' && character_counter > 0) {
			send('\n');
			char* text = "Invalid instruction!";
			for (uint8_t i = 0; i < 20; i++) {
				send(text[i]);
			}
			character_counter = 0;
			send('\n');
		}

		if (character_counter > 6) {
			character_counter = 0;
		}
    }
}

void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
		ADC_SoftwareStartConv(ADC1);
		while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
		ADC_result = ADC_GetConversionValue(ADC1)/190.5;
		ADC_result += 87.5;
		change_frequency = 1;
		if(ADC_result < 87.5) {
			ADC_result = 87.5;
			current_frequency = ADC_result;
			EXTI_ClearITPendingBit(EXTI_Line0);
		}
		else if(ADC_result > 108.0) {
			ADC_result = 108.0;
			current_frequency = ADC_result;
			EXTI_ClearITPendingBit(EXTI_Line0);
		}
		else {
			current_frequency = ADC_result;
			EXTI_ClearITPendingBit(EXTI_Line0);
		}
		TIM_Cmd(TIM2, DISABLE);
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}

void EXTI0_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line0) != RESET) {
    	TIM_Cmd(TIM2, ENABLE);
    }

}

int main(void) {
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA , ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitTypeDef GPIO_InitStructure2;
	GPIO_InitStructure2.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure2.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure2.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure2);

	GPIO_InitTypeDef GPIO_InitStructure3;
	GPIO_InitStructure3.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure3.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure3.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure3.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure3.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure3);

	GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_USART3);

	I2C_DeInit(I2C1);

	I2C_InitTypeDef I2C_InitStructure;
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = 0x020;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = 400000;
	I2C_Init(I2C1, &I2C_InitStructure);

	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART3, &USART_InitStructure);

	USART_Cmd(USART3, ENABLE);

	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure2;

	TIM_TimeBaseStructure2.TIM_Period = 2999;
	TIM_TimeBaseStructure2.TIM_Prescaler = 8399;
	TIM_TimeBaseStructure2.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure2.TIM_CounterMode =  TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure2);

	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);

	ADC_InitTypeDef ADC_InitStructure;
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 1;
	ADC_Init(ADC1, &ADC_InitStructure);

	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_84Cycles);

	NVIC_InitTypeDef NVIC_InitStructure;
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	NVIC_EnableIRQ(USART3_IRQn);

	NVIC_InitTypeDef NVIC_InitStructure2;
	NVIC_InitStructure2.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure2.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStructure2.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure2.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure2);

	NVIC_InitTypeDef NVIC_InitStructure3;
	NVIC_InitStructure3.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure3.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStructure3.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure3.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure3);

	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = EXTI_Line0;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);
	ADC_Cmd(ADC1, ENABLE);

	for (;;) {
		if (change_frequency == 1) {
			uint8_t text_buffer[5];
			uint16_t frequency = (current_frequency * 1000000 + 225000) / 8192;

			text_buffer[0] = (frequency >> 8) & 0x3f;
			text_buffer[1] = frequency & 0xff;
			text_buffer[2] = 0x10;
			text_buffer[3] = 0x10;
			text_buffer[4] = 0x00;

			I2C_AcknowledgeConfig(I2C1, ENABLE);
			I2C_GenerateSTART(I2C1, ENABLE);
			while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) {

			}

			I2C_Send7bitAddress(I2C1, 0xC0, I2C_Direction_Transmitter);
			while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {

			}

			for (uint8_t i = 0; i < 5; i++) {
				I2C_SendData(I2C1, text_buffer[i]);
				while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {

				}
			}

			I2C_AcknowledgeConfig(I2C1, DISABLE);
			I2C_GenerateSTOP(I2C1, ENABLE);

			change_frequency = 0;
		}
	}
}
