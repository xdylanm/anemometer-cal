# anemometer-cal
Calibration suite for an anemometer

## Overview
The anemometer uses an optical interrupter (RPI-441C1E) to count revolutions. An Arduino board (Seeeduino Xiao) is used to read the analog signal from the photointerrupter and write json packets over a serial interface. 

To calibrate the anemometer (relate rotational velocity and wind speed), a GPS is used to monitor speed over average intervals. Log files are generated which can be post-processed.
