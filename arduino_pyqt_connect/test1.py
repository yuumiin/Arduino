import serial

port = 'COM4'
rate = 9600

ard = serial.Serial(port, rate)


a = str(input())
trans = a.encode('utf-8')

if a == '1' :
    ard.write(trans)