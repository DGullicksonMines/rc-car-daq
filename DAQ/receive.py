#! /usr/bin/env python

import dataclasses as dc
import typing as t
import struct
import subprocess as sp

ip_addr = input("IP Addr: ")
SSH_CMD = ["ssh", f"rcCar@{ip_addr}", "~/rc-car-daq/DAQ/out/daq"]
# SSH_CMD = ['type', 'data.bin']
CSV_FILE = "DAQ.csv"
LOG_FILE = "LOG.csv"

@dc.dataclass
class Entry:
	time: float | None
	cell_volts: tuple[float, float, float, float, float, float] # lowest->highest; V
	acceleration: tuple[float, float, float] # x, y, z; m/s^2
	angle: tuple[float, float, float] # roll, pitch, yaw; degrees
	location: tuple[float, float] # latitude, longitude
	satellites: int
	velocity: float
	rpms: tuple[float, float, float, float] # lr, rr, lf, rf
	steering: float
	throttle: float

	@staticmethod
	def new() -> "Entry":
		return Entry(
			None,
			(0.0, 0.0, 0.0, 0.0, 0.0, 0.0),
			(0.0, 0.0, 0.0),
			(0.0, 0.0, 0.0),
			(0.0, 0.0),
			0,
			0.0,
			(0.0, 0.0, 0.0, 0.0),
			0.0,
			0.0
		)

	@staticmethod
	def write_header(writer: t.TextIO):
		writer.write("time,cell0,cell1,cell2,cell3,cell4,cell5,")
		writer.write("acc_x,acc_y,acc_z,roll,pitch,yaw,")
		writer.write("latitude,longitude,satellites,velocity,")
		writer.write("rpm_lr,rpm_rr,rpm_lf,rpm_rf,")
		writer.write("steering,throttle \n")

	def write(self, writer: t.TextIO):
		if self.time is None: return
		writer.write(f"{self.time},")
		for cell in self.cell_volts: writer.write(f"{cell},")
		for acc in self.acceleration: writer.write(f"{acc},")
		for ang in self.angle: writer.write(f"{ang},")
		writer.write(f"{self.location[0]},{self.location[1]},")
		writer.write(f"{self.satellites},{self.velocity},")
		for rpm in self.rpms: writer.write(f"{rpm},")
		writer.write(f"{self.steering},{self.throttle} \n")

def read_double(reader: t.IO[bytes]) -> float:
	buffer = reader.read(8) #XXX double may be different sizes
	return struct.unpack("d", buffer)[0]

def main():
	# Start SSH
	print("Starting SSH...")
	ssh = sp.Popen(SSH_CMD, stdout=sp.PIPE, stdin=sp.PIPE, shell=True)
	# Login
	#TODO
	ssh_out = ssh.stdout
	assert ssh_out is not None
	# Open CSV
	with open(CSV_FILE, "w") as csv_file, open(LOG_FILE, "w") as log_file:
		Entry.write_header(log_file)
		entry = Entry.new()
		try:
			for c in iter(lambda: ssh_out.read(1), b""):
				# Get datum
				match c[0]:
					case 0: # Time
						# Send current data
						csv_file.truncate(0)
						Entry.write_header(csv_file)
						entry.write(csv_file)
						csv_file.flush()
						entry.write(log_file)
						log_file.flush()
						# Get time
						entry.time = read_double(ssh_out)
					case 1: # ADC Sample
						entry.cell_volts = (
							read_double(ssh_out),
							read_double(ssh_out),
							read_double(ssh_out),
							read_double(ssh_out),
							read_double(ssh_out),
							read_double(ssh_out)
						)
					case 2: # IMU Sample
						entry.acceleration = (
							read_double(ssh_out),
							read_double(ssh_out),
							read_double(ssh_out)
						)
						entry.angle = (
							read_double(ssh_out),
							read_double(ssh_out),
							read_double(ssh_out)
						)
					case 3: # GPS Sample
						entry.satellites = int(ssh_out.read(1))
						entry.location = (
							read_double(ssh_out),
							read_double(ssh_out)
						)
						entry.velocity = read_double(ssh_out)
					case 4: # RPM Sample
						entry.rpms = (
							read_double(ssh_out),
							read_double(ssh_out),
							read_double(ssh_out),
							read_double(ssh_out)
						)
					case 5: # PWM Sample
						entry.steering = read_double(ssh_out)
						#TODO add throttle
					case _: pass #TODO fine for now, but might make some garbage data to start
		except KeyboardInterrupt: pass

if __name__ == "__main__": main()
