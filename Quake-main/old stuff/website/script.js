function generateRandomData() {
    const now = new Date().toLocaleTimeString();
    const x = Math.random() * 10;
    const y = Math.random() * 10;
    const z = Math.random() * 10;

    const rms = Math.sqrt((x ** 2 + y ** 2 + z ** 2) / 3).toFixed(2); // RMS Calculation

    return { timestamp: now, x: x.toFixed(2), y: y.toFixed(2), z: z.toFixed(2), rms };
}

function generateGPSData() {
    const now = new Date().toLocaleTimeString();
    const latitude = (Math.random() * 180 - 90).toFixed(4);
    const longitude = (Math.random() * 360 - 180).toFixed(4);
    const altitude = (Math.random() * 100).toFixed(2);

    // Format coordinates
    const coordinates = `${latitude}°N, ${longitude}°E`;

    return { timestamp: now, latitude, longitude, altitude, coordinates };
}

function updateStream(streamId, data, columns) {
    const stream = document.getElementById(streamId);
    const row = document.createElement('div');
    row.className = 'stream-row';

    // Create a row based on the provided data and number of columns
    row.innerHTML = columns.map((col) => `<span>${data[col]}</span>`).join('');

    // Prepend the new row to the top of the stream
    stream.insertBefore(row, stream.firstChild);

    // Timeout to allow DOM to update before checking scroll position
    setTimeout(() => {
        // Scroll to the top if not already at the top
        const isAtTop = stream.scrollTop === 0;
        if (!isAtTop) {
            stream.scrollTop = stream.scrollHeight; // Keep scroll position at the bottom
        }
    }, 0);
}

// Simulate data updates every second
setInterval(() => updateStream('xyz-stream', generateRandomData(), ['timestamp', 'x', 'y', 'z', 'rms']), 1000);
setInterval(() => updateStream('gps-stream', generateGPSData(), ['timestamp', 'latitude', 'longitude', 'altitude', 'coordinates']), 1000);

function exportData() {
    alert('Export functionality not implemented yet.');
}
