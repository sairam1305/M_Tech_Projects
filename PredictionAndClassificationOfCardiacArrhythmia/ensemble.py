import numpy
import csv
import math
import operator
import random
from sklearn.ensemble import RandomForestClassifier
from sklearn.ensemble import AdaBoostClassifier
from sklearn.tree import DecisionTreeClassifier
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

    # apply principal componet analysis        
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

    # train three classifier independently and merge them

    # Randforest
    clf_rf = RandomForestClassifier(n_estimators=160)
    clf_rf.fit(X_train, Y_train)

    #Adaboost
    clf_ab = AdaBoostClassifier(DecisionTreeClassifier(max_depth=10),algorithm="SAMME",n_estimators=100)
    clf_ab.fit(X_train, Y_train)

    # support vector machine
    clf_svm = svm.SVC(C=1,kernel="linear")
    clf_svm.fit(X_train,Y_train)

    pred_rf=clf_rf.predict(X_test)
    pred_ab=clf_ab.predict(X_test)
    pred_svm=clf_svm.predict(X_test)

    pred= (pred_rf+pred_ab)/2;
    final_pred=numpy.round(pred)
    #print(final_pred)
    #print(Y_test)
    accuracy = (final_pred == Y_test).mean()


    print("Test Accuracy="+str(accuracy*100)+"%")

    

if __name__ == '__main__':
    main()

