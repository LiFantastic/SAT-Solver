import os
import sys
import csv

if len(sys.argv) != 3:
	print("Usage: python get_data.py inputFile outputFile")
else:
	inFile = sys.argv[1]
	outFile = sys.argv[2]

exist = False

if os.path.exists(outFile):
	exist = True

# 	print "exist"
	
# else:
# 	#print "exist"
# 	fout = open(outFile, 'wb')
fout = open(outFile, 'a+b')
spamwriter = csv.writer(fout, dialect='excel')

if exist==False:
	header = ["Test Case(Index)","Compile Time","Total Time", "Nodes", "Edges", "Models", "Decomposability", "Entailment"]
	spamwriter.writerow(header)

line = []

TestCase = inFile.split("/")
TestCase = TestCase[-1]
TestCase = TestCase.split(".")
TestCase = TestCase[0:-2]
TestCase = ".".join(TestCase)
line.append(TestCase)

fin = open(inFile)
lines = fin.readlines()

if len(lines) <= 30:
	header = [TestCase, "-1","-1", "-1", "-1", "-1", "-1", "-1"]
	spamwriter.writerow(header)
	print "Error Data"
	fin.close()
	fout.close()
	os.remove(inFile)
	sys.exit()

time = lines[20][14:-1]
time = time.strip()
time = time[:-1]
line.append(time)

totaltime = lines[35][11:-1]
totaltime = totaltime.strip()
totaltime = totaltime[:-1]
line.append(totaltime)

nodes = lines[30][7:-1]
nodes = nodes.strip()
line.append(nodes)

edges = lines[31][7:-1]
edges = edges.strip()
line.append(edges)

models = lines[32][13:-1]
models = models.strip()
models = models.split(" ")
models = models[0]
line.append(models)

dec = lines[33][29:-1]
dec = dec.strip()
dec = dec.split(" ")
dec = dec[0]
line.append(1)

ent = lines[34][24:-1]
ent = ent.strip()
ent = ent.split(" ")
ent = ent[0]
line.append(1)

spamwriter.writerow(line)

fin.close()
fout.close()
