import math
import tensorflow as tf
import numpy as np
import matplotlib.pyplot as plt
import csv
import random
from tensorflow.python.framework import ops
from sklearn.decomposition import PCA

def convert_to_one_hot(Y, C):
    Y = np.eye(C)[Y.reshape(-1)].T
    return Y

def random_mini_batches(X, Y, mini_batch_size = 64, seed = 0):
    
    m = X.shape[1]                  # number of training examples
    mini_batches = []
    np.random.seed(seed)
    
    # Step 1: Shuffle (X, Y)
    permutation = list(np.random.permutation(m))
    shuffled_X = X[:, permutation]
    shuffled_Y = Y[:, permutation].reshape((Y.shape[0],m))

    # Step 2: Partition (shuffled_X, shuffled_Y). Minus the end case.
    num_complete_minibatches = math.floor(m/mini_batch_size) # number of mini batches of size mini_batch_size in your partitionning
    for k in range(0, num_complete_minibatches):
        mini_batch_X = shuffled_X[:, k * mini_batch_size : k * mini_batch_size + mini_batch_size]
        mini_batch_Y = shuffled_Y[:, k * mini_batch_size : k * mini_batch_size + mini_batch_size]
        mini_batch = (mini_batch_X, mini_batch_Y)
        mini_batches.append(mini_batch)
    
    # Handling the end case (last mini-batch < mini_batch_size)
    if m % mini_batch_size != 0:
        mini_batch_X = shuffled_X[:, num_complete_minibatches * mini_batch_size : m]
        mini_batch_Y = shuffled_Y[:, num_complete_minibatches * mini_batch_size : m]
        mini_batch = (mini_batch_X, mini_batch_Y)
        mini_batches.append(mini_batch)
    
    return mini_batches

# spilt the data into train and test set
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
    pca = PCA(n_components=143)
    pca.fit(X_train)
    X_train=pca.transform(X_train)
    X_test=pca.transform(X_test)        
    return train_size        
    

def create_placeholders(n_x, n_y):
    X = tf.placeholder(tf.float32, [175, None], name="X")
    Y = tf.placeholder(tf.float32, [14, None], name="Y")
    return X, Y    

def initialize_parameters():
    tf.set_random_seed(1)              
    L1=36
    L2=25
    L3=14
    # [structure 36--> 25-->14]    
    W1 = tf.get_variable("W1", [L1, 175], initializer = tf.contrib.layers.xavier_initializer(seed=1))
    b1 = tf.get_variable("b1", [L1, 1], initializer = tf.zeros_initializer())
    W2 = tf.get_variable("W2", [L2, L1], initializer = tf.contrib.layers.xavier_initializer(seed=1))
    b2 = tf.get_variable("b2", [L2, 1], initializer = tf.zeros_initializer())
    W3 = tf.get_variable("W3", [L3, L2], initializer = tf.contrib.layers.xavier_initializer(seed=1))
    b3 = tf.get_variable("b3", [L3, 1], initializer = tf.zeros_initializer())

    parameters = {"W1": W1,"b1": b1,"W2": W2,"b2": b2,"W3": W3,"b3": b3}
    
    return parameters  

def forward_propagation(X, parameters): 
    W1 = parameters['W1']
    b1 = parameters['b1']
    W2 = parameters['W2']
    b2 = parameters['b2']
    W3 = parameters['W3']
    b3 = parameters['b3']
                
    Z1 = tf.add(tf.matmul(W1, X), b1)                      # Z1 = np.dot(W1, X) + b1
    A1 = tf.nn.relu(Z1)                                    # A1 = relu(Z1)
    Z2 = tf.add(tf.matmul(W2, A1), b2)                     # Z2 = np.dot(W2, a1) + b2
    A2 = tf.nn.relu(Z2)                                    # A2 = relu(Z2)
    Z3 = tf.add(tf.matmul(W3, A2), b3)                     # Z3 = np.dot(W3,Z2) + b3
    
    return Z3

def compute_cost(Z3, Y):
    
    # to fit the tensorflow requirement for tf.nn.softmax_cross_entropy_with_logits(...,...)
    logits = tf.transpose(Z3)
    labels = tf.transpose(Y)
    cost = tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits_v2(logits=logits, labels=labels))
    return cost 

def model(X_train, Y_train, X_test, Y_test, learning_rate = 0.0001,num_epochs = 1500, minibatch_size = 32, print_cost = True):
    ops.reset_default_graph()                         # to be able to rerun the model without overwriting tf variables
    tf.set_random_seed(1)                             # to keep consistent results
    seed = 3                                          # to keep consistent results
    (n_x, m) = X_train.shape                          # (n_x: input size, m : number of examples in the train set)
    n_y = Y_train.shape[0]                            # n_y : output size
    costs = []                                        # To keep track of the cost

    X, Y = create_placeholders(n_x, n_y)

    parameters = initialize_parameters()
    Z3 = forward_propagation(X, parameters)
    cost = compute_cost(Z3, Y)
    
    optimizer = tf.train.AdamOptimizer(learning_rate=learning_rate).minimize(cost)
    init = tf.global_variables_initializer()
    with tf.Session() as sess:
        
        # Run the initialization
        sess.run(init)
        
        # Do the training loop
        for epoch in range(num_epochs):
            epoch_cost = 0.                          # Defines a cost related to an epoch
            num_minibatches = int(m / minibatch_size) # number of minibatches of size minibatch_size in the train set
            seed = seed + 1
            minibatches = random_mini_batches(X_train, Y_train, minibatch_size, seed)

            for minibatch in minibatches:

                # Select a minibatch
                (minibatch_X, minibatch_Y) = minibatch
                
                # IMPORTANT: The line that runs the graph on a minibatch.
                # Run the session to execute the "optimizer" and the "cost", the feedict should contain a minibatch for (X,Y).
                _ , minibatch_cost = sess.run([optimizer, cost], feed_dict={X: minibatch_X, Y: minibatch_Y})
                
                epoch_cost += minibatch_cost / num_minibatches

            # Print the cost every epoch
            if print_cost == True and epoch % 100 == 0:
                print ("Cost after epoch %i: %f" % (epoch, epoch_cost))
            if print_cost == True and epoch % 5 == 0:
                costs.append(epoch_cost)
                
        # plot the cost
        plt.plot(np.squeeze(costs))
        plt.ylabel('cost')
        plt.xlabel('iterations (per tens)')
        plt.title("Learning rate =" + str(learning_rate))
        plt.show()

        # lets save the parameters in a variable
        parameters = sess.run(parameters)
        print("Parameters have been trained!")

        # Calculate the correct predictions
        correct_prediction = tf.equal(tf.argmax(Z3), tf.argmax(Y))

        # Calculate accuracy on the test set
        accuracy = tf.reduce_mean(tf.cast(correct_prediction, "float"))

        print("Train Accuracy:"+str (accuracy.eval({X: X_train, Y: Y_train})*100)+"%")
        print("Test Accuracy:"+str(accuracy.eval({X: X_test, Y: Y_test})*100)+"%")
        return parameters       

# read data and call model
def main():
  # Import data
  import os
  os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2'
  X_train=[];Y_train=[];X_test=[];Y_test=[]
  reader=csv.reader(open("reduced_features.csv","r"),delimiter=",")
  X=list(reader)
  X=np.array(X)
  X=X.astype(np.float)

  #create result
  reader=csv.reader(open("target_output.csv","r"),delimiter=",")
  Y=list(reader)
  Y=np.array(Y)
  Y=Y.astype(np.int)
  Y=Y.ravel()

  train_size=loadDataset(0.8,X,Y, X_train , Y_train,  X_test,  Y_test)
  X_train=np.transpose(X_train)
  X_test=np.transpose(X_test)
  Y_train=np.transpose(Y_train)
  Y_train=convert_to_one_hot(Y_train,14)
  Y_test=np.transpose(Y_test)
  Y_test=convert_to_one_hot(Y_test,14)
  parameters = model(X_train, Y_train, X_test, Y_test)

  

if __name__ == '__main__':
    main()

     
