#!/bin/python

import struct
from socket import *
from argparse import ArgumentParser


if __name__ == '__main__':
    parser = ArgumentParser()

    parser.add_argument('--type', type=str, default='GENERAL', choices={'GENERAL', 'TRANS', 'YAW', 'PITCH'})
    parser.add_argument('--action', type=str, choices={'RESET'})
    parser.add_argument('-s', '--speed', required=False, type=int)
    parser.add_argument('-a', '--accel', required=False, type=int)
    parser.add_argument('-t', '--target', required=False, type=int)

    args = parser.parse_args()

    sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM)

    sock.bind(('A8:7E:EA:BF:40:27', 1))
    sock.connect(('98:D3:71:F9:84:96', 1))

    args.type = args.type.upper()

    if args.type == 'GENERAL':
        if args.action == 'RESET':
            sock.send(int.to_bytes(0, 3, signed=True, byteorder='big'))
    elif args.type == 'TRANS':
        if args.speed:
            sock.send(bytes([65]) + int.to_bytes(args.speed, 2, signed=True, byteorder='little'))
        if args.accel:
            sock.send(bytes([66]) + int.to_bytes(args.accel, 2, signed=True, byteorder='little'))
        if args.target:
            sock.send(bytes([64]) + int.to_bytes(args.target, 2, signed=True, byteorder='little'))
    elif args.type == 'YAW':
        if args.speed:
            sock.send(bytes([129]) + int.to_bytes(args.speed, 2, signed=True, byteorder='big'))
        if args.accel:
            sock.send(bytes([130]) + int.to_bytes(args.accel, 2, signed=True, byteorder='big'))
        if args.target:
            sock.send(bytes([128]) + int.to_bytes(args.target, 2, signed=True, byteorder='big'))
    elif args.type == 'PITCH':
        if args.speed:
            sock.send(bytes([193]) + int.to_bytes(args.speed, 2, signed=True, byteorder='big'))
        if args.accel:
            sock.send(bytes([194]) + int.to_bytes(args.accel, 2, signed=True, byteorder='big'))
        if args.target:
            sock.send(bytes([192]) + int.to_bytes(args.target, 2, signed=True, byteorder='big'))
    print(int.from_bytes(sock.recv(2), byteorder='big'))
