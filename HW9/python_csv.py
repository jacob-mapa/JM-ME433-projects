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

#def calculate_sample_rate(t):
    # calculate the sample rate from the time data
    dt = np.mean(np.diff(t)) # time difference between samples
    fs = 1.0 / dt # sample rate in Hz
    return fs

files = ["sigA.csv", "sigB.csv", "sigC.csv", "sigD.csv"]
for filename in files:
    t, data1 = read_csv_file(filename)
    #fs = calculate_sample_rate(t)
    #print("File:", filename)
    #print("Sample rate:", fs, "Hz")
    #print("Time step:", 1.0/fs, "s")
    #print()
    plt.figure()
    plt.plot(t, data1, "k")
    plt.xlabel("Time (s)")
    plt.ylabel("Signal")
    plt.title(filename + " RawSignal")
    plt.grid(True)
    plt.show()
