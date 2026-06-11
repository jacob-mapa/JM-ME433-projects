import serial
import time

PORT = 'COM3'  # Update this to your Pico's serial port
BAUD_RATE = 115200

serial_port = serial.Serial(PORT, BAUD_RATE, timeout=0.01)
time.sleep(2)  # Wait for the serial connection to initialize

while True:
    line = serial_port.readline().decode(errors="ignore").strip()
    if line:
        print(line)