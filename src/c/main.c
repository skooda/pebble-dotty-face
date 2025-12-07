#include <pebble.h>

#define GRID_SIZE 16
#define DOT_SIZE 7
#define DOT_MARGIN 2
#define DOT_SPACING (DOT_SIZE + DOT_MARGIN)

static Window *s_main_window;
static Layer *s_canvas_layer;
static int s_hours;
static int s_minutes;

// 3x7 digit patterns (0-9)
// 1 = filled dot, 0 = hollow dot
static const uint8_t DIGIT_PATTERNS[10][7][3] = {
  { // 0
    {1,1,1},
    {1,0,1},
    {1,0,1},
    {1,0,1},
    {1,0,1},
    {1,0,1},
    {1,1,1}
  },
  { // 1
    {0,1,0},
    {1,1,0},
    {0,1,0},
    {0,1,0},
    {0,1,0},
    {0,1,0},
    {1,1,1}
  },
  { // 2
    {1,1,1},
    {0,0,1},
    {0,0,1},
    {1,1,1},
    {1,0,0},
    {1,0,0},
    {1,1,1}
  },
  { // 3
    {1,1,1},
    {0,0,1},
    {0,0,1},
    {1,1,1},
    {0,0,1},
    {0,0,1},
    {1,1,1}
  },
  { // 4
    {1,0,1},
    {1,0,1},
    {1,0,1},
    {1,1,1},
    {0,0,1},
    {0,0,1},
    {0,0,1}
  },
  { // 5
    {1,1,1},
    {1,0,0},
    {1,0,0},
    {1,1,1},
    {0,0,1},
    {0,0,1},
    {1,1,1}
  },
  { // 6
    {1,1,1},
    {1,0,0},
    {1,0,0},
    {1,1,1},
    {1,0,1},
    {1,0,1},
    {1,1,1}
  },
  { // 7
    {1,1,1},
    {0,0,1},
    {0,0,1},
    {0,1,0},
    {0,1,0},
    {0,1,0},
    {0,1,0}
  },
  { // 8
    {1,1,1},
    {1,0,1},
    {1,0,1},
    {1,1,1},
    {1,0,1},
    {1,0,1},
    {1,1,1}
  },
  { // 9
    {1,1,1},
    {1,0,1},
    {1,0,1},
    {1,1,1},
    {0,0,1},
    {0,0,1},
    {1,1,1}
  }
};

static bool is_dot_filled(int col, int row) {
  // Vertical centering: use rows 4-11 (8 rows) for digits (7 rows + 1 padding)
  if (row < 4 || row > 11) {
    return false;
  }

  int digit_row = row - 5; // Map to 0-6 for digit pattern
  if (digit_row < 0 || digit_row > 6) {
    return false;
  }

  // Digit layout across 16 columns:
  // Cols 0-2: H tens
  // Col 3: space
  // Cols 4-6: H ones
  // Cols 7-8: colon
  // Cols 9-11: M tens
  // Col 12: space
  // Cols 13-15: M ones

  int digit = -1;
  int digit_col = -1;

  if (col >= 0 && col <= 2) {
    digit = s_hours / 10;
    digit_col = col;
  } else if (col >= 4 && col <= 6) {
    digit = s_hours % 10;
    digit_col = col - 4;
  } else if (col >= 7 && col <= 8) {
    // Colon - show dots at rows 5 and 9 (digit_row 0 and 4)
    if ((digit_row == 1 || digit_row == 5) && col == 7) {
      return true;
    }
    return false;
  } else if (col >= 9 && col <= 11) {
    digit = s_minutes / 10;
    digit_col = col - 9;
  } else if (col >= 13 && col <= 15) {
    digit = s_minutes % 10;
    digit_col = col - 13;
  }

  if (digit >= 0 && digit <= 9 && digit_col >= 0 && digit_col <= 2) {
    return DIGIT_PATTERNS[digit][digit_row][digit_col] == 1;
  }

  return false;
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Calculate grid dimensions
  int grid_width = GRID_SIZE * DOT_SIZE + (GRID_SIZE - 1) * DOT_MARGIN;
  int grid_height = GRID_SIZE * DOT_SIZE + (GRID_SIZE - 1) * DOT_MARGIN;

  // Center the grid
  int offset_x = (bounds.size.w - grid_width) / 2;
  int offset_y = (bounds.size.h - grid_height) / 2;

  // Draw 16x16 grid of dots
  for (int row = 0; row < GRID_SIZE; row++) {
    for (int col = 0; col < GRID_SIZE; col++) {
      int x = offset_x + col * DOT_SPACING;
      int y = offset_y + row * DOT_SPACING;

      bool filled = is_dot_filled(col, row);

      if (filled) {
        // Draw filled circle
        graphics_context_set_fill_color(ctx, GColorWhite);
        graphics_fill_circle(ctx, GPoint(x + DOT_SIZE/2, y + DOT_SIZE/2), DOT_SIZE/2);
      } else {
        // Draw hollow circle
        graphics_context_set_stroke_color(ctx, GColorWhite);
        graphics_draw_circle(ctx, GPoint(x + DOT_SIZE/2, y + DOT_SIZE/2), DOT_SIZE/2);
      }
    }
  }
}

static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  s_hours = tick_time->tm_hour;
  s_minutes = tick_time->tm_min;

  // Use 12-hour format if not 24h style
  if (!clock_is_24h_style() && s_hours > 12) {
    s_hours -= 12;
  }
  if (!clock_is_24h_style() && s_hours == 0) {
    s_hours = 12;
  }

  layer_mark_dirty(s_canvas_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Set background to black
  window_set_background_color(window, GColorBlack);

  // Create canvas layer
  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(window_layer, s_canvas_layer);
}

static void main_window_unload(Window *window) {
  layer_destroy(s_canvas_layer);
}

static void init() {
  s_main_window = window_create();

  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  window_stack_push(s_main_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  update_time();
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
