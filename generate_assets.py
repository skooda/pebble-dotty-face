#!/usr/bin/env python3
import math
from PIL import Image, ImageDraw

def create_icon(size, filename):
    """Create a simple 5x5 dot matrix icon showing clock hands at 10:15"""
    img = Image.new('RGB', (size, size), color='black')
    draw = ImageDraw.Draw(img)

    # 5x5 grid
    grid_size = 5
    dot_size = int(size / (grid_size + 1))  # Leave some margin
    spacing = size // grid_size

    # Calculate offset to center the grid
    offset_x = (size - (grid_size - 1) * spacing) // 2
    offset_y = (size - (grid_size - 1) * spacing) // 2

    center_x = 2  # Center of 5x5 grid
    center_y = 2

    # Time to display: 10:15
    hours = 10
    minutes = 15

    hour_hand_length = 1.5
    minute_hand_length = 2.0

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

    # Calculate hand positions
    hour_angle = (hours % 12) * 30 + minutes // 2
    minute_angle = minutes * 6

    hour_end_x = int(center_x + hour_hand_length * math.sin(math.radians(hour_angle)))
    hour_end_y = int(center_y - hour_hand_length * math.cos(math.radians(hour_angle)))

    minute_end_x = int(center_x + minute_hand_length * math.sin(math.radians(minute_angle)))
    minute_end_y = int(center_y - minute_hand_length * math.cos(math.radians(minute_angle)))

    hour_hand_points = set(bresenham_line(center_x, center_y, hour_end_x, hour_end_y))
    minute_hand_points = set(bresenham_line(center_x, center_y, minute_end_x, minute_end_y))

    # Draw 5x5 grid
    for row in range(grid_size):
        for col in range(grid_size):
            x = offset_x + col * spacing
            y = offset_y + row * spacing

            # Check if this dot is on a hand
            if (col, row) in hour_hand_points or (col, row) in minute_hand_points:
                # Draw filled circle for hands
                draw.ellipse([x - dot_size//2, y - dot_size//2,
                            x + dot_size//2, y + dot_size//2], fill='white')
            else:
                # Draw hollow circle for background
                draw.ellipse([x - dot_size//2, y - dot_size//2,
                            x + dot_size//2, y + dot_size//2], outline='white', width=max(1, size//50))

    img.save(filename)
    print(f"Created {filename}")

# Create icons for Pebble App Store
create_icon(25, 'resources/icon_25.png')  # Menu icon
create_icon(48, 'resources/icon_48.png')  # App list icon (old Pebbles)
create_icon(144, 'resources/icon_144.png')  # App Store icon

print("\nIcons created successfully!")
