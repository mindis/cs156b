#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <cassert>

#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <ctime>
#include <algorithm>

using namespace std;

// is sushkov using his mac or not?
const int MAC = 0;

// Filenames
const char dataFilePath[] = "/home/msushkov/Dropbox/Caltech/Caltech Classes/CS/CS 156b/data/mu/data123.txt";
const char dataFilePathUM[] = "/home/msushkov/Dropbox/Caltech/Caltech Classes/CS/CS 156b/data/um/data123.txt";
const char outOfSampleFile[] = "/home/msushkov/Dropbox/Caltech/Caltech Classes/CS/CS 156b/data/mu/data4.txt";
const char qualFilePath[] = "/home/msushkov/Dropbox/Caltech/Caltech Classes/CS/CS 156b/data/mu/qual.dta";
const char outputFilePath[] = "/home/msushkov/Dropbox/Caltech/Caltech Classes/CS/CS 156b/submissions/SVD++/test.txt";

const char outputUserFeatureFile[] = "/home/msushkov/Dropbox/Caltech/Caltech Classes/CS/CS 156b/data/feature_matrices/userFeatures_SVD++_50f_K=0.02.txt";
const char outputMovieFeatureFile[] = "/home/msushkov/Dropbox/Caltech/Caltech Classes/CS/CS 156b/data/feature_matrices/movieFeatures_SVD++_50f_K=0.02.txt";

const char dataFilePathMAC[] = "/Users/msushkov/Dropbox/Caltech/Caltech Classes/CS/CS 156b/data/mu/data123.txt";
const char dataFilePathUMMAC[] = "/Users/msushkov/Dropbox/Caltech/Caltech Classes/CS/CS 156b/data/um/data123.txt";
const char outOfSampleFileMAC[] = "/Users/msushkov/Dropbox/Caltech/Caltech Classes/CS/CS 156b/data/mu/data4.txt";
const char qualFilePathMAC[] = "/Users/msushkov/Dropbox/Caltech/Caltech Classes/CS/CS 156b/data/mu/qual.dta";
const char outputFilePathMAC[] = "/Users/msushkov/Dropbox/Caltech/Caltech Classes/CS/CS 156b/submissions/SVD/SVDreg_40f_min150e_K=0.02.txt";

const char outputUserFeatureFileMAC[] = "/Users/msushkov/Dropbox/Caltech/Caltech Classes/CS/CS 156b/data/feature_matrices/userFeatures_SVD_40f_K=0.02_min150e.txt";
const char outputMovieFeatureFileMAC[] = "/Users/msushkov/Dropbox/Caltech/Caltech Classes/CS/CS 156b/data/feature_matrices/movieFeatures_SVD_40f_K=0.02_min150e.txt";

// constants
const double GLOBAL_AVG_MOVIE_RATING = 3.60951619727;
const int NUM_USERS = 458293;
const int NUM_MOVIES = 17770;

const double K = 0.015;
const int NUM_FEATURES = 50;
const int NUM_EPOCHS = 30;

double LEARNING_RATE_Y = 0.007;
double  LEARNING_RATE_USER = 0.007;
double LEARNING_RATE_MOVIE = 0.007;
double LRATE_ub = 0.007;
double LRATE_mb = 0.007;
double LAMBDA_ub = 0.005;
double LAMBDA_mb = 0.005;

const double LEARNING_RATE_DECREASE_RATE = 1;
const double EPSILON = 0.0001;
const double VARIANCE_RATIO = 25;

// defines a single training data point
class DataPoint {
public:
    int user;
    int movie;
    double rating;

    DataPoint() {
        this->user = 0;
        this->movie = 0;
        this->rating = 0;
    }

    DataPoint(int user, int movie, double rating) {
        this->user = user;
        this->movie = movie;
        this->rating = rating;
    }
};

// a vector of datapoints to hold data
//vector<DataPoint*> *trainingData = new vector<DataPoint*>;
vector<DataPoint*> **trainingData = new vector<DataPoint*>*[NUM_USERS]; // an array of vector pointers to DataPoint pointers

// feature matrices (2D arrays); feature k for user u is userFeatures[u][k]
double **userFeatures = new double*[NUM_USERS];
double **movieFeatures = new double*[NUM_MOVIES];

// arrays to hold the average ratings for each movie and for each user
double *avgUserRatings = new double[NUM_USERS];
double *avgMovieRatings = new double[NUM_MOVIES];

// the average offset for a given user
double *avgUserOffset = new double[NUM_USERS];
double *avgMovieOffset = new double[NUM_MOVIES];

// b_u and b_i
double *userBias = new double[NUM_USERS];
double *movieBias = new double[NUM_MOVIES];

// For each user, the number of movies they rated
//int *movieRatingsByUser = new int[NUM_USERS];

// the inverse square root of the number of movie ratings
double *norm = new double[NUM_USERS];

// stores the y-values of each y[movie][feature]
double **y = new double*[NUM_MOVIES];

// stores the sum of the y's: sumY[user][feature]
double **sumY = new double*[NUM_USERS];

// an array of vector pointers
//vector<int> **moviesByUser = new vector<int>*[NUM_USERS];

void computeNorms() {
    for (int i = 0; i < NUM_USERS; i++) {
        //int numMovies = movieRatingsByUser[i];
        int numMovies = trainingData[i]->size();

        if (numMovies > 0)
            norm[i] = pow(numMovies, -0.5);
    }
}

// returns a random value between -0.01 and 0.01
double getRandom() {
	//return 10.0 / 10.0 * ((double) rand() / ((double) RAND_MAX)) - 0.0;
	int num = ((int) rand()) % 100;
	return 2.0 * num / 10000.0 - 0.01;
}

// initialize each element of the feature matrices
void initialize() {
    // initialize the user feature matrix
    for (int i = 0; i < NUM_USERS; i++)
        userFeatures[i] = new double[NUM_FEATURES];
    for (int i = 0; i < NUM_USERS; i++) {
        for (int j = 0; j < NUM_FEATURES; j++) {
            //double rand = sqrt(GLOBAL_AVG_MOVIE_RATING / (double) NUM_FEATURES[0]) + getRandom();
            userFeatures[i][j] = getRandom();
        }
    }
        
    // initialize the movie feature matrix
    for (int i = 0; i < NUM_MOVIES; i++)
        movieFeatures[i] = new double[NUM_FEATURES];
    for (int i = 0; i < NUM_MOVIES; i++) {
        for (int j = 0; j < NUM_FEATURES; j++) {
            //double rand = sqrt(GLOBAL_AVG_MOVIE_RATING / (double) NUM_FEATURES[0]) + getRandom();
            movieFeatures[i][j] = getRandom();
        }
    }

    // initialize y
    for (int i = 0; i < NUM_MOVIES; i++)
        y[i] = new double[NUM_FEATURES];
    for (int i = 0; i < NUM_MOVIES; i++) {
        for (int j = 0; j < NUM_FEATURES; j++)
            y[i][j] = getRandom();
    }
        
    for (int i = 0; i < NUM_USERS; i++)
        sumY[i] = new double[NUM_FEATURES];
        
    for (int i = 0; i < NUM_USERS; i++) {
        trainingData[i] = new vector<DataPoint*>;
    }

    computeNorms();
}

// helper function that splits a string
vector<string> &split(const string &s, char delim, vector<string> &elems) {
    stringstream ss(s);
    string item;
    while(getline(ss, item, delim))
        elems.push_back(item);
    return elems;
}

// helper function to split a string
vector<string> split(const string &s, char delim) {
    vector<string> elems;
    return split(s, delim, elems);
}

double predictRating(int movie, int user) {
	double result = GLOBAL_AVG_MOVIE_RATING + userBias[user] + movieBias[movie];

    // find the dot product
    for (int f = 0; f < NUM_FEATURES; f++)
        result += movieFeatures[movie][f] * (userFeatures[user][f] + norm[user] * sumY[user][f]);

    return result;
}

// trains our algorithm given an input data point
void trainSVD(int user, int movie, double rating) {
    double *userFeature = userFeatures[user - 1];
    double *movieFeature = movieFeatures[movie - 1];

    double error = rating - predictRating(movie - 1, user - 1);

    userBias[user - 1] += (LRATE_ub * (error - LAMBDA_ub * userBias[user - 1]));
    movieBias[movie - 1] += (LRATE_mb * (error - LAMBDA_mb * movieBias[movie - 1]));

    // iterate over features 0 through N - 1
    for (int k = 0; k < NUM_FEATURES; k++) {
        double oldUserFeature = userFeature[k];
        double oldMovieFeature = movieFeature[k];

        // update user feature vector
        //userFeatures[user - 1][k] += (LEARNING_RATE_USER * (error * oldMovieFeature - K * oldUserFeature));
        userFeature[k] += (LEARNING_RATE_USER * (error * oldMovieFeature - K * oldUserFeature));

        // update movie feature vector
        //movieFeatures[movie - 1][k] += (LEARNING_RATE_MOVIE * (error * oldUserFeature - K * oldMovieFeature));
        //movieFeature[k] += (LEARNING_RATE_MOVIE * (error * oldUserFeature - K * oldMovieFeature));
        movieFeature[k] += LEARNING_RATE_MOVIE * (error * (oldUserFeature + norm[user - 1] * sumY[user - 1][k]) - K * oldMovieFeature);
        
        // for each movie that this user has rated
        for (int m = 0; m < trainingData[user - 1]->size(); m++) {
            double update = LEARNING_RATE_Y * (error * norm[user - 1] * oldMovieFeature - K * y[trainingData[user - 1]->at(m)->movie - 1][k]);
            
            y[trainingData[user - 1]->at(m)->movie - 1][k] += update;
            sumY[user - 1][k] += update;
        }

    } // end for loop over features
}

// compute the in-sample error
double computeInSample(int numFeatures) {
    double error = 0;
    int size = 0;

    // go through each training point and calculate the error for it
    for (int i = 0; i < NUM_USERS; i++) {
        if (trainingData[i] != NULL) {
            for (int j = 0; j < trainingData[i]->size(); j++) {
                DataPoint *currData = trainingData[i]->at(j);
                size++;
                
                if (currData != NULL) {
                    int currUser = currData->user;
                    int currMovie = currData->movie;
                    double actualRating = currData->rating;

                    double predicted = predictRating(currMovie - 1, currUser - 1);

		            // make sure the rating is between 1 and 5
                    if (predicted > 5)
                        predicted = 5;
                    else if (predicted < 1)
                        predicted = 1;

                    error += (predicted - actualRating) * (predicted - actualRating);
                }
            }
        }
    }

    return sqrt(error / size);
}

// compute the out of sample error
double computeOutOfSample(int numFeatures) {
    double error = 0;
    int numPoints = 0;

    string line;
    ifstream outOfSample;
    
    if (MAC == 0)
    	outOfSample.open(outOfSampleFile, ios::in);
    else
    	outOfSample.open(outOfSampleFileMAC, ios::in);

    // go through the input data line by line
    while (getline(outOfSample, line)) {
        // where are we in the current line?
        int count = 0;

        int user = -1;
        int movie = -1;
        double actualRating = -1;

        istringstream lineIn(line);
        while (lineIn) {
            int val = 0;
            if (lineIn >> val) {
                if (count == 0)
                    user = val;
                else if (count == 1)
                    movie = val;
                else if (count == 3)
                    actualRating = val;
                count++;
            }
        }

        double predictedRating = predictRating(movie - 1, user - 1);

        // make sure the rating is between 1 and 5
        if (predictedRating > 5)
            predictedRating = 5;
        else if (predictedRating < 1)
            predictedRating = 1;
            
        assert(actualRating != -1);

        // get the squared error for this point
        error += (predictedRating - actualRating) * (predictedRating - actualRating);

        numPoints++;
    }

    outOfSample.close();

    return sqrt(error / numPoints);
}

// iterate through the epochs and all the data points for each epoch
void learn() {
    double Eout = 10000;
    double prevEout = 1000000;

    int i = 0;
    
    int num = 0;

    // go N times through the data
    //for (int i = 0; i < NUM_EPOCHS; i++) {
    while (i < NUM_EPOCHS || (prevEout - Eout) > EPSILON) {
        const clock_t begin_time = clock();

        // go through each point in the data set
        for (int p = 0; p < NUM_USERS; p++) {
        
            if (trainingData[p] != NULL) {
            
                for (int j = 0; j < trainingData[p]->size(); j++) {
                
                    DataPoint *currData = trainingData[p]->at(j);
                
                    if (currData != NULL) {
                    
                        if (num % 1000000 == 0)
                            cout << "Data point: " << num / 1000000 << " million" << endl;
                        num++;

                        // train the SVD on each point (will go through all features)
                        trainSVD(currData->user, currData->movie, currData->rating);
                    }
                    
                } // end movie loop
                
            }
            
        } // end user loop

        // randomize the training data
        //next_permutation(trainingData->begin(), trainingData->end());

        // decrease the learning rates
        LEARNING_RATE_USER *= LEARNING_RATE_DECREASE_RATE;
        LEARNING_RATE_MOVIE *= LEARNING_RATE_DECREASE_RATE;

        // only check Eout every 5 iterations
        if (i % 3 == 0) {
            prevEout = Eout;
            Eout = computeOutOfSample(NUM_FEATURES);
            cout << "    Eout = " << Eout << endl;
        }

        cout << "Finished epoch: " << i + 1 << " out of " << NUM_EPOCHS << endl;
        cout << " --> " << double(clock() - begin_time) / CLOCKS_PER_SEC << " seconds" << endl;

        i++;

    } // end epoch loop

    // print out the results
    cout << endl << "RESULTS: (K, # of features, Ein, Eout)" << endl;
    cout << K << ", " << NUM_FEATURES << ", " << computeInSample(NUM_FEATURES) << ", " << computeOutOfSample(NUM_FEATURES) << endl;
}

// computes the average ratings for each user (uses UM data set)
// also computes the user offset
void getAvgUserRatings() {
    string line;

    // get the UM data file
    ifstream dataFile;
    if (MAC == 0)
    	dataFile.open(dataFilePathUM, ios::in);
    else
    	dataFile.open(dataFilePathUMMAC, ios::in);
    
    int numLinesSoFar = 0;

    int prevUser = 1;
    double totalRating = 0;
    int index = 0; // counts the # of movies each user rated

    double currTotalUserOffset = 0;

    // go through the input data line by line
    while (getline(dataFile, line)) {
        // where are we in the current line?
        int count = 0;

        int currUser = -1;
        int currMovie = -1;
        double currRating = -1;

        istringstream lineIn(line);
        while (lineIn) {
            int val = 0;
            if (lineIn >> val) {
                if (count == 0)
                    currUser = val;
                else if (count == 1)
                    currMovie = val;
                else if (count == 3)
                    currRating = val;
                count++;
            }
        }
        
        DataPoint *currPoint = new DataPoint(currUser, currMovie, currRating);

        // to calculate the average ratings for the movies

        // if the user changes
        if (prevUser != currUser) {
            //avgUserRatings[prevUser - 1] = totalRating / index;
            avgUserRatings[prevUser - 1] = (GLOBAL_AVG_MOVIE_RATING * VARIANCE_RATIO + totalRating) / (VARIANCE_RATIO + index);
            totalRating = currRating;

            avgUserOffset[prevUser - 1] = currTotalUserOffset / index;
            currTotalUserOffset = currRating - avgMovieRatings[currMovie - 1];
            //currTotalUserOffset = currRating - GLOBAL_AVG_MOVIE_RATING;
            //movieRatingsByUser[prevUser - 1] = index;
            
            // we see a new user, so he starts off with the only rating we have seen for him
            //moviesByUser[currUser - 1]->push_back(currMovie);
            trainingData[currUser - 1]->push_back(currPoint);

            index = 1;
        } else { // if we keep seeing data for the same user
            if (currRating != 0) {
                index++;
                totalRating += currRating;
                currTotalUserOffset += (currRating - avgMovieRatings[currMovie - 1]);
                //currTotalUserOffset += (currRating - GLOBAL_AVG_MOVIE_RATING);
                
                //moviesByUser[currUser - 1]->push_back(currMovie);
                trainingData[currUser - 1]->push_back(currPoint);
            }
        }

        prevUser = currUser;
            
        numLinesSoFar++;
        if (numLinesSoFar % 1000000 == 0)
            cout << "Computing avg user ratings: " << numLinesSoFar / 1000000 << " million" << endl;
    }    

    // deals with end of file
    avgUserRatings[prevUser - 1] = totalRating / index;
    avgUserOffset[prevUser - 1] = currTotalUserOffset / index;
    //movieRatingsByUser[prevUser - 1] = index;

    dataFile.close();
}

// reads data in (MU) and also computes avg ratings for each movie
// also computes the movie offset
void getData() {
    string line;
    ifstream dataFile;
    
    if (MAC)
    	dataFile.open(dataFilePathMAC, ios::in);
    else
    	dataFile.open(dataFilePath, ios::in);
    
    int numLinesSoFar = 0;

    int prevMovie = 1;
    double totalRating = 0;
    int index = 0; // counts the # of users that watched each movie

    double currTotalMovieOffset = 0;

    // go through the input data line by line
    while (getline(dataFile, line)) {
        // where are we in the current line?
        int count = 0;
        
        int currUser = -1;
        int currMovie = -1;
        double currRating = -1;

        istringstream lineIn(line);
        while (lineIn) {
            int val = 0;
            if (lineIn >> val) {
                if (count == 0)
                    currUser = val;
                else if (count == 1)
                    currMovie = val;
                else if (count == 3)
                    currRating = val;
                count++;
            }
        }
        // add the point to our data vector
        //trainingData->push_back(currPoint);

        // to calculate the average ratings for the movies
        if (prevMovie != currMovie) {
            //avgMovieRatings[prevMovie - 1] = totalRating / index;
            avgMovieRatings[prevMovie - 1] = (VARIANCE_RATIO * GLOBAL_AVG_MOVIE_RATING + totalRating) / (VARIANCE_RATIO + index);
            
            totalRating = currRating;

            avgMovieOffset[prevMovie - 1] = currTotalMovieOffset / index;

            //currTotalMovieOffset = currPoint->rating - avgUserRatings[currPoint->user - 1];
            currTotalMovieOffset = currRating - GLOBAL_AVG_MOVIE_RATING;

            index = 1;
        } else {
            if (currRating != 0) {
                index++;
                totalRating += currRating;
                //currTotalMovieOffset += (currPoint->rating - avgUserRatings[currPoint->user - 1]);
                currTotalMovieOffset += (currRating - GLOBAL_AVG_MOVIE_RATING);
            }
        }

        prevMovie = currMovie;
            
        numLinesSoFar++;
        if (numLinesSoFar % 1000000 == 0)
            cout << numLinesSoFar / 1000000 << " million" << endl;
    }    

    // deals with last line with respect to calculating the average movie rating
    //avgMovieRatings[prevMovie - 1] = totalRating / index;
    avgMovieRatings[prevMovie - 1] = (VARIANCE_RATIO * GLOBAL_AVG_MOVIE_RATING + totalRating) / (VARIANCE_RATIO + index);
    
    avgMovieOffset[prevMovie - 1] = currTotalMovieOffset / index;

    dataFile.close();

    // at this point we have the avg movie ratings filled out
    // now calculate the avg user ratings
    getAvgUserRatings();
}

void outputVectors() {
    cout << "Output vectors" << endl;

    // output files
    ofstream outputFileUser;
    
    if (MAC == 0)
	    outputFileUser.open(outputUserFeatureFile, ios::out);
	else
		outputFileUser.open(outputUserFeatureFileMAC, ios::out);
	
    ofstream outputFileMovie;
    if (MAC == 0)
	    outputFileMovie.open(outputMovieFeatureFile, ios::out);
    else
    	outputFileMovie.open(outputMovieFeatureFileMAC, ios::out);
    
    // output the user features
    for (int i = 0; i < NUM_USERS; i++) {
        for (int k = 0; k < NUM_FEATURES; k++) {
            outputFileUser << userFeatures[i][k] << " ";
        }

        outputFileUser << endl;
    }

    // output the movie features
    for (int i = 0; i < NUM_MOVIES; i++) {
        for (int k = 0; k < NUM_FEATURES; k++) {
            outputFileMovie << movieFeatures[i][k] << " ";
        }

        outputFileMovie << endl;
    }

    outputFileMovie.close();
    outputFileUser.close();
}

// writes data out
void outputResults() {
    // output file
    ofstream outputFile;
    if (MAC)
    	outputFile.open(outputFilePathMAC, ios::out);
    else
    	outputFile.open(outputFilePath, ios::out);
    
    // qual file
    string line;
    ifstream qualFile;
    if (MAC)
    	qualFile.open(qualFilePathMAC, ios::in);
    else
    	qualFile.open(qualFilePath, ios::in);
    
    // go through each line of the qual file
    while (getline(qualFile, line)) {
        // where are we in the current line?
        int count = 0;

        int user = -1;
        int movie = -1;

        // read the values on the current line one by one
        istringstream lineIn(line);
        while (lineIn) {
            int val = 0;
            if (lineIn >> val) {
                if (count == 0)
                    user = val;
                else if (count == 1)
                    movie = val;
                count++;
            }
        }

        // calculate the rating for user, movie
        double predictedScore = predictRating(movie - 1, user - 1);

        // make sure the rating is between 1 and 5
        if (predictedScore > 5)
            predictedScore = 5;
        else if (predictedScore < 1)
            predictedScore = 1;

        // write to file
        outputFile << predictedScore << endl;
    }

    qualFile.close();
    outputFile.close();

    //outputVectors();
}

void cleanup() {
    for (int i = 0; i < NUM_USERS; i++)
        delete[] userFeatures[i];
        
    for (int i = 0; i < NUM_MOVIES; i++)
        delete[] movieFeatures[i];
        
    delete[] userFeatures;
    delete[] movieFeatures;

    

    /*delete[] avgMovieRatings;
    delete[] avgUserRatings;

    delete[] avgUserOffset;
    delete[] avgMovieOffset;*/

    //delete[] userBias;
    //delete[] movieBias;

    //delete[] movieRatingsByUser;

    //delete[] norm;

    
}

// K-value, # features, # epochs
int main(int argc, char *argv[]) {
	srand((unsigned) time(0));

    initialize();

    getData();
    cout << "Done with getData()" << endl;
    
    // run the SVD algorithm
    learn();
    cout << "Done learning" << endl;

    //outputResults();
    cout << "Done with outputResults()" << endl;

    //cleanup();
    cout << "DONE!" << endl;
    
    return 0;
}
