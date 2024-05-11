#include "dr_button_reg.h"

#define EVENT_CB(ev)          \
	if ((handle->event) & ev)   \
	handle->cb(handle, handle->event)

// button handle list head.
static struct Button *head_handle = NULL;

/**
 * @brief  Initializes the button struct handle.
 * @retval None
 */
void button_init(void)
{
	head_handle = NULL;
}

/**
 * @brief  Button driver core function, driver state machine.
 * @param  handle: the button handle strcut.
 * @param  pin_level: the button status 0:unpress 1:pressed.
 * @param  cb: the button press call function.
 * @param  event: the button event trigger mask.
 * @retval None
 */
void button_attach(struct Button *handle, uint8_t (*pin_level)(void), BtnCallback cb, uint8_t event)
{
	memset(handle, 0, sizeof(struct Button));
	handle->event = (uint8_t)BTN_UNPRESS;
	handle->hal_button_Level = pin_level;
	handle->button_level = handle->hal_button_Level(); //读取IO口的函数
	handle->cb = cb;

	/* start the button */
	button_start(handle);
}

/**
 * @brief  Inquire the button event happen.
 * @param  handle: the button handle strcut.
 * @retval button event.
 */
PressEvent get_button_event(struct Button *handle)
{
	return (PressEvent)(handle->event);
}

/**
 * @brief  Button driver core function, driver state machine.
 * @param  handle: the button handle strcut.
 * @retval None
 */
void button_handler(struct Button *handle)
{
	uint8_t read_gpio_level = handle->hal_button_Level();

	// ticks counter working..
	if ((handle->state) > 0)
		handle->ticks++;

	/*------------button debounce handle---------------*/
	if (read_gpio_level != handle->button_level)
	{ // not equal to prev one
		// continue read 3 times same new level change
		if (++(handle->debounce_cnt) >= DEBOUNCE_TICKS)
		{
			handle->button_level = read_gpio_level;
			handle->debounce_cnt = 0;
		}
	}
	else
	{ // leved not change ,counter reset.
		handle->debounce_cnt = 0;
	}

	handle->event = read_gpio_level;
	/*-----------------State machine-------------------*/
	switch (handle->state)
	{
	case 0:
		if (handle->button_level == BTN_PRESSED)
		{ // start press down
			handle->event |= (uint8_t)PRESS_DOWN;
			EVENT_CB(PRESS_DOWN);
			handle->ticks = 0;
			handle->repeat = 1;
			handle->state = 1;
		}
		else
		{
			handle->event |= (uint8_t)BTN_UNPRESS;
		}
		break;

	case 1:
		if (handle->button_level != BTN_PRESSED)
		{ // released press up
			handle->event |= (uint8_t)PRESS_UP;
			EVENT_CB(PRESS_UP);
			handle->ticks = 0;
			handle->state = 2;
		}
		else if (handle->ticks > LONG_TICKS)
		{
			handle->event |= (uint8_t)LONG_RRESS_START;
			EVENT_CB(LONG_RRESS_START);
			handle->state = 5;
		}
		break;

	case 2:
		if (handle->button_level == BTN_PRESSED)
		{ // press down again
			handle->event |= (uint8_t)PRESS_DOWN;
			EVENT_CB(PRESS_DOWN);
			handle->repeat++;
			if (handle->repeat == 2)
			{
				EVENT_CB(DOUBLE_CLICK); // repeat hit
			}
			EVENT_CB(PRESS_REPEAT); // repeat hit
			handle->ticks = 0;
			handle->state = 3;
		}
		else if (handle->ticks > SHORT_TICKS)
		{ // released timeout
			if (handle->repeat == 1)
			{
				handle->event |= (uint8_t)SINGLE_CLICK;
				EVENT_CB(SINGLE_CLICK);
			}
			else if (handle->repeat == 2)
			{
				handle->event |= (uint8_t)DOUBLE_CLICK;
				EVENT_CB(DOUBLE_CLICK); //++
			}
			handle->state = 0;
		}
		break;

	case 3:
		if (handle->button_level != BTN_PRESSED)
		{ // released press up
			handle->event |= (uint8_t)PRESS_UP;
			EVENT_CB(PRESS_UP);
			if (handle->ticks < SHORT_TICKS)
			{
				handle->ticks = 0;
				handle->state = 2; // repeat press
			}
			else
			{
				handle->state = 0;
			}
		}
		break;

	case 5:
		if (handle->button_level == BTN_PRESSED)
		{
			// continue hold trigger
			handle->event |= (uint8_t)LONG_PRESS_HOLD;
			EVENT_CB(LONG_PRESS_HOLD);
		}
		else
		{ // releasd
			handle->event |= (uint8_t)PRESS_UP;
			EVENT_CB(PRESS_UP);
			handle->state = 0; // reset
		}
		break;
	}
}

/**
 * @brief  Start the button work, add the handle into work list.
 * @param  handle: target handle strcut.
 * @retval 0: succeed. -1: already exist.
 */
int button_start(struct Button *handle)
{
	struct Button *target = head_handle;
	while (target)
	{
		if (target == handle)
			return -1; // already exist.
		target = target->next;
	}
	handle->next = head_handle;
	head_handle = handle;
	return 0;
}

/**
 * @brief  Stop the button work, remove the handle off work list.
 * @param  handle: target handle strcut.
 * @retval None
 */
void button_stop(struct Button *handle)
{
	struct Button **curr;
	for (curr = &head_handle; *curr;)
	{
		struct Button *entry = *curr;
		if (entry == handle)
		{
			*curr = entry->next;
			//free(entry);
		}
		else
			curr = &entry->next;
	}
}

/**
 * @brief  background ticks, timer repeat invoking interval 5ms.
 * @param  None.
 * @retval None
 */
void button_ticks()
{
	struct Button *target;
	for (target = head_handle; target; target = target->next)
	{
		button_handler(target);
	}
}
