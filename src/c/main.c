#include <pebble.h>

#define GRID_SIZE 17
#define DOT_SIZE 7
#define DOT_MARGIN 1
#define DOT_SPACING (DOT_SIZE + DOT_MARGIN)

static Window *s_main_window;
static Layer *s_canvas_layer;
static int s_hours;
static int s_minutes;
static int s_day_of_month;
static int s_day_of_week;

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

// 3x5 letter patterns for days of week
// Letters: A, D, E, F, H, I, M, N, O, R, S, T, U, W
static const uint8_t LETTER_PATTERNS[14][5][3] = {
  { // A (0)
    {1,1,1},
    {1,0,1},
    {1,1,1},
    {1,0,1},
    {1,0,1}
  },
  { // D (1)
    {1,1,0},
    {1,0,1},
    {1,0,1},
    {1,0,1},
    {1,1,0}
  },
  { // E (2)
    {1,1,1},
    {1,0,0},
    {1,1,1},
    {1,0,0},
    {1,1,1}
  },
  { // F (3)
    {1,1,1},
    {1,0,0},
    {1,1,0},
    {1,0,0},
    {1,0,0}
  },
  { // H (4)
    {1,0,1},
    {1,0,1},
    {1,1,1},
    {1,0,1},
    {1,0,1}
  },
  { // I (5)
    {1,1,1},
    {0,1,0},
    {0,1,0},
    {0,1,0},
    {1,1,1}
  },
  { // M (6)
    {1,0,1},
    {1,1,1},
    {1,1,1},
    {1,0,1},
    {1,0,1}
  },
  { // N (7)
    {1,0,1},
    {1,1,1},
    {1,1,1},
    {1,0,1},
    {1,0,1}
  },
  { // O (8)
    {1,1,1},
    {1,0,1},
    {1,0,1},
    {1,0,1},
    {1,1,1}
  },
  { // R (9)
    {1,1,0},
    {1,0,1},
    {1,1,0},
    {1,0,1},
    {1,0,1}
  },
  { // S (10)
    {1,1,1},
    {1,0,0},
    {1,1,1},
    {0,0,1},
    {1,1,1}
  },
  { // T (11)
    {1,1,1},
    {0,1,0},
    {0,1,0},
    {0,1,0},
    {0,1,0}
  },
  { // U (12)
    {1,0,1},
    {1,0,1},
    {1,0,1},
    {1,0,1},
    {1,1,1}
  },
  { // W (13)
    {1,0,1},
    {1,0,1},
    {1,1,1},
    {1,1,1},
    {1,0,1}
  }
};

// Day of week letter indices: 0=A, 1=D, 2=E, 3=F, 4=H, 5=I, 6=M, 7=N, 8=O, 9=R, 10=S, 11=T, 12=U, 13=W
static const uint8_t DAY_LETTERS[7][3] = {
  {10, 12, 7},  // SUN
  {6, 8, 7},    // MON
  {11, 12, 2},  // TUE
  {13, 2, 1},   // WED
  {11, 4, 12},  // THU
  {3, 9, 5},    // FRI
  {10, 0, 11}   // SAT
};

static bool is_dot_filled(int col, int row) {
  // Layout:
  // Rows 0-4: Day of week (3 letters, 5 rows)
  // Row 5: spacing
  // Rows 6-12: Time HH:MM (7 rows)
  // Row 13: spacing
  // Rows 14-16: Day of month (2 digits, use top 3 rows of digit pattern)

  // DAY OF WEEK (rows 0-4)
  if (row >= 0 && row <= 4) {
    // Center 3 letters (each 3 cols + 1 space = 11 cols total)
    // Layout: cols 3-5, 6-8, 9-11 (starting at col 3)
    int letter_idx = -1;
    int letter_col = -1;

    if (col >= 3 && col <= 5) {
      letter_idx = DAY_LETTERS[s_day_of_week][0];
      letter_col = col - 3;
    } else if (col >= 7 && col <= 9) {
      letter_idx = DAY_LETTERS[s_day_of_week][1];
      letter_col = col - 7;
    } else if (col >= 11 && col <= 13) {
      letter_idx = DAY_LETTERS[s_day_of_week][2];
      letter_col = col - 11;
    }

    if (letter_idx >= 0 && letter_col >= 0 && letter_col <= 2) {
      return LETTER_PATTERNS[letter_idx][row][letter_col] == 1;
    }
    return false;
  }

  // TIME (rows 6-12)
  if (row >= 6 && row <= 12) {
    int digit_row = row - 6; // Map to 0-6 for digit pattern

    // Digit layout across 17 columns:
    // Cols 0-2: H tens
    // Col 3: space
    // Cols 4-6: H ones
    // Col 7: space
    // Col 8: colon (centered)
    // Col 9: space
    // Cols 10-12: M tens
    // Col 13: space
    // Cols 14-16: M ones

    int digit = -1;
    int digit_col = -1;

    if (col >= 0 && col <= 2) {
      digit = s_hours / 10;
      digit_col = col;
    } else if (col >= 4 && col <= 6) {
      digit = s_hours % 10;
      digit_col = col - 4;
    } else if (col == 8) {
      // Colon - show dots at digit_row 1 and 5
      if (digit_row == 1 || digit_row == 5) {
        return true;
      }
      return false;
    } else if (col >= 10 && col <= 12) {
      digit = s_minutes / 10;
      digit_col = col - 10;
    } else if (col >= 14 && col <= 16) {
      digit = s_minutes % 10;
      digit_col = col - 14;
    }

    if (digit >= 0 && digit <= 9 && digit_col >= 0 && digit_col <= 2) {
      return DIGIT_PATTERNS[digit][digit_row][digit_col] == 1;
    }
    return false;
  }

  // DAY OF MONTH (rows 14-16)
  if (row >= 14 && row <= 16) {
    int digit_row = row - 14; // Map to 0-2 (use top 3 rows of digit pattern)

    // Center 2 digits (each 3 cols + 1 space = 7 cols total, start at col 5)
    int digit = -1;
    int digit_col = -1;

    if (col >= 5 && col <= 7) {
      digit = s_day_of_month / 10;
      digit_col = col - 5;
    } else if (col >= 9 && col <= 11) {
      digit = s_day_of_month % 10;
      digit_col = col - 9;
    }

    if (digit >= 0 && digit <= 9 && digit_col >= 0 && digit_col <= 2) {
      return DIGIT_PATTERNS[digit][digit_row][digit_col] == 1;
    }
    return false;
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

  // Draw grid of dots
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
  s_day_of_month = tick_time->tm_mday;
  s_day_of_week = tick_time->tm_wday; // 0 = Sunday, 1 = Monday, etc.

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
