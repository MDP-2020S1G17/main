import Arduino
import Android
import Algorithm
import threading
import queue
#AND = Android
#ARD = Arduino
#ALG = Algorithm

class Rpi:
	def __init__(self):
		self.android = Android.Android()
		#self.arduino = Arduino.Arduino()
		self.algorithm = Algorithm.Algorithm()

		print("Establishing connection to Android, Arduino and Algorithm..")

		thread_connectAndroid = threading.Thread(target=self.android.connect)
		#thread_connectArduino = threading.Thread(target=self.arduino.connect)
		thread_connectAlgorithm= threading.Thread(target=self.algorithm.connect)

		connectThreads = []
		connectThreads.append(thread_connectAndroid)
		#connectThreads.append(thread_connectArduino)
		connectThreads.append(thread_connectAlgorithm)

		for thread in connectThreads:
			thread.start()

		for thread in connectThreads:
			thread.join()

		print("All connection established!")

		self.androidQueue = queue.Queue(maxsize=0)
		#self.arduinoQueue = queue.Queue(maxsize=0)
		self.algorithmQueue = queue.Queue(maxsize=0)


	def receiveAndroid(self):
		while True:
			message = self.android.receive()

			if message[0:3] == "ARD":
				self.arduinoQueue.put_nowait(message[4:])

			elif message[0:3] == "ALG":
				self.algorithmQueue.put_nowait(message[4:])

			else:
				print("Message syntax error in AND. Message not send to ARD or ALG.")


	def sendAndroid(self):
		while True:
			if not self.androidQueue.empty():
				message = self.androidQueue.get_nowait()
				self.android.send(message)


	def receiveArduino(self):
		while True:
			message = self.arduino.receive()

			if message[0:3] == "AND":
				self.androidQueue.put_nowait(message[4:])

			elif message[0:3] == "ALG":
				self.algorithmQueue.put_nowait(message[4:])
			else:
				print("Message syntax error in ARD. Message not send to AND or ALG.")


	def sendArduino(self):
		while True:
			if not self.arduinoQueue.empty():
				message = self.arduinoQueue.get_nowait()
				self.arduino.send(message)


	def receiveAlgorithm(self):
		while True:
			message = self.algorithm.receive()

			if message[0:3] == "AND":
				self.androidQueue.put_nowait(message[4:])

			elif message[0:3] == "ARD":
				self.arduinoQueue.put_nowait(message[4:])
			else:
				print("Message syntax error in ALG. Message not send to AND or ARD.")


	def sendAlgorithm(self):
		while True:
			if not self.algorithmQueue.empty():
				message = self.algorithmQueue.get_nowait()
				self.algorithm.send(message)


	def startMultithreading(self):
		try:
			threads = []
			thread_sendAndroid = threading.Thread(target=self.sendAndroid)
			thread_receiveAndroid = threading.Thread(target=self.receiveAndroid)
			thread_sendArduino = threading.Thread(target=self.sendArduino)
			thread_receiveArduino = threading.Thread(target=self.receiveArduino)
			thread_sendAlgorithm = threading.Thread(target=self.sendAlgorithm)
			thread_receiveAlgorithm = threading.Thread(target=self.receiveAlgorithm)

			threads.append(thread_sendAndroid)
			threads.append(thread_receiveAndroid)
			threads.append(thread_sendArduino)
			threads.append(thread_receiveArduino)
			threads.append(thread_sendAlgorithm)
			threads.append(thread_receiveAlgorithm)

			for thread in threads:
				thread.daemon = True #To terminate thread when main process ends.
				thread.start()

			print("Started all threads.")

			#Wait for threads to finish, to keep main process alive.
			for thread in threads:
				thread.join()

		except Exception as e:
			print("Error in multithreading. Error {}".format(e))


if __name__ == "__main__":
	rpi = Rpi()
	try:
		rpi.startMultithreading()
	except KeyboardInterrupt:
		rpi.android.disconnect()
		rpi.arduino.disconnect()
		rpi.algorithm.disconnect()


