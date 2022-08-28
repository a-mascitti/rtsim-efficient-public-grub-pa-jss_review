#!/usr/bin/python

# python3 ./mcm.py "1 2 3"

from math import gcd
import sys

a = []   #will work for an int array of any length

for elem in sys.argv[1].split(' '):
    if elem != '':
        a.append(int(elem))

#print (a)

# Euclide's lcm algorithm from https://stackoverflow.com/questions/37237954/calculate-the-lcm-of-a-list-of-given-numbers-in-python
lcm = a[0]
for i in a[1:]:
    lcm = int(lcm * i / gcd(lcm, i))
print (lcm)

