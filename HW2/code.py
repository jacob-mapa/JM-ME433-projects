import board
import pwmio # get access to PWM
import time

# control GP14 with PWM
led = pwmio.PWMOut(board.GP14, variable_frequency=True)
led.frequency = 50 # in hz
led.duty_cycle = 0 # initially off, at 16bit number so max on is 65535

while True:
    # start duty cycle at 0, every loop increase by 100 
    # until getting to the max of 65535
    for i in range(0, 65535, 100):
        led.duty_cycle = i
        time.sleep(0.01)
