import math
import random
import argparse

def circle(out, centre, radius, resolution = 1000):
    for i in range(0, resolution):
        angle = i / resolution * 2 * math.pi
        y = math.sin(angle) * radius + centre[0]
        x = math.cos(angle) * radius + centre[1]
        out.write("{} {}\n".format(x, y))

def circles():
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

def line(out, x1, y1, x2, y2, resolution = 1000):
    step = 1/resolution
    for i in range(0, resolution):
        z = i * step
        x = z * x1 + (1-z) * x2
        y = z * y1 + (1-z) * y2
        out.write("{} {}\n".format(x, y))


def lines():
    x_range = (-1000, 1000)
    y_range = (-1000, 1000)
    max_len = 100
    count = 1000
    resolution = 1000

    def rand(rng):
        return random.uniform(rng[0], rng[1])

    out = open('lines.txt', 'w')

    for i in range(0, count):
        x1 = rand(x_range)
        y1 = rand(y_range)
        x2 = x1 + rand((-max_len, +max_len))
        y2 = y1 + rand((-max_len, +max_len))
        line(out, x1, y1, x2, y2, resolution)

def main():

    parser = argparse.ArgumentParser()
    parser.add_argument('mode', default='lines', nargs='?')
    args = parser.parse_args()

    if args.mode == 'lines':
        lines()
    elif args.mode == 'circles':
        circles()
    else:
        print("ERROR: Unknown mode: " + args.mode)

main()
