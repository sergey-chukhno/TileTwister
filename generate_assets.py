import math
from PIL import Image, ImageDraw

def create_rounded_rect(size, radius, output_path):
    # Create valid size (supersampled for quality)
    scale = 4
    w, h = size[0] * scale, size[1] * scale
    r = radius * scale
    
    # Create transparent image
    img = Image.new('RGBA', (w, h), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # Draw White Rounded Rect
    draw.rounded_rectangle([(0, 0), (w-1, h-1)], radius=r, fill=(255, 255, 255, 255))
    
    # Downsample for Anti-Aliasing
    img = img.resize(size, resample=Image.LANCZOS)
    
    img.save(output_path)
    print(f"Generated {output_path}")

if __name__ == "__main__":
    # Create 128x128 tile with 16px radius
    create_rounded_rect((128, 128), 20, "assets/tile_rounded.png")
