#!/usr/bin/env python3
import math
from PIL import Image, ImageDraw

def create_icon(size, filename):
    """Create a watchface icon showing the dot matrix design"""
    img = Image.new('RGB', (size, size), color='black')
    draw = ImageDraw.Draw(img)

    # Calculate grid parameters
    grid_size = 17
    dot_size = int(size / grid_size * 0.7)
    margin = int(size / grid_size * 0.1)
    dot_spacing = dot_size + margin

    # Calculate offset to center the grid
    grid_width = grid_size * dot_size + (grid_size - 1) * margin
    offset_x = (size - grid_width) // 2
    offset_y = (size - grid_width) // 2

    center_x = 8
    center_y = 8
    clock_radius = 8.0

    # Time to display: 10:10
    hours = 10
    minutes = 10

    hour_hand_length = 4
    minute_hand_length = 6

    def is_on_circle(col, row):
        dx = col - center_x
        dy = row - center_y
        dist_squared = dx * dx + dy * dy
        return 56 <= dist_squared <= 72

    def is_on_marker(col, row):
        # 12 o'clock
        if col == center_x and center_y - clock_radius + 1 <= row <= center_y - clock_radius + 2:
            return True
        # 3 o'clock
        if row == center_y and center_x + clock_radius - 2 <= col <= center_x + clock_radius - 1:
            return True
        # 6 o'clock
        if col == center_x and center_y + clock_radius - 2 <= row <= center_y + clock_radius - 1:
            return True
        # 9 o'clock
        if row == center_y and center_x - clock_radius + 1 <= col <= center_x - clock_radius + 2:
            return True
        return False

    def bresenham_line(x0, y0, x1, y1):
        points = []
        dx = abs(x1 - x0)
        dy = abs(y1 - y0)
        sx = 1 if x0 < x1 else -1
        sy = 1 if y0 < y1 else -1
        err = dx - dy
        x, y = x0, y0

        while True:
            points.append((x, y))
            if x == x1 and y == y1:
                break
            e2 = 2 * err
            if e2 > -dy:
                err -= dy
                x += sx
            if e2 < dx:
                err += dx
                y += sy
        return points

    def get_sin(angle):
        rad = math.radians(angle)
        return int(math.sin(rad) * 1000)

    def get_cos(angle):
        rad = math.radians(angle)
        return int(math.cos(rad) * 1000)

    # Calculate hand positions
    hour_angle = (hours % 12) * 30 + minutes // 2
    minute_angle = minutes * 6

    hour_end_x = (center_x * 1000 + hour_hand_length * 1000 * get_sin(hour_angle) // 1000) // 1000
    hour_end_y = (center_y * 1000 - hour_hand_length * 1000 * get_cos(hour_angle) // 1000) // 1000

    minute_end_x = (center_x * 1000 + minute_hand_length * 1000 * get_sin(minute_angle) // 1000) // 1000
    minute_end_y = (center_y * 1000 - minute_hand_length * 1000 * get_cos(minute_angle) // 1000) // 1000

    hour_hand_points = set(bresenham_line(center_x, center_y, hour_end_x, hour_end_y))
    minute_hand_points = set(bresenham_line(center_x, center_y, minute_end_x, minute_end_y))

    # Draw dots
    for row in range(grid_size):
        for col in range(grid_size):
            x = offset_x + col * dot_spacing
            y = offset_y + row * dot_spacing

            is_filled = False

            # Center dot
            if col == center_x and row == center_y:
                is_filled = True
            # Hour and minute hands
            elif (col, row) in hour_hand_points or (col, row) in minute_hand_points:
                is_filled = True
            # Markers
            elif is_on_marker(col, row):
                is_filled = True
            # Circle
            elif is_on_circle(col, row):
                is_filled = True

            if is_filled:
                # Draw filled circle
                draw.ellipse([x, y, x + dot_size, y + dot_size], fill='white')
            else:
                # Draw dotted circle (4 small pixels)
                cx = x + dot_size // 2
                cy = y + dot_size // 2
                r = dot_size // 2
                pixel_size = max(1, dot_size // 8)

                draw.ellipse([cx - pixel_size, cy - r - pixel_size, cx + pixel_size, cy - r + pixel_size], fill='white')  # top
                draw.ellipse([cx + r - pixel_size, cy - pixel_size, cx + r + pixel_size, cy + pixel_size], fill='white')  # right
                draw.ellipse([cx - pixel_size, cy + r - pixel_size, cx + pixel_size, cy + r + pixel_size], fill='white')  # bottom
                draw.ellipse([cx - r - pixel_size, cy - pixel_size, cx - r + pixel_size, cy + pixel_size], fill='white')  # left

    img.save(filename)
    print(f"Created {filename}")

# Create icons for Pebble App Store
create_icon(25, 'resources/icon_25.png')  # Menu icon
create_icon(48, 'resources/icon_48.png')  # App list icon (old Pebbles)
create_icon(144, 'resources/icon_144.png')  # App Store icon

print("\nIcons created successfully!")
print("\nFor screenshots, run the watchface in the emulator and use:")
print("  pebble screenshot")
