void init_mrduino(mrb_state *mrb);
mrb_value mrduino_pin_mode(mrb_state *mrb, mrb_value obj);
mrb_value mrduino_digital_read(mrb_state *mrb, mrb_value obj);
mrb_value mrduino_digital_write(mrb_state *mrb, mrb_value obj);
mrb_value mrduino_delay(mrb_state *mrb, mrb_value obj);
