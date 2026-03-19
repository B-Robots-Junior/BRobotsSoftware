import pygame
from typing import Tuple

pygame.init()

width = 5
height = 5

PIX_PER_SQARE = 100

WALL_WIDTH = PIX_PER_SQARE * 0.2

WALL_COLOR = (100, 100, 100)

GRID_COLOR = (0, 0, 0)

surface = pygame.display.set_mode((width * PIX_PER_SQARE, height * PIX_PER_SQARE))

bot_tiles = [[[False, False, False, False, 0] for _ in range(width)] for _ in range(height)]
top_tiles = [[[False, False, False, False, 0] for _ in range(width)] for _ in range(height)]

type_colors = [
    (180, 180, 180), # normal tile
    (50, 50, 200), # blue tile
    (0, 0, 0), # black tile
    (255, 255, 0), # ramp tile
    (255, 255, 255) # checkpoint
]

def draw_tile(surface: pygame.Surface, tile_pos: Tuple[int, int], tiles):
    tile = tiles[tile_pos[1]][tile_pos[0]]
    tl = (tile_pos[0] * PIX_PER_SQARE, tile_pos[1] * PIX_PER_SQARE)
    pygame.draw.rect(surface, type_colors[tile[4]], pygame.Rect(tl[0], tl[1], PIX_PER_SQARE, PIX_PER_SQARE))

    pygame.draw.rect(surface, WALL_COLOR, pygame.Rect(tl[0], tl[1], WALL_WIDTH, WALL_WIDTH))
    pygame.draw.rect(surface, WALL_COLOR, pygame.Rect(tl[0] + PIX_PER_SQARE - WALL_WIDTH, tl[1], WALL_WIDTH, WALL_WIDTH))
    pygame.draw.rect(surface, WALL_COLOR, pygame.Rect(tl[0], tl[1] + PIX_PER_SQARE - WALL_WIDTH, WALL_WIDTH, WALL_WIDTH))
    pygame.draw.rect(surface, WALL_COLOR, pygame.Rect(tl[0] + PIX_PER_SQARE - WALL_WIDTH, tl[1] + PIX_PER_SQARE - WALL_WIDTH, WALL_WIDTH, WALL_WIDTH))

    if tile[0]:
        pygame.draw.rect(surface, WALL_COLOR, pygame.Rect(tl[0], tl[1], PIX_PER_SQARE, WALL_WIDTH))
    if tile[1]:
        pygame.draw.rect(surface, WALL_COLOR, pygame.Rect(tl[0] + PIX_PER_SQARE - WALL_WIDTH, tl[1], WALL_WIDTH, PIX_PER_SQARE))
    if tile[2]:
        pygame.draw.rect(surface, WALL_COLOR, pygame.Rect(tl[0], tl[1] + PIX_PER_SQARE - WALL_WIDTH, PIX_PER_SQARE, WALL_WIDTH))
    if tile[3]:
        pygame.draw.rect(surface, WALL_COLOR, pygame.Rect(tl[0], tl[1], WALL_WIDTH, PIX_PER_SQARE))

def draw_grid(surface: pygame.Surface):
    for x in range(width):
        pygame.draw.line(surface, GRID_COLOR, (x * PIX_PER_SQARE, 0), (x * PIX_PER_SQARE, height * PIX_PER_SQARE))
    for y in range(height):
        pygame.draw.line(surface, GRID_COLOR, (0, y * PIX_PER_SQARE), (width * PIX_PER_SQARE, y * PIX_PER_SQARE))

def click(pos: Tuple[int, int], tiles):
    tile_pos = (pos[0] // PIX_PER_SQARE, pos[1] // PIX_PER_SQARE)
    sub_pos = (pos[0] % PIX_PER_SQARE, pos[1] % PIX_PER_SQARE)

    if sub_pos[1] <= WALL_WIDTH:
        if tile_pos[1] - 1 >= 0:
            tiles[tile_pos[1]][tile_pos[0]][0] = not tiles[tile_pos[1]][tile_pos[0]][0]
            tiles[tile_pos[1] - 1][tile_pos[0]][2] = tiles[tile_pos[1]][tile_pos[0]][0]

    if sub_pos[0] >= (PIX_PER_SQARE - WALL_WIDTH):
        if tile_pos[0] + 1 < width:
            tiles[tile_pos[1]][tile_pos[0]][1] = not tiles[tile_pos[1]][tile_pos[0]][1]
            tiles[tile_pos[1]][tile_pos[0] + 1][3] = tiles[tile_pos[1]][tile_pos[0]][1]

    if sub_pos[1] >= (PIX_PER_SQARE - WALL_WIDTH):
        if tile_pos[1] + 1 < height:
            tiles[tile_pos[1]][tile_pos[0]][2] = not tiles[tile_pos[1]][tile_pos[0]][2]
            tiles[tile_pos[1] + 1][tile_pos[0]][0] = tiles[tile_pos[1]][tile_pos[0]][2]
    
    if sub_pos[0] <= WALL_WIDTH:
        if tile_pos[0] - 1 >= 0:
            tiles[tile_pos[1]][tile_pos[0]][3] = not tiles[tile_pos[1]][tile_pos[0]][3]
            tiles[tile_pos[1]][tile_pos[0] - 1][1] = tiles[tile_pos[1]][tile_pos[0]][3]
    
    if sub_pos[0] > WALL_WIDTH and sub_pos[0] < (PIX_PER_SQARE - WALL_WIDTH) and sub_pos[1] > WALL_WIDTH and sub_pos[1] < (PIX_PER_SQARE - WALL_WIDTH):
        tiles[tile_pos[1]][tile_pos[0]][4] = (tiles[tile_pos[1]][tile_pos[0]][4] + 1) % len(type_colors)
    
    return tiles

def save():
    global bot_tiles
    global top_tiles
    filepath = input("Output file name: ")
    with open(filepath, "w") as file:
        for line in bot_tiles:
            for tile in line[:-1:]:
                file.write(f"{int(tile[0])};{int(tile[1])};{int(tile[2])};{int(tile[3])};{tile[4]}")
                file.write(", ")
            file.write(f"{int(line[-1][0])};{int(line[-1][1])};{int(line[-1][2])};{int(line[-1][3])};{line[-1][4]}\n")
        file.write("\n")
        for line in top_tiles:
            for tile in line[:-1:]:
                file.write(f"{int(tile[0])};{int(tile[1])};{int(tile[2])};{int(tile[3])};{tile[4]}")
                file.write(", ")
            file.write(f"{int(line[-1][0])};{int(line[-1][1])};{int(line[-1][2])};{int(line[-1][3])};{line[-1][4]}\n")

def load():
    global width
    global height
    global surface
    global bot_tiles
    global top_tiles
    filepath = input("Input file name: ")
    content = ""
    with open(filepath, "r") as file:
        content = file.read()
    bot_tiles = []
    top_tiles = []
    top = False
    for line in content.replace(" ", "").split("\n"):
        if line == "":
            top = True
            continue
        if top:
            top_tiles.append([])
        else:
            bot_tiles.append([])
        for tile in line.split(","):
            vals = tile.split(";")
            if top:
                top_tiles[-1].append([bool(int(vals[0])), bool(int(vals[1])), bool(int(vals[2])), bool(int(vals[3])), int(vals[4])])
            else:
                bot_tiles[-1].append([bool(int(vals[0])), bool(int(vals[1])), bool(int(vals[2])), bool(int(vals[3])), int(vals[4])])
    height = len(bot_tiles)
    width = len(bot_tiles[0])

def write_out_map(file, tiles, bottom):
    for j, line in enumerate(tiles):
            file.write("\t\t{")
            for n, tile in enumerate(line):
                if tile[4] == 3: # ramp tile
                    dir = 0
                    for i, val in enumerate(tile):
                        if not val:
                            dir = i
                            break
                    file.write(f"RTC({dir}, {bottom}, 1, {int(tile[4]) + 1})")
                else:
                    file.write(f"TC({int(tile[0])}, {int(tile[1])}, {int(tile[2])}, {int(tile[3])}, 1, {int(tile[4]) + 1})")
                if n != len(line) - 1:
                    file.write(", ")
            file.write("}")
            if j != len(tiles) - 1:
                file.write(",")
            file.write("\n")
    
def output():
    global bot_tiles
    global top_tiles
    global width
    global height
    filepath = input("Input output file: ")
    with open(filepath, "w") as file:
        file.write(
            "#ifndef _TEST_MAZE_H_\n" +
            "#define _TEST_MAZE_H_\n\n" +
            "#include <map.h>\n\n"
            f"#define MAZE_WIDTH {width}\n" +
            f"#define MAZE_HEIGHT {height}\n" +
            f"#define MAZE_LAYERS 2\n\n" +
            "uint8_t test_maze[MAZE_LAYERS][MAZE_HEIGHT][MAZE_WIDTH] = {\n"
        )
        file.write("\t{\n")

        write_out_map(file, bot_tiles, 1)

        file.write("\t},\n")
        file.write("\t{\n")

        write_out_map(file, top_tiles, 0)

        file.write("\t}\n")

        file.write("};\n\n")
        file.write("#endif")

for x in range(width):
    top_tiles[0][x][0] = True
    top_tiles[-1][x][2] = True
    bot_tiles[0][x][0] = True
    bot_tiles[-1][x][2] = True

for y in range(height):
    top_tiles[y][0][3] = True
    top_tiles[y][-1][1] = True
    bot_tiles[y][0][3] = True
    bot_tiles[y][-1][1] = True

running = True
on_top = False
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        elif event.type == pygame.KEYDOWN:
            if event.key == pygame.K_s:
                save()
            elif event.key == pygame.K_l:
                load()
            elif event.key == pygame.K_o:
                output()
            elif event.key == pygame.K_UP:
                on_top = True
            elif event.key == pygame.K_DOWN:
                on_top = False
        elif event.type == pygame.MOUSEBUTTONDOWN:
            if on_top:
                click(pygame.mouse.get_pos(), top_tiles)
            else:
                click(pygame.mouse.get_pos(), bot_tiles)

    for x in range(width):
        for y in range(height):
            if on_top:
                draw_tile(surface, (x, y), top_tiles)
            else:
                draw_tile(surface, (x, y), bot_tiles)

    draw_grid(surface)

    pygame.display.flip()

pygame.quit()