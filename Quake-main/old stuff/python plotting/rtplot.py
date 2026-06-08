import csv
import serial
import time
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# Constants
SERIAL_PORT = 'COM5'  # Adjust this to your COM port
BAUD_RATE = 115200

# Start serial connection
ser = serial.Serial(SERIAL_PORT, BAUD_RATE)

# Initialize empty lists to store data
x_vals = []
sensor1_data = {'x': [], 'y': [], 'z': [], 'rms': []}
sensor2_data = {'x': [], 'y': [], 'z': [], 'rms': []}
sensor3_data = {'x': [], 'y': [], 'z': [], 'rms': []}
sensor4_data = {'x': [], 'y': [], 'z': [], 'rms': []}

# Track the starting time
start_time = time.time()


def read_and_process_data():
    """Reads data from the serial port, processes, and stores it."""
    line = ser.readline().decode('utf-8').strip()
    sensor_values = line.split(', ')

    # Calculate elapsed time since the program started
    elapsed_time = time.time() - start_time
    x_vals.append(elapsed_time)

    # Store X, Y, Z values for each sensor
    for i, sensor in enumerate([sensor1_data, sensor2_data, sensor3_data, sensor4_data]):
        x = float(sensor_values[1 + i * 3])
        y = float(sensor_values[2 + i * 3])
        z = float(sensor_values[3 + i * 3])

        # Store individual readings
        sensor['x'].append(x)
        sensor['y'].append(y)
        sensor['z'].append(z)

        # Calculate RMS and store it
        rms_value = (x**2 + y**2 + z**2) ** 0.5
        sensor['rms'].append(rms_value)

    # Print data to the console (for optional debugging)
    print(f"Time: {elapsed_time:.2f}s, Data: {sensor_values}")


def update_plot(frame):
    """Updates the plot with new data."""
    read_and_process_data()

    plt.cla()  # Clear the plot

    # Plot RMS values for each sensor over time
    plt.plot(x_vals, sensor1_data['rms'], label='Sensor 1 RMS')
    plt.plot(x_vals, sensor2_data['rms'], label='Sensor 2 RMS')
    plt.plot(x_vals, sensor3_data['rms'], label='Sensor 3 RMS')
    plt.plot(x_vals, sensor4_data['rms'], label='Sensor 4 RMS')

    plt.xlabel("Time (sec)")
    plt.ylabel("RMS Value")
    plt.legend()


def on_close(event):
    """Writes all collected data and RMS values to a CSV file when the plot closes."""
    with open('SensorData.csv', 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)

        # Write the header
        writer.writerow(['Time',
                         'Sensor1_X', 'Sensor1_Y', 'Sensor1_Z', 'Sensor1_RMS',
                         'Sensor2_X', 'Sensor2_Y', 'Sensor2_Z', 'Sensor2_RMS',
                         'Sensor3_X', 'Sensor3_Y', 'Sensor3_Z', 'Sensor3_RMS',
                         'Sensor4_X', 'Sensor4_Y', 'Sensor4_Z', 'Sensor4_RMS'])

        # Write data rows
        for i in range(len(x_vals)):
            row = [x_vals[i]]  # Time value
            for sensor in [sensor1_data, sensor2_data, sensor3_data, sensor4_data]:
                row.extend([sensor['x'][i], sensor['y'][i],
                           sensor['z'][i], sensor['rms'][i]])
            writer.writerow(row)


# Setup the plot
fig, ax = plt.subplots()
fig.canvas.mpl_connect('close_event', on_close)  # Save data on plot close

# Animation function to update the plot in real-time
ani = FuncAnimation(fig, update_plot, interval=1000)

# Start the plot
plt.show()
