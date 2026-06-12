import csv
import numpy as np
import matplotlib.pyplot as plt

def read_csv_file(filename):
    t = [] # column 0
    data1 = [] # column 1

    with open(filename, newline='') as f:
        # open the csv file
        reader = csv.reader(f)
        for row in reader:
            # read the rows 1 one by one
            t.append(float(row[0])) # leftmost column
            data1.append(float(row[1])) # second column
    return np.array(t), np.array(data1)

def calculate_sample_rate(t):
    # calculate the sample rate from the time data
    dt = np.mean(np.diff(t)) # time difference between samples
    fs = 1.0 / dt # sample rate in Hz
    return fs

def iir_filter(signal, A, B):
    filtered = np.zeros(len(signal))
    filtered[0] = signal[0]

    for i in range(1, len(signal)):
        filtered[i] = A * filtered[i-1] + B * signal[i]

    return filtered

#def moving_average_filter(signal, X):
    filtered = np.zeros(len(signal))
    for i in range(len(signal)):
        if i < X:
            filtered[i] = np.mean(signal[0:i+1]) # average of available samples
        else:
            filtered[i] = np.mean(signal[i-X+1:i+1]) # average of last X samples
    return filtered

def get_fft(t, data1):
    fs = calculate_sample_rate(t)
    n = len(data1) # number of samples
    signal_no_dc = data1 - np.mean(data1) # remove DC component
    fft_values = np.fft.fft(signal_no_dc) / n # compute FFT
    freqs = np.fft.rfftfreq(n, d=1.0/fs) #

    freqs = freqs[:n//2]
    magnitude = np.abs(fft_values[:n//2])
    return freqs, magnitude

def plot_time_comparision(t, raw, filtered, title):
    plt.figure()
    plt.plot(t, raw, "k", label="Unfiltered Signal")
    plt.plot(t, filtered, "r", label="Filtered Signal")
    plt.xlabel("Time (s)")
    plt.ylabel("Signal")
    plt.title(title)
    plt.grid(True)
    plt.legend()
    plt.show()

def plot_fft_comparision(t, raw, filtered, title):
    raw_freqs, raw_mag = get_fft(t, raw)
    filt_freqs, filt_mag = get_fft(t, filtered)

    plt.figure()
    plt.plot(raw_freqs, raw_mag, "k", label="Unfiltered FFT")
    plt.plot(filt_freqs, filt_mag, "r", label="Filtered FFT")
    plt.xlabel("Frequency (Hz)")
    plt.ylabel("Magnitude")
    plt.title(title)
    plt.legend()
    plt.grid(True)
    plt.show()

files = ["sigA.csv", "sigB.csv", "sigC.csv", "sigD.csv"]

iir_settings = {"sigA.csv": (0.98, 0.02), "sigB.csv": (0.95, 0.05), "sigC.csv": (0.90, 0.10), "sigD.csv": (0.90, 0.10)}

for filename in files:
    t, data1 = read_csv_file(filename)
    A, B = iir_settings[filename]
    filtered = iir_filter(data1, A, B)

    plot_time_comparision(t, data1, filtered, filename + " IIR Filter, A = " + str(A) + ", B = " + str(B))
    plot_fft_comparision(t, data1, filtered, filename + " IIR FFT Comparision, A = " + str(A) + ", B = " + str(B))
    #freqs, magnitude = get_fft(t, data1)
    #fs = calculate_sample_rate(t)
    #print("File:", filename)
    #print("Sample rate:", fs, "Hz")
    #print("Time step:", 1.0/fs, "s")
    #print()
    #plt.figure(figsize=(9, 6))

    #plt.subplot(2, 1, 1)
    #plt.plot(t, data1, "k")
    #plt.xlabel("Time (s)")
    #plt.ylabel("Signal")
    #plt.title(filename + " Signal vs. Time")
    #plt.grid(True)

    #plt.subplot(2, 1, 2)
    #plt.plot(freqs, magnitude, "b")
    #plt.xlabel("Frequency (Hz)")
    #plt.ylabel("Magnitude")
    #plt.title(filename + " FFT")
    #plt.grid(True)
    
    #plt.tight_layout()
    #plt.show()
