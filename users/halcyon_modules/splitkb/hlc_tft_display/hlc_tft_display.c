// Copyright 2024 splitkb.com (support@splitkb.com)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "halcyon.h"
#include "hlc_tft_display.h"

#include "hardware/structs/rosc.h"

// Fonts mono2
#include "graphics/fonts/Retron2000-27.qff.h"
#include "graphics/fonts/Retron2000-underline-27.qff.h"

// Numbers mono2
#include "graphics/numbers/0.qgf.h"
#include "graphics/numbers/1.qgf.h"
#include "graphics/numbers/2.qgf.h"
#include "graphics/numbers/3.qgf.h"
#include "graphics/numbers/4.qgf.h"
#include "graphics/numbers/5.qgf.h"
#include "graphics/numbers/6.qgf.h"
#include "graphics/numbers/7.qgf.h"
#include "graphics/numbers/8.qgf.h"
#include "graphics/numbers/9.qgf.h"
#include "graphics/numbers/undef.qgf.h"

static const char *caps =        "Caps";
static const char *num =         "Num";
static const char *scroll =      "Scroll";

static painter_font_handle_t Retron27;
static painter_font_handle_t Retron27_underline;
static painter_image_handle_t layer_number;

static uint8_t lcd_surface_fb[SURFACE_REQUIRED_BUFFER_BYTE_SIZE(135, 240, 16)];

int color_value = 0;

painter_device_t lcd;
painter_device_t lcd_surface;

led_t last_led_usb_state = {0};
layer_state_t last_layer_state = {0};

#define GRID_WIDTH 27
#define GRID_HEIGHT 48
#define CELL_SIZE 4  // Cell size excluding outline
#define OUTLINE_SIZE 1

// Define the probability factor for initial alive cells
#define INITIAL_ALIVE_PROBABILITY 0.2  // 20% chance of being alive

static bool first_run_led = false;

typedef struct {
    const char *name;
    uint8_t effect_id;    
} rgb_map_t;

static const rgb_map_t rgb_animations [] = {
    {"< Back", 0},
    {"Solid color", RGB_MATRIX_SOLID_COLOR},
    {"Alphas mods", RGB_MATRIX_ALPHAS_MODS },
    {"Gradient up-down", RGB_MATRIX_GRADIENT_UP_DOWN },
    {"Gradient left-right", RGB_MATRIX_GRADIENT_LEFT_RIGHT },
    {"Breathing", RGB_MATRIX_BREATHING },
    {"Band sat", RGB_MATRIX_BAND_SAT },
    {"Band val", RGB_MATRIX_BAND_VAL },
    {"Band pinwheel sat", RGB_MATRIX_BAND_PINWHEEL_SAT },
    {"Band pinwheel val", RGB_MATRIX_BAND_PINWHEEL_VAL },
    {"Band spiral sat", RGB_MATRIX_BAND_SPIRAL_SAT },
    {"Band spiral val", RGB_MATRIX_BAND_SPIRAL_VAL },
    {"Cycle all", RGB_MATRIX_CYCLE_ALL },
    {"Cycle left-right", RGB_MATRIX_CYCLE_LEFT_RIGHT },
    {"Cycle up-down", RGB_MATRIX_CYCLE_UP_DOWN },
    {"Cycle out-in", RGB_MATRIX_CYCLE_OUT_IN },
    {"Cycle out-in dual", RGB_MATRIX_CYCLE_OUT_IN_DUAL },
    {"Rainbow chevron", RGB_MATRIX_RAINBOW_MOVING_CHEVRON },
    {"Cycle pinwheel", RGB_MATRIX_CYCLE_PINWHEEL },
    {"Cycle spiral", RGB_MATRIX_CYCLE_SPIRAL },
    {"Dual beacon", RGB_MATRIX_DUAL_BEACON },
    {"Rainbow beacon", RGB_MATRIX_RAINBOW_BEACON },
    {"Rainbow pinwheels", RGB_MATRIX_RAINBOW_PINWHEELS },
    // {"flower blooming", RGB_MATRIX_FLOWER_BLOOMING },
    {"Raindrops", RGB_MATRIX_RAINDROPS },
    {"Jellybean raindrops", RGB_MATRIX_JELLYBEAN_RAINDROPS },
    {"Hue breathing", RGB_MATRIX_HUE_BREATHING },
    {"Hue pendulum", RGB_MATRIX_HUE_PENDULUM },
    {"Hue wave", RGB_MATRIX_HUE_WAVE },
    {"Pixel fractal", RGB_MATRIX_PIXEL_FRACTAL },
    {"Pixel flow", RGB_MATRIX_PIXEL_FLOW },
    {"Pixel rain", RGB_MATRIX_PIXEL_RAIN },
    {"Typing heatmap", RGB_MATRIX_TYPING_HEATMAP },
    {"Digital rain", RGB_MATRIX_DIGITAL_RAIN },
    {"Solid reactive simple", RGB_MATRIX_SOLID_REACTIVE_SIMPLE },
    {"Solid reactive", RGB_MATRIX_SOLID_REACTIVE },
    {"Solid reactive wide", RGB_MATRIX_SOLID_REACTIVE_WIDE },
    {"Solid reactive multiwide", RGB_MATRIX_SOLID_REACTIVE_MULTIWIDE },
    {"Solid reactive cross", RGB_MATRIX_SOLID_REACTIVE_CROSS },
    {"Solid reactive multicross", RGB_MATRIX_SOLID_REACTIVE_MULTICROSS },
    {"Solid reactive nexus", RGB_MATRIX_SOLID_REACTIVE_NEXUS },
    {"Solid reactive multinexus", RGB_MATRIX_SOLID_REACTIVE_MULTINEXUS },
    {"Splash", RGB_MATRIX_SPLASH },
    {"Multisplash", RGB_MATRIX_MULTISPLASH },
    {"Solid splash", RGB_MATRIX_SOLID_SPLASH },
    {"Solid multisplash", RGB_MATRIX_SOLID_MULTISPLASH }, //RGB_MATRIX_CUSTOM_MULTISPLASH_BG }
    // {"palettefx gradient", RGB_MATRIX_CUSTOM_PALETTEFX_GRADIENT },
    // {"palettefx flow", RGB_MATRIX_CUSTOM_PALETTEFX_FLOW },
    // {"palettefx ripple", RGB_MATRIX_CUSTOM_PALETTEFX_RIPPLE },
    // {"palettefx sparkle", RGB_MATRIX_CUSTOM_PALETTEFX_SPARKLE },
    // {"palettefx vortex", RGB_MATRIX_CUSTOM_PALETTEFX_VORTEX },
    // {"palettefx reactive", RGB_MATRIX_CUSTOM_PALETTEFX_REACTIVE }
    // {"starlight", RGB_MATRIX_STARLIGHT },
    // {"starlight smooth", RGB_MATRIX_STARLIGHT_SMOOTH },
    // {"starlight dual hue", RGB_MATRIX_STARLIGHT_DUAL_HUE },
    // {"starlight dual sat", RGB_MATRIX_STARLIGHT_DUAL_SAT },
    // {"riverflow", RGB_MATRIX_RIVERFLOW },
    // {"effect max", RGB_MATRIX_EFFECT_MAX }
};
typedef enum {
    MENU_OFF = 0,
    MENU_MAIN,
    MENU_RGB,
    MENU_RGB_ANIM,
    MENU_RGB_COLOR,
    MENU_RGB_COLOR_PALETTE,
    MENU_RGB_COLOR_DIAL,
    MENU_RGB_SPEED_DIAL,
    MENU_LEFT_ANIM,
    MENU_RIGHT_ANIM,
    MENU_DIALOG
} menu_state_t;

typedef struct {
    menu_state_t menu;
    menu_state_t parent;
} menu_parent_map_t;

const menu_parent_map_t menu_parents[] = {
    {MENU_MAIN,              MENU_OFF},
    {MENU_RGB,               MENU_MAIN},

    {MENU_RGB_ANIM,          MENU_RGB},
    {MENU_RGB_COLOR,         MENU_RGB},
    {MENU_RGB_SPEED_DIAL,    MENU_RGB},

    {MENU_RGB_COLOR_PALETTE, MENU_RGB_COLOR},
    {MENU_RGB_COLOR_DIAL,    MENU_RGB_COLOR},

    {MENU_LEFT_ANIM,         MENU_MAIN},
    {MENU_RIGHT_ANIM,        MENU_MAIN},

    {MENU_DIALOG,            MENU_MAIN},
};

menu_state_t get_parent_menu(menu_state_t menu) {
    for (uint8_t i = 0; i < ARRAY_SIZE(menu_parents); i++) {
        if (menu_parents[i].menu == menu) {
            return menu_parents[i].parent;
        }
    }

    return MENU_OFF;
}

typedef struct {
    const char *text;
    uint8_t id;
    menu_state_t menu;
    menu_state_t dest_menu;
} menu_item_map_t;


static const menu_item_map_t menu_items [] = {    
    {"Exit", 0, MENU_MAIN, MENU_MAIN},
    {"Screen", 1, MENU_MAIN, MENU_LEFT_ANIM },
    {"RGB", 2, MENU_MAIN, MENU_RGB },
    {"Save", 3, MENU_MAIN, MENU_MAIN }, 
    
    {"< Back", 0, MENU_RGB, MENU_MAIN},
    {"Animation", 1, MENU_RGB, MENU_RGB_ANIM},
    {"Color", 2, MENU_RGB, MENU_RGB_COLOR},

    {"", -1, MENU_RGB_ANIM, MENU_RGB}, //animations array

    {"< Back", 0, MENU_RGB_COLOR, MENU_RGB},
    {"Palette", 1, MENU_RGB_COLOR, MENU_RGB_COLOR_PALETTE},    
    {"Dial", 2, MENU_RGB_COLOR, MENU_RGB_COLOR_DIAL},

    {"< Back", 0, MENU_RGB_COLOR_PALETTE, MENU_RGB_COLOR},
    {"PALETTE", 1, MENU_RGB_COLOR_PALETTE, MENU_RGB_COLOR},
    {"DIAL", 1, MENU_RGB_COLOR_DIAL, MENU_RGB_COLOR}
};

//______________________________________________________________________________________________
//MENU SPECIFIC CONTROLS
//______________________________________________________________________________________________
static menu_state_t g_state = MENU_OFF;
static menu_state_t last_state = MENU_OFF;
static uint8_t g_index = 0;
static uint8_t last_g_index = 0;
static uint32_t menu_visible_time;

//horizontal control for scrolling and viewport
static uint32_t g_text_position_timer = 0;
static uint8_t g_text_first_idx = 0;
static int wait_before_scroll = 0;
uint8_t vp_start = 0;
uint8_t vp_end = 8; //TODO: change VP_END in favor of global maxchars
uint8_t vp_index_incr = 0;

HSV get_menu_color(int item_index) {
    if (item_index == g_index) {
        return (HSV){HSV_CAPS_ON};
    }
    return (HSV){HSV_CAPS_OFF};
}

#define MENU_COLOR(index) \
    get_menu_color(index).h, \
    get_menu_color(index).s, \
    get_menu_color(index).v


void update_viewport(void) {
    uint8_t by_how_much = 0;
    uint8_t array_size = 0;

    if (g_state == MENU_RGB_ANIM) {
        array_size = ARRAY_SIZE(rgb_animations);
    } else if (g_state == MENU_RGB_COLOR_PALETTE) {
        array_size = 0; //TODO: size of palettes
    }

    if (g_index > vp_end) {
        by_how_much = g_index - 8;
        vp_start = by_how_much;
        vp_end = 8 + by_how_much;
        if (vp_end > array_size) {
            vp_end = array_size;
        }
        vp_index_incr = by_how_much;        
        return;
    } 

    //CURRENT_INDEX IS LOWER THAN THECURRENT VIEWPORT
    if (g_index < vp_start) {
        by_how_much = vp_start - g_index;

        vp_start -= by_how_much;
        vp_end -= by_how_much;

        vp_index_incr -= by_how_much;
        // uprintf("g_index: %u, vp_start: %u, vp_end: %u, by_how_much: %u, vp_index_incr: %u \n", g_index, vp_start, vp_end, by_how_much, vp_index_incr);    
    }
}

uint8_t draw_item(const char* text, int item_id, uint8_t line, bool multiline) {
    uint8_t max_width = 9;//get_oled_limit('c');
    uint8_t text_length = strlen(text);
    char buf[10];

    bool is_selected = item_id == g_index;

    if (text_length <= max_width) {
        uint16_t line_height = line * Retron27->line_height;

        //clear background
        qp_rect(
            lcd_surface,
            0,
            line_height,
            LCD_WIDTH - 1,
            line_height + Retron27->line_height - 1,
            HSV_BLACK,
            true
        );

        qp_drawtext_recolor(lcd_surface, 5, line * Retron27->line_height, Retron27, text,  MENU_COLOR(item_id),  HSV_BLACK); 

    } else {
        //is the selected item
        if (is_selected) {
            //read it the first time            
            if (g_text_position_timer == 0) {
                g_text_position_timer = timer_read32();
                // g_text_first_idx = 0;
                // wait_before_scroll = 1000;
            } else if (g_text_position_timer == -1) {
                //means that i have finished looking to the timer
                return ++line;
            }

            if (timer_elapsed32(g_text_position_timer) > wait_before_scroll) {
                g_text_position_timer = timer_read32();
                wait_before_scroll = 400;

                if (g_text_first_idx + max_width > text_length) {
                     //set the timer to a suspended state
                    g_text_position_timer = -1;
                    return ++line;
                } else {
                    //advance to the next horizontal char
                    g_text_first_idx++;
                }
            } else {
                return ++line;
            }

            text += g_text_first_idx;
        }

        snprintf(buf, sizeof(buf), "%-*.*s", max_width, max_width, text);
        qp_drawtext_recolor(lcd_surface, 5, line * Retron27->line_height, Retron27, buf,  MENU_COLOR(item_id),  HSV_BLACK);        
    }
    

    return ++line;
}

void draw_menu(void) {
    //MAX 8 LINES
    //its only rendered when the menu is not MENU_OFF

    //ignore render if there are no changes pending (index change etc.)
    if (last_state == g_state && last_g_index == g_index && g_text_position_timer == 0) {
        return;
    }   

    if (last_state != g_state) {
        //changed menu page, clear the lcd surface 
        qp_rect(
            lcd_surface,
            0,
            0,
            LCD_WIDTH - 1,
            LCD_HEIGHT - 1,
            HSV_BLACK,
            true
        );
    }

    if (last_g_index != g_index) {
        //changed selected item        
        g_text_position_timer = 0;
        g_text_first_idx = 0;
        wait_before_scroll = 1000;
    }

    // int index_incr = 0;
    //draw the "go back" element only if the start viewport is 0
    // if (vp_start == 0) {
    //     index_incr++;
    //     draw_item(
    //         g_state == MENU_MAIN ? "Exit" : "< Back",
    //         0,
    //         0,
    //         false 
    //     );
    // }
    
    switch (g_state) {
        case MENU_RGB_ANIM: {

            if (g_text_position_timer > 0 && last_g_index == g_index) {
                //index has not changed but the timer for horizontal scroll is running
                //redraw the selected item

                //calculate the item line
                int element_idx = (g_index - vp_start);

                draw_item(
                    rgb_animations[g_index].name,
                    g_index,
                    element_idx,
                    false
                );
            } else {
                //rendering the entire screen
                for (int idx = vp_start, line = 0; idx < vp_end ; idx++, line++) {
                    draw_item(
                        rgb_animations[idx].name,
                        idx,
                        line,
                        false
                    );
                }
                
            }

            break;
        }
        default: {  
            int rendered_items = 1;
            for (int i = 0; i < ARRAY_SIZE(menu_items); i++) {
                if(menu_items[i].menu != g_state) {
                    continue;
                }
                
                draw_item(
                    menu_items[i].text,
                    menu_items[i].id,
                    rendered_items,
                    false
                ); 
                 
                rendered_items++;

                // qp_drawtext_recolor(lcd_surface, 5, height, Retron27, menu_items[i].text, MENU_COLOR(menu_items[i].id), HSV_BLACK);
            }
            break;
        }
    }

 
    last_state = g_state;
    last_g_index = g_index;    
}


//______________________________________________________________________________________________
//______________________________________________________________________________________________



bool grid[GRID_HEIGHT][GRID_WIDTH];  // Current state
bool new_grid[GRID_HEIGHT][GRID_WIDTH];  // Next state
bool changed_grid[GRID_HEIGHT][GRID_WIDTH]; // Tracks changed cells

bool display_module_menu_is_open(void) {
    return g_state != MENU_OFF;
}

uint32_t get_random_32bit(void) {
    uint32_t random_value = 0;
    for (int i = 0; i < 32; i++) {
        wait_ms(1);
        random_value = (random_value << 1) | (rosc_hw->randombit & 1);
    }
    return random_value;
}

void init_grid() {
    // Initialize grid with alive cells
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            grid[y][x] = (rand() < INITIAL_ALIVE_PROBABILITY * RAND_MAX);  // Use probability factor
            changed_grid[y][x] = true;      // Mark all as changed initially
        }
    }
}

void draw_grid() {
    uint8_t hue = 0;  // Hue for alive cells
    uint8_t sat = 0;  // Saturation for alive cells
    uint8_t val_dead = 0;  // Brightness for dead cells

    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            if (changed_grid[y][x]) { // Only update changed cells
                uint16_t left = x * (CELL_SIZE + OUTLINE_SIZE);
                uint16_t top = y * (CELL_SIZE + OUTLINE_SIZE);
                uint16_t right = left + CELL_SIZE + OUTLINE_SIZE;
                uint16_t bottom = top + CELL_SIZE + OUTLINE_SIZE;

                // Draw the outline
                qp_rect(lcd_surface, left, top, right, bottom, hue, sat, val_dead, true);

                // Draw the filled cell inside the outline if it's alive
                if (grid[y][x]) {
                    switch (color_value) {
                    case 0:
                        qp_rect(lcd_surface, left + OUTLINE_SIZE, top + OUTLINE_SIZE, right - OUTLINE_SIZE, bottom - OUTLINE_SIZE, HSV_LAYER_0, true);
                        break;
                    case 1:
                        qp_rect(lcd_surface, left + OUTLINE_SIZE, top + OUTLINE_SIZE, right - OUTLINE_SIZE, bottom - OUTLINE_SIZE, HSV_LAYER_1, true);
                        break;
                    case 2:
                        qp_rect(lcd_surface, left + OUTLINE_SIZE, top + OUTLINE_SIZE, right - OUTLINE_SIZE, bottom - OUTLINE_SIZE, HSV_LAYER_2, true);
                        break;
                    case 3:
                        qp_rect(lcd_surface, left + OUTLINE_SIZE, top + OUTLINE_SIZE, right - OUTLINE_SIZE, bottom - OUTLINE_SIZE, HSV_LAYER_3, true);
                        break;
                    case 4:
                        qp_rect(lcd_surface, left + OUTLINE_SIZE, top + OUTLINE_SIZE, right - OUTLINE_SIZE, bottom - OUTLINE_SIZE, HSV_LAYER_4, true);
                        break;
                    case 5:
                        qp_rect(lcd_surface, left + OUTLINE_SIZE, top + OUTLINE_SIZE, right - OUTLINE_SIZE, bottom - OUTLINE_SIZE, HSV_LAYER_5, true);
                        break;
                    case 6:
                        qp_rect(lcd_surface, left + OUTLINE_SIZE, top + OUTLINE_SIZE, right - OUTLINE_SIZE, bottom - OUTLINE_SIZE, HSV_LAYER_6, true);
                        break;
                    case 7:
                        qp_rect(lcd_surface, left + OUTLINE_SIZE, top + OUTLINE_SIZE, right - OUTLINE_SIZE, bottom - OUTLINE_SIZE, HSV_LAYER_7, true);
                        break;
                    default:
                        qp_rect(lcd_surface, left + OUTLINE_SIZE, top + OUTLINE_SIZE, right - OUTLINE_SIZE, bottom - OUTLINE_SIZE, HSV_LAYER_UNDEF, true);
                    }
                }
            }
        }
    }
}

void update_grid() {
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            int alive_neighbors = 0;

            // Count alive neighbors
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    if (dy == 0 && dx == 0) continue;  // Skip the current cell
                    int ny = y + dy;
                    int nx = x + dx;
                    if (ny >= 0 && ny < GRID_HEIGHT && nx >= 0 && nx < GRID_WIDTH) {
                        alive_neighbors += grid[ny][nx];
                    }
                }
            }

            // Apply the rules of the Game of Life
            if (grid[y][x]) {
                // Any live cell with two or three live neighbours survives.
                new_grid[y][x] = (alive_neighbors == 2 || alive_neighbors == 3);
            } else {
                // Any dead cell with exactly three live neighbours becomes a live cell.
                new_grid[y][x] = (alive_neighbors == 3);
            }

            // Track changed cells
            changed_grid[y][x] = (grid[y][x] != new_grid[y][x]);
        }
    }

    // Copy new grid state to current grid
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            grid[y][x] = new_grid[y][x];
        }
    }
}

void display_module_menu_open(void){
    menu_visible_time = timer_read32();
    g_state = MENU_MAIN;    
    g_index = 0;
}

void display_module_menu_down(void) {
    menu_visible_time = timer_read32();
    // g_text_position_timer = timer_read32();
    //get max id available for the current menu
    int max = 0;
    if (g_state == MENU_RGB_ANIM) {
        max = ARRAY_SIZE(rgb_animations) + 1; //Forse manca l'id zero
    } else {
        for (int i = 0; i < ARRAY_SIZE(menu_items); i++) {
            if(menu_items[i].menu == g_state) {
                if (menu_items[i].id > max) {
                    max = menu_items[i].id;
                }
            }
        }
    }

    if (g_index < max) {
        g_index++;        
    }

    update_viewport();
}

void display_module_menu_up(void) {
    menu_visible_time = timer_read32();
    // g_text_position_timer = timer_read32();

    if (g_index > 0) {
        g_index--;
    }
    update_viewport();

    g_text_first_idx = 0;
}

void display_module_menu_enter(void) {
    menu_visible_time = timer_read32();
    // g_text_position_timer = timer_read32();

    //if back or exit condition
    if (g_index == 0) {
        g_state = get_parent_menu(g_state);
        g_index = 0;

        if (g_state == MENU_OFF) {
            display_module_menu_close();
        }
        return;
    }

    if (g_state == MENU_RGB_ANIM) {
        //index -1 because zero should be "GO BACK"
        rgb_matrix_mode_noeeprom(rgb_animations[g_index -1].effect_id); 
    } else {        
        for (int i = 0; i < ARRAY_SIZE(menu_items); i++) {
            if(menu_items[i].menu == g_state && menu_items[i].id == g_index) {
                g_state = menu_items[i].dest_menu;
                g_index = 0;
                return;
            }        
        }
    }

}

void display_module_menu_close(void) {
    //cleanup and close
    menu_visible_time = 0;
    g_state = MENU_OFF;
    last_state = MENU_OFF;
    last_g_index = 0;

    qp_rect(
        lcd_surface,
        0,
        0,
        LCD_WIDTH - 1,
        LCD_HEIGHT - 1,
        HSV_BLACK,
        true
    );

    //force refresh entire screen
    first_run_led = false;
    update_display();
}


// Function to add a cluster of cells at a random position
void add_cell_cluster() {
    int cluster_size = 3;  // Size of the cluster (3x3)
    int x = rand() % (GRID_WIDTH - cluster_size);
    int y = rand() % (GRID_HEIGHT - cluster_size);

    for (int dy = 0; dy < cluster_size; dy++) {
        for (int dx = 0; dx < cluster_size; dx++) {
            bool is_alive = rand() % 2; // Randomly choose between 0 and 1
            grid[y + dy][x + dx] = is_alive;  // Set the cell to be alive
            changed_grid[y + dy][x + dx] = true; // Mark the cell as changed
        }
    }
}

void update_display(void) {    
    static bool first_run_layer = false;

    if( first_run_layer == false) {
        // Load fonts
        Retron27 = qp_load_font_mem(font_Retron2000_27);
        Retron27_underline = qp_load_font_mem(font_Retron2000_underline_27);
    }
    if (display_module_menu_is_open()) {
        if (menu_visible_time > 0){
            if (timer_elapsed32(menu_visible_time) > 30000) {                
                display_module_menu_close();
                return;
            }
        }        
        draw_menu();
    } else if(last_led_usb_state.raw != host_keyboard_led_state().raw || first_run_led == false) {
        // draw the locks state

        led_t led_usb_state = host_keyboard_led_state();

        led_usb_state.caps_lock   ? qp_drawtext_recolor(lcd_surface, 5, LCD_HEIGHT - Retron27->line_height * 3 - 15, Retron27_underline, caps,   HSV_CAPS_ON,   HSV_BLACK) : qp_drawtext_recolor(lcd_surface, 5, LCD_HEIGHT - Retron27->line_height * 3 - 15, Retron27, caps,   HSV_CAPS_OFF,   HSV_BLACK);
        led_usb_state.num_lock    ? qp_drawtext_recolor(lcd_surface, 5, LCD_HEIGHT - Retron27->line_height * 2 - 10, Retron27_underline, num,    HSV_NUM_ON,    HSV_BLACK) : qp_drawtext_recolor(lcd_surface, 5, LCD_HEIGHT - Retron27->line_height * 2 - 10, Retron27, num,    HSV_NUM_OFF,    HSV_BLACK);
        led_usb_state.scroll_lock ? qp_drawtext_recolor(lcd_surface, 5, LCD_HEIGHT - Retron27->line_height - 5,      Retron27_underline, scroll, HSV_SCROLL_ON, HSV_BLACK) : qp_drawtext_recolor(lcd_surface, 5, LCD_HEIGHT - Retron27->line_height - 5,      Retron27, scroll, HSV_SCROLL_OFF, HSV_BLACK);

        last_led_usb_state = led_usb_state;
        first_run_led = true;
    }

    // draw the current layer square number
    if(!display_module_menu_is_open() && (last_layer_state != layer_state || first_run_layer == false)) {
        switch (get_highest_layer(layer_state|default_layer_state)) {
        case 0:
            layer_number = qp_load_image_mem(gfx_0);
            qp_drawimage_recolor(lcd_surface, 5, 5, layer_number, HSV_LAYER_0, HSV_BLACK);
            break;
        case 1:
            layer_number = qp_load_image_mem(gfx_1);
            qp_drawimage_recolor(lcd_surface, 5, 5, layer_number, HSV_LAYER_1, HSV_BLACK);
            break;
        case 2:
            layer_number = qp_load_image_mem(gfx_2);
            qp_drawimage_recolor(lcd_surface, 5, 5, layer_number, HSV_LAYER_2, HSV_BLACK);
            break;
        case 3:
            layer_number = qp_load_image_mem(gfx_3);
            qp_drawimage_recolor(lcd_surface, 5, 5, layer_number, HSV_LAYER_3, HSV_BLACK);
            break;
        case 4:
            layer_number = qp_load_image_mem(gfx_4);
            qp_drawimage_recolor(lcd_surface, 5, 5, layer_number, HSV_LAYER_4, HSV_BLACK);
            break;
        case 5:
            layer_number = qp_load_image_mem(gfx_5);
            qp_drawimage_recolor(lcd_surface, 5, 5, layer_number, HSV_LAYER_5, HSV_BLACK);
            break;
        case 6:
            layer_number = qp_load_image_mem(gfx_6);
            qp_drawimage_recolor(lcd_surface, 5, 5, layer_number, HSV_LAYER_6, HSV_BLACK);
            break;
        case 7:
            layer_number = qp_load_image_mem(gfx_7);
            qp_drawimage_recolor(lcd_surface, 5, 5, layer_number, HSV_LAYER_7, HSV_BLACK);
            break;
        default:
            layer_number = qp_load_image_mem(gfx_undef);
            qp_drawimage_recolor(lcd_surface, 5, 5, layer_number, HSV_LAYER_UNDEF, HSV_BLACK);
        }
        qp_close_image(layer_number);
        last_layer_state = layer_state;
        first_run_layer = true;
    }
}


// Called from halcyon.c
void module_suspend_power_down_kb(void) {
    qp_power(lcd, false);
}

// Called from halcyon.c
void module_suspend_wakeup_init_kb(void) {
    qp_power(lcd, true);
}

// Called from halcyon.c
bool module_post_init_kb(void) {
    // Turn on backlight
    backlight_enable();

    // Make the devices
    lcd = qp_st7789_make_spi_device(LCD_WIDTH, LCD_HEIGHT, LCD_CS_PIN, LCD_DC_PIN, LCD_RST_PIN, LCD_SPI_DIVISOR, LCD_SPI_MODE);
    lcd_surface = qp_make_rgb565_surface(LCD_WIDTH, LCD_HEIGHT, lcd_surface_fb);

    // Initialise the LCD
    qp_init(lcd, LCD_ROTATION);
    qp_set_viewport_offsets(lcd, LCD_OFFSET_X, LCD_OFFSET_Y);
    qp_clear(lcd);
    qp_rect(lcd, 0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1, HSV_BLACK, true);
    qp_power(lcd, true);
    qp_flush(lcd);

    // Initialise the LCD surface
    qp_init(lcd_surface, LCD_ROTATION);
    qp_rect(lcd_surface, 0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1, HSV_BLACK, true);
    qp_surface_draw(lcd_surface, lcd, 0, 0, 0);
    qp_flush(lcd);

    if(!module_post_init_user()) { return false; }

    return true;
}

// Called from halcyon.c
bool display_module_housekeeping_task_kb(bool second_display) {
    if(!display_module_housekeeping_task_user(second_display)) { return false; }

    if(second_display) {
        static uint32_t last_draw = 0;
        static bool second_display_set = false;
        static uint32_t previous_matrix_activity_time = 0;

        if(!second_display_set) {
            srand(get_random_32bit());
            init_grid();
            color_value = rand() % 8;
            second_display_set = true;
        }

        if (timer_elapsed32(last_draw) >= 100) { // Throttle to 10 fps
            draw_grid();
            update_grid();

            if (previous_matrix_activity_time != last_matrix_activity_time()) {
                color_value = rand() % 8;
                add_cell_cluster();
                previous_matrix_activity_time = last_matrix_activity_time();
            }

            last_draw = timer_read32();
        }
    }

    // Update display information (layers, numlock, etc.)
    if(!second_display) {
        update_display();
    }

    // Move surface to lcd
    qp_surface_draw(lcd_surface, lcd, 0, 0, 0);
    qp_flush(lcd);

    return true;
}
