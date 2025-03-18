import argparse
import serial
import sys
import time

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('port', help='Port device', type=str)
    parser.add_argument('--open', help='Open the roof', action='store_true')
    parser.add_argument('--close', help='Close the roof', action='store_true')
    parser.add_argument('--heartbeat', help='Set a heartbeat value', default=-1, type=int)
    args = parser.parse_args()
    
    port = serial.Serial(args.port, 4800, 5)
    if args.open:
        port.write(b'\xf1')
    elif args.close:
        port.write(b'\xf2')

    if 0 <= args.heartbeat <= 240:
        port.write(bytes([args.heartbeat]))

    try:
        while True:
            sys.stdout.write(port.readline().decode('ascii'))
    except KeyboardInterrupt:
        port.write(b'\xff')
        sys.stdout.flush()
