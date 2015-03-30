#ifndef GPIO_H
#define GPIO_H

/* export a gpio as an output */
void gpio_setup(char *pin) ;

/* change the value of an already setup gpio */
void gpio_set_value(char *pin, char *value) ;

#endif
