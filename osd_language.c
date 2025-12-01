#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/XInput2.h> 
#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

// إعدادات المظهر
#define OSD_WIDTH 200
#define OSD_HEIGHT 100
#define FONT_SIZE 40.0
#define SHOW_DURATION_MS 1500 

Display *display;
Window win;
cairo_surface_t *surface;
cairo_t *cr;
int screen;
int width, height;
int is_visible = 0;
// متغير لتخزين آخر لغة تم عرضها لمنع التكرار
int current_group_index = -1; 

pthread_t timer_thread;

void setup_window_properties() {
    XserverRegion region = XFixesCreateRegion(display, NULL, 0);
    XFixesSetWindowShapeRegion(display, win, ShapeInput, 0, 0, region);
    XFixesDestroyRegion(display, region);

    Atom wm_state = XInternAtom(display, "_NET_WM_STATE", False);
    Atom state_above = XInternAtom(display, "_NET_WM_STATE_ABOVE", False);
    XChangeProperty(display, win, wm_state, XA_ATOM, 32, PropModeReplace, (unsigned char *)&state_above, 1);
}

void draw_osd(const char *text) {
    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    double x = 10, y = 10, w = OSD_WIDTH - 20, h = OSD_HEIGHT - 20;
    double radius = 20.0; 

    cairo_set_source_rgba(cr, 0.1, 0.1, 0.1, 0.85); 
    
    cairo_new_sub_path(cr);
    cairo_arc(cr, x + w - radius, y + radius, radius, -90 * 3.14 / 180, 0);
    cairo_arc(cr, x + w - radius, y + h - radius, radius, 0, 90 * 3.14 / 180);
    cairo_arc(cr, x + radius, y + h - radius, radius, 90 * 3.14 / 180, 180 * 3.14 / 180);
    cairo_arc(cr, x + radius, y + radius, radius, 180 * 3.14 / 180, 270 * 3.14 / 180);
    cairo_close_path(cr);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); 
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, FONT_SIZE);

    cairo_text_extents_t extents;
    cairo_text_extents(cr, text, &extents);

    double text_x = (OSD_WIDTH - extents.width) / 2.0 - extents.x_bearing;
    double text_y = (OSD_HEIGHT - extents.height) / 2.0 - extents.y_bearing;

    cairo_move_to(cr, text_x, text_y);
    cairo_show_text(cr, text);
}

void *hide_timer(void *arg) {
    usleep(SHOW_DURATION_MS * 1000);
    XUnmapWindow(display, win);
    XFlush(display);
    is_visible = 0;
    return NULL;
}

const char* clean_lang_name(char *raw_name) {
    if (strcasestr(raw_name, "Arabic") || strcasestr(raw_name, "ara")) return "AR";
    if (strcasestr(raw_name, "English") || strcasestr(raw_name, "US")) return "EN";
    if (strcasestr(raw_name, "German")) return "DE";
    return raw_name; 
}

int main() {
    display = XOpenDisplay(NULL);
    if (!display) return 1;

    screen = DefaultScreen(display);
    width = DisplayWidth(display, screen);
    height = DisplayHeight(display, screen);

    XVisualInfo vinfo;
    if (!XMatchVisualInfo(display, screen, 32, TrueColor, &vinfo)) {
        fprintf(stderr, "No visual found supporting 32 bit color\n");
        return 1;
    }

    XSetWindowAttributes attrs;
    attrs.colormap = XCreateColormap(display, DefaultRootWindow(display), vinfo.visual, AllocNone);
    attrs.background_pixel = 0;
    attrs.border_pixel = 0;
    attrs.override_redirect = True; 

    win = XCreateWindow(display, DefaultRootWindow(display),
                        (width - OSD_WIDTH) / 2, (height - OSD_HEIGHT) / 2,
                        OSD_WIDTH, OSD_HEIGHT, 0, vinfo.depth, InputOutput,
                        vinfo.visual, CWColormap | CWBorderPixel | CWBackPixel | CWOverrideRedirect, &attrs);

    setup_window_properties();

    surface = cairo_xlib_surface_create(display, win, vinfo.visual, OSD_WIDTH, OSD_HEIGHT);
    cr = cairo_create(surface);

    int xkbEventType;
    XkbQueryExtension(display, 0, &xkbEventType, 0, 0, 0);
    // نراقب فقط تغيرات الحالة
    XkbSelectEvents(display, XkbUseCoreKbd, XkbStateNotifyMask, XkbStateNotifyMask);
    
    XkbDescPtr desc = XkbAllocKeyboard();
    XkbGetNames(display, XkbGroupNamesMask, desc);

    // جلب الحالة الابتدائية لمنع الظهور عند التشغيل (اختياري)
    XkbStateRec state;
    XkbGetState(display, XkbUseCoreKbd, &state);
    current_group_index = state.group;

    printf("OSD Daemon Running (Smart Filter Active)...\n");

    XEvent event;
    while (1) {
        XNextEvent(display, &event);

        if (event.type == xkbEventType) {
            XkbEvent *xkbEvent = (XkbEvent*)&event;
            
            if (xkbEvent->any.xkb_type == XkbStateNotify) {
                
                // === الفلتر السحري ===
                // التحقق 1: هل التغيير يخص المجموعة (اللغة)؟
                if (!(xkbEvent->state.changed & XkbGroupStateMask)) {
                    continue; // إذا كان مجرد ضغط زر Shift أو تغيير نافذة، تجاهله
                }

                // التحقق 2: هل تغيرت اللغة فعلاً؟ (أحياناً يرسل النظام إشارة وهمية لنفس اللغة)
                if (xkbEvent->state.group == current_group_index) {
                    continue; 
                }

                // تحديث اللغة الحالية
                current_group_index = xkbEvent->state.group;
                
                // === منطق العرض ===
                if (desc->names && desc->names->groups) {
                     char *groupName = XGetAtomName(display, desc->names->groups[current_group_index]);

                    if (groupName) {
                        const char *shortName = clean_lang_name(groupName);
                        
                        draw_osd(shortName);
                        XMapWindow(display, win);
                        XRaiseWindow(display, win);
                        XFlush(display);

                        if (is_visible) pthread_cancel(timer_thread);
                        pthread_create(&timer_thread, NULL, hide_timer, NULL);
                        is_visible = 1;

                        XFree(groupName);
                    }
                }
            }
        }
    }

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    XCloseDisplay(display);
    return 0;
}