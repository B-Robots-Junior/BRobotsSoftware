import serial
import struct
import time
from enum import Enum, IntEnum

class PacketType(IntEnum):
    INVALID             = 0, # invalid packet type (placeholder)
    DEBUG_CONSOLE       = 1, # packet to send to console
    CAM_RESET           = 2, # send this and reason to restart cam communication if anything goes out of sequence or is invalid (must also reset state if this is recieved)
    CAM_DETECT_SIDE     = 3, # initial packet that the camera sends to the mega containing one byte (first and only byte: rigth: 0, left: 1)
    CAM_TOF_DATA        = 4, # send the respective side 2 bytes val front and val back
    CAM_VICTIM_VALID    = 5, # get from the camera that the victim is valid, 1 byte type
    CAM_VICTIM_INVALID  = 6, # get from the camera that the victim is invalid, no data
    MAP_TILE            = 7, # send map tile data 3 bytes x, y, z position and 1 byte map data
    LOCATION            = 8  # send 3 bytes x, y, z location and 1 byte rotation 0-3

class Packet:
    type: PacketType = PacketType.INVALID
    data: bytes = bytes()

    def __init__(self, type: PacketType, data: bytes):
        self.type = type
        self.data = data

def getPacket(ser) -> Packet:
    type, = struct.unpack("<b", ser.read())
    #if type not in PacketType._value2member_map_:
    #    print("Packet is not in PacketType enum!")
    #    return Packet(PacketType.INVALID, bytes())
    amount, = struct.unpack("<b", ser.read())
    return Packet(PacketType(type), ser.read(amount))

def sendPacket(ser, packet: Packet):
    ser.write(struct.pack("<b", packet.type.value))
    ser.write(struct.pack("<b", len(packet.data)))
    ser.write(packet.data)

ser = serial.Serial("COM10", 115200)

time.sleep(2)

sendPacket(ser, Packet(PacketType.CAM_DETECT_SIDE, struct.pack("<b", 1)))

while True:

    packet = getPacket(ser)

    match packet.type:
        case PacketType.DEBUG_CONSOLE:
            print("debug log: " + packet.data.decode("utf-8"))
        case PacketType.CAM_RESET:
            print("Cam comms error: " + packet.data.decode("utf-8"))
            sendPacket(ser, Packet(PacketType.CAM_DETECT_SIDE, struct.pack("<b", 1)))
        case PacketType.CAM_DETECT_SIDE:
            print("Wrong packet type should not recieve: CAM_DETECT_SIDE!")
        case PacketType.CAM_TOF_DATA:
            fr, bk, =struct.unpack("<bb", packet.data)
            print(f"front: {fr}, back: {bk}")
            sendPacket(ser, Packet(PacketType.CAM_VICTIM_VALID, struct.pack("<b", 2)))