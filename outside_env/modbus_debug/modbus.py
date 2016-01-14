#!/usr/bin/env python
import sys
import serial
from CRCModules.CRC16 import CRC16

def main():
    data = "\x01\x03\x22\x00\x01 \x00\x03\xd0\x90  \x01\xf4 \x00\x1e  \x00\x04 \x00\x05 \x00\x6E \x00\x07 \x00\x08 \x00\x09 \x00\x0A \x00\x0B \x02 \x00 \x00\x00\x00\x00\x00\x00"
    data = ''.join(data.split(" "))
    crc = CRC16(modbus_flag = True).calculate(data)
    data += chr(crc%0x100)
    data += chr(crc/0x100)
    ser = serial.Serial("/dev/ttyUSB0", 9600, timeout=0.01)
    is_end = False
    while True:
        head = ser.read(1)
        if head:
            cmd = ser.read(1)
            print repr(cmd)
            while True:
                temp = ser.read(1)
                if not temp:
                    print "send"
                    if cmd == '\x03':
                        ser.write(data)
                    elif cmd == '\x10':
                        response_data = '\x01\x10\x00\x13\x00\x08'
                        crc = CRC16(modbus_flag = True).calculate(response_data)
                        response_data += chr(crc%0x100)
                        response_data += chr(crc/0x100)
                        ser.write(response_data)
                    break

    ser.close()
    return 0

if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        pass
