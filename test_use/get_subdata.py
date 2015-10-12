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
	header = ["Test Case(Index)","Compile Time", "Nodes", "Edges"]
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

if len(lines) <= 24:
	print "Error Data"
	header = [TestCase, "-1", "-1", "-1"]
	spamwriter.writerow(header)
	fin.close()
	fout.close()
	os.remove(inFile)
	sys.exit()

if lines[6][1]=='D':
	i = 1
else:
	i = 0

for i in range(len(lines)):
	if lines[i][2:9] == 'Compile':
		time = lines[i][14:-1]
		time = time.strip()
		time = time[:-1]
		line.append(time)

if lines[i][2:7] == 'Nodes':
	nodes = lines[i][7:-1]
	nodes = nodes.strip()
	line.append(nodes)

if lines[i][2:7] == 'Edges':
	edges = lines[i][7:-1]
	edges = edges.strip()
	line.append(edges)

# if i==1:
# 	line.append("Wrong Cut")

spamwriter.writerow(line)

fin.close()
fout.close()
