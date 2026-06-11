import numpy as np
import matplotlib.pyplot as plt

# Normalized displacement from -1 to 1
x = np.linspace(-1, 1, 100)

# Choice of Haptic effect: center detent / bump
w = 0.30
F = -x * np.exp(-(x / w) ** 2)

# Normalize the force to the range [-1, 1]
F_normalized = F / np.max(np.abs(F))

# Plot the normalized force
plt.figure()
plt.plot(x, F_normalized, label='Normalized Force')
plt.axhline(0, linewidth=0.8, color='k')
plt.axvline(0, linewidth=0.8, color='k')
plt.xlabel('Normalized Displacement, x')
plt.ylabel('Normalized Desired Force, F_desired')
plt.title('Haptic Effect: Center Detent')
plt.grid(True)
plt.xlim(-1, 1)
plt.ylim(-1.1, 1.1)
plt.legend()
plt.savefig('python plot.png', dpi=300)
plt.show()