import serial
import time
import numpy as np

# File names for storing calibration data
file_names = ["sensor1_data.txt", "sensor2_data.txt"]

# Open serial connection
# Adjust '/dev/ttyUSB0' as needed for your port
ser = serial.Serial('COM14', 115200, timeout=1)


def read_accelerometer_data():
    # Attempt to read a line from the serial connection and parse it
    line = ser.readline().decode().strip()
    if line:
        try:
            # Extract the accelerometer ID and x, y, z values from the line
            parts = line.split(',')
            accel_id = int(parts[0])
            x = float(parts[1].split(':')[1])
            y = float(parts[2].split(':')[1])
            z = float(parts[3].split(':')[1])
            return accel_id, x, y, z
        except (ValueError, IndexError):
            # Skip if there's any parsing error
            return None
    return None


def take_measurement():
    # Initialize arrays to store 25 samples for each accelerometer
    samples = {1: [], 2: [], 3: [], 4: []}

    print("Taking 25 samples for each accelerometer...")

    # Collect 25 samples for each accelerometer
    while any(len(samples[i]) < 25 for i in range(1, 3)):
        data = read_accelerometer_data()
        if data:
            accel_id, x, y, z = data
            if len(samples[accel_id]) < 25:  # Only store if under 25 samples
                samples[accel_id].append([x, y, z])

    # Calculate averages for each accelerometer
    averages = {}
    for accel_id in range(1,3):
        if samples[accel_id]:
            samples_array = np.array(samples[accel_id])
            averages[accel_id] = np.mean(samples_array, axis=0)

    # Save calibration data to respective files
    for accel_id, avg_values in averages.items():
        if accel_id <= len(file_names):
            with open(file_names[accel_id - 1], 'a') as f:
                f.write(f"{avg_values[0]} {avg_values[1]} {avg_values[2]}\n")
    else:
        print("Calibration data saved for all accelerometers.")


# Main loop
try:
    while True:
        input("Press Enter to take a calibration measurement...")
        take_measurement()
except KeyboardInterrupt:
    print("Calibration terminated.")
finally:
    ser.close()
