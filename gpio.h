int gpio_open(int gpio, int direction);
void gpio_close(int gpio);
int gpio_set(int gpio, int b);
int gpio_read(int gpio);
int gpio_setedge(int gpio, char *edge);
int gpio_getfd(int gpio);

