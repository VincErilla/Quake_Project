import csv
import serial
import time
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from datetime import datetime  # Import datetime for timestamp

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

# Generate a timestamp for the filename
timestamp = datetime.now().strftime('%Y-%m-%d_%H-%M-%S')
output_filename = f"SensorData_{timestamp}.csv"

# Open CSV file for writing
with open(output_filename, 'w', newline='') as csvfile:
    csv_writer = csv.writer(csvfile)
    csv_writer.writerow(['Time', 'Sensor1_X', 'Sensor1_Y', 'Sensor1_Z', 'Sensor1_RMS',
                         'Sensor2_X', 'Sensor2_Y', 'Sensor2_Z', 'Sensor2_RMS',
                         'Sensor3_X', 'Sensor3_Y', 'Sensor3_Z', 'Sensor3_RMS',
                         'Sensor4_X', 'Sensor4_Y', 'Sensor4_Z', 'Sensor4_RMS'])

    # Real-time plotting
    # plt.style.use('seaborn-darkgrid')
    fig, axs = plt.subplots(4, 1, figsize=(12, 8))

    def update(frame):
        # Read a line from the serial port
        line = ser.readline().decode('utf-8').strip()
        data = line.split(', ')

        if len(data) == 17:  # Ensure correct number of data points
            try:
                # Parse the data
                current_time = float(data[0])
                sensor1_data['x'].append(float(data[1]))
                sensor1_data['y'].append(float(data[2]))
                sensor1_data['z'].append(float(data[3]))
                sensor1_data['rms'].append(float(data[4]))

                sensor2_data['x'].append(float(data[5]))
                sensor2_data['y'].append(float(data[6]))
                sensor2_data['z'].append(float(data[7]))
                sensor2_data['rms'].append(float(data[8]))

                sensor3_data['x'].append(float(data[9]))
                sensor3_data['y'].append(float(data[10]))
                sensor3_data['z'].append(float(data[11]))
                sensor3_data['rms'].append(float(data[12]))

                sensor4_data['x'].append(float(data[13]))
                sensor4_data['y'].append(float(data[14]))
                sensor4_data['z'].append(float(data[15]))
                sensor4_data['rms'].append(float(data[16]))

                # Append the time
                x_vals.append(current_time)

                # Write the data to CSV
                csv_writer.writerow([current_time] +
                                    [sensor1_data['x'][-1], sensor1_data['y'][-1], sensor1_data['z'][-1], sensor1_data['rms'][-1]] +
                                    [sensor2_data['x'][-1], sensor2_data['y'][-1], sensor2_data['z'][-1], sensor2_data['rms'][-1]] +
                                    [sensor3_data['x'][-1], sensor3_data['y'][-1], sensor3_data['z'][-1], sensor3_data['rms'][-1]] +
                                    [sensor4_data['x'][-1], sensor4_data['y'][-1], sensor4_data['z'][-1], sensor4_data['rms'][-1]])

                # Plotting
                axs[0].clear()
                axs[1].clear()
                axs[2].clear()
                axs[3].clear()

                # Plot RMS of each sensor
                axs[0].plot(x_vals, sensor1_data['rms'], label='Sensor 1 RMS', color='b')
                axs[0].set_title('Sensor 1 RMS')
                axs[1].plot(x_vals, sensor2_data['rms'], label='Sensor 2 RMS', color='g')
                axs[1].set_title('Sensor 2 RMS')
                axs[2].plot(x_vals, sensor3_data['rms'], label='Sensor 3 RMS', color='r')
                axs[2].set_title('Sensor 3 RMS')
                axs[3].plot(x_vals, sensor4_data['rms'], label='Sensor 4 RMS', color='c')
                axs[3].set_title('Sensor 4 RMS')

                for ax in axs:
                    ax.set_xlabel('Time (s)')
                    ax.set_ylabel('RMS')
                    ax.legend(loc='upper right')
                    ax.grid()

            except ValueError as e:
                print(f"Error parsing line: {line} - {e}")

    ani = FuncAnimation(fig, update, interval=1000)  # Update every second
    plt.tight_layout()
    plt.show()
