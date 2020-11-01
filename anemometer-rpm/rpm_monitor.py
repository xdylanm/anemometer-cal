import serial
import time
import json

class RpmMonitor:
    def __init__(self, port):
        self.ser = serial.Serial()
        self.ser.baudrate = 9600
        self.ser.port = port

        timeout_count = 0
        while not self.ser.is_open:
            try:
                self.ser.open()
            except serial.SerialException:
                print(".",end="",flush=True)
            time.sleep(1)
            timeout_count += 1
            if timeout_count > 10:
                break

        print("")

    def ready(self):
        return self.ser.is_open

    def port(self):
        return self.ser.port

    def start(self):
        if self.ready():
            self.ser.write(b'R')     # resume

    def stop(self):
        if self.ready():
            self.ser.write(b'P')     # pause

    def setAvgInterval(self, msecs):
        if self.ready():
            self.ser.write(bytes("T{:d};".format(msecs),'utf-8'));

    def next(self):
        data = json.loads(self.ser.readline().decode("utf-8"))
        return (data['vtheta'], data['vtheta']/6.)
        
    def log(self):
        while self.ser.is_open:
            data = json.loads(self.ser.readline().decode("utf-8"))
            rpms = data['vtheta']/6.    # deg/s * 60s/min * 1rev/360degs
            print("{}  {:7.1f}  {:7.1f}".format(time.strftime("%H:%M:%S"), data['vtheta'], rpms))

      
