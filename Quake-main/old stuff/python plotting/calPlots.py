import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

# Load accelerometer data from text files
file_names = ["accel1.txt", "accel2.txt", "accel3.txt", "accel4.txt"]
data = [pd.read_csv(file, sep=" ", header=None, names=["x", "y", "z"]) for file in file_names]

# Define calibration matrices and vectors for each accelerometer
# Replace these with your actual calibration matrices and vectors
A1 = np.array([[1.012109, 0.019718, 0.005139], [0.019718, 1.004314, -0.004488], [0.005139, -0.004488, 0.996756]])  # Example matrix for accelerometer 1
A2 = np.array([[1.041262, 0.014128, 0.013988], [0.014128, 1.031887, -0.006028], [0.013988, -0.006028, 0.997951]])  # Example matrix for accelerometer 2
A3 = np.array([[1.027074, 0.015129, 0.014124], [0.015129, 1.045481, -0.001403], [0.014124, -0.001403, 0.989934]])  # Example matrix for accelerometer 3
A4 = np.array([[1.168706, -0.025001, 0.002490], [-0.025001, 1.398430, -0.037674], [0.002490, -0.037674, 1.016952]])  # Example matrix for accelerometer 4

b1 = np.array([0.511286, 0.101569, -0.385165])  # Example bias vector for accelerometer 1
b2 = np.array([0.068587, -0.002000, 0.079425])  # Example bias vector for accelerometer 2
b3 = np.array([0.019478, 0.056889, 0.197483])  # Example bias vector for accelerometer 3
b4 = np.array([7.778200, -1.290990, 0.009814])  # Example bias vector for accelerometer 4

# Store matrices and vectors in lists for easy iteration
A_matrices = [A1, A2, A3, A4]
b_vectors = [b1, b2, b3, b4]

# Apply calibration for each accelerometer
calibrated_data = []
for i in range(4):
    raw_coords = data[i][["x", "y", "z"]].values.T  # Get the raw data (h)
    
    # Perform the calibration: A^(-1) * (b - h)
    # Invert the matrix A
    A_inv = np.linalg.inv(A_matrices[i])
    
    # Subtract bias vector b from raw data h
    bias_subtracted = b_vectors[i].reshape(3, 1) - raw_coords
    
    # Calibrate the data
    calibrated_coords = A_inv @ bias_subtracted
    calibrated_data.append(calibrated_coords.T)

# Plotting function for 2D (XY, XZ, YZ) and 3D views
def plot_data(raw, calibrated, title_prefix):
    fig, axs = plt.subplots(2, 2, figsize=(12, 10), subplot_kw={'projection': None})
    
    # XY, XZ, YZ plots for raw and calibrated data
    axs[0, 0].plot(raw[:, 0], raw[:, 1], 'r.', label="Raw")
    axs[0, 0].plot(calibrated[:, 0], calibrated[:, 1], 'b.', label="Calibrated")
    axs[0, 0].set_xlabel("X"); axs[0, 0].set_ylabel("Y")
    axs[0, 0].set_title(f"{title_prefix} XY Plane")
    
    axs[0, 1].plot(raw[:, 0], raw[:, 2], 'r.', label="Raw")
    axs[0, 1].plot(calibrated[:, 0], calibrated[:, 2], 'b.', label="Calibrated")
    axs[0, 1].set_xlabel("X"); axs[0, 1].set_ylabel("Z")
    axs[0, 1].set_title(f"{title_prefix} XZ Plane")
    
    axs[1, 0].plot(raw[:, 1], raw[:, 2], 'r.', label="Raw")
    axs[1, 0].plot(calibrated[:, 1], calibrated[:, 2], 'b.', label="Calibrated")
    axs[1, 0].set_xlabel("Y"); axs[1, 0].set_ylabel("Z")
    axs[1, 0].set_title(f"{title_prefix} YZ Plane")

    # 3D plot for raw and calibrated data
    ax3d = fig.add_subplot(224, projection='3d')
    ax3d.scatter(raw[:, 0], raw[:, 1], raw[:, 2], c='r', label="Raw")
    ax3d.scatter(calibrated[:, 0], calibrated[:, 1], calibrated[:, 2], c='b', label="Calibrated")
    ax3d.set_xlabel("X"); ax3d.set_ylabel("Y"); ax3d.set_zlabel("Z")
    ax3d.set_title(f"{title_prefix} 3D Plot")

    # Legends and layout adjustments
    for ax in axs.flat: ax.legend()
    plt.tight_layout()
    plt.show()

# Plot each accelerometer's data
for i in range(4):
    plot_data(data[i].values, calibrated_data[i], title_prefix=f"Accelerometer {i+1}")
