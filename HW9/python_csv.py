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

def get_fft(t, data1):
    fs = calculate_sample_rate(t)
    n = len(data1) # number of samples
    signal_no_dc = data1 - np.mean(data1) # remove DC component
    fft_values = np.fft.fft(signal_no_dc) / n # compute FFT
    freqs = np.fft.rfftfreq(n, d=1.0/fs) #

    positive = freqs > 0 # only positive frequencies
    freqs = freqs[positive]
    magnitude = np.abs(fft_values[positive]) # magnitude of FFT
    return freqs, magnitude

files = ["sigA.csv", "sigB.csv", "sigC.csv", "sigD.csv"]
for filename in files:
    t, data1 = read_csv_file(filename)
    freqs, magnitude = get_fft(t, data1)
    #fs = calculate_sample_rate(t)
    #print("File:", filename)
    #print("Sample rate:", fs, "Hz")
    #print("Time step:", 1.0/fs, "s")
    #print()
    plt.figure(figsize=(9, 6))

    plt.subplot(2, 1, 1)
    plt.plot(t, data1, "k")
    plt.xlabel("Time (s)")
    plt.ylabel("Signal")
    plt.title(filename + " Signal vs. Time")
    plt.grid(True)

    plt.subplot(2, 1, 2)
    plt.plot(freqs, magnitude, "b")
    plt.xlabel("Frequency (Hz)")
    plt.ylabel("Magnitude")
    plt.title(filename + " FFT")
    plt.grid(True)
    
    plt.tight_layout()
    plt.show()
