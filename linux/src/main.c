#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <png.h>
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>

int main()
{
    const int dim = 900;
    const int height = dim, width = dim;
    Display *d;
    int s;
    XImage *image;
    png_image png_img;
    XShmSegmentInfo shminfo;
    d = XOpenDisplay(NULL);
    s = DefaultScreen(d);

    image = XShmCreateImage(d, 
        DefaultVisual(d,0), // Use a correct visual. Omitted for brevity     
        24,   // Determine correct depth from the visual. Omitted for brevity
        ZPixmap, NULL, &shminfo, width, height); 

    shminfo.shmid = shmget(IPC_PRIVATE,
        image->bytes_per_line * image->height,
        IPC_CREAT|0777);

    shminfo.shmaddr = image->data = shmat(shminfo.shmid, 0, 0);
    shminfo.readOnly = False;

    XShmAttach(d, &shminfo);

    XShmGetImage(d,
        RootWindow(d,0),
        image,
        50,
        50,
        AllPlanes);

    memset(&png_img, 0, sizeof(png_img));
    png_img.opaque = NULL;                 /* Initialize to NULL, free with png_image_free */
    png_img.version = PNG_IMAGE_VERSION;   /* Set to PNG_IMAGE_VERSION */
    png_img.width = width;                    /* Image width in pixels (columns) */
    png_img.height = height;                   /* Image height in pixels (rows) */
    //png_img.format;                        /* Image format as defined below */
    //png_img->flags;                       /* A bit mask containing informational flags */
    //png_img->colormap_entries;
    
    png_image_write_to_file((png_imagep)(&png_img), "test.png", False, image->data, 0, NULL);

    return 0;
}