from __future__ import division
import numpy
import csv
import math
import operator
import random

#Function to split the dataset into trainingset and testset
def splitDataset(dataset,split):
	trainSize=int(len(dataset)*split)
	trainSet=[]
	copy=list(dataset)
	while len(trainSet)<trainSize:
		index=random.randrange(len(copy))
		trainSet.append(copy.pop(index))
	return [trainSet,copy]

#Function separating instances depending on their class and calculate mean, standard deviation for each attribute	
def calccluster(dataset,label):
	cluster={}
	for i in range(len(dataset)):
		item=numpy.asscalar(label[i])	
		if (item not in cluster):
			cluster[item]=[]
		cluster[item].append(dataset[i])
	stat={}
	for item,examples in cluster.items():
		stat[item]=[(meandev(iteration))for iteration in zip(*examples)]
	return stat

#Function calculating mean and standard deviation
def meandev(item):
	avg=sum(item)/float(len(item))
	var=sum([pow(k-avg,2) for k in item])/float(len(item)-1)
	return avg,math.sqrt(var)		

#Function calculating the probability for each attribute given a class
def calcpred(stat,item):
	pred=[]
	for i in range(len(item)):
		tup=item[i]
		probs={}
		for next,cs in stat.items():
			probs[next]=1
			for j in range(len(cs)):
				avg,std=cs[j]
				k=tup[j]
				probs[next]*=findprob(k,avg,std)
		lab,nprob=None,-1
		for next,prob in probs.items():
			if lab is None or prob>nprob:
				nprob=prob
				lab=next
		pred.append(lab)
	return pred
	
def findprob(k,avg,std):
	try:
		expo=calcexp(avg,std)
		return float((1/(math.sqrt(2*math.pi)*std)))*expo
	except ZeroDivisionError:
		return 0

def calcexp(avg,std):
	exp1=math.pow(4-avg,2)
	exp2=2*math.pow(std,2)
	exp3=float(math.exp(-(exp1/exp2)))
	return exp3

#Accuracy function for trainingset
def trainacc(data,label,sample,pred):
	correct=0
	end=len(sample)
	end=end-1
	j=-1
	for i in range(end):
		j=j+1
		if label[i]==pred[j]:
			correct+=1
	return (correct/float(len(sample)))*100.0

#Accuracy function for testset	
def testacc(data,label,sample,pred):
	correct=0
	start=len(data)-len(sample)
	start=start+1
	j=-1
	for i in range(start,len(data)):
		j=j+1
		if label[i]==pred[j]:
			correct+=1
	return (correct/float(len(sample)))*100.0

if __name__ == '__main__':
	reader=csv.reader(open("reduced_features.csv","r"),delimiter=",")
	split=0.80
	dataset=list(reader)
	dataset=numpy.array(dataset)
	dataset=dataset.astype(numpy.float)
	trainingSet,testSet=splitDataset(dataset,split)
	reader=csv.reader(open("target_output.csv","r"),delimiter=",")
	label=list(reader)
	label=numpy.array(label)
	label=label.astype(numpy.int)
	res=calccluster(trainingSet,label)
	predtrain=calcpred(res,trainingSet)
	predtest=calcpred(res,testSet)
	print('Training Accuracy ='+str(trainacc(dataset,label,trainingSet,predtrain))+"%")
	print('Test Accuracy ='+str(testacc(dataset,label,testSet,predtest))+"%")
