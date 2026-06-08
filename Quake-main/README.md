# Quake
MJ's Quake project Bert Cooper Engineering Lab 2024

## Setup
Plug the microcontroller into your computer via USB and open the Arduino IDE. At the top left, select the dropdown and then select the connected device (should be Adafruit Feather ESP32 V2). This is what device Arduino will be uploading the code to every time.
![image](https://github.com/user-attachments/assets/947dad64-3156-420c-8916-0195e75f2935)

## Calibration
First, you will need to upload calibration345.ino to the microcontroller, which will output the data in the format that cal.py needs to do the calibration. You should rename the output files in cal.py each time you calibrate a sensor so that it's easy to keep track of everything. Download the magneto software (https://atadiat.com/en/download/magneto-v1-2-magnetometer-soft-hard-iron-calibration-tool-by-sailboatinstruments/) that will give us our A^-1 matrix and b matrix that will be changed in each of the files before being uploaded to the microcontroller. To get the proper matrices set the graviational norm to 1 and upload the respective sensor, it will be one at a time.

