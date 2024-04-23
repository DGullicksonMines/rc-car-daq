import struct

with open("DAQ/data.bin", "wb") as f:
	f.write(bytes(bytearray([0])))
	f.write(struct.pack("d", 9.5))
	f.write(bytes(bytearray([1])))
	f.write(struct.pack("d", 1.3))
	f.write(struct.pack("d", 2.3))
	f.write(struct.pack("d", 3.3))
	f.write(struct.pack("d", 4.3))
	f.write(struct.pack("d", 5.3))
	f.write(struct.pack("d", 6.3))
	f.write(bytes(bytearray([5])))
	f.write(struct.pack("d", 102.5))
	f.write(bytes(bytearray([0])))
	f.write(struct.pack("d", 10.2))
