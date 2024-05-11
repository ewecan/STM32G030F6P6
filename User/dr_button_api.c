
/**
 * @file dr_button_api.c
 * @author  Vecang
 * @brief 按钮驱动
 * 识别单击、双击、长按
 * 0、初始化：DR_Key_Init() ,打开定时器
 * 1、在定时器回调里5ms 调用Key_Tick()函数
 * 2、读取按键状态：Get_Key() => 0：无按键 1：单击 2：双击 3：长按
 *
 * @version 0.1
 * @date 2023-06-16
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "main.h" //use KEY_GPIO_Port, KEY_Pin

#include "dr_button_api.h"

static ClickEvent key_num = KEY_None;
static Button key;

/*********************************************************/
// ++ 需要另外定义引脚: KEY##x##_GPIO_Port, KEY##x##_Pin ;
// 例如: KEY1_GPIO_Port, KEY1_Pin
#define ReadKey(x) \
	static uint8_t ReadKey##x(void) { return (HAL_GPIO_ReadPin(KEY##x##_GPIO_Port, KEY##x##_Pin) ? 0 : 1); }


/* 注册按键按下 KEY1 */
ReadKey(1);
//ReadKey(2);
//ReadKey(3);
	
	
	
static void Key_PRESS_DOWN_Handler(void *btn, PressEvent event)
{
	if (event & SINGLE_CLICK)
		key_num = KEY_SINGLE_CLICK;
	else if (event & DOUBLE_CLICK)
		key_num = KEY_DOUBLE_CLICK;
	else if (event & LONG_RRESS_START)
		key_num = KEY_LONG_RRESS_START;
}

void DR_Key_Init(void)
{
	/* 注册按键 */
	button_init();
	// ++ ReadKey(1) 对应 ReadKey1,以此类推
	button_attach(&key, ReadKey1, Key_PRESS_DOWN_Handler, (PressEvent)(SINGLE_CLICK | DOUBLE_CLICK | LONG_RRESS_START));
}

/**
 * @brief 获取按键
 * @param  *ucQueueMsgValue   获取的按键键值
 * @param  xMaxBlockTime    没有按下按键的阻塞时间(操作系统)
 * @return BaseType_t 当前是否有按键按下
 */
int Get_Key(void)
{
	int key = (int)key_num;
	key_num = KEY_None;
	return key;
}

void Key_Tick()
{
	button_ticks();
}
