#!/usr/bin/env python3
from PIL import Image, ImageDraw, ImageFont

# Create banner (625x250)
img = Image.new('RGB', (625, 250), color='black')
draw = ImageDraw.Draw(img)

# Draw title
try:
    font_large = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 60)
    font_small = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 24)
except:
    font_large = ImageFont.load_default()
    font_small = ImageFont.load_default()

# Title
draw.text((20, 80), "Dotty Face", fill='white', font=font_large)
draw.text((20, 160), "Minimalist Analog Watchface", fill='white', font=font_small)

# Draw a small preview of the watchface on the right
preview_size = 180
preview_x = 625 - preview_size - 35
preview_y = 35

# Mini dot matrix preview
grid_size = 17
dot_size = int(preview_size / grid_size * 0.7)
margin = int(preview_size / grid_size * 0.1)
dot_spacing = dot_size + margin

grid_width = grid_size * dot_size + (grid_size - 1) * margin
offset_x = preview_x + (preview_size - grid_width) // 2
offset_y = preview_y + (preview_size - grid_width) // 2

center_x = 8
center_y = 8

# Draw simplified clock face
for row in range(grid_size):
    for col in range(grid_size):
        x = offset_x + col * dot_spacing
        y = offset_y + row * dot_spacing

        dx = col - center_x
        dy = row - center_y
        dist_squared = dx * dx + dy * dy

        # Just draw the circle and center
        if 56 <= dist_squared <= 72 or (col == center_x and row == center_y):
            draw.ellipse([x, y, x + dot_size, y + dot_size], fill='white')

img.save('resources/banner.png')
print("Created resources/banner.png")
