/**
 * @file game_state.h
 * @brief Central game data structures, enums, and state-management function declarations.
 */

#ifndef PROJECT_GAME_STATE_H
#define PROJECT_GAME_STATE_H

#include <lcom/lcf.h>
#include <stdbool.h>
#include <stdint.h>
#include "rtc.h"

#define GAME_VIDEO_MODE 0x115  /**< VBE mode used by the game (800x600, 24 bpp). */
#define GAME_FPS 60            /**< Target frames per second. */
#define MAX_PLACEMENTS 15      /**< Maximum number of topping dots that can be placed. */
#define MAX_CUT_LINES  4       /**< Maximum number of cut lines that can be drawn. */

/**
 * @brief Describes a single straight cut line on the pizza by its two endpoint indices.
 */
typedef struct {
  int a; /**< Index of the first cut-point on the pizza rim. */
  int b; /**< Index of the second cut-point on the pizza rim. */
} CutLine;

/**
 * @brief Top-level state of the game's state machine.
 */
typedef enum {
  GAME_STATE_MENU,          /**< Main menu screen. */
  GAME_STATE_PLAYING,       /**< Active gameplay. */
  GAME_STATE_SETTINGS,      /**< Settings screen. */
  GAME_STATE_DAY_INTRO,     /**< Day introduction screen shown at the start of each day. */
  GAME_STATE_OPEN_SCREEN,   /**< Animated "open" transition before gameplay starts. */
  GAME_STATE_CLOSED_SCREEN, /**< Animated "closed" transition after the last order of the day. */
  GAME_STATE_DAY_SUMMARY    /**< End-of-day summary screen. */
} GameState;

/**
 * @brief Player-selected difficulty level.
 */
typedef enum {
  DIFF_EASY,   /**< Easy: wider oven tolerance, lower star thresholds, fewer required toppings. */
  DIFF_NORMAL, /**< Normal: standard progressive difficulty. */
  DIFF_HARD    /**< Hard: tight oven tolerance, doubled penalties, higher star thresholds. */
} Difficulty;

/**
 * @brief Sub-state used while @c GAME_STATE_PLAYING is active.
 */
typedef enum {
  PLAYING_TAKE_ORDER,    /**< Customer order is displayed; player must confirm to proceed. */
  PLAYING_PREPARE_PIZZA, /**< Player selects sauce, places toppings, and starts the oven. */
  PLAYING_CUT,           /**< Player draws cut lines on the baked pizza. */
  PLAYING_SERVE,         /**< Player types the customer name and submits the delivery. */
  PLAYING_DELIVERED      /**< Score and star result screen after a delivery. */
} PlayingState;

/**
 * @brief Represents a single customer order.
 */
typedef struct {
  int sauce;         /**< Required sauce index (0 = tomato, 1 = white). */
  int toppings[3];   /**< Indices of the three required topping types (0–5). */
  int topping_qty[3];/**< Required quantity of each topping (dots to place). */
  int cook_seconds;  /**< Required oven duration in seconds. */
  int slices;        /**< Required number of slices (4, 6, or 8). */
  char name[8];      /**< Customer name (null-terminated, max 7 chars). */
} Order;

/**
 * @brief Records where a single topping dot was placed on the pizza.
 */
typedef struct {
  int type; /**< Topping type index (0–5). */
  int dx;   /**< Horizontal offset from the pizza centre, in pixels. */
  int dy;   /**< Vertical offset from the pizza centre, in pixels. */
} ToppingPlacement;

/**
 * @brief Complete game state passed through the entire MVC pipeline each tick.
 */
typedef struct {
  GameState state;         /**< Current top-level state. */
  PlayingState playing_state; /**< Current gameplay sub-state. */
  bool running;            /**< False when the main loop should exit. */
  int tick;                /**< Frame counter, wraps at 60000. */
  int order_number;        /**< Total orders processed since the session started. */

  Order order;             /**< Active customer order. */
  int selected_sauce;      /**< Currently selected sauce index (-1 = none). */
  int active_topping;      /**< Currently selected topping type (-1 = none). */
  ToppingPlacement topping_placements[MAX_PLACEMENTS]; /**< Array of placed topping dots. */
  int num_placements;      /**< Number of valid entries in @c topping_placements. */
  int oven_ticks;          /**< How many ticks the pizza has been in the oven. */
  bool pizza_in_oven;      /**< True while the oven is running. */
  int cut_selected;        /**< Index of the first selected cut-point (-1 = none). */
  int num_cut_lines;       /**< Number of cut lines drawn so far. */
  CutLine cut_lines[MAX_CUT_LINES]; /**< Array of cut lines. */

  char typed_name[8];      /**< Name typed by the player in the SERVE state. */
  int typed_len;           /**< Length of @c typed_name (excluding null terminator). */
  char delivery_time_str[9]; /**< HH:MM:SS string set when the pizza is delivered. */
  char order_time_str[9];  /**< HH:MM:SS string set when the order preparation begins. */

  int mouse_x;             /**< Current horizontal mouse cursor position. */
  int mouse_y;             /**< Current vertical mouse cursor position. */
  bool mouse_left_click;   /**< True for one frame when a rising edge left-click is detected. */

  int menu_option;         /**< Keyboard cursor position in the main menu (0–2). */
  Difficulty difficulty;   /**< Currently selected difficulty level. */
  int settings_option;     /**< Keyboard cursor position in the settings screen (0–7). */
  int day_increment;       /**< Number of extra orders added each new day. */

  int day_number;          /**< Current day number (starts at 1). */
  int day_orders_total;    /**< Total orders required to finish the current day. */
  int day_orders_done;     /**< Orders successfully completed today. */
  int day_total_stars;     /**< Sum of stars earned across all orders today. */
  int day_total_score;     /**< Sum of score_10 values earned across all orders today. */
  int state_ticks;         /**< Tick counter used by timed states (open/close screen). */

  int last_score_10;       /**< Score (0–50) from the most recently completed order. */
  int last_stars;          /**< Stars (0–3) from the most recently completed order. */

  bool show_back_popup;    /**< True when the "return to menu?" confirmation popup is visible. */
  bool show_exit_popup;    /**< True when the "quit game?" confirmation popup is visible. */

  RtcTime current_time;    /**< Real-time clock value, refreshed every second. */
  RtcTime order_time;      /**< RTC snapshot taken when the game session started. */
} Game;

/**
 * @brief Initialises all fields of the @c Game struct to their starting values.
 * @param game Pointer to the @c Game struct to initialise.
 */
void game_init(Game *game);

/**
 * @brief Returns whether the game loop should keep running.
 * @param game Pointer to the current game state.
 * @return @c true if the loop should continue, @c false if it should stop.
 */
bool game_is_running(Game *game);

/**
 * @brief Called once per timer tick to advance game logic and handle time-based transitions.
 * @param game Pointer to the current game state.
 */
void game_update(Game *game);

#endif
