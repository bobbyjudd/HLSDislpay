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
#include <X11/extensions/Xrandr.h>

int main()
{
    int height, width, xOffset, yOffset, numMonitors;
    Display *d;
    Window w;
    XRRMonitorInfo* monitorInfo;

    XImage *image;
    png_image png_img;
    XShmSegmentInfo shminfo;

    d = XOpenDisplay(NULL);
    w = DefaultRootWindow(d);

    
    
    monitorInfo = XRRGetMonitors(d, w, True, &numMonitors);

    for (int i = 0; i < numMonitors; i++)
    {
        XRRMonitorInfo *current = &(monitorInfo[i]);
        char filename[20];
        printf("%lu\n", current->name);
        width = current->width;
        height = current->height;
        xOffset = current->x;
        yOffset = current->y;

        image = XShmCreateImage(d, 
        DefaultVisual(d,0), // Use a correct visual. Omitted for brevity     
        32,                 // Determine correct depth from the visual. Omitted for brevity
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
            xOffset,
            yOffset,
            AllPlanes);

        memset(&png_img, 0, sizeof(png_img));
        png_img.opaque = NULL;
        png_img.version = PNG_IMAGE_VERSION;
        png_img.width = width;
        png_img.height = height;
        png_img.format = PNG_FORMAT_BGRA;
        
        sprintf(filename, "test_%d.png", i);
        png_image_write_to_file((png_imagep)(&png_img), filename, False, image->data, 0, NULL);
        
        XShmDetach(d, &shminfo);
    }

    return 0;
}