import socket
import os

WIFI_IP = '192.168.17.17'
WIFI_PORT = 1234
SOCKET_BUFFER_SIZE = 1024
LOCALE = "utf-8"

class Algorithm:
	def __init__(self):
		self.server_sock = None
		self.client_sock = None

		self.server_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.server_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		self.server_sock.bind((WIFI_IP,WIFI_PORT))
		self.server_sock.listen(1)

		print("Listening for WIFI connection on port: {}..".format(WIFI_PORT))


	def connect(self):
		retry = True
		while retry:
			try:
				self.client_sock,client_address = self.server_sock.accept()
				print("Wifi connection for Algorithm accepted from {}".format(client_address))
				retry = False

			except Exception as e:
				print("Wifi connection failed. Error: {}".format(e))

				if self.client_sock is not None:
					try:
						self.client_sock.close()
					except:
						pass

					self.client_sock = None

				print("Retrying Wifi connection to Algorithm..")
					

	def disconnect(self):
		try:
			if self.client_sock is not None:
				self.client_sock.close()
				self.server_sock.close()
				self.client_sock = None
				self.server_sock = None

				print("Algorithm Wifi disconnected successfully")

			else:
				print("Algorithm Wifi is not connected to disconnect.")

		except Exception as e:
			print("Algorithm Wifi failed to disconnect. Error: {}".format(e))


	def receive(self):
		try:
			message = self.client_sock.recv(SOCKET_BUFFER_SIZE).strip()
			if len(message) > 0:
				print("From Algorithm:")
				print(message.decode(LOCALE))
				return message
			
			return None

		except Exception as e:
			print("Failed to receive input from Algorithm to RPi. Error: {}".format(e))


	def send(self,message):
		try:
			print("To Algorithm:")
			print(message)
			self.client_sock.send(message.encode(LOCALE))

		except Exception as e:
			print("Failed to send from RPI to Algorithm. Error {}".format(e))


	def receivefromAlgorithm(self):
		while True:
			message = algorithm.receive()


	def sendtoAlgorithm(self):
		message = input()
		while not(message =="quit"): 
		#	print(message)
			algorithm.send(message)
			message = input()


if __name__ == "__main__":

	algorithm = Algorithm()
	algorithm.connect()

	#algorithm.sendtoAlgorithm()
	algorithm.receivefromAlgorithm()