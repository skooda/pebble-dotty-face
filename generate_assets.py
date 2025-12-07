#!/usr/bin/env python3
from PIL import Image, ImageDraw

def create_icon(size, filename):
    """Create a simple 3x3 dot matrix icon with specific pattern"""
    img = Image.new('RGB', (size, size), color='black')
    draw = ImageDraw.Draw(img)

    # 3x3 grid
    grid_size = 3
    dot_size = int(size / 4)  # Leave margin
    spacing = size // 3

    # Calculate offset to center the grid
    offset_x = size // 2 - spacing
    offset_y = size // 2 - spacing

    # Pattern: row 0: [full, empty, empty]
    #          row 1: [empty, full, full]
    #          row 2: [empty, empty, empty]
    pattern = [
        [True, False, False],
        [False, True, True],
        [False, False, False]
    ]

    # Draw 3x3 grid
    for row in range(grid_size):
        for col in range(grid_size):
            x = offset_x + col * spacing
            y = offset_y + row * spacing

            if pattern[row][col]:
                # Draw filled circle
                draw.ellipse([x - dot_size//2, y - dot_size//2,
                            x + dot_size//2, y + dot_size//2], fill='white')
            else:
                # Draw hollow circle
                draw.ellipse([x - dot_size//2, y - dot_size//2,
                            x + dot_size//2, y + dot_size//2], outline='white', width=max(1, size//40))

    img.save(filename)
    print(f"Created {filename}")

# Create icons for Pebble App Store
create_icon(25, 'resources/icon_25.png')  # Menu icon
create_icon(48, 'resources/icon_48.png')  # App list icon (old Pebbles)
create_icon(144, 'resources/icon_144.png')  # App Store icon

print("\nIcons created successfully!")
