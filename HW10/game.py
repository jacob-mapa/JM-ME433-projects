import pgzrun
import random
import time

try:
    import serial
except ImportError:
    serial = None


WIDTH = 600
HEIGHT = 500

# Change this if your Pico uses a different COM port
PORT = "COM3"
BAUD = 115200

# Game layout
LANES = [150, 300, 450]
PLAYER_Y = 430
PLAYER_SIZE = 40

# Serial variables
ser = None
button_state = 0

# Debounce / repeat-control variable
last_move_time = 0
MOVE_DELAY = 0.25  # seconds between allowed button actions

# Game variables
player_lane = 1
blocks = []
score = 0
game_over = False
spawn_timer = 0
fall_speed = 180


def connect_serial():
    global ser

    if serial is None:
        print("pyserial is not installed. Run: pip install pyserial")
        return

    try:
        ser = serial.Serial(PORT, BAUD, timeout=0.01)
        time.sleep(2)
        print(f"Connected to Pico on {PORT}")
    except Exception as e:
        ser = None
        print(f"Could not connect to Pico on {PORT}: {e}")
        print("The game will still run with SPACE as a backup control.")


def read_pico_serial():
    global button_state

    if ser is None:
        return

    try:
        line = ser.readline().decode(errors="ignore").strip()

        if line:
            print(line)  # debug: should print BTN,0 or BTN,1 in terminal

            parts = line.split(",")

            if len(parts) == 2 and parts[0] == "BTN":
                button_state = int(parts[1])

    except Exception as e:
        print(f"Serial read error: {e}")


def reset_game():
    global player_lane, blocks, score, game_over, spawn_timer, fall_speed

    player_lane = 1
    blocks = []
    score = 0
    game_over = False
    spawn_timer = 0
    fall_speed = 180


def move_player():
    global player_lane

    player_lane += 1

    if player_lane >= len(LANES):
        player_lane = 0


def spawn_block():
    lane = random.randint(0, len(LANES) - 1)

    block = {
        "lane": lane,
        "x": LANES[lane],
        "y": -30,
        "size": 45
    }

    blocks.append(block)


def rectangles_overlap(ax, ay, aw, ah, bx, by, bw, bh):
    return (
        ax < bx + bw and
        ax + aw > bx and
        ay < by + bh and
        ay + ah > by
    )


def update(dt):
    global spawn_timer, score, game_over, fall_speed, last_move_time

    read_pico_serial()

    now = time.time()

    # Physical Pico button OR backup keyboard spacebar
    control_pressed = button_state == 1 or keyboard.space

    # Debounced action:
    # Allows one movement/restart every MOVE_DELAY seconds while button is held
    if control_pressed and now - last_move_time > MOVE_DELAY:
        last_move_time = now

        if game_over:
            reset_game()
        else:
            move_player()

    if game_over:
        return

    spawn_timer += dt

    if spawn_timer > 0.9:
        spawn_block()
        spawn_timer = 0

    # Slowly make the game harder
    fall_speed = 180 + score * 3

    for block in blocks:
        block["y"] += fall_speed * dt

    # Remove blocks that passed the bottom
    remaining_blocks = []

    for block in blocks:
        if block["y"] > HEIGHT + 50:
            score += 1
        else:
            remaining_blocks.append(block)

    blocks[:] = remaining_blocks

    # Collision check
    player_x = LANES[player_lane] - PLAYER_SIZE / 2
    player_y = PLAYER_Y - PLAYER_SIZE / 2

    for block in blocks:
        block_x = block["x"] - block["size"] / 2
        block_y = block["y"] - block["size"] / 2

        if rectangles_overlap(
            player_x,
            player_y,
            PLAYER_SIZE,
            PLAYER_SIZE,
            block_x,
            block_y,
            block["size"],
            block["size"]
        ):
            game_over = True


def draw():
    screen.clear()

    # Background
    screen.fill((20, 20, 30))

    # Lane lines
    for x in LANES:
        screen.draw.line((x, 0), (x, HEIGHT), (70, 70, 80))

    # Score
    screen.draw.text(
        f"Score: {score}",
        (20, 20),
        fontsize=36,
        color="white"
    )

    # Serial status
    if ser is None:
        status = "Pico: not connected | SPACE backup enabled"
        status_color = "orange"
    else:
        status = f"Pico: connected on {PORT} | BTN={button_state}"
        status_color = "lightgreen"

    screen.draw.text(
        status,
        (20, 60),
        fontsize=24,
        color=status_color
    )

    # Draw player
    player_x = LANES[player_lane]

    screen.draw.filled_rect(
        Rect(
            (player_x - PLAYER_SIZE / 2, PLAYER_Y - PLAYER_SIZE / 2),
            (PLAYER_SIZE, PLAYER_SIZE)
        ),
        "cyan"
    )

    # Draw falling blocks
    for block in blocks:
        screen.draw.filled_rect(
            Rect(
                (block["x"] - block["size"] / 2, block["y"] - block["size"] / 2),
                (block["size"], block["size"])
            ),
            "red"
        )

    # Instructions
    screen.draw.text(
        "Press the physical Pico button to switch lanes.",
        (20, HEIGHT - 40),
        fontsize=24,
        color="white"
    )

    if game_over:
        screen.draw.text(
            "GAME OVER",
            center=(WIDTH / 2, HEIGHT / 2 - 40),
            fontsize=64,
            color="yellow"
        )

        screen.draw.text(
            "Press the Pico button to restart.",
            center=(WIDTH / 2, HEIGHT / 2 + 20),
            fontsize=32,
            color="white"
        )


connect_serial()
pgzrun.go()
