import random
import pgzrun

try:
    import serial
except ImportError:
    serial = None


# -----------------------------
# User settings
# -----------------------------

WIDTH = 600
HEIGHT = 500

# Change this if your Pico appears on a different COM port.
# Check VSCode Serial Monitor or Device Manager if needed.
SERIAL_PORT = "COM3"
BAUD_RATE = 115200

LANE_LEFT_X = WIDTH // 3
LANE_RIGHT_X = 2 * WIDTH // 3

PLAYER_Y = HEIGHT - 60
PLAYER_SIZE = 40

OBSTACLE_SIZE = 45
OBSTACLE_SPEED_START = 3
SPAWN_TIME_START = 70


# -----------------------------
# Serial setup
# -----------------------------

ser = None
serial_connected = False
serial_message = "Serial: not connected"

if serial is not None:
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0)
        ser.reset_input_buffer()
        serial_connected = True
        serial_message = f"Serial: connected to {SERIAL_PORT}"
    except Exception as e:
        serial_connected = False
        serial_message = f"Serial: not connected ({SERIAL_PORT})"


# -----------------------------
# Game variables
# -----------------------------

player_lane = 0
player_x = LANE_LEFT_X

obstacles = []
score = 0
game_over = False

spawn_timer = 0
spawn_time = SPAWN_TIME_START
obstacle_speed = OBSTACLE_SPEED_START

button_down = False
last_button_down = False


def reset_game():
    global player_lane, player_x
    global obstacles, score, game_over
    global spawn_timer, spawn_time, obstacle_speed
    global button_down, last_button_down

    player_lane = 0
    player_x = LANE_LEFT_X

    obstacles = []
    score = 0
    game_over = False

    spawn_timer = 0
    spawn_time = SPAWN_TIME_START
    obstacle_speed = OBSTACLE_SPEED_START

    button_down = False
    last_button_down = False


def switch_lane():
    global player_lane, player_x

    if player_lane == 0:
        player_lane = 1
        player_x = LANE_RIGHT_X
    else:
        player_lane = 0
        player_x = LANE_LEFT_X


def read_pico_button():
    """
    Reads serial messages from the Pico.

    Expected Pico messages:
        B,0
        B,1
    """

    if ser is None:
        return None

    latest_value = None

    try:
        while True:
            line = ser.readline().decode(errors="ignore").strip()
            print(line)

            if line == "":
                break

            print(line)  # debug line; remove later if you want

            parts = line.split(",")

            if len(parts) == 2 and parts[0] == "B":
                if parts[1] == "1":
                    latest_value = True
                elif parts[1] == "0":
                    latest_value = False

    except Exception as e:
        print("Serial read error:", e)
        return None

    return latest_value


def update():
    global button_down, last_button_down
    global spawn_timer, score, game_over
    global spawn_time, obstacle_speed

    # -----------------------------
    # Read physical Pico button
    # -----------------------------
    pico_value = read_pico_button()

    if pico_value is not None:
        button_down = pico_value
    else:
        # Keyboard fallback for testing without Pico
        button_down = keyboard.space

    # Detect only the instant the button becomes pressed.
    # This prevents one long press from switching lanes repeatedly.
    button_pressed_event = button_down and not last_button_down
    last_button_down = button_down

    if button_pressed_event:
        if game_over:
            reset_game()
            return
        else:
            switch_lane()

    if game_over:
        return

    # -----------------------------
    # Spawn obstacles
    # -----------------------------
    spawn_timer += 1

    if spawn_timer >= spawn_time:
        spawn_timer = 0

        lane = random.choice([0, 1])

        if lane == 0:
            x = LANE_LEFT_X
        else:
            x = LANE_RIGHT_X

        obstacle = {
            "x": x,
            "y": -OBSTACLE_SIZE,
            "lane": lane
        }

        obstacles.append(obstacle)

    # -----------------------------
    # Move obstacles
    # -----------------------------
    for obstacle in obstacles:
        obstacle["y"] += obstacle_speed

    # Remove obstacles that left the screen
    obstacles[:] = [o for o in obstacles if o["y"] < HEIGHT + OBSTACLE_SIZE]

    # -----------------------------
    # Score and difficulty
    # -----------------------------
    score += 1

    if score % 400 == 0:
        obstacle_speed += 0.5

        if spawn_time > 35:
            spawn_time -= 5

    # -----------------------------
    # Collision detection
    # -----------------------------
    player_rect = Rect(
        (player_x - PLAYER_SIZE // 2, PLAYER_Y - PLAYER_SIZE // 2),
        (PLAYER_SIZE, PLAYER_SIZE)
    )

    for obstacle in obstacles:
        obstacle_rect = Rect(
            (obstacle["x"] - OBSTACLE_SIZE // 2, obstacle["y"] - OBSTACLE_SIZE // 2),
            (OBSTACLE_SIZE, OBSTACLE_SIZE)
        )

        if player_rect.colliderect(obstacle_rect):
            game_over = True


def draw():
    screen.clear()

    # Background
    screen.fill((20, 20, 30))

    # Title
    screen.draw.text(
        "Pico Lane Dodger",
        center=(WIDTH // 2, 25),
        fontsize=36,
        color="white"
    )

    # Serial status
    screen.draw.text(
        serial_message,
        topleft=(10, 55),
        fontsize=22,
        color="gray"
    )

    # Instructions
    screen.draw.text(
        "Press Pico button or SPACE to switch lanes",
        center=(WIDTH // 2, 85),
        fontsize=24,
        color="white"
    )

    # Draw lane divider
    screen.draw.line(
        (WIDTH // 2, 110),
        (WIDTH // 2, HEIGHT),
        "gray"
    )

    # Draw lane labels
    screen.draw.text(
        "LEFT",
        center=(LANE_LEFT_X, 120),
        fontsize=24,
        color="gray"
    )

    screen.draw.text(
        "RIGHT",
        center=(LANE_RIGHT_X, 120),
        fontsize=24,
        color="gray"
    )

    # Draw player
    player_rect = Rect(
        (player_x - PLAYER_SIZE // 2, PLAYER_Y - PLAYER_SIZE // 2),
        (PLAYER_SIZE, PLAYER_SIZE)
    )

    screen.draw.filled_rect(player_rect, "cyan")

    # Draw obstacles
    for obstacle in obstacles:
        obstacle_rect = Rect(
            (obstacle["x"] - OBSTACLE_SIZE // 2, obstacle["y"] - OBSTACLE_SIZE // 2),
            (OBSTACLE_SIZE, OBSTACLE_SIZE)
        )

        screen.draw.filled_rect(obstacle_rect, "red")

    # Draw score
    screen.draw.text(
        f"Score: {score}",
        topleft=(10, 10),
        fontsize=30,
        color="white"
    )

    # Game over message
    if game_over:
        screen.draw.text(
            "GAME OVER",
            center=(WIDTH // 2, HEIGHT // 2 - 30),
            fontsize=60,
            color="yellow"
        )

        screen.draw.text(
            "Press Pico button or SPACE to restart",
            center=(WIDTH // 2, HEIGHT // 2 + 30),
            fontsize=30,
            color="white"
        )


pgzrun.go()
