from PIL import Image, ImageDraw, ImageFont

im = Image.new("RGB", (128,24))
d = ImageDraw.Draw(im)
font = ImageFont.load_default(size=12)
d.text((12,0), "Teststation", (200,200,200), font=font)
im.save("test.png")

