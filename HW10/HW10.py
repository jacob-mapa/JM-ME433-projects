import pgzrun
import serial
import threading
import time

WIDTH = 600
HEIGHT = 400

PORT = "COM3"
BAUD = 115200

button_value = 0
serial_status = "Disconnected"

def serial_thread():
    global button_value, serial_status
    try:
        ser = serial.Serial(PORT, BAUD, timeout=1)
        time.sleep(2)  # Wait for the serial connection to initialize
        serial_status = "Connected"
        while True:
            line = ser.readline().decode(errors='ignore').strip()
            if line == "0" or line == "1":
                button_value = int(line)
    except Exception as e:
        serial_status = "Serial Error: " + str(e)

thread = threading.Thread(target=serial_thread, daemon=True)
thread.start()

def draw():
    screen.clear()
    screen.draw.text("HW10 Python Graphics", center=(WIDTH / 2, 40), fontsize=40,)
    screen.draw.text("Serial: " + serial_status, center=(WIDTH / 2, 85), fontsize=28,)
    screen.draw.text("Button Value: " + str(button_value), center=(WIDTH / 2, 130), fontsize=32,)

    if button_value == 1:
        screen.draw.filled_circle((WIDTH / 2, HEIGHT / 2 + 50), 80, "green")
        screen.draw.text("BUTTON PRESSED", center=(WIDTH / 2, HEIGHT - 50), fontsize=45,)
    else:
        screen.draw.filled_circle((WIDTH / 2, HEIGHT / 2 + 50), 80, "gray")
        screen.draw.text("Press the Pico button", center=(WIDTH / 2, HEIGHT - 50), fontsize=35,)

pgzrun.go()