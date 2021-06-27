import getopt
import sys

# you will need to install scikit-learn, which will already require NumPy and pandas;
#	read more here: https://scikit-learn.org/stable/install.html
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

from sklearn.linear_model import LinearRegression


def load_data(train_file, test_file):
    # Part 1: load contents from training and testing files into a Pandas dataframe.
    # -- Hint: use NumPy or Pandas to load the file's contents into a PD dataframe
    # 		for your regression model and for scikit-learn's regression function.


    # -- Hint: there are 2 input variables and 1 output.
	training_data, testing_data = [], []

	# Part 1a: load the training and testing data points from CSV.
	# -- Hint: read about how to use Pandas DataFrame objects. Is there any function that works with CSV files?

	# NOTE: use Panda's read_csv(...) function:
	training_data = pd.read_csv("lecture2_training_data.csv")
	testing_data = pd.read_csv("lecture2_testing_data.csv")

	return training_data, testing_data
# end


def learn_regression_model(X, Y):
    # Part 2: using the training set, use the normal equation to find the ideal values for
    #		coefficients w, as in the equation found in the lecture.

    # Part 2a: using the matrices X and Y, perform normal equation computation to acquire coefficients (w*).
    w_star = None

    # -- Hint: remember that we want to find a w* such that:	w* = (X^T.X)^-1 (X^T.Y)

    # -- Hint: how can you use NumPy to perform matrix calculations?

    # -- Hint: remember that we need to form the training data X such that the 1st column has only ones (1).
    #		refer to the comment on line 98 about np.column_stack(...).

    # NOTE: the following is exactly w* = (X^T.X)^-1 (X^T.Y):
    w_star = np.dot(np.linalg.inv(np.dot(np.transpose(X), X)),
                    np.dot(np.transpose(X), Y))

    return w_star
# enddef


def start():
    print(' ### MACHINE LEARNING IN ROBOTICS: Lecture 2 - Programming Assignment\n')
    # define the variable names for the training and testing file names:
    training_file, testing_file = None, None

    # check arguments:
    try:
        opts, _ = getopt.getopt(sys.argv[1:], 'tr:te', [
                                'training_set=', 'testing_set='])
    except getopt.GetoptError:
        print('ERROR: missing arguments!')
        exit(2)
    else:
        # read arguments from the command-line:
        for opt, arg in opts:
            if opt in ('-tr', '--training_set'):
                training_file = arg
            if opt in ('-te', '--testing_set'):
                testing_file = arg
            else:
                pass
            # end
        # end
    # end

    # load data from provided files:
    training_data, testing_data = load_data(training_file, testing_file)

    print('  TRAINING SET:\tnumber of data points -- ' + str(len(training_data)))
    print('  TESTING SET:\tnumber of data points -- ' + str(len(testing_data)))

    # Part 1b: extract the matrices for inputs (X) and outputs (Y).
    # -- Hint: if you use a Pandas dataframe, how can you extract columns? Remember we have multiple inputs!
    train_X, train_Y = training_data[['x_1', 'x_2']], training_data[['y']]

    # be sure to append to a column of ones so that the dot product works;
    # 	you can use the following to do this:
    train_X = np.column_stack((np.ones(len(train_X)), train_X))

    # PART 3: using the learned coefficients, predict the possible outputs for the testing data!
    # Part 3a: get the weight matrix from your implementation of the regression model:

    w_star = learn_regression_model(train_X, train_Y)

    # Part 3b: perform prediction using the learned coefficients (w*) and the testing dataset.
    # -- Hint: how do we use the coefficients we acquired to obtain the predicted output Y (or f(x))?

    # extract the data from the dataset object 'testing_data'.
    # -- Hint: how did you do this for the 'train_X' object?
    test_X = testing_data[['x_1', 'x_2']]

    # be sure to append to a column of ones so that the dot product works;
    # 	you can use the following to do this:
    test_X = np.column_stack((np.ones(len(test_X)), test_X))

    # < INSERT CODE HERE FOR PART 3b >

    print('\n ### Self-computed Linear Regression:')
    print('\nComputed w*:')
    print(w_star)

    print('\nTesting data:')
    print(test_X)

    # -- Hint: how do you use your acquired coefficients to predict unseen values? How do we use w*?
    test_Y = None
    print('\nPredicted output:')

    # NOTE: remember from the lecture notes that you need to take your w* vector and apply dot product to X:
    test_Y = np.dot(test_X, w_star)

    print(test_Y)

    # plot the training and testing data points as well as the line of best fit using the coefficients we derived:

    # NOTE: this was a very quick 3D plot designed for this problem. It is very useful to visualize the best fit this way.
    # -- if you rotate the plot, you can see that a plane is formed from the red dots. This is what we learned from the regression model.
    plt.figure(figsize=(10, 7))
    ax = plt.axes(projection='3d')
    ax.scatter(train_X[:, 1], train_X[:, 2], train_Y, color='yellow')
    ax.scatter(train_X[:, 1], train_X[:, 2],
               np.dot(train_X, w_star), color='red')
    ax.scatter(test_X[:, 1], test_X[:, 2],
               np.dot(test_X, w_star), color='blue')
    plt.show()

    # Part 4a: use scikit-learn's LinearRegression module to train a regression model
    #		to compare with your hand-written code.
    # -- Hint: read about how to use sklearn.linear_model.LinearRegression() for training model.

    # NOTE: remember that the appending of column of 1's is only needed for the normal equation:
    train_X, train_Y = training_data[['x_1', 'x_2']], training_data[['y']]

    # NOTE: create a LinearRegression model object:
    model = LinearRegression()

    # NOTE: Use the LinearRegression.fit() function:
    #	Read here: https://scikit-learn.org/stable/modules/generated/sklearn.linear_model.LinearRegression.html#sklearn.linear_model.LinearRegression
    model.fit(train_X, train_Y)

    # Part 4b: use your trained regression model to predict output for the test set.
    # -- Hint: read about how to use sklearn.linear_model.LinearRegression() for prediction.
    # -- Hint: use the same NumPy array called 'test_X' from above.

    test_X = testing_data[['x_1', 'x_2']]

    print('\n ### Scikit-Learn Linear Regression Model:')
    # -- Hint: how do you acquire the coefficients of your trained scikit-learn LinearRegression model?
    #		remember that there is also an intercept in the equation.
    print('\nComputed w*:')

    # NOTE: Remember that the model is in the form f(x) = w_0 + w_1 * x_1 + w_2 * x_2;
    #		model.coef_ gives us w_1 and w_2, while model.intercept_ gives us w_0:

    # NOTE: you can print them like this:
    # print(model.coef_) - remember this will give us w_1 and w_2
    # print(model.intercept_) - remember this will give us w_0

    # NOTE: you can stack them together like this:
    w_star = []
    w_star.append([float(model.intercept_)])
    for w in model.coef_[0]:
        w_star.append([float(w)])
    print(np.array(w_star))

    print('\nTesting data:')
    print(test_X)

    test_Y = None
    print('\nPredicted output:')

    # NOTE: Use the LinearRegression.predict() function:
    #	Read here: https://scikit-learn.org/stable/modules/generated/sklearn.linear_model.LinearRegression.html#sklearn.linear_model.LinearRegression
    test_Y = model.predict(test_X)

    print(test_Y)

    print('\n ### Do your values match up? :)\n')

# end


if __name__ == "__main__":
    start()
# end
