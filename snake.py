import curses
import random
import time

def main(stdscr):
    curses.curs_set(0)
    stdscr.nodelay(1)
    stdscr.timeout(150)
    sh, sw = stdscr.getmaxyx()

    curses.start_color()
    curses.init_pair(1, curses.COLOR_GREEN, curses.COLOR_BLACK)
    curses.init_pair(2, curses.COLOR_YELLOW, curses.COLOR_BLACK)
    curses.init_pair(3, curses.COLOR_YELLOW, curses.COLOR_BLACK)
    curses.init_pair(4, curses.COLOR_RED, curses.COLOR_BLACK)
    curses.init_pair(5, curses.COLOR_CYAN, curses.COLOR_BLACK)

    stdscr.clear()
    stdscr.border()
    stdscr.addstr(1, 2, "Score: 0", curses.color_pair(3))
    msg = "Arrows/2/8/4/6, Space/5=PAUSE, Q=quit"
    stdscr.addstr(sh-1, 2, msg[:sw-4])
    stdscr.refresh()

    score = 0
    snake = [[sh//2, sw//4], [sh//2, sw//4-1], [sh//2, sw//4-2]]
    food = [sh//2, sw//2]
    direction = curses.KEY_RIGHT
    paused = False
    game_over = False

    for i, s in enumerate(snake):
        stdscr.addch(s[0], s[1], '@' if i == 0 else '×', curses.color_pair(1))
    stdscr.addch(food[0], food[1], '●', curses.color_pair(2))
    stdscr.refresh()

    for i in range(3, 0, -1):
        stdscr.addstr(sh//2, sw//2-1, str(i), curses.A_BOLD)
        stdscr.refresh()
        time.sleep(1)
        stdscr.addch(sh//2, sw//2-1, ' ')

    while not game_over:
        if paused:
            stdscr.timeout(-1)
            stdscr.addstr(sh//8, sw//2-6, "( PAUSED )", curses.color_pair(5) | curses.A_BOLD)
            key = stdscr.getch()
            stdscr.addstr(sh//8, sw//2-6, "          ")
            stdscr.timeout(150)
            if key == ord('q'):
                break
            if key in [ord(' '), ord('5')]:
                paused = False
            continue

        key = stdscr.getch()
        if key == ord('q'):
            break
        if key in [ord(' '), ord('5')]:
            paused = True
            continue
        if key in [curses.KEY_UP, ord('2')]:
            direction = curses.KEY_UP
        elif key in [curses.KEY_DOWN, ord('8')]:
            direction = curses.KEY_DOWN
        elif key in [curses.KEY_LEFT, ord('4')]:
            direction = curses.KEY_LEFT
        elif key in [curses.KEY_RIGHT, ord('6')]:
            direction = curses.KEY_RIGHT

        tail = snake[-1]
        stdscr.addch(tail[0], tail[1], ' ')

        head = snake[0]
        if direction == curses.KEY_UP:
            new_head = [head[0]-1, head[1]]
        elif direction == curses.KEY_DOWN:
            new_head = [head[0]+1, head[1]]
        elif direction == curses.KEY_LEFT:
            new_head = [head[0], head[1]-1]
        else:
            new_head = [head[0], head[1]+1]

        if new_head[0] == 0:
            new_head[0] = sh-2
        elif new_head[0] == sh-1:
            new_head[0] = 1
        if new_head[1] == 0:
            new_head[1] = sw-2
        elif new_head[1] == sw-1:
            new_head[1] = 1

        if new_head in snake:
            game_over = True
            break

        snake.insert(0, new_head)
        stdscr.addch(new_head[0], new_head[1], '@', curses.color_pair(1))
        stdscr.addch(head[0], head[1], '×', curses.color_pair(1))

        if new_head == food:
            score += 10
            stdscr.addstr(1, 2, f"Score: {score:2d} ", curses.color_pair(3))
            stdscr.addch(food[0], food[1], ' ')
            while True:
                food = [random.randint(1, sh-2), random.randint(1, sw-2)]
                if food not in snake:
                    stdscr.addch(food[0], food[1], '●', curses.color_pair(2))
                    break
        else:
            snake.pop()

        stdscr.refresh()

    stdscr.nodelay(0)
    stdscr.clear()
    stdscr.addstr(sh//2-2, sw//2-10, "GAME OVER", curses.color_pair(4) | curses.A_BOLD)
    stdscr.addstr(sh//2, sw//2-15, f"Final Score: {score}", curses.color_pair(3))
    stdscr.addstr(sh//2+2, sw//2-12, "Press any key to exit")
    stdscr.refresh()
    stdscr.getch()

if __name__ == "__main__":
    curses.wrapper(main)

