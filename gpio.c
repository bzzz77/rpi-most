#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>

#define BUFFER_MAX 50

static int fd[32] = {0};

int gpio_getfd(int gpio)
{
	if (gpio < 0 || gpio > 31)
		return -1;
	if (fd[gpio] == 0)
		return -1;
	return fd[gpio];
}

/*int main(int argc, char** argv)
{
    if (!bcm2835_init())
        return 1;

    openGPIO(4, 0);
    for (;;) {

        fdset[0].fd = fd[4];
        fdset[0].events = POLLPRI;
        fdset[0].revents = 0;

        int rc = poll(fdset, 1, 5000);
        if (rc < 0) {
            printf("\npoll() failed!\n");
            return -1;
        }
        if (rc == 0) {
            printf(".");
        }
        if (fdset[0].revents & POLLPRI) {
              lseek(fd[4], 0, SEEK_SET);
              int val=readGPIO(4);
            printf("\npoll() GPIO 4 interrupt occurred %d\n\r",val);

        }
        fflush(stdout);
    }
    return 0;
}*/

int gpio_open(int gpio, int direction)
{
    char buf[BUFFER_MAX];
    int len;

    if (gpio < 0 || gpio > 31)
	    return -1;
    if (direction < 0 || direction > 1)
	    return -2;

    if (fd[gpio] != 0) {
        close(fd[gpio]);
        fd[gpio] = open("/sys/class/gpio/unexport", O_WRONLY);
        len = snprintf(buf, BUFFER_MAX, "%d", gpio);
        write(fd[gpio], buf, len);
        close(fd[gpio]);
        fd[gpio] = 0;
    }

    fd[gpio] = open("/sys/class/gpio/export", O_WRONLY);
    len = snprintf(buf, BUFFER_MAX, "%d", gpio);
    write(fd[gpio], buf, len);
    close(fd[gpio]);

    len = snprintf(buf, BUFFER_MAX, "/sys/class/gpio/gpio%d/direction", gpio);
    fd[gpio] = open(buf, O_WRONLY);
    if (direction == 1) {
        write(fd[gpio], "out", 4);
        close(fd[gpio]);
        len = snprintf(buf, BUFFER_MAX, "/sys/class/gpio/gpio%d/value", gpio);
        fd[gpio] = open(buf, O_WRONLY);

    } else {
        write(fd[gpio], "in", 3);
        close(fd[gpio]);
        len = snprintf(buf, BUFFER_MAX, "/sys/class/gpio/gpio%d/value", gpio);
        fd[gpio] = open(buf, O_RDONLY);
    }
    return 0;
}

void gpio_close(int gpio)
{
	char buf[BUFFER_MAX];
	int len;
	if (fd[gpio] != 0) {
		close(fd[gpio]);
		fd[gpio] = open("/sys/class/gpio/unexport", O_WRONLY);
		len = snprintf(buf, BUFFER_MAX, "%d", gpio);
		write(fd[gpio], buf, len);
		close(fd[gpio]);
		fd[gpio] = 0;
	}
}

int gpio_set(int gpio, int b)
{
    if (b == 0) {
        write(fd[gpio], "0", 1);
    } else {
        write(fd[gpio], "1", 1);
    }

    lseek(fd[gpio], 0, SEEK_SET);
    return 0;
}

int gpio_read(int gpio)
{
    char value_str[3];
    int c = read(fd[gpio], value_str, 1);
    lseek(fd[gpio], 0, SEEK_SET);

    if (value_str[0] == '0') {
        return 0;
    } else {
        return 1;
    }

}

int gpio_setedge(int gpio, char *edge)
{
    char buf[BUFFER_MAX];
    int len = snprintf(buf, BUFFER_MAX, "/sys/class/gpio/gpio%d/edge", gpio);
    int fd = open(buf, O_WRONLY);
    write(fd, edge, strlen(edge) + 1);
    close(fd);

#if 1
    len = snprintf(buf, BUFFER_MAX, "/sys/class/gpio/gpio%d/active_low", gpio);
    fd = open(buf, O_WRONLY);
    sprintf(buf, "0");
    write(fd, buf, 1);
    close(fd);
#endif

    return 0;
}

