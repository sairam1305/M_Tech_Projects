import numpy
import csv
import math
import operator
import random
from sklearn import svm
from sklearn.decomposition import PCA
import matplotlib.pyplot as plt


# randomly distribute data between training and test set  
def loadDataset(split,X,Y, X_train=[] , Y_train=[],  X_test=[],  Y_test=[]):
    '''
         split  ----> define the splitting ration
         X_train----> training data set
         Y_train----> label for training data set
         X_test ----> Test data set
         Y_test ----> label for test data set
    '''
    train_size=0
    for i in range(0,X.shape[0]):
        if random.random() < split:
            X_train.append(X[i])
            Y_train.append(Y[i])
            train_size=train_size+1
        else:
            X_test.append(X[i])
            Y_test.append(Y[i])
    #apply principal component analysis        
    pca = PCA(n_components=143)
    pca.fit(X_train)
    X_train=pca.transform(X_train)
    X_test=pca.transform(X_test)        
    return train_size



# read data,train and test
def main():
    X_train=[];Y_train=[];X_test=[];Y_test=[]
    reader=csv.reader(open("reduced_features.csv","r"),delimiter=",")
    X=list(reader)
    X=numpy.array(X)
    X=X.astype(numpy.float)

    #create result
    reader=csv.reader(open("target_output.csv","r"),delimiter=",")
    Y=list(reader)
    Y=numpy.array(Y)
    Y=Y.astype(numpy.int)
    Y=Y.ravel()

    train_size=loadDataset(0.7,X,Y, X_train , Y_train,  X_test,  Y_test)

    #create classififer, train and use the classifier
    clf = svm.SVC(C=1,kernel="linear")
    clf.fit(X_train,Y_train)
    acc = clf.score(X_train, Y_train)
    print "Training Set size = ", train_size
    print "Training Set accuracy = ", acc*100
    print "Training Set error = ", (1-acc)*100

    acc = clf.score(X_test, Y_test)
    print "Test Set size = ",X.shape[0]-train_size
    print "Test Set accuracy = ", acc*100
    print "Test Set error = ",(1-acc)*100

    test_pred=clf.predict(X_test)
    test_err=abs(test_pred-Y_test)

    # plot error vs test examples graph
    plt.xlabel('Test Data Set')
    plt.ylabel('Error')
    #print(test_pred)
    #print(Y_test)
    plt.plot(test_err)
    plt.show()

    

if __name__ == '__main__':
    main()

