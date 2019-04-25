import random

path = "scenes/balls.scene"
template = "sphere {} {} {} {} 200 200 200"

f = open(path, "w")

for i in range(20):
  x = random.uniform(-2,2)
  y = random.uniform(-4, 4)
  z = random.uniform(9,12)
  radius = random.uniform(.5, 1.5)
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

f.write(box)