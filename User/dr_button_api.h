#ifndef _dr_key_H
#define _dr_key_H
 
#include "dr_button_reg.h"

typedef enum
{
    KEY_None = 0,
    KEY_SINGLE_CLICK,
    KEY_DOUBLE_CLICK,
    KEY_LONG_RRESS_START
} ClickEvent;
 
void DR_Key_Init(void); 

int Get_Key(void);

void Key_Tick(void);

#endif
