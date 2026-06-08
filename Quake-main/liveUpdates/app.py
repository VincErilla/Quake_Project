from flask import Flask, jsonify, render_template
from threading import Thread
import time
import random  # Simulating sensor data

app = Flask(__name__)

# Global buffer to store all data points
data_buffer = {'x': [], 'y': []}


def sensor_simulator():
    """Simulates incoming data from sensors."""
    while True:
        # Simulate new sensor data
        new_x = len(data_buffer['x'])  # Increment x (time or sample number)
        new_y = random.randint(0, 100)  # Simulate y (sensor reading)

        # Add new data to the buffer
        data_buffer['x'].append(new_x)
        data_buffer['y'].append(new_y)

        time.sleep(2)  # Simulate 2-second intervals between readings


@app.route('/data')
def get_data():
    """API endpoint to send the complete buffer to the frontend."""
    return jsonify(data_buffer)


@app.route('/')
def index():
    """Serve the HTML page."""
    return render_template('index.html')


if __name__ == '__main__':
    # Start sensor simulator thread in the background
    Thread(target=sensor_simulator, daemon=True).start()
    app.run(debug=True)
