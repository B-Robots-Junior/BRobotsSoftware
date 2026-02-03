import serial
import struct
import time
from enum import Enum

class VictimType(Enum):
    INVALID     = 0
    PSI         = 1
    PHI         = 2
    OMEGA       = 3
    RING_SUM_0  = 4
    RING_SUM_1  = 5
    RING_SUM_2  = 6

class Side(Enum):
    INVALID = 0
    RIGHT   = 1
    LEFT    = 2

ser = serial.Serial("COM10", 9600)

print("Start!")

time.sleep(2)

ser.read_all()

# write that something has been found and on which side it has been found
ser.write(struct.pack("<b", Side.LEFT.value))

# read the next two bytes for front tof and back tof for the specific side
data = ser.read(2)
front, back = struct.unpack("<bb", data)

print(data)
print(f"{front}, {back}")

# send the victim type and the location
ser.write(struct.pack("<b", VictimType.PSI.value))

while True: # recieve and print debug data (not needed)
    if ser.in_waiting > 0:
        i, = struct.unpack("<b",ser.read(1))
        print(chr(i), end='', flush=True)
