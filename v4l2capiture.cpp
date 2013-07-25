#include <stdio.h>
#include <assert.h>

#include <getopt.h>             /* getopt_long() */

#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <asm/types.h>          /* for videodev2.h */

#include <linux/videodev2.h>

#define CLEAR(x) memset (&(x), 0, sizeof (x))

#include "v4l2capiture.h"
//http://v4l2spec.bytesex.org/spec/a16706.htm

#include "glwidget.h"

v4l2capiture::v4l2capiture()
{
    fd              = -1;
    buffers         = NULL;
    n_buffers       = 0;
    pwidth = 720;
    pheigth = 576;
}

v4l2capiture::~v4l2capiture()
{
}

void  v4l2capiture::errno_exit(const char *s)
{
        fprintf (stderr, "%s error %d, %s\n", s, errno, strerror (errno));
        exit (EXIT_FAILURE);
}

int v4l2capiture::xioctl(int fd, int request, void *arg)
{
      int r;

      do {
          r = ioctl (fd, request, arg);
      } while (-1 == r && EINTR == errno);

      return r;
}

void v4l2capiture::process_image(const void *p, uint ilen)
{
/*    if (bufTV)
        delete []bufTV;
    bufTV = new uchar[ilen];
    memcpy(bufTV, p, ilen);*/
    bufTV = (uchar*)p;
    sizebuf = ilen;
}

int v4l2capiture::read_frame(void)
{
    if (iTypeDev != 3) {
        struct v4l2_buffer buf;

        CLEAR (buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        if (-1 == xioctl (fd, VIDIOC_DQBUF, &buf)) {
            switch (errno) {
             case EAGAIN:
                return 0;

             case EIO:

                /* Could ignore EIO, see spec. */
                /* fall through */

             default:

                errno_exit ("VIDIOC_DQBUF");

            }
        }

        assert (buf.index < n_buffers);

        process_image (buffers[buf.index].start, buf.length);

        if (-1 == xioctl (fd, VIDIOC_QBUF, &buf))
            errno_exit ("VIDIOC_QBUF");
    }
    return 1;
}

void v4l2capiture::mainloop(void)
{
    if (iTypeDev != 3) {

        unsigned int count;

        count = 1;

        while (count-- > 0) {
            fd_set fds;
            struct timeval tv;
            int r;

            FD_ZERO (&fds);
            FD_SET (fd, &fds);

            /* Timeout. */
            tv.tv_sec = 2;
            tv.tv_usec = 0;

            r = select (fd + 1, &fds, NULL, NULL, &tv);

            if (-1 == r) {
                if (EINTR == errno)
                    continue;

                errno_exit ("select");
            }

            if (0 == r) {
                fprintf (stderr, "select timeout\n");
                exit (EXIT_FAILURE);
            }

            if (read_frame ())
                break;

            /* EAGAIN - continue select loop. */
        }
    }
}

void v4l2capiture::stop_capturing(void)
{
    //mute
    v4l2_control ctrl;
    ctrl.id = V4L2_CID_AUDIO_MUTE;
    ctrl.value = 1;
    if (-1 == xioctl (fd, VIDIOC_S_CTRL, &ctrl))
        errno_exit ("VIDIOC_S_CTRL");

    if (iTypeDev != 3) {
        enum v4l2_buf_type type;
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == xioctl (fd, VIDIOC_STREAMOFF, &type))
            errno_exit ("VIDIOC_STREAMOFF");
    }
}

void v4l2capiture::start_capturing(void)
{
    if (iTypeDev != 3) {
        unsigned int i;
        enum v4l2_buf_type type;

        for (i = 0; i < n_buffers; ++i) {
            struct v4l2_buffer buf;
            CLEAR (buf);

            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;

            if (-1 == xioctl (fd, VIDIOC_QBUF, &buf))
                errno_exit ("VIDIOC_QBUF");
        }

        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if (-1 == xioctl (fd, VIDIOC_STREAMON, &type))
            errno_exit ("VIDIOC_STREAMON");
    }
}

void v4l2capiture::uninit_device(void)
{
    if (iTypeDev != 3) {
        unsigned int i;
        for (i = 0; i < n_buffers; ++i)
            if (-1 == munmap (buffers[i].start, buffers[i].length))
                errno_exit ("munmap");
        free (buffers);
    }
}

void v4l2capiture::init_mmap(void)
{
    struct v4l2_requestbuffers req;

    CLEAR (req);

    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl (fd, VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            fprintf (stderr, "%s does not support "
                     "memory mapping\n", dev_name);
            exit (EXIT_FAILURE);
        } else {
            errno_exit ("VIDIOC_REQBUFS");
        }
    }

    if (req.count < 2) {
        fprintf (stderr, "Insufficient buffer memory on %s\n",
                 dev_name);
        exit (EXIT_FAILURE);
    }

    buffers = (buffer*)calloc (req.count, sizeof (*buffers));

    if (!buffers) {
        fprintf (stderr, "Out of memory\n");
        exit (EXIT_FAILURE);
    }

    for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
        struct v4l2_buffer buf;
        CLEAR (buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n_buffers;

        if (-1 == xioctl (fd, VIDIOC_QUERYBUF, &buf))
            errno_exit ("VIDIOC_QUERYBUF");

        buffers[n_buffers].length = buf.length;
        buffers[n_buffers].start =
                mmap (NULL /* start anywhere */,
                      buf.length,
                      PROT_READ | PROT_WRITE /* required */,
                      MAP_SHARED /* recommended */,
                      fd,
                      buf.m.offset);

        if (MAP_FAILED == buffers[n_buffers].start)
            errno_exit ("mmap");
    }
}

void v4l2capiture::init_device(void)
{
    struct v4l2_capability cap;
    //struct v4l2_cropcap cropcap;
    // struct v4l2_crop crop;
    struct v4l2_format fmt;
    unsigned int min;

    if (-1 == xioctl (fd, VIDIOC_QUERYCAP, &cap)) {
        if (EINVAL == errno) {
            fprintf (stderr, "%s is no V4L2 device\n", dev_name);
            exit (EXIT_FAILURE);
        } else {
            errno_exit ("VIDIOC_QUERYCAP");
        }
    } else {
        qDebug ("%s / %s / Version: %u.%u.%u\n0x%x",
                cap.card,
                cap.driver,
                (cap.version >> 16) & 0xFF,
                (cap.version >> 8) & 0xFF,
                cap.version & 0xFF,
                cap.capabilities);
    }


    if (iTypeDev != 3) {
        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
            fprintf (stderr, "%s is no video capture device\n",
                     dev_name);
            exit (EXIT_FAILURE);
        }

        if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
            fprintf (stderr, "%s does not support streaming i/o\n",
                     dev_name);
            exit (EXIT_FAILURE);
        }

        CLEAR (fmt);
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == xioctl (fd, VIDIOC_G_FMT, &fmt))
            errno_exit ("VIDIOC_G_FMT");

        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = pwidth;//720
        fmt.fmt.pix.height = pheigth;//576;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR32;//V4L2_PIX_FMT_YUYV;//V4L2_PIX_FMT_BGR32;
        fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
        fmt.fmt.pix.bytesperline = 0;

        if (-1 == xioctl (fd, VIDIOC_S_FMT, &fmt))
                errno_exit ("VIDIOC_S_FMT");

        buffbutesperline = fmt.fmt.pix.bytesperline;

        /* Note VIDIOC_S_FMT may change width and height. */
    }

    //включить звук
    v4l2_control ctrl;
    ctrl.id = V4L2_CID_AUDIO_MUTE;
    ctrl.value = 0;
    if (-1 == xioctl (fd, VIDIOC_S_CTRL, &ctrl))
        qDebug ("error VIDIOC_S_CTRL");

    /* Buggy driver paranoia. */
    min = fmt.fmt.pix.width * 2;
    if (fmt.fmt.pix.bytesperline < min)
        fmt.fmt.pix.bytesperline = min;
    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
    if (fmt.fmt.pix.sizeimage < min)
        fmt.fmt.pix.sizeimage = min;

    if (iTypeDev != 3)
        init_mmap ();
}

void v4l2capiture::close_device(void)
{
    if (-1 == close (fd))
        errno_exit ("close");
    fd = -1;
}

void v4l2capiture::open_device(void)
{
    struct stat st;

    if (-1 == stat (dev_name, &st)) {
        fprintf (stderr, "Cannot identify '%s': %d, %s\n",
                 dev_name, errno, strerror (errno));
        exit (EXIT_FAILURE);
    }

    if (!S_ISCHR (st.st_mode)) {
        fprintf (stderr, "%s is no device\n", dev_name);
        exit (EXIT_FAILURE);
    }

    fd = open (dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

    if (-1 == fd) {
        fprintf (stderr, "Cannot open '%s': %d, %s\n",
                 dev_name, errno, strerror (errno));
        exit (EXIT_FAILURE);
    }
}

void v4l2capiture::start(const char devname[], int iType)
{
    iTypeDev = iType;
    strcpy(dev_name, devname);
    open_device ();
    init_device ();
    start_capturing ();
}

void v4l2capiture::end()
{
    stop_capturing ();
    uninit_device ();
    close_device ();
}


int v4l2capiture::selTvChannel(int iFreqHz, int iVideoMode, int iAudioType)
{
    /* Select video input, video standard and tune here. */

    //запрос параметров тюнера по индексу
    v4l2_input inpt;
    CLEAR (inpt);
    //0 - S-Video / 1 - Composite / 2 - Television
    inpt.index = iTypeDev;
    if (iTypeDev == 3)
        inpt.index = 0;
    if (xioctl (fd, VIDIOC_ENUMINPUT, &inpt) != 0)
        qDebug("error VIDIOC_ENUMINPUT");
    //выбор видео устройства
    int index = inpt.index; //индекс входа
    if (-1 == ioctl (fd, VIDIOC_S_INPUT, &index)) {
            perror ("VIDIOC_S_INPUT");
            exit (EXIT_FAILURE);
    }

    //Выбор тюнера
    v4l2_tuner tnr;
    CLEAR (tnr);
    tnr.index = 0; //один тюнер
    tnr.audmode = V4L2_TUNER_MODE_STEREO;//iAudioType
    tnr.rxsubchans = V4L2_TUNER_SUB_STEREO;
    tnr.capability = V4L2_TUNER_CAP_STEREO|V4L2_TUNER_CAP_NORM;
    tnr.reserved[0] = 0;
    if (iTypeDev == 3)
        tnr.capability = V4L2_TUNER_CAP_STEREO|V4L2_TUNER_CAP_LOW;
    if (xioctl (fd, VIDIOC_S_TUNER, &tnr) != 0)
        qDebug("error VIDIOC_S_TUNER");
    if (xioctl (fd, VIDIOC_G_TUNER, &tnr) != 0)
        qDebug("error VIDIOC_G_TUNER");

    if (iTypeDev == 2 || iTypeDev == 3) {
        //частота канала
        v4l2_frequency freqq;
        CLEAR (freqq);
        freqq.tuner = 0; //один тюнер
        freqq.type = V4L2_TUNER_ANALOG_TV;
        freqq.frequency = iFreqHz/(62.5*1000);//111.25*1000/62.5 = 111.25*16;
        if (iTypeDev == 3) {
            freqq.type = V4L2_TUNER_RADIO;
            freqq.frequency = iFreqHz*10/(62.5)*1000;
            qDebug("STATION: %d FM\n", freqq.frequency);//%3.2f/100.0
        } else
         qDebug("STATION: %d TV\n", freqq.frequency);//%3.2f/100.0
        if (xioctl (fd, VIDIOC_S_FREQUENCY, &freqq) != 0)
            qDebug ("VIDIOC_S_FREQUENCY error %d, %s\n", errno, strerror (errno));
        else
            qDebug("signal strength: %d\n", tnr.signal);
        //qDebug("error VIDIOC_S_FREQUENCY");
    }
    if (xioctl (fd, VIDIOC_ENUMINPUT, &inpt) == 0)
    qDebug("status 0x%x", inpt.status);

    //стандарт видео
    if (iTypeDev == 0 || iTypeDev == 1 || iTypeDev == 2) {
        v4l2_standard  stndlst;
        /*
           BEHOLD 407
           0 - NTSC
           1 - NTSC-M
           2 - NTSC-M-JP
           3 - NTSC-M-KR
           4 - PAL
           5 - PAL-BG
           6 - PAL-H
           7 - PAL-I
           8 - PAL-DK
           9 - PAL-M
           10 - PAL-N
           11 - PAL-Nc
           12 - PAL-60
           13 - SECAM
           14 - SECAM-B
           15 - SECAM-G
           16 - SECAM-H
           17 - SECAM-DK
           18 - SECAM-L
           19 - SECAM-Lc
        */
        if (iVideoMode > 0) {
            CLEAR (stndlst);
            if (iVideoMode == 1)
                iVideoMode = 17;
            else if (iVideoMode == 2)
                iVideoMode = 8;
            else if (iVideoMode == 3)
                iVideoMode = 0;
            stndlst.index = iVideoMode;//17;//8;
            if (xioctl (fd, VIDIOC_ENUMSTD, &stndlst) != 0)
                qDebug("error VIDIOC_ENUMSTD");

            v4l2_std_id stnd;
            CLEAR (stnd);
            stnd = stndlst.id;
            if (xioctl (fd, VIDIOC_S_STD, &stnd) != 0)
                qDebug("error VIDIOC_S_STD");
        } else {
            //автоопределение пал/секам

            v4l2_std_id stnd;
            CLEAR (stnd);
            if (xioctl (fd, VIDIOC_QUERYSTD, &stnd) != 0)
                qDebug("error VIDIOC_QUERYSTD");
            else {
                CLEAR (stndlst);
                stndlst.index = 17;
                if (xioctl (fd, VIDIOC_ENUMSTD, &stndlst) != 0)
                    qDebug("error VIDIOC_ENUMSTD");
                stnd = stndlst.id;
            }

            if (xioctl (fd, VIDIOC_S_STD, &stnd) != 0)
                qDebug("error VIDIOC_S_STD");

//            if (xioctl (fd, VIDIOC_G_STD, &stnd) != 0)
//                qDebug("error VIDIOC_G_STD");
        }
    }

    //звук
    v4l2_audio aud;
    CLEAR (aud);
    aud.index = 0;
    if (xioctl (fd, VIDIOC_G_AUDIO, &aud) != 0)
        qDebug("error VIDIOC_G_AUDIO");
    aud.index = 0;
    if (xioctl (fd, VIDIOC_S_AUDIO, &aud) != 0)
     qDebug("error VIDIOC_S_AUDIO");


    /*CLEAR (cropcap);

    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (0 == xioctl (fd, VIDIOC_CROPCAP, &cropcap)) {
            crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            crop.c = cropcap.defrect; // reset to default


            if (-1 == xioctl (fd, VIDIOC_S_CROP, &crop)) {
                switch (errno) {
                 case EINVAL:
                    // Cropping not supported.
                    break;
                 default:
                    // Errors ignored.
                    break;
                }
            }
    } else {
        // Errors ignored.
    }*/

    return 1;
}
