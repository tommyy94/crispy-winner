import math


'''
Used for calculating duty cycle values for the lookup table.
'''

def f(D):
	return (-((1170 * D) - 1170))

for i in range(0, 100):
	print(str(math.floor(f(i/100))) + ", ", end="", flush=True)
	
