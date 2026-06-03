import serial
import time
import numpy as np
import matplotlib.pyplot as plt

PORT = 'COM3'  # Update this to your Pico's COM port
BAUD_RATE = 115200
N_SAMPLES = 800

def read_hx711_data(port: str, baud_rate: int, n_samples: int):
    time_ms = []
    raw_values = []
    filtered_values = []

    with serial.Serial(port, baud_rate, timeout=1) as ser:
        time.sleep(2)  # Wait for the serial connection to initialize

        ser.reset_input_buffer()  # Clear any existing data in the buffer
        ser.reset_output_buffer()

        ser.write(f"{n_samples}\n".encode("utf-8"))  # Send the number of samples to the Pico

        reading_table = False
    
        while True:
            line = ser.readline().decode("utf-8", errors="ignore").strip()
            if line == "":
                continue
            print("RX:", line)  # Debug: print received line

            if line.startswith("time_ms"):
                reading_table = True
                continue  # Skip header line
            if line.startswith("END"):
                print("Data collection complete.")
                break  # End of data
            if reading_table:
                parts = line.split(",")
                if len(parts) == 3:
                    try:
                        time_ms.append(float(parts[0]))
                        raw_values.append(float(parts[1]))
                        filtered_values.append(float(parts[2]))
                    except ValueError:
                        print("Skipped bad data line:", line)
    
    return np.array(time_ms), np.array(raw_values), np.array(filtered_values)

def plot_time_data(time_ms, raw_values, filtered_values):
    time_s = time_ms / 1000.0  # Convert ms to seconds
    plt.figure()
    plt.plot(time_s, raw_values, label='Raw', alpha=0.5)
    plt.plot(time_s, filtered_values, label='IIR filtered', linewidth=2)
    plt.xlabel('Time (s)')
    plt.ylabel('HX711 Value')
    plt.title('HX711 Data Over Time')
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.savefig('hx711_time_data.png', dpi=300)
    plt.show()

def plot_fft(time_ms, raw_values, filtered_values):
    time_s = time_ms / 1000.0  # Convert ms to seconds
    #n = len(time_s)
    dt = np.mean(np.diff(time_s))  # Average sampling interval
    fs = 1.0 / dt  # Sampling frequency

    print(f"Estimated sampling rate: {fs:.2f} Hz")
    print(f"Estimated Nyquist frequency: {fs/2:.2f} Hz")

    raw_zero_mean = raw_values - np.mean(raw_values)
    filtered_zero_mean = filtered_values - np.mean(filtered_values)

    n = len(time_s)

    raw_fft = np.abs(np.fft.rfft(raw_zero_mean)) 
    filtered_fft = np.abs(np.fft.rfft(filtered_zero_mean))
    freqs = np.fft.rfftfreq(n, d=dt)

    plt.figure()
    plt.plot(freqs, raw_fft, label='Raw', alpha=0.5)
    plt.plot(freqs, filtered_fft, label='IIR filtered', linewidth=2)
    plt.xlabel('Frequency (Hz)')
    plt.ylabel('Magnitude')
    plt.title('FFT of HX711 Data')
    plt.xlim(0, fs/2)
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.savefig('hx711_fft.png', dpi=300)
    plt.show()

def main():
    time_ms, raw_values, filtered_values = read_hx711_data(PORT, BAUD_RATE, N_SAMPLES)
    print(f"Received {len(time_ms)} samples.")
    if len(time_ms) < 2:
        print("Not enough data received to plot.") #check COM port and Pico serial output
        print(f"Number of samples received: {len(time_ms)}")
        return
    data = np.column_stack((time_ms, raw_values, filtered_values))
    np.savetxt('hx711_data.csv', data, delimiter=',', header='time_ms,raw,filtered', comments='', fmt=['%f', '%d', '%.2f'])
    plot_time_data(time_ms, raw_values, filtered_values)
    plot_fft(time_ms, raw_values, filtered_values)

if __name__ == "__main__":
    main()