#include "stm32f4_discovery.h"
#include "stm32f4xx_conf.h"
#include "mruby.h"
#include "mrduino.h"

#define NUM2PIN(NUM) (GPIO_Pin_7 << (NUM))

void
init_mrduino(mrb_state *mrb)
{
	struct RClass *krn;

	krn = mrb->kernel_module;

	mrb_define_method(mrb, krn, "pin_mode", mrduino_pin_mode, ARGS_ANY());
	mrb_define_method(mrb, krn, "digital_read", mrduino_digital_read, ARGS_ANY());
	mrb_define_method(mrb, krn, "digital_write", mrduino_digital_write, ARGS_ANY());
	mrb_define_method(mrb, krn, "delay", mrduino_delay, ARGS_ANY());

	// digital I/O PE7 - PE15 
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
}

mrb_value
mrduino_pin_mode(mrb_state *mrb, mrb_value obj)
{
	mrb_int num, mode;
  	mrb_get_args(mrb, "ii", &num, &mode);

  	if (!IS_GET_GPIO_PIN(NUM2PIN(num))) {
  		return mrb_false_value();
  	}
	GPIO_InitTypeDef gpio;
	gpio.GPIO_Pin = NUM2PIN(num);
	gpio.GPIO_Mode = (mode & 0x001) ? GPIO_Mode_OUT : GPIO_Mode_IN;
	gpio.GPIO_OType = GPIO_OType_PP;
	gpio.GPIO_PuPd = (mode & 0x010) ? GPIO_PuPd_UP : GPIO_PuPd_NOPULL;
	gpio.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOE, &gpio);
	return mrb_true_value();
}

mrb_value
mrduino_digital_write(mrb_state *mrb, mrb_value obj)
{
	mrb_int num, val;
  	mrb_get_args(mrb, "ii", &num, &val);

  	uint32_t pin = NUM2PIN(num);
  	if (!IS_GET_GPIO_PIN(pin)) {
  		return mrb_false_value();
  	}
  	if (val) {
  		GPIO_SetBits(GPIOE, pin);
  	} else {
  		GPIO_ResetBits(GPIOE, pin);  		
  	}
	return mrb_true_value();
}

mrb_value
mrduino_digital_read(mrb_state *mrb, mrb_value obj)
{
	mrb_int num;
  	mrb_get_args(mrb, "i", &num);

  	uint32_t pin = NUM2PIN(num);
  	if (!IS_GET_GPIO_PIN(pin)) {
  		return mrb_nil_value();
  	}
  	uint8_t val = GPIO_ReadInputDataBit(GPIOE, pin);
  	return mrb_fixnum_value(val);
}

mrb_value
mrduino_delay(mrb_state *mrb, mrb_value obj)
{
	mrb_int val;
  	mrb_get_args(mrb, "i", &val);
  	uint32_t tick;

  	while (val--) {
	  	tick = 0xFFFFF;
		while(tick--){
		}
	}
	return mrb_nil_value();
}
