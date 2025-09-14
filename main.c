#include <X11/X.h>
#include <getopt.h>
#include <libinput.h> 
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <libudev.h>
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <stdio.h> 
#include <stdint.h>
#include <string.h>




typedef struct Hints
{
    unsigned long   flags;
    unsigned long   functions;
    unsigned long   decorations;
    long            inputMode;
    unsigned long   status;
} Hints;



void add_new_combo_string_to_end(int len_string, int num_strings, char strings[num_strings][len_string], char *user_and_hostname, int combo) {
    char buff[len_string];

    buff[0] = '[';
    memcpy(&buff[1], user_and_hostname, strlen(user_and_hostname));
    strcpy(&buff[strlen(user_and_hostname) + 1], "]$ combo ");
    snprintf(&buff[strlen(user_and_hostname) + 10], 4, "%d", combo);
    memmove(strings + 1, strings, (num_strings - 1) * len_string);
    strncpy(strings[0], buff, len_string);
}



void add_new_non_combo_string_to_end(int len_string, int num_strings, char strings[num_strings][len_string], char *user_and_hostname, int non_combo) {
    char buff[len_string];

    buff[0] = '[';
    memcpy(&buff[1], user_and_hostname, strlen(user_and_hostname));
    strcpy(&buff[strlen(user_and_hostname) + 1], "]$ not a combo ");
    snprintf(&buff[strlen(user_and_hostname) + 16], 4, "%d", non_combo);
    memmove(strings + 1, strings, (num_strings - 1) * len_string);
    strncpy(strings[0], buff, len_string);
}




char *get_user_and_hostname(void) {
    char *user = getenv("USER");
    char hostname[128];

    gethostname(hostname, 128);

    int length = strlen(user) + strlen(hostname);

    char *user_and_hostname = malloc(length + 2);

    strcpy(user_and_hostname, user);
    user_and_hostname[strlen(user)] = '@';
    strcpy(&user_and_hostname[strlen(user) + 1], hostname);

    return user_and_hostname;
}



char all_true(char arr[77]) {
    for (int counter = 0; counter < 77; counter++) {
        if (arr[counter] == 0) {
            return 0;
        }
    }
    return 1;  
}



void right_shift(char arr[12], char letter) {
    for (int counter = 11; counter > 0; counter--) {
        arr[counter] = arr[counter - 1];
    }
    arr[0] = letter;
}



char handle_keywords(char letter, char keywords[77][13], char status[77], char inconsistency[77]) {
    char inconsistency_flag = 0;
    
    for (char counter = 0; counter < 77; counter++) {
        if (keywords[counter][status[counter]] == letter) {
            if (keywords[counter][status[counter] + 1] == '\0') {
                memset(status, 0, 77);
                memset(inconsistency, 0, 77);
                return 2;
            } else {
                status[counter] += 1;
            }
        } else {
            status[counter] = 0;
            inconsistency[counter] = 1;

            if (!(inconsistency_flag) && all_true(inconsistency)) {
                inconsistency_flag = 1;
                memset(inconsistency, 0, 77);
            }
        }
    }
    if (inconsistency_flag) {
        return 0;
    }
    return 1;
}



static int open_restricted(const char *path, int flags, void *user_data)
{
    int fd = open(path, flags);
    return fd < 0 ? -errno : fd;
}
 


static void close_restricted(int fd, void *user_data)
{
    close(fd);
}
 


const static struct libinput_interface interface = {
    .open_restricted = open_restricted,
    .close_restricted = close_restricted,
};



char convert_to_ascii(uint32_t key) {
    switch (key) { // too lazy to add _ . sometime later maybe
        case 16: return 113; // q
        case 17: return 119; // w
        case 18: return 101; // e
        case 19: return 114; // r
        case 20: return 116; // t
        case 21: return 121; // y
        case 22: return 117; // u
        case 23: return 105; // i
        case 24: return 111; // o
        case 25: return 112; // p
        case 30: return 97; // a
        case 31: return 115; // s
        case 32: return 100; // d
        case 33: return 102; // f
        case 34: return 103; // g
        case 35: return 104; // h
        case 36: return 106; // j
        case 37: return 107; // k
        case 38: return 108; // z
        case 44: return 122; // x
        case 45: return 120; // c
        case 46: return 99; // v
        case 47: return 118; // b
        case 48: return 98; // n
        case 49: return 110; // n
        case 50: return 109; // m
        default: return 0;
    }
}



char capitalized_convert_to_ascii(uint32_t key) {
    switch (key) { // too lazy to add _ . sometime later maybe
        case 16: return 81;  // Q
        case 17: return 87;  // W
        case 18: return 69;  // E
        case 19: return 82;  // R
        case 20: return 84;  // T
        case 21: return 89;  // Y
        case 22: return 85;  // U
        case 23: return 73;  // I
        case 24: return 79;  // O
        case 25: return 80;  // P
        case 30: return 65;  // A
        case 31: return 83;  // S
        case 32: return 68;  // D
        case 33: return 70;  // F
        case 34: return 71;  // G
        case 35: return 72;  // H
        case 36: return 74;  // J
        case 37: return 75;  // K
        case 38: return 76;  // L
        case 44: return 90;  // Z
        case 45: return 88;  // X
        case 46: return 67;  // C
        case 47: return 86;  // V
        case 48: return 66;  // B
        case 49: return 78;  // N
        case 50: return 77;  // M
        default: return 0;
    }
}



void shake_window(Display *display, Window *window, int pos_x, int pos_y) {
    int offsets[][2] = {{-5, 5}, {0, 0}, {5, -5}, {0, 0}, {5, 5}, {0, 0}, {-5, -5}, {0, 0}};

    for (int counter = 0; counter < 8; counter++) {
        XMoveWindow(display, *window, pos_x + offsets[counter][0], pos_y + offsets[counter][1]);
        XFlush(display);         
        usleep(25000);             
    }

}

void map_and_set_up_decorations(XEvent *xevent, Display *display, GC *gc, Window *window, int border_width, int combo_window_size_x, int combo_window_size_y, XftDraw *draw, XftColor *color, XftFont *font, int font_width, int font_height, char *header_string, int x, int y) {
    XMapWindow(display, *window);
    XFlush(display);

    while (1) {
        XNextEvent(display, xevent);

        if (xevent->type == Expose) {
            XSetForeground(display, *gc, 0x000000);
            XFillRectangle(display, *window, *gc, border_width, (combo_window_size_y / 9) + border_width, combo_window_size_x - (2 * border_width), (combo_window_size_y / 9 * 8) - border_width);
            XSetForeground(display, *gc, 0xFF0000);
            XFillRectangle(display, *window, *gc, 0, 0, combo_window_size_x, combo_window_size_y / 9);
            
            XftDrawString8(draw, color, font, (combo_window_size_x - font_width) / 2, (combo_window_size_y / 9) / 2 + (font_height / 2), (XftChar8 *)header_string, strlen(header_string));

            XMoveWindow(display, *window, x, y);

            XFlush(display);

            break;
        }
    }
}

void display_combo_text(Display *display, Window *window, GC *gc, int border_width, int combo_window_size_x, int combo_window_size_y, XftDraw *draw, XftColor *color, XftFont *font, int font_width, int font_height, int len_string, int num_strings, char combo_strings[][len_string], float line_spacing_multiplier) {
    XSetForeground(display, *gc, 0x000000);
    XFillRectangle(display, *window, *gc, border_width, (combo_window_size_y / 9) + border_width, combo_window_size_x - (2 * border_width), (combo_window_size_y / 9 * 8) -  border_width - 1);

    for (int counter = 0; counter < num_strings; counter++) {
        if (combo_strings[counter][0] == '\0') {
            break;
        }

        XftDrawString8(draw, color, font, border_width * 3, (combo_window_size_y / 9) + border_width * 3 + ((counter + 1) * font_height) + (int)(counter * font_height * line_spacing_multiplier), (XftChar8 *)combo_strings[counter], strlen(combo_strings[counter]));
    }
    XFlush(display);
}

int main(int argc, char *argv[]) {
    int combo_window_size_x = 300;
    int combo_window_size_y = 200;
    int pos_x = 0;
    int pos_y = 0;
    float line_spacing_multiplier = 0.5;
    int border_width = 1;
    int autopos_corner = 0;
    int autopos_from_side = 50;
    int autopos_from_top_bottom = 50;
    int caps_lock_tracking = 1;

    int max_non_combo_word_to_interrupt = 3;

    // ---------------------------getting arguments--------------------------------
    int opt;

    static struct option long_options[] =
    {
        {"autopos", required_argument, NULL, 'a'},
        {"x_pos", required_argument, NULL, 'x'},
        {"y_pos", required_argument, NULL, 'y'},
        {"nocltrack", no_argument, NULL, 'n'},
        {"border", required_argument, NULL, 'b'},
        {NULL, 0, NULL, 0}
    };

    while ((opt = getopt_long(argc, argv, "x:y:a:b:n", long_options, NULL)) != -1) {
        switch (opt) {
            case 'x':
                pos_x = atoi(optarg);
                break;
            case 'y':
                pos_y = atoi(optarg);
                break;
            case 'a':
                autopos_corner = atoi(optarg);
                break;
            case 'b':
                border_width = atoi(optarg);
                break;
            case 'n':
                caps_lock_tracking = 0;
                break;
        }
  }

    // ---------------------------setup window--------------------------------

    Display *display;
    Window window;

    display = XOpenDisplay(NULL);

    if (display == NULL) {
        return 1;
    }

    int screen;
    screen = DefaultScreen(display);

    if (autopos_corner) {
        int screen_width = DisplayWidth(display, screen);
        int screen_height = DisplayHeight(display, screen);

        switch (autopos_corner) {
            case 1:
                pos_x = 0 + autopos_from_side;
                pos_y = 0 + autopos_from_top_bottom;
                break;

            case 2:
                pos_x = screen_width - autopos_from_side - combo_window_size_x;
                pos_y = 0 + autopos_from_top_bottom;
                break;
            
            case 3:
                pos_x = 0 + autopos_from_side;
                pos_y = screen_height - autopos_from_top_bottom - combo_window_size_y;
                break;
            
            case 4:
                pos_x = screen_width - autopos_from_side - combo_window_size_x;;
                pos_y = screen_height - autopos_from_top_bottom - combo_window_size_y;
                break;

            default:
                break;
        }
    }

    XSetWindowAttributes attrs;
    attrs.override_redirect = True;
    attrs.background_pixel = 0x888888;

    window = XCreateWindow(
        display,
        RootWindow(display, screen),
        100, 200,
        combo_window_size_x, combo_window_size_y,
        0,                    
        CopyFromParent,          
        InputOutput,             
        CopyFromParent,         
        CWOverrideRedirect | CWBackPixel,
        &attrs
    );

    XSelectInput(display, window, ExposureMask);

    struct Hints hints;
    Atom property;
    hints.flags = 2;
    hints.decorations = 0;
    property = XInternAtom(display, "_MOTIF_WM_HINTS", 1);
    XChangeProperty(display, window, property, property, 32, PropModeReplace, (unsigned char *)&hints, 5);

    GC gc = XCreateGC(display, window, 0, NULL);

    XRenderColor render_color = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
    XftColor color;
    XftColorAllocValue(display, DefaultVisual(display, screen), DefaultColormap(display, screen),
                       &render_color, &color);

    XftDraw *draw = XftDrawCreate(display, window, DefaultVisual(display, 0), DefaultColormap(display, 0));
    XftFont *font = XftFontOpenName(display, screen, "Hack Nerd Font Regular-11");

    char *user_and_hostname = get_user_and_hostname();

    XGlyphInfo extents;
    XftTextExtentsUtf8(display, font, (XftChar8 *)user_and_hostname, strlen(user_and_hostname), &extents);

    int num_lines_on_window = ((combo_window_size_y / 9 * 8) -  2 * border_width) / (int)(extents.height * (1 + line_spacing_multiplier));
    char combo_strings[num_lines_on_window][strlen(user_and_hostname) + 20];
    memset(combo_strings, 0, sizeof(combo_strings));
    

    // ---------------------------setup keylogger--------------------------------


    struct udev *udev = udev_new();
    struct libinput *licontext;
    struct libinput_event *event;
    struct libinput_event_keyboard *keyboard_event;

    uint32_t keycode;
    char key_ascii;
    char caps_lock = 0;

    char keywords[77][13] = {{'a', 'b', 's', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'a', 'l', 'l', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'a', 'n', 'y', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'a', 's', 'c', 'i', 'i', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'b', 'i', 'n', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'b', 'r', 'e', 'a', 'k', 'p', 'o', 'i', 'n', 't', '\0', '\0', '\0'}, {'c', 'a', 'l', 'l', 'a', 'b', 'l', 'e', '\0', '\0', '\0', '\0', '\0'}, {'c', 'h', 'r', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'c', 'o', 'm', 'p', 'i', 'l', 'e', '\0', '\0', '\0', '\0', '\0', '\0'}, {'d', 'e', 'l', 'a', 't', 't', 'r', '\0', '\0', '\0', '\0', '\0', '\0'}, {'d', 'i', 'r', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'d', 'i', 'v', 'm', 'o', 'd', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'e', 'v', 'a', 'l', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'e', 'x', 'e', 'c', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'f', 'o', 'r', 'm', 'a', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'g', 'e', 't', 'a', 't', 't', 'r', '\0', '\0', '\0', '\0', '\0', '\0'}, {'g', 'l', 'o', 'b', 'a', 'l', 's', '\0', '\0', '\0', '\0', '\0', '\0'}, {'h', 'a', 's', 'a', 't', 't', 'r', '\0', '\0', '\0', '\0', '\0', '\0'}, {'h', 'a', 's', 'h', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'h', 'e', 'x', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'i', 'd', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'i', 'n', 'p', 'u', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'i', 's', 'i', 'n', 's', 't', 'a', 'n', 'c', 'e', '\0', '\0', '\0'}, {'i', 's', 's', 'u', 'b', 'c', 'l', 'a', 's', 's', '\0', '\0', '\0'}, {'i', 't', 'e', 'r', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'a', 'i', 't', 'e', 'r', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'l', 'e', 'n', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'l', 'o', 'c', 'a', 'l', 's', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'m', 'a', 'x', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'m', 'i', 'n', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'n', 'e', 'x', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'a', 'n', 'e', 'x', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'o', 'c', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'o', 'r', 'd', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'p', 'o', 'w', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'p', 'r', 'i', 'n', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'r', 'e', 'p', 'r', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'r', 'o', 'u', 'n', 'd', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'s', 'e', 't', 'a', 't', 't', 'r', '\0', '\0', '\0', '\0', '\0', '\0'}, {'s', 'o', 'r', 't', 'e', 'd', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'s', 'u', 'm', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'v', 'a', 'r', 's', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'b', 'o', 'o', 'l', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'m', 'e', 'm', 'o', 'r', 'y', 'v', 'i', 'e', 'w', '\0', '\0', '\0'}, {'b', 'y', 't', 'e', 'a', 'r', 'r', 'a', 'y', '\0', '\0', '\0', '\0'}, {'b', 'y', 't', 'e', 's', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'c', 'l', 'a', 's', 's', 'm', 'e', 't', 'h', 'o', 'd', '\0', '\0'}, {'c', 'o', 'm', 'p', 'l', 'e', 'x', '\0', '\0', '\0', '\0', '\0', '\0'}, {'d', 'i', 'c', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'e', 'n', 'u', 'm', 'e', 'r', 'a', 't', 'e', '\0', '\0', '\0', '\0'}, {'f', 'i', 'l', 't', 'e', 'r', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'f', 'l', 'o', 'a', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'f', 'r', 'o', 'z', 'e', 'n', 's', 'e', 't', '\0', '\0', '\0', '\0'}, {'p', 'r', 'o', 'p', 'e', 'r', 't', 'y', '\0', '\0', '\0', '\0', '\0'}, {'i', 'n', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'l', 'i', 's', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'m', 'a', 'p', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'o', 'b', 'j', 'e', 'c', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'r', 'a', 'n', 'g', 'e', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'r', 'e', 'v', 'e', 'r', 's', 'e', 'd', '\0', '\0', '\0', '\0', '\0'}, {'s', 'e', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'s', 'l', 'i', 'c', 'e', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'s', 't', 'a', 't', 'i', 'c', 'm', 'e', 't', 'h', 'o', 'd', '\0'}, {'s', 't', 'r', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'s', 'u', 'p', 'e', 'r', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'t', 'u', 'p', 'l', 'e', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'t', 'y', 'p', 'e', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'z', 'i', 'p', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'o', 'p', 'e', 'n', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'q', 'u', 'i', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'e', 'x', 'i', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'c', 'o', 'p', 'y', 'r', 'i', 'g', 'h', 't', '\0', '\0', '\0', '\0'}, {'c', 'r', 'e', 'd', 'i', 't', 's', '\0', '\0', '\0', '\0', '\0', '\0'}, {'l', 'i', 'c', 'e', 'n', 's', 'e', '\0', '\0', '\0', '\0', '\0', '\0'}, {'h', 'e', 'l', 'p', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'e', 'x', 'e', 'c', 'f', 'i', 'l', 'e', '\0', '\0', '\0', '\0', '\0'}, {'r', 'u', 'n', 'f', 'i', 'l', 'e', '\0', '\0', '\0', '\0', '\0', '\0'}};
    char status_keywords[77] = {0};
    char inconsistency[77] = {0};

    char complete_combo = 0;

    int combo = 0;
    int non_combo = 0;

    licontext = libinput_udev_create_context(&interface, NULL, udev);
    libinput_udev_assign_seat(licontext, "seat0");
    libinput_dispatch(licontext);

    // ---------------------------main cycle--------------------------------

    int window_is_active = 0;

    XEvent xevent;
    
    while (1) {
        event = libinput_get_event(licontext);

        if (event != NULL) {
            if (libinput_event_get_type(event) == LIBINPUT_EVENT_KEYBOARD_KEY) {
                keyboard_event = libinput_event_get_keyboard_event(event);

                keycode = libinput_event_keyboard_get_key(keyboard_event);

                if (keycode == 58) { // caps lock
                    if (caps_lock_tracking) {
                        if (libinput_event_keyboard_get_key_state(keyboard_event) == LIBINPUT_KEY_STATE_PRESSED) {
                            if (caps_lock) {
                                caps_lock = 0;
                            } else {
                                caps_lock = 1;
                            }
                        }
                    }

                } else if (libinput_event_keyboard_get_key_state(keyboard_event) == LIBINPUT_KEY_STATE_RELEASED) {
                    if (caps_lock) {
                        key_ascii = capitalized_convert_to_ascii(keycode);
                    } else {
                        key_ascii = convert_to_ascii(keycode);
                    }
                    if (key_ascii != 0) {
                        complete_combo = handle_keywords(key_ascii, keywords, status_keywords, inconsistency);

                        if (complete_combo == 2) {
                            combo++;

                            if (combo == 1) {
                                memset(combo_strings, 0, sizeof(combo_strings));
                                map_and_set_up_decorations(&xevent, display, &gc,
                                                           &window, border_width,
                                                           combo_window_size_x, combo_window_size_y,
                                                           draw, &color, font,
                                                           extents.width, extents.height,
                                                           user_and_hostname, 
                                                           pos_x, pos_y);
                            }
                            
                            add_new_combo_string_to_end(strlen(user_and_hostname) + 20, num_lines_on_window, combo_strings, user_and_hostname, combo);

                            display_combo_text(display, &window, &gc, border_width,
                                                combo_window_size_x, combo_window_size_y,
                                                draw, &color, font,
                                                extents.width, extents.height,
                                                strlen(user_and_hostname) + 20, num_lines_on_window,
                                                combo_strings, line_spacing_multiplier);
                            
                            shake_window(display, &window, pos_x, pos_y);
                            
                            non_combo = 0;

                        } else if (complete_combo == 0) {
                            non_combo++;

                            if (non_combo == max_non_combo_word_to_interrupt) {
                                non_combo = 0;
                                combo = 0; 
                                XUnmapWindow(display, window);
                                

                            } else {
                                add_new_non_combo_string_to_end(strlen(user_and_hostname) + 20, num_lines_on_window, combo_strings, user_and_hostname, non_combo);
                                display_combo_text(display, &window, &gc, border_width,
                                                combo_window_size_x, combo_window_size_y,
                                                draw, &color, font,
                                                extents.width, extents.height,
                                                strlen(user_and_hostname) + 20, num_lines_on_window,
                                                combo_strings, line_spacing_multiplier);
                            }
                        }
                    }
                }
    
            }
        }
        libinput_event_destroy(event);
        libinput_dispatch(licontext);
    }
}