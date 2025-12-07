#include <pebble.h>

#define GRID_SIZE 17
#define DOT_SIZE 7
#define DOT_MARGIN 1
#define DOT_SPACING (DOT_SIZE + DOT_MARGIN)
#define CENTER_X 8
#define CENTER_Y 8
#define CLOCK_RADIUS 7.5
#define HOUR_HAND_LENGTH 4
#define MINUTE_HAND_LENGTH 6

static Window *s_main_window;
static Layer *s_canvas_layer;
static int s_hours;
static int s_minutes;

// Check if a dot is on the clock circle perimeter
static bool is_on_circle(int col, int row) {
  int dx = col - CENTER_X;
  int dy = row - CENTER_Y;
  int dist_squared = dx * dx + dy * dy;

  // Approximate: radius 7.5 squared = 56.25, so check around 49-64
  return (dist_squared >= 49 && dist_squared <= 64);
}

// Check if a dot is on a marker line (12, 3, 6, 9)
static bool is_on_marker(int col, int row) {
  // 12 o'clock (top) - vertical line
  if (col == CENTER_X && row >= CENTER_Y - CLOCK_RADIUS - 1 && row <= CENTER_Y - CLOCK_RADIUS + 1) {
    return true;
  }

  // 3 o'clock (right) - horizontal line
  if (row == CENTER_Y && col >= CENTER_X + CLOCK_RADIUS - 1 && col <= CENTER_X + CLOCK_RADIUS + 1) {
    return true;
  }

  // 6 o'clock (bottom) - vertical line
  if (col == CENTER_X && row >= CENTER_Y + CLOCK_RADIUS - 1 && row <= CENTER_Y + CLOCK_RADIUS + 1) {
    return true;
  }

  // 9 o'clock (left) - horizontal line
  if (row == CENTER_Y && col >= CENTER_X - CLOCK_RADIUS - 1 && col <= CENTER_X - CLOCK_RADIUS + 1) {
    return true;
  }

  return false;
}

// Simple integer sqrt approximation
static int isqrt(int n) {
  if (n == 0) return 0;
  int x = n;
  int y = (x + 1) / 2;
  while (y < x) {
    x = y;
    y = (x + n / x) / 2;
  }
  return x;
}

// Check if a dot is on a line from center to a point
static bool is_on_line(int col, int row, int end_x, int end_y) {
  int dx = end_x - CENTER_X;
  int dy = end_y - CENTER_Y;

  // Use perpendicular distance formula
  // Distance from point to line = |ax + by + c| / sqrt(a^2 + b^2)
  // For line from (CENTER_X, CENTER_Y) to (end_x, end_y)

  int px = col - CENTER_X;
  int py = row - CENTER_Y;

  // Cross product gives distance
  int cross = px * dy - py * dx;
  int len_squared = dx * dx + dy * dy;

  if (len_squared == 0) return false;

  // Check if point is between start and end
  int dot = px * dx + py * dy;
  if (dot < 0 || dot > len_squared) return false;

  // Distance squared from point to line
  int dist_squared = (cross * cross) / len_squared;

  return dist_squared <= 1; // Thickness of 1
}

// Sine/cosine using lookup table (0-90 degrees in steps of 6)
static const int sin_table[] = {0, 105, 208, 309, 407, 500, 588, 669, 743, 809, 866, 914, 951, 978, 995, 1000};
static const int cos_table[] = {1000, 995, 978, 951, 914, 866, 809, 743, 669, 588, 500, 407, 309, 208, 105, 0};

static int get_sin(int angle) {
  angle = angle % 360;
  if (angle < 0) angle += 360;

  if (angle <= 90) return sin_table[angle / 6];
  if (angle <= 180) return sin_table[(180 - angle) / 6];
  if (angle <= 270) return -sin_table[(angle - 180) / 6];
  return -sin_table[(360 - angle) / 6];
}

static int get_cos(int angle) {
  return get_sin(angle + 90);
}

// Check if a dot is on the hour hand
static bool is_on_hour_hand(int col, int row) {
  // Calculate hour hand angle (in degrees, 0 = 12 o'clock, clockwise)
  int hour_angle = (s_hours % 12) * 30 + s_minutes / 2;

  // Calculate end point of hour hand (scaled by 1000)
  int end_x = CENTER_X * 1000 + HOUR_HAND_LENGTH * 1000 * get_sin(hour_angle) / 1000;
  int end_y = CENTER_Y * 1000 - HOUR_HAND_LENGTH * 1000 * get_cos(hour_angle) / 1000;

  return is_on_line(col, row, end_x / 1000, end_y / 1000);
}

// Check if a dot is on the minute hand
static bool is_on_minute_hand(int col, int row) {
  // Calculate minute hand angle (in degrees, 0 = 12 o'clock, clockwise)
  int minute_angle = s_minutes * 6;

  // Calculate end point of minute hand (scaled by 1000)
  int end_x = CENTER_X * 1000 + MINUTE_HAND_LENGTH * 1000 * get_sin(minute_angle) / 1000;
  int end_y = CENTER_Y * 1000 - MINUTE_HAND_LENGTH * 1000 * get_cos(minute_angle) / 1000;

  return is_on_line(col, row, end_x / 1000, end_y / 1000);
}

// Check if a dot is the center dot
static bool is_center_dot(int col, int row) {
  return (col == CENTER_X && row == CENTER_Y);
}

static bool is_dot_filled(int col, int row) {
  // Priority: hands > markers > circle

  // Center dot is always filled
  if (is_center_dot(col, row)) {
    return true;
  }

  // Hour and minute hands
  if (is_on_hour_hand(col, row) || is_on_minute_hand(col, row)) {
    return true;
  }

  // Markers at 12, 3, 6, 9
  if (is_on_marker(col, row)) {
    return true;
  }

  // Circle perimeter
  if (is_on_circle(col, row)) {
    return true;
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
