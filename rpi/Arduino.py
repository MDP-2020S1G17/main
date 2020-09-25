import serial

SERIAL_PORT = "/dev/ttyACM0"
BAUD_RATE = 57600
LOCALE = "utf-8"

class Arduino:
	def __init__(self):
		self.connection = None


	def connect(self):
		retry = True
		while retry:
			try:
				print("Connecting to Arduino..")

				self.connection = serial.Serial(SERIAL_PORT, BAUD_RATE)

				print("Successfully connected to Arduino")
				retry = False

			except Exception as e:
				print("Arduino connection failed. Error: {}".format(e))
				if self.connection is not None:
					try:
						self.connection.close()
					except:
						pass

					self.connection = None

				print("Retrying connection to Arduino..")
	

	def disconnect(self):
		try:
			if self.connection is not None:
				self.connection.close()
				self.connection = None

				print("Arduino disconnected successfully")

			else:
				print("Arduino is not connected to disconnect.")

		except Exception as e:
			print("Arduino failed to disconnect. Error: {}".format(e))


	def receive(self):
		try:
			message = self.connection.readline().strip()
			if len(message) > 0:
				print("From Arduino:")
				print(message.decode(LOCALE))
				return message.decode(LOCALE)
			
			return None

		except Exception as e:
			print("Failed to receive input from Arduino to RPi. Error: {}".format(e))


	def send(self,message):
		try:
			print("To Arduino:")
			print(message)
			self.connection.write(message.encode(LOCALE))

		except Exception as e:
			print("Failed to send from RPI to Arduino. Error {}".format(e))

 
	def receivefromArduino(self):
		while True:
			message = self.receive()
			#if message is not None:
			#	print("readArduino {}".format(message))


	def sendtoArduino(self):
		message = input()
		while not(message =="quit"): 
		#	print(message)
			self.send(message)
			message = input()


if __name__ == "__main__":

	arduino = Arduino()
	arduino.connect()

	#arduino.sendtoArduino()
	arduino.receivefromArduino()
