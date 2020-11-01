/* ./01_get_input_info /dev/input/event0 */

/*
I: Bus=0003 Vendor=046d Product=4023 Version=0111
N: Name="Logitech Wireless Keyboard PID:4023"
P: Phys=usb-0000:01:00.0-1.3/input1:1
S: Sysfs=/devices/platform/scb/fd500000.pcie/pci0000:00/0000:00:00.0/0000:01:00.0/usb1/1-1/1-1.3/1-1.3:1.1/0003:046D:C534.0002/0003:046D:4023.0003/input/input17
U: Uniq=4023-00-00-00-00
H: Handlers=sysrq kbd leds event0 
B: PROP=0
B: EV=12001f
B: KEY=3f 301ff 0 0 0 0 483ffff 17aff32d bfd44446 0 0 1 130ff3 8b17c007 ffff7bfa d941dfff ffbeffdf ffefffff ffffffff fffffffe
B: REL=1040
B: ABS=1 0
B: MSC=10
B: LED=1f
*/
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <poll.h>

int main(int argc, char const *argv[])
{
    int fd;
    int err;
    int len;
    int ret;
    int i;
    unsigned char byte;
    int bit;
    struct input_id id;
    unsigned int evbit[2];
    struct input_event event;
    struct pollfd fds[1];
    nfds_t nfds = 1;

    char *ev_names[] = {
        "EV_SYN ",
        "EV_KEY ",
        "EV_REL ",
        "EV_ABS ",
        "EV_MSC ",
        "EV_SW	",
        "NULL ",
        "NULL ",
        "NULL ",
        "NULL ",
        "NULL ",
        "NULL ",
        "NULL ",
        "NULL ",
        "NULL ",
        "NULL ",
        "NULL ",
        "EV_LED ",
        "EV_SND ",
        "NULL ",
        "EV_REP ",
        "EV_FF	",
        "EV_PWR ",
    };

    if (argc != 2)
    {
        printf("Usage: %s <dev>\n", argv[0]);
        return -1;
    }

    fd = open(argv[1], O_RDWR | O_NONBLOCK);
    if (fd < 0)
    {
        printf("open %s err\n", argv[1]);
        return -1;
    }

    err = ioctl(fd, EVIOCGID, &id);
    if (err == 0)
    {
        printf("bustype = 0x%x\n", id.bustype);
        printf("vendor  = 0x%x\n", id.vendor);
        printf("product = 0x%x\n", id.product);
        printf("version = 0x%x\n", id.version);
    }

    len = ioctl(fd, EVIOCGBIT(0, sizeof(evbit)), &evbit);
    if (len > 0 && len <= sizeof(evbit))
    {
        printf("support ev type: ");
        for (i = 0; i < len; i++)
        {
            byte = ((unsigned char *)evbit)[i];
            for (bit = 0; bit < 8; bit++)
            {
                if (byte & (1 << bit))
                {
                    printf("%s ", ev_names[i * 8 + bit]);
                }
            }
        }
        printf("\n");
    }

    while (1)
    {
        fds[0].fd = fd;
        fds[0].events = POLLIN;
        fds[0].revents = 0;
        ret = poll(fds, nfds, 5000);
        if (ret > 0)
        {
            if (fds[0].revents == POLLIN)
            {
                while (read(fd, &event, sizeof(event)) == sizeof(event))
                {
                    printf("get event: type = 0x%x, code = 0x%x, value = 0x%x\n", event.type, event.code, event.value);
                }
            }
        }
        else if (ret == 0)
        {
            printf("time out \n");
        }
        else
        {
            printf("poll err\n");
        }
    }

    return 0;
}
