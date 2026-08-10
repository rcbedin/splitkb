// Copyright 2024 splitkb.com (support@splitkb.com)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

#ifdef HLC_TFT_DISPLAY 
void display_module_menu_open(void);
void display_module_menu_navigate(bool downward);
void display_module_menu_enter(void);
void display_module_menu_back(void);
void display_module_menu_close(void);
bool display_module_menu_is_open(void);
#endif

enum layers {
    _QWERTY = 0,
    _NAV_FN,
    _NUMPAD,
    _DVORAK,
    _COLEMAK_DH,
    _FUNCTION,
    _ADJUST,
};

// Aliases for readability
#define QWERTY   DF(_QWERTY)
#define COLEMAK  DF(_COLEMAK_DH)
#define DVORAK   DF(_DVORAK)

#define NUMPAD   MO(_NUMPAD)
#define NAVFN    MO(_NAV_FN)
#define FKEYS    MO(_FUNCTION)
#define ADJUST   MO(_ADJUST)

#define CTL_ESC  MT(MOD_LCTL, KC_ESC)
#define CTL_QUOT MT(MOD_RCTL, KC_QUOTE)
#define CTL_MINS MT(MOD_RCTL, KC_MINUS)
#define ALT_ENT  MT(MOD_LALT, KC_ENT)
#define CTLSFT_UP LCTL(LSFT(KC_UP))
#define CTLSFT_DN LCTL(LSFT(KC_DOWN))

// Note: LAlt/Enter (ALT_ENT) is not the same thing as the keyboard shortcut Alt+Enter.
// The notation `mod/tap` denotes a key that activates the modifier `mod` when held down, and
// produces the key `tap` when tapped (i.e. pressed and released).

enum combo_events {
    GOTO_BOOT_LEFT,
};

const uint16_t PROGMEM goto_boot_combo_left[] = {
    KC_ESC,
    KC_LSFT,
    COMBO_END
};

combo_t key_combos[] = {
    [GOTO_BOOT_LEFT] = COMBO_ACTION(goto_boot_combo_left),
};

void process_combo_event(uint16_t combo_index, bool pressed) {
    if (
        combo_index == GOTO_BOOT_LEFT &&
        pressed &&
        layer_state_is(_NAV_FN)
    ) {
        //send boot command to the left zkeyboard
        reset_keyboard();
    } 
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {

    #ifdef HLC_TFT_DISPLAY

        if (record->event.pressed) {
            if (keycode == KC_F24 && layer_state_is(_NAV_FN)) {
                display_module_menu_open();
                return false;
            }

            if (display_module_menu_is_open() && layer_state_is(_NAV_FN)) {
                switch (keycode) {
                    case KC_UP: {
                        display_module_menu_navigate(false);                        
                        return false;
                    }
                    case KC_DOWN: {
                        display_module_menu_navigate(true);
                        return false;
                    }
                    case KC_RIGHT: {
                        display_module_menu_enter();
                        return false;
                    }
                    case KC_LEFT: {
                        display_module_menu_back();
                        return false;
                    }
                    default: {
                        break;
                    }
                }            
            }
        }
    #endif




    return true;
}

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
/*
 * Base Layer: QWERTY
 *
 * ,-------------------------------------------.                              ,-------------------------------------------.
 * |  Esc   |   1  |   2  |   3  |   4  |   5  |                              |   6  |   7  |   8  |   9  |   0  |  Bksp  |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |  Tab   |   Q  |   W  |   E  |   R  |   T  |                              |   Y  |   U  |   I  |   O  |   P  |  Esc   |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |LShift  |   A  |   S  |   D  |   F  |   G  |                              |   H  |   J  |   K  |   L  | ;  : |  ' "   |
 * |--------+------+------+------+------+------+-------------.  ,-------------+------+------+------+------+------+--------|
 * |Ctrl    |   Z  |   X  |   C  |   V  |   B  | LY-4 | LY-3 |  |  [{  |  UP  |   N  |   M  | ,  < | . >  | /  ? | ENTER  |
 * `----------------------+------+------+------+------+------|  |------+------+------+------+------+----------------------'
 *                        |Grave | LYR-1| LAlt | Space| LGUI |  | LEFT | DOWN |RIGHT | LY-2 |  -_  |
 *                        |      |      |      |      |      |  |      |      |      |      |      |
 *                        `----------------------------------'  `----------------------------------'
 */
    [_QWERTY] = LAYOUT(
     KC_ESC  , KC_1 ,  KC_2   ,  KC_3  ,   KC_4 ,   KC_5 ,                                           KC_6,  KC_7 ,   KC_8 ,   KC_9 ,   KC_0 ,   KC_BSPC ,
     KC_TAB  , KC_Q ,  KC_W   ,  KC_E  ,   KC_R ,   KC_T ,                                           KC_Y,  KC_U ,   KC_I ,   KC_O ,   KC_P ,   KC_BSLS,
     KC_LSFT , KC_A ,  KC_S   ,  KC_D  ,   KC_F ,   KC_G ,                                           KC_H,  KC_J ,   KC_K ,   KC_L ,   KC_SCLN, KC_QUOTE,
     KC_LCTL , KC_Z ,  KC_X   ,  KC_C  ,   KC_V ,   KC_B , KC_EQL , KC_LBRC,      KC_RBRC, KC_UP   , KC_N,  KC_M ,   KC_COMM, KC_DOT,  KC_SLSH, KC_ENT,
                                KC_GRV ,   NAVFN,   KC_LALT, KC_SPC , KC_LGUI,      KC_LEFT, KC_DOWN , KC_RIGHT, NUMPAD , KC_MINUS
    ),

/*
 * Nav Layer: Media, navigation
 *
 * ,-------------------------------------------.                              ,-------------------------------------------.
 * |        |      |      |      |      |      |                              | mPrev|mPause|mNext |      |      |        |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |        |      |      |      |      |      |                              | vol- | vol+ |      |      |      | del    |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |        |      |      |      | PGUP | HOME |                              |c+s+up|      |      |      |      | ins    |
 * |--------+------+------+------+------+------+-------------.  ,-------------+------+------+------+------+------+--------|
 * |        |      |      |      | PGDN | END  |      |      |  |      | ]}   |c+s+dn|      |      |      |      |        |
 * `----------------------+------+------+------+------+------|  |------+------+------+------+------+----------------------'
 *                        |      |      |      |      |      |  | =+   |      |      |      |      |
 *                        |      |      |      |      |      |  |      |      |      |      |      |
 *                        `----------------------------------'  `----------------------------------'
 */
    [_NAV_FN] = LAYOUT(
      _______, KC_F1, KC_F2,   KC_F3,    KC_F4,   KC_F5,                                                           KC_MPRV, KC_MPLY, KC_MNXT, _______, _______, _______,
      _______,KC_F6,   KC_F7,   KC_F8,    KC_F9,   KC_F10,                                                         KC_VOLD, KC_VOLU, _______,   _______,  _______, KC_DELETE,
      _______, KC_F11, KC_F12, _______, KC_PGUP, KC_HOME,                                                          CTLSFT_UP, _______, _______, _______, _______, KC_INSERT,
      _______, _______, _______, _______, KC_PGDN, KC_END , LSFT(KC_EQL), LSFT(KC_LBRC),   LSFT(KC_RBRC), _______, CTLSFT_DN, _______, _______, _______, _______, _______,
                                 _______, NAVFN    , _______, _______, KC_F24, /*menu*/             _______ ,      _______, _______, XXXXXXX, _______
    ),

/*
 * Sym Layer: Numbers and symbols
 *
 * ,-------------------------------------------.                              ,-------------------------------------------.
 * |        |      |      |      |      |      |                              |      |      |      |      |      |        |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |    `   |  1   |  2   |  3   |  4   |  5   |                              |   6  |  7   |  8   |  9   |  0   |   =    |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |    ~   |  !   |  @   |  #   |  $   |  %   |                              |   ^  |  &   |  *   |  (   |  )   |   +    |
 * |--------+------+------+------+------+------+-------------.  ,-------------+------+------+------+------+------+--------|
 * |    |   |   \  |  :   |  ;   |  -   |  [   |  {   |      |  |      |   }  |   ]  |  _   |  ,   |  .   |  /   |   ?    |
 * `----------------------+------+------+------+------+------|  |------+------+------+------+------+----------------------'
 *                        |      |      |      |      |      |  |      |      |      |      |      |
 *                        |      |      |      |      |      |  |      |      |      |      |      |
 *                        `----------------------------------'  `----------------------------------'
 */
    [_NUMPAD] = LAYOUT(  
       XXXXXXX, LSG(KC_LEFT), LSG(KC_RIGHT), LGUI(KC_LEFT),  LGUI(KC_RIGHT), LGUI(KC_UP),                                         XXXXXXX, LSFT(KC_9), KC_PSLS, LSFT(KC_8), LSFT(KC_0), _______,
      _______, _______, _______, _______, _______, LGUI(KC_DOWN),                                          KC_PSCR, KC_P7, KC_P8, KC_P9, KC_PMNS, KC_DELETE,
      _______, _______, _______, _______, _______, KC_PSCR,                                         KC_PDOT, KC_P4, KC_P5, KC_P6, KC_PPLS, KC_INSERT,
      _______, _______, _______, _______, _______, _______ , _______, _______,     _______, _______, KC_P0, KC_P1, KC_P2, KC_P3, _______, _______,
                                 _______, XXXXXXX, _______, KC_CAPS, KC_NUM,     _______ , _______, _______, NUMPAD, KC_APP
    ),

/*
 * Base Layer: Dvorak
 *
 * ,-------------------------------------------.                              ,-------------------------------------------.
 * |  Esc   |   1  |   2  |   3  |   4  |   5  |                              |   6  |   7  |   8  |   9  |   0  |  Esc   |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |  Tab   | ' "  | , <  | . >  |   P  |   Y  |                              |   F  |   G  |   C  |   R  |   L  |  Bksp  |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |Ctrl/Esc|   A  |   O  |   E  |   U  |   I  |                              |   D  |   H  |   T  |   N  |   S  |Ctrl/- _|
 * |--------+------+------+------+------+------+-------------.  ,-------------+------+------+------+------+------+--------|
 * | LShift | ; :  |   Q  |   J  |   K  |   X  | [ {  |CapsLk|  |F-keys|  ] } |   B  |   M  |   W  |   V  |   Z  | RShift |
 * `----------------------+------+------+------+------+------|  |------+------+------+------+------+----------------------'
 *                        |Adjust| LGUI | LAlt/| Space| Nav  |  | Sym  | Space| AltGr| RGUI | Menu |
 *                        |      |      | Enter|      |      |  |      |      |      |      |      |
 *                        `----------------------------------'  `----------------------------------'
 */
    [_DVORAK] = LAYOUT(
     KC_ESC  , KC_1 ,  KC_2   ,  KC_3  ,   KC_4 ,   KC_5 ,                                        KC_6 ,  KC_7 ,  KC_8 ,   KC_9 ,  KC_0 , KC_ESC ,
     KC_TAB  ,KC_QUOTE,KC_COMM,  KC_DOT,   KC_P ,   KC_Y ,                                         KC_F,   KC_G ,  KC_C ,   KC_R ,  KC_L , KC_BSPC,
     CTL_ESC , KC_A ,  KC_O   ,  KC_E  ,   KC_U ,   KC_I ,                                        KC_D,   KC_H ,  KC_T ,   KC_N ,  KC_S , CTL_MINS,
     KC_LSFT ,KC_SCLN, KC_Q   ,  KC_J  ,   KC_K ,   KC_X , KC_LBRC,KC_CAPS,     FKEYS  , KC_RBRC, KC_B,   KC_M ,  KC_W ,   KC_V ,  KC_Z , KC_RSFT,
                                 ADJUST, KC_LGUI, ALT_ENT, KC_SPC , NAVFN   ,     NUMPAD    , KC_SPC ,KC_RALT, KC_RGUI, KC_APP
    ),

/*
 * Base Layer: Colemak DH
 *
 * ,-------------------------------------------.                              ,-------------------------------------------.
 * |  Esc   |   1  |   2  |   3  |   4  |   5  |                              |   6  |   7  |   8  |   9  |   0  |  Esc   |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |  Tab   |   Q  |   W  |   F  |   P  |   B  |                              |   J  |   L  |   U  |   Y  | ;  : |  Bksp  |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |Ctrl/Esc|   A  |   R  |   S  |   T  |   G  |                              |   M  |   N  |   E  |   I  |   O  |Ctrl/' "|
 * |--------+------+------+------+------+------+-------------.  ,-------------+------+------+------+------+------+--------|
 * | LShift |   Z  |   X  |   C  |   D  |   V  | [ {  |CapsLk|  |F-keys|  ] } |   K  |   H  | ,  < | . >  | /  ? | RShift |
 * `----------------------+------+------+------+------+------|  |------+------+------+------+------+----------------------'
 *                        |Adjust| LGUI | LAlt/| Space| Nav  |  | Sym  | Space| AltGr| RGUI | Menu |
 *                        |      |      | Enter|      |      |  |      |      |      |      |      |
 *                        `----------------------------------'  `----------------------------------'
 */
    [_COLEMAK_DH] = LAYOUT(
     KC_ESC  , KC_1 ,  KC_2   ,  KC_3  ,   KC_4 ,   KC_5 ,                                        KC_6 ,  KC_7 ,  KC_8 ,   KC_9 ,  KC_0 , KC_ESC ,
     KC_TAB  , KC_Q ,  KC_W   ,  KC_F  ,   KC_P ,   KC_B ,                                        KC_J,   KC_L ,  KC_U ,   KC_Y ,KC_SCLN, KC_BSPC,
     CTL_ESC , KC_A ,  KC_R   ,  KC_S  ,   KC_T ,   KC_G ,                                        KC_M,   KC_N ,  KC_E ,   KC_I ,  KC_O , CTL_QUOT,
     KC_LSFT , KC_Z ,  KC_X   ,  KC_C  ,   KC_D ,   KC_V , KC_LBRC,KC_CAPS,     FKEYS  , KC_RBRC, KC_K,   KC_H ,KC_COMM, KC_DOT ,KC_SLSH, KC_RSFT,
                                 ADJUST, KC_LGUI, ALT_ENT, KC_SPC , NAVFN   ,     NUMPAD    , KC_SPC ,KC_RALT, KC_RGUI, KC_APP
    ),

/*
 * Function Layer: Function keys
 *
 * ,-------------------------------------------.                              ,-------------------------------------------.
 * |        |      |      |      |      |      |                              |      |      |      |      |      |        |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |        |  F9  | F10  | F11  | F12  |      |                              |      |      |      |      |      |        |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |        |  F5  |  F6  |  F7  |  F8  |      |                              |      | Shift| Ctrl |  Alt |  GUI |        |
 * |--------+------+------+------+------+------+-------------.  ,-------------+------+------+------+------+------+--------|
 * |        |  F1  |  F2  |  F3  |  F4  |      |      |      |  |      |      |      |      |      |      |      |        |
 * `----------------------+------+------+------+------+------|  |------+------+------+------+------+----------------------'
 *                        |      |      |      |      |      |  |      |      |      |      |      |
 *                        |      |      |      |      |      |  |      |      |      |      |      |
 *                        `----------------------------------'  `----------------------------------'
 */
    [_FUNCTION] = LAYOUT(
      _______, KC_F1,   KC_F2,   KC_F3,    KC_F4,   KC_F5,                                        _______, _______, _______, _______, _______, _______,
      _______, KC_F6,   KC_F7,   KC_F8,    KC_F9,   KC_F10,                                       _______, _______, _______, _______, _______, _______,
      _______, _______, _______, _______,  _______, KC_F11,                                       _______, _______, _______, _______, _______, _______,
      _______, _______, _______, _______,  _______, KC_F12, FKEYS,   XXXXXXX,   _______, _______, _______, _______, _______, _______, _______, _______,
                                 _______, XXXXXXX, _______, _______, _______,   _______, _______, _______, XXXXXXX, _______
    ),

/* 
 * Adjust Layer: Default layer settings, RGB
 *
 * ,-------------------------------------------.                              ,-------------------------------------------.
 * |        |      |      |      |      |      |                              |      |      |      |      |      |        |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |        |      |      |QWERTY|      |      |                              |      |      |      |      |      |        |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |        |      |      |Dvorak|      |      |                              | TOG  | SAI  | HUI  | VAI  | MOD  |        |
 * |--------+------+------+------+------+------+-------------.  ,-------------+------+------+------+------+------+--------|
 * |        |      |      |Colmak|      |      |      |      |  |      |      |      | SAD  | HUD  | VAD  | RMOD |        |
 * `----------------------+------+------+------+------+------|  |------+------+------+------+------+----------------------'
 *                        |      |      |      |      |      |  |      |      |      |      |      |
 *                        |      |      |      |      |      |  |      |      |      |      |      |
 *                        `----------------------------------'  `----------------------------------'
 */
    [_ADJUST] =  LAYOUT(
      XXXXXXX, LSG(KC_LEFT), LSG(KC_RIGHT), LGUI(KC_LEFT),  LGUI(KC_RIGHT), LGUI(KC_UP),                                       XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
      XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,  XXXXXXX, LGUI(KC_DOWN),                                       XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
      XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,  XXXXXXX, XXXXXXX,                                       XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
      XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,  XXXXXXX, XXXXXXX, XXXXXXX,  ADJUST,   XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
                                 XXXXXXX,  XXXXXXX, XXXXXXX, XXXXXXX, KC_F24,   XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX
    )

// /*
//  * Layer template
//  *
//  * ,-------------------------------------------.                              ,-------------------------------------------.
//  * |        |      |      |      |      |      |                              |      |      |      |      |      |        |
//  * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
//  * |        |      |      |      |      |      |                              |      |      |      |      |      |        |
//  * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
//  * |        |      |      |      |      |      |                              |      |      |      |      |      |        |
//  * |--------+------+------+------+------+------+-------------.  ,-------------+------+------+------+------+------+--------|
//  * |        |      |      |      |      |      |      |      |  |      |      |      |      |      |      |      |        |
//  * `----------------------+------+------+------+------+------|  |------+------+------+------+------+----------------------'
//  *                        |      |      |      |      |      |  |      |      |      |      |      |
//  *                        |      |      |      |      |      |  |      |      |      |      |      |
//  *                        `----------------------------------'  `----------------------------------'
//  */
//     [_LAYERINDEX] = LAYOUT(
//       _______, _______, _______, _______, _______, _______,                                     _______, _______, _______, _______, _______, _______,
//       _______, _______, _______, _______, _______, _______,                                     _______, _______, _______, _______, _______, _______,
//       _______, _______, _______, _______, _______, _______,                                     _______, _______, _______, _______, _______, _______,
//       _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
//                                  _______, _______, _______, _______, _______, _______, _______, _______, _______, _______
//     ),
//
};

#if defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [0] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU),  ENCODER_CCW_CW(KC_VOLD, KC_VOLU),  ENCODER_CCW_CW(KC_PGUP, KC_PGDN),  ENCODER_CCW_CW(KC_PGUP, KC_PGDN)  },
    [1] = { ENCODER_CCW_CW(_______, _______),  ENCODER_CCW_CW(_______, _______),  ENCODER_CCW_CW(_______, _______),  ENCODER_CCW_CW(_______, _______)  },
    [2] = { ENCODER_CCW_CW(_______, _______),  ENCODER_CCW_CW(_______, _______),  ENCODER_CCW_CW(_______, _______),  ENCODER_CCW_CW(_______, _______)  },
    [3] = { ENCODER_CCW_CW(_______, _______),  ENCODER_CCW_CW(_______, _______),  ENCODER_CCW_CW(_______, _______),  ENCODER_CCW_CW(_______, _______)  },
    [4] = { ENCODER_CCW_CW(_______, _______),  ENCODER_CCW_CW(_______, _______),  ENCODER_CCW_CW(_______, _______),  ENCODER_CCW_CW(_______, _______)  },
    [5] = { ENCODER_CCW_CW(_______, _______),  ENCODER_CCW_CW(_______, _______),  ENCODER_CCW_CW(_______, _______),  ENCODER_CCW_CW(_______, _______)  },
    [6] = { ENCODER_CCW_CW(_______, _______),  ENCODER_CCW_CW(_______, _______),  ENCODER_CCW_CW(_______, _______),  ENCODER_CCW_CW(_______, _______)  },
};
#endif

#if defined (HALCYON_ENABLE)
const uint16_t left_halcyon_buttons[10][5] = {
    [_QWERTY] =     { KC_MUTE, _______, _______, _______, _______ },
    [_DVORAK] =     { _______, _______, _______, _______, _______ },
    [_COLEMAK_DH] = { _______, _______, _______, _______, _______ },
    [_NAV_FN] =        { _______, _______, _______, _______, _______ },
    [_NUMPAD] =        { _______, _______, _______, _______, _______ },
    [_FUNCTION] =   { _______, _______, _______, _______, _______ },
    [_ADJUST] =     { _______, _______, _______, _______, _______ }
};

const uint16_t right_halcyon_buttons[10][5] = {
    [_QWERTY] =     { KC_MUTE, _______, _______, _______, _______ },
    [_DVORAK] =     { _______, _______, _______, _______, _______ },
    [_COLEMAK_DH] = { _______, _______, _______, _______, _______ },
    [_NAV_FN] =        { _______, _______, _______, _______, _______ },
    [_NUMPAD] =        { _______, _______, _______, _______, _______ },
    [_FUNCTION] =   { _______, _______, _______, _______, _______ },
    [_ADJUST] =     { _______, _______, _______, _______, _______ }
};
#endif
