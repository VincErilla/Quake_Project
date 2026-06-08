import serial
import time
import numpy as np

# File name for storing calibration data
file_name = "sensor10_data.txt"

# Open serial connection
# Adjust 'COM14' as needed for your port
ser = serial.Serial('COM13', 115200, timeout=1)


def read_accelerometer_data():
    # Attempt to read a line from the serial connection and parse it
    line = ser.readline().decode().strip()
    if line:
        try:
            # Extract the x, y, z values from the line
            # Matches the format: "1,x:0.000,y:0.000,z:0.000"
            parts = line.split(',')
            x = float(parts[1].split(':')[1])
            y = float(parts[2].split(':')[1])
            z = float(parts[3].split(':')[1])
            return x, y, z
        except (ValueError, IndexError):
            # Skip if there's any parsing error
            return None
    return None


def take_measurement():
    # Initialize list to store 25 samples for the accelerometer
    samples = []

    print("Taking 25 samples for the accelerometer...")

    # Collect 25 samples
    while len(samples) < 25:
        data = read_accelerometer_data()
        if data:
            x, y, z = data
            samples.append([x, y, z])

    # Calculate averages
    samples_array = np.array(samples)
    averages = np.mean(samples_array, axis=0)

    # Save calibration data to file
    with open(file_name, 'a') as f:
        f.write(f"{averages[0]} {averages[1]} {averages[2]}\n")
        
    print("Calibration data saved successfully.")


# Main loop
try:
    while True:
        input("Press Enter to take a calibration measurement...")
        take_measurement()
except KeyboardInterrupt:
    print("Calibration terminated.")
finally:
    ser.close()