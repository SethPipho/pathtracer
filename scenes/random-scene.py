import random

path = "scenes/balls.4.scene"
template = "sphere {} {} {} {} 230 126 34"

f = open(path, "w")

for i in range(400):
  x = random.uniform(-3,3)
  y = random.uniform(-3, 3)
  z = random.uniform(9,12)
  radius = random.uniform(.1, .6)
  f.write(template.format(x,y,z,radius) + "\n")


box = '''
#top bottom
sphere 0 100004 0 100000 200 200 200
sphere 0 -100004  0 100000 200 200 200

#right left
sphere  100004 0 0 100000 52 152 219
sphere -100004 0 0 100000 231 76 60

#far
sphere 0 0 100020 100000 200 200 200 

#near
sphere 0 0 -100020 100000 200 200 200 
'''

#f.write(box)