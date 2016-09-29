#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DEFAULT_DRI_CARD "/dev/dri/card0"

static int verbose = 0;

static void print_output_info(Display* dpy, XRRScreenResources* resources, RROutput output)
{
    XRROutputInfo* info = XRRGetOutputInfo(dpy, resources, output);
    if (!info) {
        fprintf(stderr, "Get output info failed\n");
        return;
    }

    printf("\tOutput name: %s\n", info->name);
    printf("\tOutput connected: %s\n", (info->connection == RR_Connected) ? "Connected" : "Disconnected");
    printf("\tOutput crtc: %lu\n", info->crtc);
    XRRFreeOutputInfo(info);
}

static void print_screen_resources(Display* dpy, XRRScreenResources* resources)
{
    printf("Timestamp: %lu\nConfigTimestamp: %lu\n", resources->timestamp, resources->configTimestamp);
    printf("Num of output: %d, num of crtc: %d\n", resources->noutput, resources->ncrtc);
    int i = 0;
    for (; i < resources->noutput; i++) {
        printf("[%d] output:\n", i);
        print_output_info(dpy, resources, resources->outputs[i]);
    }
}

static void query_screen_resources(Display* dpy, Window xid)
{
    XRRScreenResources* resources = XRRGetScreenResources(dpy, xid);
    if (!resources) {
        fprintf(stderr, "Get screen resources failed\n");
        return;
    }

    if (verbose) {
        print_screen_resources(dpy, resources);
    }
    XRRFreeScreenResources(resources);
    return;
}

static int get_connected_output_number(int fd)
{
    // acquire drm resources
    drmModeRes *resources = drmModeGetResources(fd);
    if (!resources) {
        fprintf(stderr, "Get drm resources failed\n");
        return -1;
    }

    int i = 0;
    int cnt = 0;
    drmModeConnector *connector = NULL;
    for (; i < resources->count_connectors; i++) {
        connector = drmModeGetConnector(fd, resources->connectors[i]);
        if (!connector) {
            continue;
        }

        if (connector->connection == DRM_MODE_CONNECTED &&
            connector->count_modes > 0) {
            cnt += 1;
            if (verbose) {
                printf("Monitor connected: %u\n", connector->encoder_id);
            }
        }

        drmModeFreeConnector(connector);
        connector = NULL;
    }

    drmModeFreeResources(resources);
    return cnt;
}

int main(int argc, char* argv[])
{
    int fd = open(DEFAULT_DRI_CARD, O_RDWR|O_CLOEXEC|O_NONBLOCK);
    if (fd < 1) {
        fprintf(stderr, "Open default card '%s' failed\n", DEFAULT_DRI_CARD);
        return -1;
    }

    Display* dpy = XOpenDisplay(NULL);
    if (!dpy) {
        fprintf(stderr, "Open display failed\n");
        close(fd);
        return -1;
    }

    Window root = DefaultRootWindow(dpy);
    drmSetMaster(fd);

    int seconds = 5;
    int i = 1;
    for (; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            verbose = 1;
        } else {
            seconds = atoi(argv[i]);
        }
    }
    if (seconds < 1) {
        seconds = 5;
    }

    if (verbose) {
        printf("Loop seconds: %d\n", seconds);
    }

    int prev_cnt = 0;
    int failed = 0;
    while (1) {
        if (failed > 10) {
            fprintf(stderr, "Too many failure, quit...\n");
            break;
        }

        int cnt = get_connected_output_number(fd);
        if (cnt == -1) {
            failed += 1;
            if (verbose) {
                fprintf(stderr, "Get output number failed\n");
            }
            sleep(seconds);
            continue;
        }

        // reset failed
        failed = 0;

        if (cnt != prev_cnt) {
            prev_cnt = cnt;
            query_screen_resources(dpy, root);
        }
        sleep(seconds);
    }

    drmDropMaster(fd);
    XCloseDisplay(dpy);
    close(fd);
    return 0;
}
