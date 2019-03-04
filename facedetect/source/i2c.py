import smbus
import time
import math

class I2C():



	def __init__(self,frequency=50):
	        self.address = 0x40 #address of the PCA9685
	        self.pre_scale_reg=0xFE
        	self.mode1_reg = 0x00

		#these registers are regular so could manage with just led0_on_l
		#and then addition for all others, however led0 and 1 in completion
		#for clairty in setSpeeds, they go up to led15 on board
        	self.led0_on_l=0x06
        	self.led0_on_h=0x07
        	self.led0_off_l=0x08
        	self.led0_off_h=0x09
        	self.led1_on_l=0x0A
        	self.led1_on_h=0x0B
        	self.led1_off_l=0x0C
        	self.led1_off_h=0x0D

		self.bus=smbus.SMBus(0) #open the bus on /dev/i2c-0
		self.frequency=frequency
		self.pre_scale=25000000.0 #clock is at 25 MHz
		self.pre_scale /=4096 #there are 4096 divisions
		self.pre_scale /= frequency #adjust to the frequency we want
		self.pre_scale -=1 # subtract 1 to ensure we do not exceed limit
		self.pre_scale = int(math.floor(self.pre_scale)) 
		self.bus.write_byte_data(self.address,self.pre_scale_reg,self.pre_scale) #set value
		time.sleep(0.005) #at least 500 microseconds required for set
		self.bus.write_byte_data(self.address,self.mode1_reg,0x80) #set to normal mode

		#set first 2 servos to trigger on clock edges to turn on
		self.bus.write_byte_data(self.address,self.led0_on_l,0x00)
		self.bus.write_byte_data(self.address,self.led0_on_h,0x00)
		self.bus.write_byte_data(self.address,self.led1_on_l,0x00)
		self.bus.write_byte_data(self.address,self.led1_on_h,0x00)
	
	def setSpeeds(self,leftSpeed=0,rightSpeed=0):
		
		#Takes as arguments a speed between -100 and +100

		#PCA6985 does PWM out of 4096 and servos take pulses with centre
		#around 1.5 ms with ~0.75ms either side
		#this function assumes frequency is at 50 Hz for servos expecting
		#pulses every 20ms
		rightSpeed = -rightSpeed
		
                if leftSpeed < -100:
                        self.leftSpeed = 460 #460/4096 = 0.112 * 20ms ~=2.25ms
                elif leftSpeed >= 100:
                        self.leftSpeed = 155 # 155/4096 = 0.038 * 20ms ~= 0.76ms
                else:
                        self.leftSpeed = 307 - int(leftSpeed*1.52)
			#1.5ms plus adjustment according to percent speed
			# *1.52 as 155, 460 are ~152 either side so full range
	
                if rightSpeed < -100:
                        self.rightSpeed = 460
                elif rightSpeed >= 100:
                        self.rightSpeed = 155
                else:
                        self.rightSpeed = 307 - int(rightSpeed*1.52)
		
		#print("percentage speed: " + str(leftSpeed))
		#print(self.leftSpeed)
		self.bus.write_byte_data(self.address,self.led0_off_l,self.leftSpeed)
		self.bus.write_byte_data(self.address,self.led0_off_h,self.leftSpeed>>8)
		self.bus.write_byte_data(self.address,self.led1_off_l,self.rightSpeed)
		self.bus.write_byte_data(self.address,self.led1_off_h,self.rightSpeed>>8)

	def setPWM(self,ledNum,ticksOn):
		#use this to set PWM on a specific port, numbered from 0
		#usage instantiaedclass.setPWM(3,2048) turns on led3 for 2048 out of 4096 ticks
		#i.e. sets it to half brightness. If using this for LEDs you may
		#want to use a higher frequency to avoid flicker
		#this should mean you should also set servo speeds manually
		#as setSpeeds assumes 50 Hz
		assert(ledNum < 16)
		assert(ledNum >=0)
		assert(ticksOn<4096)
		
		self.bus.write_byte_data(self.address,(self.led0_off_l + 4*ledNum),ticksOn)
		self.bus.write_byte_data(self.address,(self.led0_off_h + 4*ledNum),ticksOn>>8)


if __name__ == "__main__":
	print("running i2c test")
	servos = I2C(50)
	for i in range(-100,100):
		servos.setSpeeds(i,i)
		time.sleep(0.1)
	servos.setSpeeds(0,0)
