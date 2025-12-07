# Dotty Face

A minimalist dot matrix analog watchface for Pebble smartwatch.

## Features

- Clean analog clock display using a 17x21 dot matrix grid
- Circular clock face with hour markers at 12, 3, 6, and 9 o'clock
- Precise hour and minute hands using Bresenham's line algorithm
- Subtle hollow dots for background grid (dotted pattern)
- Optimized for Pebble 2, Pebble Time, and Pebble Time Steel

## Screenshots

![Dotty Face Watchface](screenshot.png)

## Installation

### Via Pebble App Store
Coming soon!

### Manual Installation

1. Download the latest `dotty-face.pbw` from the [releases page](https://github.com/skooda/pebble-dotty-face/releases)
2. Open the Pebble app on your phone
3. Go to Settings → Apps → Install Watch App
4. Select the downloaded `.pbw` file

### Build from Source

Requirements:
- [Pebble SDK](https://developer.rebble.io/developer.pebble.com/sdk/install/index.html)

```bash
# Clone the repository
git clone https://github.com/skooda/pebble-dotty-face.git
cd pebble-dotty-face

# Build
pebble build

# Install to connected Pebble
pebble install --cloudpebble
```

## Technical Details

- Grid: 17×21 dots (7px dot size, 1px margin)
- Clock radius: 8.0 grid units
- Integer-only mathematics (no floating point)
- Sin/cos lookup tables for angle calculations
- Hand rendering: Bresenham's line algorithm for pixel-perfect 1-dot-wide lines

## Compatibility

- Pebble 2 (Basalt)
- Pebble Time (Basalt)
- Pebble Time Steel (Basalt)
- Pebble Time Round (Chalk)
- Pebble (Diorite)

## License

MIT License

## Author

V3L (p@v3l.cz)
