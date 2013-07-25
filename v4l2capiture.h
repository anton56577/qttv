#ifndef V4L2CAPITURE_H
#define V4L2CAPITURE_H

#include <sys/types.h>

class v4l2capiture
{
    struct buffer {
            void *                  start;
            size_t                  length;
    };

protected:
    void errno_exit(const char *s);
    int xioctl(int fd, int request, void *arg);
    void process_image(const void *p, uint ilen);
    void open_device(void);
    void close_device(void);
    void init_device(void);
    void init_mmap(void);
    void uninit_device(void);
    void start_capturing(void);
    void stop_capturing(void);
    int read_frame(void);

public:
    int iTypeDev   ;//0 - S-Video / 1 - Composite / 2 - Television / 3 - Radio
    char             dev_name[1024];
    int              fd;
    struct buffer *  buffers;
    unsigned int     n_buffers;

    unsigned char *bufTV;
    uint sizebuf;
    uint buffbutesperline;
    int pwidth;
    int pheigth;

    v4l2capiture();
    ~v4l2capiture();
    void start(const char devname[], int iType);
    void mainloop(void);
    void end();

    int selTvChannel(int iFreqHz, int iVideoMode, int iAudioType);
};

#endif // V4L2CAPITURE_H
