// Copyright (c) 2020 MattKC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifdef _WIN32
#include <windows.h>
#else
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <stdarg.h>
#include <string.h>
#endif

#ifdef _WIN32
#define GAME_NAME           "Snake"
#else
#define GAME_NAME           "X11 Snake"
#endif
#define WINDOW_WIDTH        800
#define WINDOW_HEIGHT       600
#define TILE_SIZE           40
#define HORIZONTAL_TILES    WINDOW_WIDTH / TILE_SIZE
#define VERTICAL_TILES      WINDOW_HEIGHT / TILE_SIZE
#define MAX_TILE_COUNT      HORIZONTAL_TILES * VERTICAL_TILES
#define START_X             HORIZONTAL_TILES / 2
#define START_Y             VERTICAL_TILES / 2
#define SNAKE_PADDING       TILE_SIZE / 4
#define SNAKE_SIZE          TILE_SIZE - SNAKE_PADDING - SNAKE_PADDING
#define FOOD_PADDING        TILE_SIZE / 8
#define FOOD_SIZE           TILE_SIZE - FOOD_PADDING - FOOD_PADDING
#define MAX_DIR_QUEUE       8

struct Position {
    int x;
    int y;
};

const char* window_name = "Snake";
struct Position* snake_pos;
struct Position food_pos;
int snake_len = 1;
int player_dir = 0; // 0 = paused, -1 = game over, -2 = game won
int* dir_queue;
int dir_queue_sz = 0;
int dir_queue_read = 0;
bool forgiveness = false;

bool PosEqual(struct Position* p1, struct Position* p2)
{
    return p1->x == p2->x && p1->y == p2->y;
}

bool PosExists(struct Position* p)
{
    for (int i=0;i<snake_len;i++) {
        if (PosEqual(&snake_pos[i], p)) {
            return true;
        }
    }

    return false;
}

void SetFood()
{
    do {
        food_pos.x = rand() % HORIZONTAL_TILES;
        food_pos.y = rand() % VERTICAL_TILES;
    } while (PosExists(&food_pos));
}

#ifdef _WIN32

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ctx;

        HDC device = BeginPaint(hwnd, &ctx);

        SelectObject(device, GetStockObject(NULL_PEN));

        struct Position food_pos_px;

        food_pos_px.x = food_pos.x * TILE_SIZE + FOOD_PADDING;
        food_pos_px.y = food_pos.y * TILE_SIZE + FOOD_PADDING;

        Rectangle(
            device,
            food_pos_px.x,
            food_pos_px.y,
            food_pos_px.x + FOOD_SIZE, 
            food_pos_px.y + FOOD_SIZE);

        if (player_dir == -1) {
            SelectObject(device, CreateSolidBrush(0x0000FF));
        } else if (player_dir == -2) {
            SelectObject(device, CreateSolidBrush(0x00FF00));
        }

        struct Position top_left;
        struct Position bottom_right;
        
        RECT draw_rect;

        for (int i=0; i<snake_len; i++) {
            top_left = snake_pos[i];
            bottom_right = snake_pos[i];

            if (i > 0) {
                top_left.x = min(top_left.x, snake_pos[i-1].x);
                top_left.y = min(top_left.y, snake_pos[i-1].y);
                bottom_right.x = max(bottom_right.x, snake_pos[i-1].x);
                bottom_right.y = max(bottom_right.y, snake_pos[i-1].y);
            }

            draw_rect.left = top_left.x*TILE_SIZE + SNAKE_PADDING;
            draw_rect.top = top_left.y*TILE_SIZE + SNAKE_PADDING;
            draw_rect.right = (1+bottom_right.x)*TILE_SIZE - SNAKE_PADDING;
            draw_rect.bottom = (1+bottom_right.y)*TILE_SIZE - SNAKE_PADDING;

            if (top_left.x == 0 && bottom_right.x == HORIZONTAL_TILES-1) {
                // Exception for wrapping around the X axis
                Rectangle(
                    device,
                    0,
                    draw_rect.top,
                    TILE_SIZE - SNAKE_PADDING,
                    draw_rect.bottom);

                draw_rect.left = WINDOW_WIDTH - TILE_SIZE + SNAKE_PADDING;
                draw_rect.right = WINDOW_WIDTH;
            } else if (top_left.y == 0 && bottom_right.y == VERTICAL_TILES-1) {
                // Exception for wrapping around the Y axis
                Rectangle(
                    device,
                    draw_rect.left,
                    0,
                    draw_rect.right,
                    TILE_SIZE - SNAKE_PADDING);

                draw_rect.top = WINDOW_HEIGHT - TILE_SIZE + SNAKE_PADDING;
                draw_rect.bottom = WINDOW_HEIGHT;
            }

            // Draw a long rectangle from the previous position to this one
            Rectangle(
                device,
                draw_rect.left,
                draw_rect.top,
                draw_rect.right,
                draw_rect.bottom);
        }

        EndPaint(hwnd, &ctx);
        break;
    }
    case WM_KEYDOWN:
    {
        if (player_dir < 0) {
            break;
        }
        switch (wParam) {
        case VK_LEFT:
        case VK_UP:
        case VK_RIGHT:
        case VK_DOWN:
            dir_queue[dir_queue_sz%MAX_DIR_QUEUE] = wParam;
            dir_queue_sz++;
            break;
        case 0x50: // Pause
        case VK_PAUSE: // Pause
            player_dir = 0;
            dir_queue_read = dir_queue_sz;
            break;
        }
        break;
    }
    case WM_TIMER:
    {
        while (dir_queue_read < dir_queue_sz) {
            int proposed_dir = dir_queue[dir_queue_read%MAX_DIR_QUEUE];
            dir_queue_read++;

            if (player_dir != proposed_dir
                && proposed_dir+2 != player_dir
                && proposed_dir-2 != player_dir) {
                player_dir = proposed_dir;
                break;
            }
        }

        struct Position new_pos = snake_pos[0];
        BOOL moved = TRUE;

        switch (player_dir) {
        case VK_LEFT:
            new_pos.x--;
            break;
        case VK_RIGHT:
            new_pos.x++;
            break;
        case VK_UP:
            new_pos.y--;
            break;
        case VK_DOWN:
            new_pos.y++;
            break;
        default:
            moved = FALSE;
        }

        if (moved) {
            if (new_pos.x < 0) {
                new_pos.x = HORIZONTAL_TILES-1;
            } else if (new_pos.x == HORIZONTAL_TILES) {
                new_pos.x = 0;
            } else if (new_pos.y < 0) {
                new_pos.y = VERTICAL_TILES-1;
            } else if (new_pos.y == VERTICAL_TILES) {
                new_pos.y = 0;
            }

            snake_len--;
            BOOL collided = PosExists(&new_pos);
            snake_len++;

            if (collided) { // Check for collision
                if (forgiveness) {
                    player_dir = -1;
                    dir_queue_read = dir_queue_sz;
                    InvalidateRect(hwnd, 0, 1);
                } else {
                    forgiveness = TRUE;
                }
            } else {
                BOOL ate_food = PosEqual(&new_pos, &food_pos);

                if (ate_food) {
                    // Ate food, increase snake size
                    snake_len++;
                }

                forgiveness = FALSE;

                for (int i=snake_len-1; i>0; i--) {
                    snake_pos[i] = snake_pos[i-1];
                }

                snake_pos[0] = new_pos;

                if (ate_food) {
                    if (snake_len == MAX_TILE_COUNT) {
                        food_pos.x = -1;
                        player_dir = -2;
                        dir_queue_read = dir_queue_sz;
                    } else {
                        SetFood();
                    }
                }

                InvalidateRect(hwnd, 0, 1);
            }
        }
        break;
    }
    case WM_CLOSE:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

int main()
{
    HMODULE instance = GetModuleHandleA(NULL);

    AdjustWindowRect(&window_rect, WINDOW_STYLE, FALSE);

    WNDCLASSEXA window_class;
    window_class.cbSize = 48;
    window_class.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW ;
    window_class.lpfnWndProc = WindowProc;
    window_class.cbClsExtra = 0;
    window_class.cbWndExtra = 0;
    window_class.hInstance = instance;
    window_class.hIcon = LoadIconA(0, IDI_APPLICATION);
    window_class.hCursor = LoadCursorA(0, IDI_APPLICATION);
    window_class.hbrBackground = CreateSolidBrush(0);
    window_class.lpszMenuName = 0;
    window_class.lpszClassName = window_name;
    window_class.hIconSm = 0;

    RegisterClassExA(&window_class);

    // Start game
    HANDLE heap = GetProcessHeap();

    snake_pos = HeapAlloc(heap, 0, sizeof(struct Position)*MAX_TILE_COUNT);
    snake_pos[0].x = START_X;
    snake_pos[0].y = START_Y;

    dir_queue = HeapAlloc(heap, 0, sizeof(struct Position)*MAX_DIR_QUEUE);

    HWND window = CreateWindowExA(
        0,
        window_name,
        window_name,
        WINDOW_STYLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        window_rect.right - window_rect.left,
        window_rect.bottom - window_rect.top,
        0,
        0,
        instance,
        NULL);

    // Don't seem to be necessary? Saves space to remove
    //ShowWindow(window, SW_SHOW);
    //UpdateWindow(window);

    // Set game timer
    SetFood();
    SetTimer(window, 0, 150, 0);

    MSG msg;

    while (GetMessage(&msg, 0, 0, 0)) {
        if (msg.message == WM_QUIT) {
            break;
        } else {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    HeapFree(heap, 0, dir_queue);
    HeapFree(heap, 0, snake_pos);

    //ExitProcess(0);

    return 0;
}

#else

Display *display;
Window window;
GC gc;
XEvent event;
int screen;
struct timespec timer;
bool game_over_displayed = false;

void draw_rectangle(int x, int y, int width, int height, unsigned long color) {
    XSetForeground(display, gc, color);
    XFillRectangle(display, window, gc, x, y, width, height);
}

void reset_game() {
    snake_len = 1;
    snake_pos[0].x = START_X;
    snake_pos[0].y = START_Y;
    player_dir = 0;
    dir_queue_sz = 0;
    dir_queue_read = 0;
    forgiveness = false;
    game_over_displayed = false;
    SetFood();
}

/*
void draw_game_over() {
    if (!game_over_displayed) {
        printf("Game Over! Your final score is: %d\n", snake_len - 1);

        char message[256];
        snprintf(message, sizeof(message), "Game Over! Your final score is: %d\n"
                                           "Press R to restart or Q to quit.", snake_len - 1);

        // Create a simple message box window
        Window msg_window = XCreateSimpleWindow(display, RootWindow(display, screen), 
                                                WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 - 50, 
                                                300, 100, 1, BlackPixel(display, screen), WhitePixel(display, screen));
        XSelectInput(display, msg_window, ExposureMask);
        XMapWindow(display, msg_window);

        // Set window size hints (min and max size)
        XSizeHints *size_hints = XAllocSizeHints();
        size_hints->flags = PMinSize | PMaxSize;
        size_hints->min_width = size_hints->max_width = 300;
        size_hints->min_height = size_hints->max_height = 100;
        XSetWMNormalHints(display, msg_window, size_hints);
        XFree(size_hints);

        // Wait for the window to be exposed
        XEvent event;
        while (1) {
            XNextEvent(display, &event);
            if (event.type == Expose && event.xexpose.window == msg_window) {
                break;
            }
        }

        // Draw the message in the message box window
        XSetForeground(display, gc, BlackPixel(display, screen));
        XSetFont(display, gc, XLoadFont(display, "fixed"));
        XDrawString(display, msg_window, gc, 20, 50, message, strlen(message));

        game_over_displayed = true;
    }
}
*/

void handle_key_press(XKeyEvent *key_event) {
    KeySym key = XLookupKeysym(key_event, 0);
    if (player_dir < 0) { // Game over state
        // XK_R and XK_Q equal 82 and 81 respectively
        // When pressing R and Q the key values are 114 and 113
        // Weird, lets just check for both

        if (key == XK_R || key == 114) {
            reset_game();
        } else if (key == XK_Q || key == 113) {
            XCloseDisplay(display);
            exit(0);
        }
        return;
    }
    switch (key) {
    case XK_Left:
    case XK_Up:
    case XK_Right:
    case XK_Down:
        dir_queue[dir_queue_sz % MAX_DIR_QUEUE] = key;
        dir_queue_sz++;
        break;
    case XK_P:
    case XK_Pause:
        player_dir = 0;
        dir_queue_read = dir_queue_sz;
        break;
    }
}

void update_game() {
    if (player_dir < 0 && !game_over_displayed) {
        // draw_game_over();
        printf("Game Over! Your final score is: %d\n", snake_len - 1);
        printf("Press R to restart or Q to quit\n");
        game_over_displayed = true;
    }

    while (dir_queue_read < dir_queue_sz) {
        int proposed_dir = dir_queue[dir_queue_read % MAX_DIR_QUEUE];
        dir_queue_read++;

        if (player_dir != proposed_dir
            && proposed_dir + 2 != player_dir
            && proposed_dir - 2 != player_dir) {
            player_dir = proposed_dir;
            break;
        }
    }

    struct Position new_pos = snake_pos[0];
    bool moved = true;

    switch (player_dir) {
    case XK_Left:
        new_pos.x--;
        break;
    case XK_Right:
        new_pos.x++;
        break;
    case XK_Up:
        new_pos.y--;
        break;
    case XK_Down:
        new_pos.y++;
        break;
    default:
        moved = false;
    }

    if (moved) {
        if (new_pos.x < 0) {
            new_pos.x = HORIZONTAL_TILES - 1;
        } else if (new_pos.x == HORIZONTAL_TILES) {
            new_pos.x = 0;
        } else if (new_pos.y < 0) {
            new_pos.y = VERTICAL_TILES - 1;
        } else if (new_pos.y == VERTICAL_TILES) {
            new_pos.y = 0;
        }

        snake_len--;
        bool collided = PosExists(&new_pos);
        snake_len++;

        if (collided) { // Check for collision
            if (forgiveness) {
                player_dir = -1;
                dir_queue_read = dir_queue_sz;
            } else {
                forgiveness = true;
            }
        } else {
            bool ate_food = PosEqual(&new_pos, &food_pos);

            if (ate_food) {
                // Ate food, increase snake size
                snake_len++;
            }

            forgiveness = false;

            for (int i = snake_len - 1; i > 0; i--) {
                snake_pos[i] = snake_pos[i - 1];
            }

            snake_pos[0] = new_pos;

            if (ate_food) {
                if (snake_len == MAX_TILE_COUNT) {
                    food_pos.x = -1;
                    player_dir = -2;
                    dir_queue_read = dir_queue_sz;
                } else {
                    SetFood();
                }
            }
        }
    }
}

void draw_game() {
    XClearWindow(display, window);

    struct Position food_pos_px;
    food_pos_px.x = food_pos.x * TILE_SIZE + FOOD_PADDING;
    food_pos_px.y = food_pos.y * TILE_SIZE + FOOD_PADDING;

    draw_rectangle(food_pos_px.x, food_pos_px.y, FOOD_SIZE, FOOD_SIZE, 0xFF0000);

    if (player_dir == -1) {
        XSetForeground(display, gc, 0x0000FF);
    } else if (player_dir == -2) {
        XSetForeground(display, gc, 0x00FF00);
    }

    struct Position top_left;
    struct Position bottom_right;

    for (int i = 0; i < snake_len; i++) {
        top_left = snake_pos[i];
        bottom_right = snake_pos[i];

        if (i > 0) {
            top_left.x = top_left.x < snake_pos[i - 1].x ? top_left.x : snake_pos[i - 1].x;
            top_left.y = top_left.y < snake_pos[i - 1].y ? top_left.y : snake_pos[i - 1].y;
            bottom_right.x = bottom_right.x > snake_pos[i - 1].x ? bottom_right.x : snake_pos[i - 1].x;
            bottom_right.y = bottom_right.y > snake_pos[i - 1].y ? bottom_right.y : snake_pos[i - 1].y;
        }

        int left = top_left.x * TILE_SIZE + SNAKE_PADDING;
        int top = top_left.y * TILE_SIZE + SNAKE_PADDING;
        int right = (1 + bottom_right.x) * TILE_SIZE - SNAKE_PADDING;
        int bottom = (1 + bottom_right.y) * TILE_SIZE - SNAKE_PADDING;

        if (top_left.x == 0 && bottom_right.x == HORIZONTAL_TILES - 1) {
            // Exception for wrapping around the X axis
            draw_rectangle(0, top, TILE_SIZE - SNAKE_PADDING, bottom - top, 0x00FF00);

            left = WINDOW_WIDTH - TILE_SIZE + SNAKE_PADDING;
            right = WINDOW_WIDTH;
        } else if (top_left.y == 0 && bottom_right.y == VERTICAL_TILES - 1) {
            // Exception for wrapping around the Y axis
            draw_rectangle(left, 0, right - left, TILE_SIZE - SNAKE_PADDING, 0x00FF00);

            top = WINDOW_HEIGHT - TILE_SIZE + SNAKE_PADDING;
            bottom = WINDOW_HEIGHT;
        }

        // Draw a long rectangle from the previous position to this one
        draw_rectangle(left, top, right - left, bottom - top, 0x00FF00);
    }
}

int main() {
    srand(time(NULL));

    display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "Cannot open display\n");
        exit(1);
    }

    screen = DefaultScreen(display);
    window = XCreateSimpleWindow(display, RootWindow(display, screen), 10, 10, WINDOW_WIDTH, WINDOW_HEIGHT, 1,
                                 WhitePixel(display, screen), BlackPixel(display, screen));
    XSelectInput(display, window, ExposureMask | KeyPressMask);
    XMapWindow(display, window);

    gc = XCreateGC(display, window, 0, NULL);

    // Set window title to Snake
    XStoreName(display, window, GAME_NAME);

    // Set window size hints (min and max size)
    XSizeHints *size_hints = XAllocSizeHints();
    size_hints->flags = PMinSize | PMaxSize; 
    size_hints->min_width = size_hints->max_width = WINDOW_WIDTH; 
    size_hints->min_height = size_hints->max_height = WINDOW_HEIGHT; 
    XSetWMNormalHints(display, window, size_hints); XFree(size_hints);

    // Start game
    snake_pos = malloc(sizeof(struct Position) * MAX_TILE_COUNT);
    snake_pos[0].x = START_X;
    snake_pos[0].y = START_Y;

    dir_queue = malloc(sizeof(int) * MAX_DIR_QUEUE);

    SetFood();

    timer.tv_sec = 0;
    timer.tv_nsec = 150000000;

    while (1) {
        while (XPending(display)) {
            XNextEvent(display, &event);
            if (event.type == Expose) {
                draw_game();
            } else if (event.type == KeyPress) {
                handle_key_press(&event.xkey);
            }
        }

        update_game();
        draw_game();
        nanosleep(&timer, NULL);
    }

    XFreeGC(display, gc);
    XDestroyWindow(display, window);
    XCloseDisplay(display);

    free(dir_queue);
    free(snake_pos);

    return 0;
}

#endif
