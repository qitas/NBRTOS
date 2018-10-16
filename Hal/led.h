#ifndef _LED_H_
#define _LED_H_







typedef struct
{

	_Bool Led1Sta;
	_Bool Led2Sta;
	_Bool Led3Sta;
	_Bool Led4Sta;

} LED_STATUS;

extern LED_STATUS ledStatus;

typedef enum
{

	LED_OFF = 0,
	LED_ON

} LED_ENUM;








void Led_Init(void);

void Led1_Set(LED_ENUM status);

void Led2_Set(LED_ENUM status);

void Led3_Set(LED_ENUM status);

void Led4_Set(LED_ENUM status);

void Led_test(int a);


#endif
