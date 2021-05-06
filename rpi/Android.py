import bluetooth as bt
import socket
import os

#os.system("hcitool dev") #Get RPi hardware address
#os.system("sudo hcitool scan") #Scan for discoverable bluetooth devices(Get Nexus 7 hardware address)
#os.system("sdptool browse 08:60:6E:A5:BB:B4 |egrep 'Service Name: |Channel:'") #Eg 08:60:6E:A5:BB:B4 is Nexus 7's address

#Bluetooth configuration settings
RFCOMM_PORT = 6
RPI_MAC_ADDRESS = "B8:27:EB:11:52:B8" 
SOCKET_BUFFER_SIZE = 1024
LOCALE = "utf-8"


class Android:
	def __init__(self):
		os.system("sudo hciconfig hci0 piscan") #Make RPi discoverable

		self.server_sock = None
		self.client_sock = None

		self.server_sock = bt.BluetoothSocket(bt.RFCOMM)
		self.server_sock.bind((RPI_MAC_ADDRESS,RFCOMM_PORT))
		self.server_sock.listen(1)

		print("Listening for Android connection on RFCOMM channel: {}..".format(RFCOMM_PORT))


	def connect(self):
		retry = True
		while retry:
			try:
				self.client_sock,client_address = self.server_sock.accept()
				print("Android connection accepted from {}".format(client_address))
				retry = False

			except Exception as e:
				print("Android connection failed. Error: {}".format(e))

				if self.client_sock is not None:
					try:
						self.client_sock.close()
					except:
						pass

					self.client_sock = None

				print("Retrying Bluetooth connection to Android..")
					

	def disconnect(self):
		try:
			if self.client_sock is not None:
				self.client_sock.close()
				self.server_sock.close()
				self.client_sock = None
				self.server_sock = None

				print("Android bluetooth disconnected successfully")

			else:
				print("Android bluetooth is not connected to disconnect.")

		except Exception as e:
			print("Android bluetooth failed to disconnect. Error: {}".format(e))


	def receive(self):
		try:
			message = self.client_sock.recv(SOCKET_BUFFER_SIZE).strip()
			if len(message) > 0:
				print("From Android:")
				print(message.decode(LOCALE))
				return message.decode(LOCALE)
			
			return None

		except Exception as e:
			print("Failed to receive input from Android to RPi. Error: {}".format(e))


	def send(self,message):
		try:
			print("To Android:")
			print(message)
			self.client_sock.send(message.encode(LOCALE))

		except Exception as e:
			print("Failed to send from RPI to Android. Error {}".format(e))


	def receivefromAndroid(self):
		while True:
			message = android.receive()
			#if message is not None:
			#	print("readAndroid {}".format(message))


	def sendtoAndroid(self):
		message = input()
		while not(message =="quit"): 
			#print(message)
			android.send(message)
			message = input()


if __name__ == "__main__":

	android = Android()
	android.connect()

	#android.sendtoAndroid()
	android.receivefromAndroid()
