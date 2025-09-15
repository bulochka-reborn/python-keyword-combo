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

#define KEYWORDS_NUM 459
#define MAX_KEYWORD_LEN 27


typedef struct Hints
{
    unsigned long   flags;
    unsigned long   functions;
    unsigned long   decorations;
    long            inputMode;
    unsigned long   status;
} Hints;



void invalid_option_exit() {
    printf("Invalid options. Exiting the program. \n");
    exit(1);
}



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



char all_true(char arr[KEYWORDS_NUM]) {
    for (int counter = 0; counter < KEYWORDS_NUM; counter++) {
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


char handle_keywords(char letter, char keywords[KEYWORDS_NUM][MAX_KEYWORD_LEN], char status[KEYWORDS_NUM], char inconsistency[KEYWORDS_NUM]) {
    char inconsistency_flag = 0;
    
    for (int counter = 0; counter < KEYWORDS_NUM; counter++) {
        if (keywords[counter][status[counter]] == letter) {
            if (keywords[counter][status[counter] + 1] == '\0') {
                memset(status, 0, KEYWORDS_NUM);
                memset(inconsistency, 0, KEYWORDS_NUM);
                return 2;
            } else {
                status[counter] += 1;
            }
        } else {
            status[counter] = 0;
            inconsistency[counter] = 1;

            if (!(inconsistency_flag) && all_true(inconsistency)) {
                inconsistency_flag = 1;
                memset(inconsistency, 0, KEYWORDS_NUM);
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



char capitalized_convert_to_ascii(uint32_t key, char shift) {
    switch (key) { 
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

        case 12: 
            if (shift) {
                return 95; // _
            }
        
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
    int shift_tracking = 1;

    int max_non_combo_word_to_interrupt = 3;

    // ---------------------------getting arguments--------------------------------
    int opt;

    static struct option long_options[] =
    {
        {"autopos", required_argument, NULL, 'a'},
        {"x_pos", required_argument, NULL, 'x'},
        {"y_pos", required_argument, NULL, 'y'},
        {"capslockoff", no_argument, NULL, 'c'},
        {"shiftoff", no_argument, NULL, 's'},
        {"border", required_argument, NULL, 'b'},
        {"width", required_argument, NULL, 'w'},
        {"height", required_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

    while ((opt = getopt_long(argc, argv, "w:h:x:y:a:b:n", long_options, NULL)) != -1) {
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

            case 'c':
                caps_lock_tracking = 0;
                break;

            case 's':
                shift_tracking = 0;
                break;
            
            case 'w':
                combo_window_size_x = atoi(optarg);
                break;

            case 'h':
                combo_window_size_y = atoi(optarg);
                break;

            case ':': 
                invalid_option_exit();

            case '?':
                invalid_option_exit();

            default:
                invalid_option_exit();

        }
    }

    if (optind < argc) {
        invalid_option_exit();
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
                invalid_option_exit();
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
    char shift = 0;

    char keywords[KEYWORDS_NUM][MAX_KEYWORD_LEN] = {{'_', '_', 'n', 'a', 'm', 'e', '_', '_', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'a', 'd', 'd', '_', '_', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'c', 'l', 'a', 's', 's', '_', '_', '\0', '\0', '\0', '\0'}, {'_', '_', 'c', 'o', 'n', 't', 'a', 'i', 'n', 's', '_', '_', '\0'}, {'_', '_', 'd', 'e', 'l', 'a', 't', 't', 'r', '_', '_', '\0', '\0'}, {'_', '_', 'd', 'i', 'r', '_', '_', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'd', 'o', 'c', '_', '_', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'e', 'q', '_', '_', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'f', 'o', 'r', 'm', 'a', 't', '_', '_', '\0', '\0', '\0'}, {'_', '_', 'g', 'e', '_', '_', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'g', 'e', 't', 'a', 't', 't', 'r', 'i', 'b', 'u', 't', 'e', '_', '_', }, {'_', '_', 'g', 'e', 't', 'i', 't', 'e', 'm', '_', '_', '\0', '\0'}, {'_', '_', 'g', 'e', 't', 'n', 'e', 'w', 'a', 'r', 'g', 's', '_', '_', }, {'_', '_', 'g', 'e', 't', 's', 't', 'a', 't', 'e', '_', '_', '\0'}, {'_', '_', 'g', 't', '_', '_', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'h', 'a', 's', 'h', '_', '_', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'i', 'n', 'i', 't', '_', '_', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'i', 'n', 'i', 't', '_', 's', 'u', 'b', 'c', 'l', 'a', 's', 's', '_', '_', }, {'_', '_', 'i', 't', 'e', 'r', '_', '_', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'l', 'e', '_', '_', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'l', 'e', 'n', '_', '_', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'l', 't', '_', '_', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'm', 'o', 'd', '_', '_', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'm', 'u', 'l', '_', '_', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'n', 'e', '_', '_', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'n', 'e', 'w', '_', '_', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'r', 'e', 'd', 'u', 'c', 'e', '_', '_', '\0', '\0', '\0'}, {'_', '_', 'r', 'e', 'd', 'u', 'c', 'e', '_', 'e', 'x', '_', '_', }, {'_', '_', 'r', 'e', 'p', 'r', '_', '_', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'r', 'm', 'o', 'd', '_', '_', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'r', 'm', 'u', 'l', '_', '_', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 's', 'e', 't', 'a', 't', 't', 'r', '_', '_', '\0', '\0'}, {'_', '_', 's', 'i', 'z', 'e', 'o', 'f', '_', '_', '\0', '\0', '\0'}, {'_', '_', 's', 't', 'r', '_', '_', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 's', 'u', 'b', 'c', 'l', 'a', 's', 's', 'h', 'o', 'o', 'k', '_', '_', }, {'c', 'a', 'p', 'i', 't', 'a', 'l', 'i', 'z', 'e', '\0', '\0', '\0'}, {'c', 'a', 's', 'e', 'f', 'o', 'l', 'd', '\0', '\0', '\0', '\0', '\0'}, {'c', 'e', 'n', 't', 'e', 'r', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'c', 'o', 'u', 'n', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'e', 'n', 'c', 'o', 'd', 'e', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'e', 'n', 'd', 's', 'w', 'i', 't', 'h', '\0', '\0', '\0', '\0', '\0'}, {'e', 'x', 'p', 'a', 'n', 'd', 't', 'a', 'b', 's', '\0', '\0', '\0'}, {'f', 'i', 'n', 'd', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'f', 'o', 'r', 'm', 'a', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'f', 'o', 'r', 'm', 'a', 't', '_', 'm', 'a', 'p', '\0', '\0', '\0'}, {'i', 'n', 'd', 'e', 'x', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'i', 's', 'a', 'l', 'n', 'u', 'm', '\0', '\0', '\0', '\0', '\0', '\0'}, {'i', 's', 'a', 'l', 'p', 'h', 'a', '\0', '\0', '\0', '\0', '\0', '\0'}, {'i', 's', 'a', 's', 'c', 'i', 'i', '\0', '\0', '\0', '\0', '\0', '\0'}, {'i', 's', 'd', 'e', 'c', 'i', 'm', 'a', 'l', '\0', '\0', '\0', '\0'}, {'i', 's', 'd', 'i', 'g', 'i', 't', '\0', '\0', '\0', '\0', '\0', '\0'}, {'i', 's', 'i', 'd', 'e', 'n', 't', 'i', 'f', 'i', 'e', 'r', '\0'}, {'i', 's', 'l', 'o', 'w', 'e', 'r', '\0', '\0', '\0', '\0', '\0', '\0'}, {'i', 's', 'n', 'u', 'm', 'e', 'r', 'i', 'c', '\0', '\0', '\0', '\0'}, {'i', 's', 'p', 'r', 'i', 'n', 't', 'a', 'b', 'l', 'e', '\0', '\0'}, {'i', 's', 's', 'p', 'a', 'c', 'e', '\0', '\0', '\0', '\0', '\0', '\0'}, {'i', 's', 't', 'i', 't', 'l', 'e', '\0', '\0', '\0', '\0', '\0', '\0'}, {'i', 's', 'u', 'p', 'p', 'e', 'r', '\0', '\0', '\0', '\0', '\0', '\0'}, {'j', 'o', 'i', 'n', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'l', 'j', 'u', 's', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'l', 'o', 'w', 'e', 'r', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'l', 's', 't', 'r', 'i', 'p', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'m', 'a', 'k', 'e', 't', 'r', 'a', 'n', 's', '\0', '\0', '\0', '\0'}, {'p', 'a', 'r', 't', 'i', 't', 'i', 'o', 'n', '\0', '\0', '\0', '\0'}, {'r', 'e', 'm', 'o', 'v', 'e', 'p', 'r', 'e', 'f', 'i', 'x', '\0'}, {'r', 'e', 'm', 'o', 'v', 'e', 's', 'u', 'f', 'f', 'i', 'x', '\0'}, {'r', 'e', 'p', 'l', 'a', 'c', 'e', '\0', '\0', '\0', '\0', '\0', '\0'}, {'r', 'f', 'i', 'n', 'd', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'r', 'i', 'n', 'd', 'e', 'x', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'r', 'j', 'u', 's', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'r', 'p', 'a', 'r', 't', 'i', 't', 'i', 'o', 'n', '\0', '\0', '\0'}, {'r', 's', 'p', 'l', 'i', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'r', 's', 't', 'r', 'i', 'p', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'s', 'p', 'l', 'i', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'s', 'p', 'l', 'i', 't', 'l', 'i', 'n', 'e', 's', '\0', '\0', '\0'}, {'s', 't', 'a', 'r', 't', 's', 'w', 'i', 't', 'h', '\0', '\0', '\0'}, {'s', 't', 'r', 'i', 'p', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'s', 'w', 'a', 'p', 'c', 'a', 's', 'e', '\0', '\0', '\0', '\0', '\0'}, {'t', 'i', 't', 'l', 'e', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'t', 'r', 'a', 'n', 's', 'l', 'a', 't', 'e', '\0', '\0', '\0', '\0'}, {'u', 'p', 'p', 'e', 'r', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'z', 'f', 'i', 'l', 'l', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'p', 'a', 'c', 'k', 'a', 'g', 'e', '_', '_', '\0', '\0'}, {'_', '_', 'l', 'o', 'a', 'd', 'e', 'r', '_', '_', '\0', '\0', '\0'}, {'_', 'O', 'R', 'I', 'G', 'I', 'N', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'd', 'i', 'c', 't', '_', '_', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'f', 'i', 'r', 's', 't', 'l', 'i', 'n', 'e', 'n', 'o', '_', '_', }, {'_', '_', 'm', 'o', 'd', 'u', 'l', 'e', '_', '_', '\0', '\0', '\0'}, {'_', '_', 's', 't', 'a', 't', 'i', 'c', '_', 'a', 't', 't', 'r', 'i', 'b', 'u', 't', 'e', 's', '_', '_', }, {'_', '_', 'w', 'e', 'a', 'k', 'r', 'e', 'f', '_', '_', '\0', '\0'}, {'c', 'r', 'e', 'a', 't', 'e', '_', 'm', 'o', 'd', 'u', 'l', 'e', }, {'e', 'x', 'e', 'c', '_', 'm', 'o', 'd', 'u', 'l', 'e', '\0', '\0'}, {'f', 'i', 'n', 'd', '_', 's', 'p', 'e', 'c', '\0', '\0', '\0', '\0'}, {'g', 'e', 't', '_', 'c', 'o', 'd', 'e', '\0', '\0', '\0', '\0', '\0'}, {'g', 'e', 't', '_', 's', 'o', 'u', 'r', 'c', 'e', '\0', '\0', '\0'}, {'i', 's', '_', 'p', 'a', 'c', 'k', 'a', 'g', 'e', '\0', '\0', '\0'}, {'l', 'o', 'a', 'd', '_', 'm', 'o', 'd', 'u', 'l', 'e', '\0', '\0'}, {'_', '_', 's', 'p', 'e', 'c', '_', '_', '\0', '\0', '\0', '\0', '\0'}, {'_', 'c', 'a', 'c', 'h', 'e', 'd', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', 's', 'e', 't', '_', 'f', 'i', 'l', 'e', 'a', 't', 't', 'r', }, {'_', 'u', 'n', 'i', 'n', 'i', 't', 'i', 'a', 'l', 'i', 'z', 'e', 'd', '_', 's', 'u', 'b', 'm', 'o', 'd', 'u', 'l', 'e', 's', }, {'c', 'a', 'c', 'h', 'e', 'd', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'h', 'a', 's', '_', 'l', 'o', 'c', 'a', 't', 'i', 'o', 'n', '\0'}, {'l', 'o', 'a', 'd', 'e', 'r', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'l', 'o', 'a', 'd', 'e', 'r', '_', 's', 't', 'a', 't', 'e', '\0'}, {'n', 'a', 'm', 'e', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'o', 'r', 'i', 'g', 'i', 'n', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'p', 'a', 'r', 'e', 'n', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'s', 'u', 'b', 'm', 'o', 'd', 'u', 'l', 'e', '_', 's', 'e', 'a', 'r', 'c', 'h', '_', 'l', 'o', 'c', 'a', 't', 'i', 'o', 'n', 's', }, {'_', '_', 'b', 'u', 'i', 'l', 'd', '_', 'c', 'l', 'a', 's', 's', '_', '_', }, {'_', '_', 'c', 'a', 'l', 'l', '_', '_', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'q', 'u', 'a', 'l', 'n', 'a', 'm', 'e', '_', '_', '\0'}, {'_', '_', 's', 'e', 'l', 'f', '_', '_', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 't', 'e', 'x', 't', '_', 's', 'i', 'g', 'n', 'a', 't', 'u', 'r', 'e', '_', '_', }, {'_', '_', 'i', 'm', 'p', 'o', 'r', 't', '_', '_', '\0', '\0', '\0'}, {'a', 'b', 's', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'a', 'l', 'l', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'a', 'n', 'y', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'a', 's', 'c', 'i', 'i', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'b', 'i', 'n', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'b', 'r', 'e', 'a', 'k', 'p', 'o', 'i', 'n', 't', '\0', '\0', '\0'}, {'c', 'a', 'l', 'l', 'a', 'b', 'l', 'e', '\0', '\0', '\0', '\0', '\0'}, {'c', 'h', 'r', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'c', 'o', 'm', 'p', 'i', 'l', 'e', '\0', '\0', '\0', '\0', '\0', '\0'}, {'d', 'e', 'l', 'a', 't', 't', 'r', '\0', '\0', '\0', '\0', '\0', '\0'}, {'d', 'i', 'r', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'d', 'i', 'v', 'm', 'o', 'd', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'e', 'v', 'a', 'l', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'e', 'x', 'e', 'c', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'g', 'e', 't', 'a', 't', 't', 'r', '\0', '\0', '\0', '\0', '\0', '\0'}, {'g', 'l', 'o', 'b', 'a', 'l', 's', '\0', '\0', '\0', '\0', '\0', '\0'}, {'h', 'a', 's', 'a', 't', 't', 'r', '\0', '\0', '\0', '\0', '\0', '\0'}, {'h', 'a', 's', 'h', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'h', 'e', 'x', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'i', 'd', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'i', 'n', 'p', 'u', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'i', 's', 'i', 'n', 's', 't', 'a', 'n', 'c', 'e', '\0', '\0', '\0'}, {'i', 's', 's', 'u', 'b', 'c', 'l', 'a', 's', 's', '\0', '\0', '\0'}, {'i', 't', 'e', 'r', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'a', 'i', 't', 'e', 'r', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'l', 'e', 'n', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'l', 'o', 'c', 'a', 'l', 's', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'m', 'a', 'x', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'m', 'i', 'n', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'n', 'e', 'x', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'a', 'n', 'e', 'x', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'o', 'c', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'o', 'r', 'd', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'p', 'o', 'w', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'p', 'r', 'i', 'n', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'r', 'e', 'p', 'r', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'r', 'o', 'u', 'n', 'd', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'s', 'e', 't', 'a', 't', 't', 'r', '\0', '\0', '\0', '\0', '\0', '\0'}, {'s', 'o', 'r', 't', 'e', 'd', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'s', 'u', 'm', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'v', 'a', 'r', 's', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'N', 'o', 'n', 'e', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'b', 'o', 'o', 'l', '_', '_', '\0', '\0', '\0', '\0', '\0'}, {'E', 'l', 'l', 'i', 'p', 's', 'i', 's', '\0', '\0', '\0', '\0', '\0'}, {'N', 'o', 't', 'I', 'm', 'p', 'l', 'e', 'm', 'e', 'n', 't', 'e', 'd', }, {'F', 'a', 'l', 's', 'e', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'a', 'b', 's', '_', '_', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'a', 'n', 'd', '_', '_', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'c', 'e', 'i', 'l', '_', '_', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'd', 'i', 'v', 'm', 'o', 'd', '_', '_', '\0', '\0', '\0'}, {'_', '_', 'f', 'l', 'o', 'a', 't', '_', '_', '\0', '\0', '\0', '\0'}, {'_', '_', 'f', 'l', 'o', 'o', 'r', '_', '_', '\0', '\0', '\0', '\0'}, {'_', '_', 'f', 'l', 'o', 'o', 'r', 'd', 'i', 'v', '_', '_', '\0'}, {'_', '_', 'i', 'n', 'd', 'e', 'x', '_', '_', '\0', '\0', '\0', '\0'}, {'_', '_', 'i', 'n', 't', '_', '_', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'i', 'n', 'v', 'e', 'r', 't', '_', '_', '\0', '\0', '\0'}, {'_', '_', 'l', 's', 'h', 'i', 'f', 't', '_', '_', '\0', '\0', '\0'}, {'_', '_', 'n', 'e', 'g', '_', '_', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'o', 'r', '_', '_', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'p', 'o', 's', '_', '_', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'p', 'o', 'w', '_', '_', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'r', 'a', 'd', 'd', '_', '_', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'r', 'a', 'n', 'd', '_', '_', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'r', 'd', 'i', 'v', 'm', 'o', 'd', '_', '_', '\0', '\0'}, {'_', '_', 'r', 'f', 'l', 'o', 'o', 'r', 'd', 'i', 'v', '_', '_', }, {'_', '_', 'r', 'l', 's', 'h', 'i', 'f', 't', '_', '_', '\0', '\0'}, {'_', '_', 'r', 'o', 'r', '_', '_', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'r', 'o', 'u', 'n', 'd', '_', '_', '\0', '\0', '\0', '\0'}, {'_', '_', 'r', 'p', 'o', 'w', '_', '_', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'r', 'r', 's', 'h', 'i', 'f', 't', '_', '_', '\0', '\0'}, {'_', '_', 'r', 's', 'h', 'i', 'f', 't', '_', '_', '\0', '\0', '\0'}, {'_', '_', 'r', 's', 'u', 'b', '_', '_', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'r', 't', 'r', 'u', 'e', 'd', 'i', 'v', '_', '_', '\0'}, {'_', '_', 'r', 'x', 'o', 'r', '_', '_', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 's', 'u', 'b', '_', '_', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 't', 'r', 'u', 'e', 'd', 'i', 'v', '_', '_', '\0', '\0'}, {'_', '_', 't', 'r', 'u', 'n', 'c', '_', '_', '\0', '\0', '\0', '\0'}, {'_', '_', 'x', 'o', 'r', '_', '_', '\0', '\0', '\0', '\0', '\0', '\0'}, {'a', 's', '_', 'i', 'n', 't', 'e', 'g', 'e', 'r', '_', 'r', 'a', 't', 'i', 'o', }, {'b', 'i', 't', '_', 'c', 'o', 'u', 'n', 't', '\0', '\0', '\0', '\0'}, {'b', 'i', 't', '_', 'l', 'e', 'n', 'g', 't', 'h', '\0', '\0', '\0'}, {'c', 'o', 'n', 'j', 'u', 'g', 'a', 't', 'e', '\0', '\0', '\0', '\0'}, {'d', 'e', 'n', 'o', 'm', 'i', 'n', 'a', 't', 'o', 'r', '\0', '\0'}, {'f', 'r', 'o', 'm', '_', 'b', 'y', 't', 'e', 's', '\0', '\0', '\0'}, {'i', 'm', 'a', 'g', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'i', 's', '_', 'i', 'n', 't', 'e', 'g', 'e', 'r', '\0', '\0', '\0'}, {'n', 'u', 'm', 'e', 'r', 'a', 't', 'o', 'r', '\0', '\0', '\0', '\0'}, {'r', 'e', 'a', 'l', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'t', 'o', '_', 'b', 'y', 't', 'e', 's', '\0', '\0', '\0', '\0', '\0'}, {'T', 'r', 'u', 'e', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'b', 'o', 'o', 'l', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'m', 'e', 'm', 'o', 'r', 'y', 'v', 'i', 'e', 'w', '\0', '\0', '\0'}, {'_', '_', 'b', 'u', 'f', 'f', 'e', 'r', '_', '_', '\0', '\0', '\0'}, {'_', '_', 'd', 'e', 'l', 'i', 't', 'e', 'm', '_', '_', '\0', '\0'}, {'_', '_', 'e', 'n', 't', 'e', 'r', '_', '_', '\0', '\0', '\0', '\0'}, {'_', '_', 'e', 'x', 'i', 't', '_', '_', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'r', 'e', 'l', 'e', 'a', 's', 'e', '_', 'b', 'u', 'f', 'f', 'e', 'r', '_', '_', }, {'_', '_', 's', 'e', 't', 'i', 't', 'e', 'm', '_', '_', '\0', '\0'}, {'_', 'f', 'r', 'o', 'm', '_', 'f', 'l', 'a', 'g', 's', '\0', '\0'}, {'c', '_', 'c', 'o', 'n', 't', 'i', 'g', 'u', 'o', 'u', 's', '\0'}, {'c', 'a', 's', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'c', 'o', 'n', 't', 'i', 'g', 'u', 'o', 'u', 's', '\0', '\0', '\0'}, {'f', '_', 'c', 'o', 'n', 't', 'i', 'g', 'u', 'o', 'u', 's', '\0'}, {'i', 't', 'e', 'm', 's', 'i', 'z', 'e', '\0', '\0', '\0', '\0', '\0'}, {'n', 'b', 'y', 't', 'e', 's', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'n', 'd', 'i', 'm', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'o', 'b', 'j', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'r', 'e', 'a', 'd', 'o', 'n', 'l', 'y', '\0', '\0', '\0', '\0', '\0'}, {'r', 'e', 'l', 'e', 'a', 's', 'e', '\0', '\0', '\0', '\0', '\0', '\0'}, {'s', 'h', 'a', 'p', 'e', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'s', 't', 'r', 'i', 'd', 'e', 's', '\0', '\0', '\0', '\0', '\0', '\0'}, {'s', 'u', 'b', 'o', 'f', 'f', 's', 'e', 't', 's', '\0', '\0', '\0'}, {'t', 'o', 'b', 'y', 't', 'e', 's', '\0', '\0', '\0', '\0', '\0', '\0'}, {'t', 'o', 'l', 'i', 's', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'t', 'o', 'r', 'e', 'a', 'd', 'o', 'n', 'l', 'y', '\0', '\0', '\0'}, {'b', 'y', 't', 'e', 'a', 'r', 'r', 'a', 'y', '\0', '\0', '\0', '\0'}, {'_', '_', 'a', 'l', 'l', 'o', 'c', '_', '_', '\0', '\0', '\0', '\0'}, {'_', '_', 'i', 'a', 'd', 'd', '_', '_', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'i', 'm', 'u', 'l', '_', '_', '\0', '\0', '\0', '\0', '\0'}, {'a', 'p', 'p', 'e', 'n', 'd', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'c', 'l', 'e', 'a', 'r', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'c', 'o', 'p', 'y', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'d', 'e', 'c', 'o', 'd', 'e', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'e', 'x', 't', 'e', 'n', 'd', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'f', 'r', 'o', 'm', 'h', 'e', 'x', '\0', '\0', '\0', '\0', '\0', '\0'}, {'i', 'n', 's', 'e', 'r', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'p', 'o', 'p', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'r', 'e', 'm', 'o', 'v', 'e', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'r', 'e', 'v', 'e', 'r', 's', 'e', '\0', '\0', '\0', '\0', '\0', '\0'}, {'b', 'y', 't', 'e', 's', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'b', 'y', 't', 'e', 's', '_', '_', '\0', '\0', '\0', '\0'}, {'c', 'l', 'a', 's', 's', 'm', 'e', 't', 'h', 'o', 'd', '\0', '\0'}, {'_', '_', 'f', 'u', 'n', 'c', '_', '_', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'g', 'e', 't', '_', '_', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'i', 's', 'a', 'b', 's', 't', 'r', 'a', 'c', 't', 'm', 'e', 't', 'h', 'o', 'd', '_', '_', }, {'_', '_', 'w', 'r', 'a', 'p', 'p', 'e', 'd', '_', '_', '\0', '\0'}, {'c', 'o', 'm', 'p', 'l', 'e', 'x', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'c', 'o', 'm', 'p', 'l', 'e', 'x', '_', '_', '\0', '\0'}, {'d', 'i', 'c', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'c', 'l', 'a', 's', 's', '_', 'g', 'e', 't', 'i', 't', 'e', 'm', '_', '_', }, {'_', '_', 'i', 'o', 'r', '_', '_', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'r', 'e', 'v', 'e', 'r', 's', 'e', 'd', '_', '_', '\0'}, {'f', 'r', 'o', 'm', 'k', 'e', 'y', 's', '\0', '\0', '\0', '\0', '\0'}, {'g', 'e', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'i', 't', 'e', 'm', 's', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'k', 'e', 'y', 's', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'p', 'o', 'p', 'i', 't', 'e', 'm', '\0', '\0', '\0', '\0', '\0', '\0'}, {'s', 'e', 't', 'd', 'e', 'f', 'a', 'u', 'l', 't', '\0', '\0', '\0'}, {'u', 'p', 'd', 'a', 't', 'e', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'v', 'a', 'l', 'u', 'e', 's', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'e', 'n', 'u', 'm', 'e', 'r', 'a', 't', 'e', '\0', '\0', '\0', '\0'}, {'_', '_', 'n', 'e', 'x', 't', '_', '_', '\0', '\0', '\0', '\0', '\0'}, {'f', 'i', 'l', 't', 'e', 'r', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'f', 'l', 'o', 'a', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'g', 'e', 't', 'f', 'o', 'r', 'm', 'a', 't', '_', '_', }, {'f', 'r', 'o', 'z', 'e', 'n', 's', 'e', 't', '\0', '\0', '\0', '\0'}, {'d', 'i', 'f', 'f', 'e', 'r', 'e', 'n', 'c', 'e', '\0', '\0', '\0'}, {'i', 'n', 't', 'e', 'r', 's', 'e', 'c', 't', 'i', 'o', 'n', '\0'}, {'i', 's', 'd', 'i', 's', 'j', 'o', 'i', 'n', 't', '\0', '\0', '\0'}, {'i', 's', 's', 'u', 'b', 's', 'e', 't', '\0', '\0', '\0', '\0', '\0'}, {'i', 's', 's', 'u', 'p', 'e', 'r', 's', 'e', 't', '\0', '\0', '\0'}, {'s', 'y', 'm', 'm', 'e', 't', 'r', 'i', 'c', '_', 'd', 'i', 'f', 'f', 'e', 'r', 'e', 'n', 'c', 'e', }, {'u', 'n', 'i', 'o', 'n', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'p', 'r', 'o', 'p', 'e', 'r', 't', 'y', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'd', 'e', 'l', 'e', 't', 'e', '_', '_', '\0', '\0', '\0'}, {'_', '_', 's', 'e', 't', '_', '_', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 's', 'e', 't', '_', 'n', 'a', 'm', 'e', '_', '_', '\0'}, {'d', 'e', 'l', 'e', 't', 'e', 'r', '\0', '\0', '\0', '\0', '\0', '\0'}, {'f', 'd', 'e', 'l', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'f', 'g', 'e', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'f', 's', 'e', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'g', 'e', 't', 't', 'e', 'r', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'s', 'e', 't', 't', 'e', 'r', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'i', 'n', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'l', 'i', 's', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'s', 'o', 'r', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'m', 'a', 'p', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'o', 'b', 'j', 'e', 'c', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'r', 'a', 'n', 'g', 'e', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'s', 't', 'a', 'r', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'s', 't', 'e', 'p', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'s', 't', 'o', 'p', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'r', 'e', 'v', 'e', 'r', 's', 'e', 'd', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'l', 'e', 'n', 'g', 't', 'h', '_', 'h', 'i', 'n', 't', '_', '_', }, {'_', '_', 's', 'e', 't', 's', 't', 'a', 't', 'e', '_', '_', '\0'}, {'s', 'e', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'i', 'a', 'n', 'd', '_', '_', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'i', 's', 'u', 'b', '_', '_', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'i', 'x', 'o', 'r', '_', '_', '\0', '\0', '\0', '\0', '\0'}, {'a', 'd', 'd', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'d', 'i', 'f', 'f', 'e', 'r', 'e', 'n', 'c', 'e', '_', 'u', 'p', 'd', 'a', 't', 'e', }, {'d', 'i', 's', 'c', 'a', 'r', 'd', '\0', '\0', '\0', '\0', '\0', '\0'}, {'i', 'n', 't', 'e', 'r', 's', 'e', 'c', 't', 'i', 'o', 'n', '_', 'u', 'p', 'd', 'a', 't', 'e', }, {'s', 'y', 'm', 'm', 'e', 't', 'r', 'i', 'c', '_', 'd', 'i', 'f', 'f', 'e', 'r', 'e', 'n', 'c', 'e', '_', 'u', 'p', 'd', 'a', 't', 'e', }, {'s', 'l', 'i', 'c', 'e', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'i', 'n', 'd', 'i', 'c', 'e', 's', '\0', '\0', '\0', '\0', '\0', '\0'}, {'s', 't', 'a', 't', 'i', 'c', 'm', 'e', 't', 'h', 'o', 'd', '\0'}, {'s', 't', 'r', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'s', 'u', 'p', 'e', 'r', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 's', 'e', 'l', 'f', '_', 'c', 'l', 'a', 's', 's', '_', '_', }, {'_', '_', 't', 'h', 'i', 's', 'c', 'l', 'a', 's', 's', '_', '_', }, {'t', 'u', 'p', 'l', 'e', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'t', 'y', 'p', 'e', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'a', 'b', 's', 't', 'r', 'a', 'c', 't', 'm', 'e', 't', 'h', 'o', 'd', 's', '_', '_', }, {'_', '_', 'a', 'n', 'n', 'o', 't', 'a', 't', 'i', 'o', 'n', 's', '_', '_', }, {'_', '_', 'b', 'a', 's', 'e', '_', '_', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'b', 'a', 's', 'e', 's', '_', '_', '\0', '\0', '\0', '\0'}, {'_', '_', 'b', 'a', 's', 'i', 'c', 's', 'i', 'z', 'e', '_', '_', }, {'_', '_', 'd', 'i', 'c', 't', 'o', 'f', 'f', 's', 'e', 't', '_', '_', }, {'_', '_', 'f', 'l', 'a', 'g', 's', '_', '_', '\0', '\0', '\0', '\0'}, {'_', '_', 'i', 'n', 's', 't', 'a', 'n', 'c', 'e', 'c', 'h', 'e', 'c', 'k', '_', '_', }, {'_', '_', 'i', 't', 'e', 'm', 's', 'i', 'z', 'e', '_', '_', '\0'}, {'_', '_', 'm', 'r', 'o', '_', '_', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'p', 'r', 'e', 'p', 'a', 'r', 'e', '_', '_', '\0', '\0'}, {'_', '_', 's', 'u', 'b', 'c', 'l', 'a', 's', 's', 'c', 'h', 'e', 'c', 'k', '_', '_', }, {'_', '_', 's', 'u', 'b', 'c', 'l', 'a', 's', 's', 'e', 's', '_', '_', }, {'_', '_', 't', 'y', 'p', 'e', '_', 'p', 'a', 'r', 'a', 'm', 's', '_', '_', }, {'_', '_', 'w', 'e', 'a', 'k', 'r', 'e', 'f', 'o', 'f', 'f', 's', 'e', 't', '_', '_', }, {'m', 'r', 'o', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'z', 'i', 'p', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'd', 'e', 'b', 'u', 'g', '_', '_', '\0', '\0', '\0', '\0'}, {'B', 'a', 's', 'e', 'E', 'x', 'c', 'e', 'p', 't', 'i', 'o', 'n', }, {'_', '_', 'c', 'a', 'u', 's', 'e', '_', '_', '\0', '\0', '\0', '\0'}, {'_', '_', 'c', 'o', 'n', 't', 'e', 'x', 't', '_', '_', '\0', '\0'}, {'_', '_', 's', 'u', 'p', 'p', 'r', 'e', 's', 's', '_', 'c', 'o', 'n', 't', 'e', 'x', 't', '_', '_', }, {'_', '_', 't', 'r', 'a', 'c', 'e', 'b', 'a', 'c', 'k', '_', '_', }, {'a', 'd', 'd', '_', 'n', 'o', 't', 'e', '\0', '\0', '\0', '\0', '\0'}, {'a', 'r', 'g', 's', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'w', 'i', 't', 'h', '_', 't', 'r', 'a', 'c', 'e', 'b', 'a', 'c', 'k', }, {'B', 'a', 's', 'e', 'E', 'x', 'c', 'e', 'p', 't', 'i', 'o', 'n', 'G', 'r', 'o', 'u', 'p', }, {'d', 'e', 'r', 'i', 'v', 'e', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'e', 'x', 'c', 'e', 'p', 't', 'i', 'o', 'n', 's', '\0', '\0', '\0'}, {'m', 'e', 's', 's', 'a', 'g', 'e', '\0', '\0', '\0', '\0', '\0', '\0'}, {'s', 'u', 'b', 'g', 'r', 'o', 'u', 'p', '\0', '\0', '\0', '\0', '\0'}, {'E', 'x', 'c', 'e', 'p', 't', 'i', 'o', 'n', '\0', '\0', '\0', '\0'}, {'G', 'e', 'n', 'e', 'r', 'a', 't', 'o', 'r', 'E', 'x', 'i', 't', }, {'K', 'e', 'y', 'b', 'o', 'a', 'r', 'd', 'I', 'n', 't', 'e', 'r', 'r', 'u', 'p', 't', }, {'S', 'y', 's', 't', 'e', 'm', 'E', 'x', 'i', 't', '\0', '\0', '\0'}, {'c', 'o', 'd', 'e', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'A', 'r', 'i', 't', 'h', 'm', 'e', 't', 'i', 'c', 'E', 'r', 'r', 'o', 'r', }, {'A', 's', 's', 'e', 'r', 't', 'i', 'o', 'n', 'E', 'r', 'r', 'o', 'r', }, {'A', 't', 't', 'r', 'i', 'b', 'u', 't', 'e', 'E', 'r', 'r', 'o', 'r', }, {'B', 'u', 'f', 'f', 'e', 'r', 'E', 'r', 'r', 'o', 'r', '\0', '\0'}, {'E', 'O', 'F', 'E', 'r', 'r', 'o', 'r', '\0', '\0', '\0', '\0', '\0'}, {'I', 'm', 'p', 'o', 'r', 't', 'E', 'r', 'r', 'o', 'r', '\0', '\0'}, {'m', 's', 'g', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'n', 'a', 'm', 'e', '_', 'f', 'r', 'o', 'm', '\0', '\0', '\0', '\0'}, {'p', 'a', 't', 'h', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'L', 'o', 'o', 'k', 'u', 'p', 'E', 'r', 'r', 'o', 'r', '\0', '\0'}, {'M', 'e', 'm', 'o', 'r', 'y', 'E', 'r', 'r', 'o', 'r', '\0', '\0'}, {'N', 'a', 'm', 'e', 'E', 'r', 'r', 'o', 'r', '\0', '\0', '\0', '\0'}, {'O', 'S', 'E', 'r', 'r', 'o', 'r', '\0', '\0', '\0', '\0', '\0', '\0'}, {'c', 'h', 'a', 'r', 'a', 'c', 't', 'e', 'r', 's', '_', 'w', 'r', 'i', 't', 't', 'e', 'n', }, {'e', 'r', 'r', 'n', 'o', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'f', 'i', 'l', 'e', 'n', 'a', 'm', 'e', '\0', '\0', '\0', '\0', '\0'}, {'f', 'i', 'l', 'e', 'n', 'a', 'm', 'e', '2', '\0', '\0', '\0', '\0'}, {'s', 't', 'r', 'e', 'r', 'r', 'o', 'r', '\0', '\0', '\0', '\0', '\0'}, {'R', 'e', 'f', 'e', 'r', 'e', 'n', 'c', 'e', 'E', 'r', 'r', 'o', 'r', }, {'R', 'u', 'n', 't', 'i', 'm', 'e', 'E', 'r', 'r', 'o', 'r', '\0'}, {'S', 't', 'o', 'p', 'A', 's', 'y', 'n', 'c', 'I', 't', 'e', 'r', 'a', 't', 'i', 'o', 'n', }, {'S', 't', 'o', 'p', 'I', 't', 'e', 'r', 'a', 't', 'i', 'o', 'n', }, {'v', 'a', 'l', 'u', 'e', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'S', 'y', 'n', 't', 'a', 'x', 'E', 'r', 'r', 'o', 'r', '\0', '\0'}, {'e', 'n', 'd', '_', 'l', 'i', 'n', 'e', 'n', 'o', '\0', '\0', '\0'}, {'e', 'n', 'd', '_', 'o', 'f', 'f', 's', 'e', 't', '\0', '\0', '\0'}, {'l', 'i', 'n', 'e', 'n', 'o', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'o', 'f', 'f', 's', 'e', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'p', 'r', 'i', 'n', 't', '_', 'f', 'i', 'l', 'e', '_', 'a', 'n', 'd', '_', 'l', 'i', 'n', 'e', }, {'t', 'e', 'x', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'S', 'y', 's', 't', 'e', 'm', 'E', 'r', 'r', 'o', 'r', '\0', '\0'}, {'T', 'y', 'p', 'e', 'E', 'r', 'r', 'o', 'r', '\0', '\0', '\0', '\0'}, {'V', 'a', 'l', 'u', 'e', 'E', 'r', 'r', 'o', 'r', '\0', '\0', '\0'}, {'W', 'a', 'r', 'n', 'i', 'n', 'g', '\0', '\0', '\0', '\0', '\0', '\0'}, {'F', 'l', 'o', 'a', 't', 'i', 'n', 'g', 'P', 'o', 'i', 'n', 't', 'E', 'r', 'r', 'o', 'r', }, {'O', 'v', 'e', 'r', 'f', 'l', 'o', 'w', 'E', 'r', 'r', 'o', 'r', }, {'Z', 'e', 'r', 'o', 'D', 'i', 'v', 'i', 's', 'i', 'o', 'n', 'E', 'r', 'r', 'o', 'r', }, {'B', 'y', 't', 'e', 's', 'W', 'a', 'r', 'n', 'i', 'n', 'g', '\0'}, {'D', 'e', 'p', 'r', 'e', 'c', 'a', 't', 'i', 'o', 'n', 'W', 'a', 'r', 'n', 'i', 'n', 'g', }, {'E', 'n', 'c', 'o', 'd', 'i', 'n', 'g', 'W', 'a', 'r', 'n', 'i', 'n', 'g', }, {'F', 'u', 't', 'u', 'r', 'e', 'W', 'a', 'r', 'n', 'i', 'n', 'g', }, {'I', 'm', 'p', 'o', 'r', 't', 'W', 'a', 'r', 'n', 'i', 'n', 'g', }, {'P', 'e', 'n', 'd', 'i', 'n', 'g', 'D', 'e', 'p', 'r', 'e', 'c', 'a', 't', 'i', 'o', 'n', 'W', 'a', 'r', 'n', 'i', 'n', 'g', }, {'R', 'e', 's', 'o', 'u', 'r', 'c', 'e', 'W', 'a', 'r', 'n', 'i', 'n', 'g', }, {'R', 'u', 'n', 't', 'i', 'm', 'e', 'W', 'a', 'r', 'n', 'i', 'n', 'g', }, {'S', 'y', 'n', 't', 'a', 'x', 'W', 'a', 'r', 'n', 'i', 'n', 'g', }, {'U', 'n', 'i', 'c', 'o', 'd', 'e', 'W', 'a', 'r', 'n', 'i', 'n', 'g', }, {'U', 's', 'e', 'r', 'W', 'a', 'r', 'n', 'i', 'n', 'g', '\0', '\0'}, {'B', 'l', 'o', 'c', 'k', 'i', 'n', 'g', 'I', 'O', 'E', 'r', 'r', 'o', 'r', }, {'C', 'h', 'i', 'l', 'd', 'P', 'r', 'o', 'c', 'e', 's', 's', 'E', 'r', 'r', 'o', 'r', }, {'C', 'o', 'n', 'n', 'e', 'c', 't', 'i', 'o', 'n', 'E', 'r', 'r', 'o', 'r', }, {'F', 'i', 'l', 'e', 'E', 'x', 'i', 's', 't', 's', 'E', 'r', 'r', 'o', 'r', }, {'F', 'i', 'l', 'e', 'N', 'o', 't', 'F', 'o', 'u', 'n', 'd', 'E', 'r', 'r', 'o', 'r', }, {'I', 'n', 't', 'e', 'r', 'r', 'u', 'p', 't', 'e', 'd', 'E', 'r', 'r', 'o', 'r', }, {'I', 's', 'A', 'D', 'i', 'r', 'e', 'c', 't', 'o', 'r', 'y', 'E', 'r', 'r', 'o', 'r', }, {'N', 'o', 't', 'A', 'D', 'i', 'r', 'e', 'c', 't', 'o', 'r', 'y', 'E', 'r', 'r', 'o', 'r', }, {'P', 'e', 'r', 'm', 'i', 's', 's', 'i', 'o', 'n', 'E', 'r', 'r', 'o', 'r', }, {'P', 'r', 'o', 'c', 'e', 's', 's', 'L', 'o', 'o', 'k', 'u', 'p', 'E', 'r', 'r', 'o', 'r', }, {'T', 'i', 'm', 'e', 'o', 'u', 't', 'E', 'r', 'r', 'o', 'r', '\0'}, {'I', 'n', 'd', 'e', 'n', 't', 'a', 't', 'i', 'o', 'n', 'E', 'r', 'r', 'o', 'r', }, {'_', 'I', 'n', 'c', 'o', 'm', 'p', 'l', 'e', 't', 'e', 'I', 'n', 'p', 'u', 't', 'E', 'r', 'r', 'o', 'r', }, {'I', 'n', 'd', 'e', 'x', 'E', 'r', 'r', 'o', 'r', '\0', '\0', '\0'}, {'K', 'e', 'y', 'E', 'r', 'r', 'o', 'r', '\0', '\0', '\0', '\0', '\0'}, {'M', 'o', 'd', 'u', 'l', 'e', 'N', 'o', 't', 'F', 'o', 'u', 'n', 'd', 'E', 'r', 'r', 'o', 'r', }, {'N', 'o', 't', 'I', 'm', 'p', 'l', 'e', 'm', 'e', 'n', 't', 'e', 'd', 'E', 'r', 'r', 'o', 'r', }, {'P', 'y', 't', 'h', 'o', 'n', 'F', 'i', 'n', 'a', 'l', 'i', 'z', 'a', 't', 'i', 'o', 'n', 'E', 'r', 'r', 'o', 'r', }, {'R', 'e', 'c', 'u', 'r', 's', 'i', 'o', 'n', 'E', 'r', 'r', 'o', 'r', }, {'U', 'n', 'b', 'o', 'u', 'n', 'd', 'L', 'o', 'c', 'a', 'l', 'E', 'r', 'r', 'o', 'r', }, {'U', 'n', 'i', 'c', 'o', 'd', 'e', 'E', 'r', 'r', 'o', 'r', '\0'}, {'B', 'r', 'o', 'k', 'e', 'n', 'P', 'i', 'p', 'e', 'E', 'r', 'r', 'o', 'r', }, {'C', 'o', 'n', 'n', 'e', 'c', 't', 'i', 'o', 'n', 'A', 'b', 'o', 'r', 't', 'e', 'd', 'E', 'r', 'r', 'o', 'r', }, {'C', 'o', 'n', 'n', 'e', 'c', 't', 'i', 'o', 'n', 'R', 'e', 'f', 'u', 's', 'e', 'd', 'E', 'r', 'r', 'o', 'r', }, {'C', 'o', 'n', 'n', 'e', 'c', 't', 'i', 'o', 'n', 'R', 'e', 's', 'e', 't', 'E', 'r', 'r', 'o', 'r', }, {'T', 'a', 'b', 'E', 'r', 'r', 'o', 'r', '\0', '\0', '\0', '\0', '\0'}, {'U', 'n', 'i', 'c', 'o', 'd', 'e', 'D', 'e', 'c', 'o', 'd', 'e', 'E', 'r', 'r', 'o', 'r', }, {'e', 'n', 'c', 'o', 'd', 'i', 'n', 'g', '\0', '\0', '\0', '\0', '\0'}, {'e', 'n', 'd', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'r', 'e', 'a', 's', 'o', 'n', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'U', 'n', 'i', 'c', 'o', 'd', 'e', 'E', 'n', 'c', 'o', 'd', 'e', 'E', 'r', 'r', 'o', 'r', }, {'U', 'n', 'i', 'c', 'o', 'd', 'e', 'T', 'r', 'a', 'n', 's', 'l', 'a', 't', 'e', 'E', 'r', 'r', 'o', 'r', }, {'E', 'x', 'c', 'e', 'p', 't', 'i', 'o', 'n', 'G', 'r', 'o', 'u', 'p', }, {'E', 'n', 'v', 'i', 'r', 'o', 'n', 'm', 'e', 'n', 't', 'E', 'r', 'r', 'o', 'r', }, {'I', 'O', 'E', 'r', 'r', 'o', 'r', '\0', '\0', '\0', '\0', '\0', '\0'}, {'o', 'p', 'e', 'n', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'q', 'u', 'i', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'e', 'o', 'f', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'e', 'x', 'i', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'c', 'o', 'p', 'y', 'r', 'i', 'g', 'h', 't', '\0', '\0', '\0', '\0'}, {'M', 'A', 'X', 'L', 'I', 'N', 'E', 'S', '\0', '\0', '\0', '\0', '\0'}, {'_', 'P', 'r', 'i', 'n', 't', 'e', 'r', '_', '_', 'd', 'a', 't', 'a', }, {'_', 'P', 'r', 'i', 'n', 't', 'e', 'r', '_', '_', 'f', 'i', 'l', 'e', 'n', 'a', 'm', 'e', 's', }, {'_', 'P', 'r', 'i', 'n', 't', 'e', 'r', '_', '_', 'l', 'i', 'n', 'e', 's', }, {'_', 'P', 'r', 'i', 'n', 't', 'e', 'r', '_', '_', 'n', 'a', 'm', 'e', }, {'_', 'P', 'r', 'i', 'n', 't', 'e', 'r', '_', '_', 's', 'e', 't', 'u', 'p', }, {'c', 'r', 'e', 'd', 'i', 't', 's', '\0', '\0', '\0', '\0', '\0', '\0'}, {'l', 'i', 'c', 'e', 'n', 's', 'e', '\0', '\0', '\0', '\0', '\0', '\0'}, {'h', 'e', 'l', 'p', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}, {'e', 'x', 'e', 'c', 'f', 'i', 'l', 'e', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'b', 'u', 'i', 'l', 't', 'i', 'n', 's', '_', '_', '\0'}, {'_', '_', 'c', 'l', 'o', 's', 'u', 'r', 'e', '_', '_', '\0', '\0'}, {'_', '_', 'c', 'o', 'd', 'e', '_', '_', '\0', '\0', '\0', '\0', '\0'}, {'_', '_', 'd', 'e', 'f', 'a', 'u', 'l', 't', 's', '_', '_', '\0'}, {'_', '_', 'g', 'l', 'o', 'b', 'a', 'l', 's', '_', '_', '\0', '\0'}, {'_', '_', 'k', 'w', 'd', 'e', 'f', 'a', 'u', 'l', 't', 's', '_', '_', }, {'r', 'u', 'n', 'f', 'i', 'l', 'e', '\0', '\0', '\0', '\0', '\0', '\0'}};
    char status_keywords[KEYWORDS_NUM] = {0};
    char inconsistency[KEYWORDS_NUM] = {0};

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

                } else if (keycode == 58 || keycode == 42) { // shift
                    if (shift_tracking) {
                        if (libinput_event_keyboard_get_key_state(keyboard_event) == LIBINPUT_KEY_STATE_PRESSED) {
                                shift = 1;
                            } else {
                                shift = 0;
                            }
                        }

                } else if (libinput_event_keyboard_get_key_state(keyboard_event) == LIBINPUT_KEY_STATE_RELEASED) {
                    if (caps_lock || shift) {
                        key_ascii = capitalized_convert_to_ascii(keycode, shift);
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