import math
import random

def circle(out, centre, radius, resolution = 1000):
    for i in range(0, resolution):
        angle = i / resolution * 2 * math.pi
        y = math.sin(angle) * radius + centre[0]
        x = math.cos(angle) * radius + centre[1]
        out.write("{} {}\n".format(x, y))

def main():
    radius_range = (5,10)
    x_range = (-1000, 1000)
    y_range = (-1000, 1000)
    count = 1000
    resolution = 1000

    out = open('circles.txt', 'w')

    for i in range(0, count):
        x = random.uniform(x_range[0], x_range[1])
        y = random.uniform(y_range[0], y_range[1])
        r = random.uniform(radius_range[0], radius_range[1])
        circle(out, (x,y), r, resolution)

main()
