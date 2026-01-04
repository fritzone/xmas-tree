# Rendering a Christmas Tree in the Terminal

### A Procedural Unicode and ANSI Graphics Approach in C

At first glance, rendering a richly decorated Christmas tree inside a text terminal might seem like a novelty exercise. In reality, the program presented here is a compact demonstration of several powerful ideas: procedural geometry, pixel abstraction, sub-cell rendering, and Unicode-aware ANSI graphics. Rather than printing precomposed ASCII art, the program *computes* each visible element dynamically, producing a shaded, ornamented tree that feels surprisingly dimensional despite the severe constraints of a terminal environment.

![image-20260104134017222](/home/fld/work/p/xmas-tree/tree.png)This quick scribbling explains how the program works, from its data model to its rendering pipeline.

## A Pixel-Based Mental Model

The most important conceptual shift in this program is the decision to treat the terminal as a **pixel device**, rather than a character grid. Each screen coordinate is evaluated independently, and the program decides what—if anything—should exist at that location.

To support this, the program introduces a `Pixel` structure that describes a logical pixel in full color:

- foreground RGB color
- background RGB color
- an optional Unicode symbol
- a flag indicating whether the symbol occupies two columns
- a marker for empty space

This abstraction allows foliage shading, decorations, and text to coexist within a single rendering model.

## Virtual Resolution and Half-Block Rendering

Terminals have limited vertical resolution, so the program employs a classic trick: **half-block rendering**.

Instead of rendering one virtual pixel per terminal row, the program maintains a virtual canvas twice as tall as the visible output. For each terminal row, two virtual rows are sampled and merged using Unicode block characters:

- `▀` for the upper half
- `▄` for the lower half

Foreground and background colors are assigned independently, effectively doubling the vertical resolution. This is the foundation that allows the tree’s silhouette and shading to appear smooth rather than jagged.

## Text as Geometry: “MERRY X-MAS”

Before any tree logic is applied, the program checks whether a given coordinate belongs to the greeting text. Each letter is defined as a bold 5×5 bitmap, hard-coded as integer matrices.

The text is rendered vertically:

- “MERRY” on the left side of the canvas
- “X-MAS” on the right

Because text rendering occurs first, it overrides all other content. The letters are colored gold for contrast, with subtle brightness variations to suggest a metallic sheen. This ensures readability without sacrificing visual cohesion.

## Procedural Tree Geometry

The tree itself is defined mathematically. Its horizontal radius increases linearly with height, forming a cone-like shape. The very top is compressed slightly to create a sharper crown, while small random perturbations along the edges prevent the outline from appearing unnaturally perfect.

Any coordinate falling outside the computed radius is considered empty space—unless it happens to host an edge decoration, such as a candle.

## Simulating Foliage Depth

Inside the tree boundary, foliage density is computed using cosine waves and depth-based biasing. This creates the illusion of layered branches, drooping slightly under their own weight.

Color is derived from this density and depth information:

- darker greens toward the interior
- lighter, warmer greens near the edges
- small random variations to break uniformity

The result is a tree that appears dense at the core and airy at the tips, despite being composed entirely of terminal characters.

## Garland System: Sine Waves in Disguise

Garlands are implemented as sinusoidal curves that wrap horizontally around the tree. Each garland has its own frequency, amplitude, and phase, causing it to weave naturally as the tree widens.

When a pixel lies close to a garland’s curve, it is replaced by a star-shaped Unicode glyph. Colors vary between gold, silver-white, and red, with occasional bright highlights to simulate twinkling lights. Garlands deliberately avoid the tree’s apex and branch tips to maintain visual clarity.

## Structural Decorations and Edge Candles

Candles placed along the outer edge of the tree serve a structural role. They appear at regular vertical intervals and are only drawn if they align exactly with the tree’s silhouette.

These candles use wide Unicode glyphs and warm orange coloring. When a candle protrudes beyond the foliage, the background is forced to black, preventing color bleed into empty space. This careful handling preserves the illusion of depth and separation.

## Randomized Interior Ornaments

Deeper within the tree, decorations are placed probabilistically using a simple value-noise function combined with structural checks. Ornaments only appear where there is sufficient “branch support,” avoiding the appearance of floating decorations.

Possible ornaments include:

- snowflakes
- sparkles
- floral shapes
- colored baubles
- additional candles

Each ornament category has its own Unicode set, color palette, and rarity, ensuring visual variety without chaos.

## The Star and the Trunk

The star at the top of the tree is rendered explicitly, outside the procedural pipeline. This guarantees perfect centering and ensures that the focal point is never obscured by foliage or decorations.

At the base, a narrow brown trunk is drawn, accompanied by sparse snow dots that fade outward. This grounds the tree visually and completes the scene.

## Rendering Pipeline Summary

For each terminal cell, the program follows a consistent process:

1. Compute two virtual pixels
2. If either pixel contains a Unicode symbol, render it directly
3. Otherwise, merge them using half-block characters
4. Apply ANSI foreground and background colors
5. Reset terminal state

This pipeline mirrors a simplified software rasterizer, adapted to the constraints of terminal output.
