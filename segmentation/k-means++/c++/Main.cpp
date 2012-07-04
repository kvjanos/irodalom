#include "Utils.h"
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>

/////////////////////
// DATA GENERATION //
/////////////////////

/**
 * A data generator.
 **/
class AbstractGen {
public:
    virtual vector<Point> generateData(ostream &out) const = 0;
};

/**
 * Generates a collection of points according to the following scheme:
 *  - Choose numCenters centers uniformly at random in a hypercube.
 *  - Choose numPoints/numCenters points offset by a normal distribution from each center.
 **/
class SeparateNormalGen: public AbstractGen {
private:
    int myNumPoints, myNumCenters, myNumDims;
    Scalar myCenterRange, myClusterStdDev;

public:
    SeparateNormalGen(int numPoints, int numCenters, int numDims, Scalar centerRange, 
        Scalar clusterStdDev) {
        myNumPoints = numPoints; myNumCenters = numCenters; myNumDims = numDims;
        myCenterRange = centerRange; myClusterStdDev = clusterStdDev;
    }

    vector<Point> generateData(ostream &out) const {

        // Generate the data
	    vector<Point> data;
	    vector<Point> realCenters;
	    for (int i = 0; i < myNumCenters; i++) {
		    Point normCenter = getRandomPoint(myNumDims) * myCenterRange;
		    realCenters.push_back(normCenter);
		    for (int j = 0; j < myNumPoints/myNumCenters; j++)
			    data.push_back(getByNormDist(normCenter, myClusterStdDev));
	    }

        // Report general info about the data set
        out << "SEPARATE NORMAL DISTRIBUTIONS" << endl;
        out << "=============================" << endl << endl;
        out << "Number of points: " << myNumPoints << endl;
        out << "Number of centers: " << myNumCenters << endl;
        out << "Number of dimensions: " << myNumDims << endl;
        out << "Cluster center range in each dimension: " << myCenterRange << endl;
        out << "Intra-cluster standard deviation: " << myClusterStdDev << endl;
	    out << "\"Real\" potential: " << getKMeansPotential(data, realCenters) << endl << endl;

        // Return the data set
        return data;
    }
};

/**
 * Generates a collection of points uniformly at random in a hypercube.
 **/
class UniformGen: public AbstractGen {
private:
    int myNumPoints, myNumDims;
    Scalar myRange;

public:
    UniformGen(int numPoints, int numDims, Scalar range) {
        myNumPoints = numPoints; myNumDims = numDims;
        myRange = range;
    }

    vector<Point> generateData(ostream &out) const {

        // Generate the data
        vector<Point> data;
	    for (int i = 0; i < myNumPoints; i++)
            data.push_back(getRandomPoint(myNumDims) * myRange);

        // Report general info about the data set
        out << "UNIFORMLY RANDOM DATA" << endl;
        out << "=====================" << endl << endl;
        out << "Number of points: " << myNumPoints << endl;
        out << "Number of dimensions: " << myNumDims << endl;
        out << "Range in each dimension: " << myRange << endl << endl;
        
        // Return the data set
        return data;
    }
};

/**
 * Returns whether the given string represents a well formatted decimal number.
 **/
bool isANumber(string s) {

    // Handle a potential minus sign
    if (s.size() == 0) return false;
    if (s[0] == '-') s = s.substr(1);

    // Handle everything else
    bool hitDecimal = false, hitDigit = false;
    for (int i = 0; i < (int)s.size(); i++) {
        if (s[i] == '.') {
            if (hitDecimal) return false;
            hitDecimal = true;
        } else if (s[i] >= '0' && s[i] <= '9')
            hitDigit = true;
        else
            return false;
    }
    return hitDigit;
}

/**
 * Generates a collection of points by reading them from a file.
 * Points should be separated by commas or spaces in the data file.
 * The file may contain C++ style comments (//) on lines by themselves. These will
 * be reported to the log.
 **/
class ReadFileGen: public AbstractGen {
private:
    string myFilename;

public:
    ReadFileGen(const string &filename) {
        myFilename = filename;
    }

    vector<Point> generateData(ostream &out) const {
        ostringstream error;

        // Open the file
        ifstream in(myFilename.c_str());
        if (in.fail()) {
            error << "Could not load data file \"" << myFilename << "\".";
            fatalError(error.str().c_str());
        }

        // Read the file
        int lineNumber = 0, expDim = -1;
        vector<Point> data;
        vector<string> fileComments;
	    string line, word;
	    while(getline(in, line)) {
            lineNumber++;

            // Parse the line into wrods
            istringstream lineStr(line);
            vector<string> wordList;
            while (lineStr >> word)
                wordList.push_back(word);

            // Handle comments and blank lines
            if (wordList.size() == 0)
                continue;
            if (wordList[0][0] == '/' && wordList[0][1] == '/') {
                fileComments.push_back(line);
                continue;
            }

            // Check point formatting and dimension consistency
            vector<Scalar> scalars;
            for (int i = 0; i < (int)wordList.size(); i++) {
                
                // Parse this word around commas
                istringstream wordStr(wordList[i]);
                while (getline(wordStr, word, ',')) {
                    if (!isANumber(word)) {
                        error << "Badly formatted point at line " << lineNumber << " of data file \""
                            << myFilename << "\".";
                        fatalError(error.str().c_str());
                    }
                    scalars.push_back(atof(word.c_str()));
                }
            }
            if (expDim >= 0 && expDim != (int)scalars.size()) {
                error << "Expected point at line " << lineNumber << " of data file \""
                    << myFilename << "\" to have " << expDim << " dimensions.";
                fatalError(error.str().c_str());
            }
            expDim = (int)scalars.size();

            // Generate and store the point
		    Point p(expDim);
		    for (int i = 0; i < expDim; i++)
			    p[i] = scalars[i];
		    data.push_back(p);
	    }

        // Make sure we got at least one valid point
        if (data.size() == 0) {
            error << "Data file \"" << myFilename << "\" contains no points.";
            fatalError(error.str().c_str());
        }

        // Report general info about the data set
        int i;
        ostringstream title;
        title << "DATA FROM FILE \"" << myFilename << "\"";
        out << title.str() << endl;
        for (i = 0; i < (int)title.str().size(); i++)
            out << "=";
        out << endl << endl;
        out << "Number of points: " << int(data.size()) << endl;
        out << "Number of dimensions: " << expDim << endl;
        out << "Comments in file: " << endl;
        for (i = 0; i < (int)fileComments.size(); i++)
            out << "    " << fileComments[i] << endl;
        out << endl;

        // Return the data set
        return data;
    }
};

///////////////////////////
// CLUSTERING ALGORITHMS //
///////////////////////////

/**
 * A clustering algorithm.
 **/
class AbstractClusterer {
public:
	/**
	 * Returns the name of this clustering technique. Used for logging.
	 **/
    virtual string getName() const = 0;
	
	/**
	 * Chooses an initial clustering for this method.
	 **/
	virtual vector<Point> initClustering(const vector<Point> &data, int numCenters) const = 0;

	/**
	 * Given an initial set of points, this refines them up to numIterations times.
	 * Each successful refinement decreases numIterations by 1.
	 **/
	virtual vector<Point> doClustering(const vector<Point> &data, vector<Point> clustering, int &numIterations) const = 0;
};

/**
 * The awful pick random centers and stop algorithm.
 **/
class UniformRandomClusterer: public AbstractClusterer {
public:
    string getName() const {
        return "TRIVIAL RANDOM CLUSTERER";
    }
	vector<Point> initClustering(const vector<Point> &data, int numCenters) const {
		return chooseUniformCenters(data, numCenters);
	}
    vector<Point> doClustering(const vector<Point> &data, vector<Point> clustering, int &numIterations) const {
        return clustering;
    }
};

/**
 * K-means with uniform clustering.
 **/
class UniformKMeans: public AbstractClusterer {
public:
    string getName() const {
        return "K-MEANS WITH UNIFORM SEEDING";
    }
	vector<Point> initClustering(const vector<Point> &data, int numCenters) const {
		return chooseUniformCenters(data, numCenters);
	}
    vector<Point> doClustering(const vector<Point> &data, vector<Point> clustering, int &numIterations) const {
        return runKMeans(data, clustering, numIterations);
    }
};

/**
 * K-means with smart clustering, no retries at each stage.
 **/
class SmartKMeans: public AbstractClusterer {
public:
    string getName() const {
        return "K-MEANS++";
    }
	vector<Point> initClustering(const vector<Point> &data, int numCenters) const {
		return chooseSmartCenters(data, numCenters, 1);
	}
    vector<Point> doClustering(const vector<Point> &data, vector<Point> clustering, int &numIterations) const {
        return runKMeans(data, clustering, numIterations);
    }
};

/**
 * K-means with smart clustering, no retries at each stage.
 **/
class SmartKMeansSeedingOnly: public AbstractClusterer {
public:
    string getName() const {
        return "K-MEANS++ CUTOFF";
    }
	vector<Point> initClustering(const vector<Point> &data, int numCenters) const {
		return chooseSmartCenters(data, numCenters, 1);
	}
    vector<Point> doClustering(const vector<Point> &data, vector<Point> clustering, int &numIterations) const {
		vector<Point> result = clustering;
		if (numIterations > 0) {
			int temp = 1;
			result = runKMeans(data, clustering, temp);
			numIterations = numIterations - 1 + temp;
		}
		return result;
    }
};

/**
 * K-means with smart clustering, log(k) retries at each stage.
 **/
class SmartKMeansRetries: public AbstractClusterer {
public:
    string getName() const {
        return "K-MEANS++ WITH RETRIES DURING SEEDING";
    }
	vector<Point> initClustering(const vector<Point> &data, int numCenters) const {
		return chooseSmartCenters(data, numCenters, 2 + (int)log(Scalar(numCenters)));
	}
    vector<Point> doClustering(const vector<Point> &data, vector<Point> clustering, int &numIterations) const {
        return runKMeans(data, clustering, numIterations);
    }
};

//////////////////
// TESTING CODE //
//////////////////

void testClusteringMethod(ostream &out, const AbstractClusterer *clusterer, const vector<Point> &data,
						  int numClusters, int numTrials, int numIterations, int measurementInterval) {
	ASSERT(numIterations % measurementInterval == 0);
	int i;

    // Output the title
    ostringstream title;
    cout << "TESTING " << clusterer->getName() << "..." << endl;
    title << "RUNNING " << clusterer->getName() << ", looking for " << numClusters << " clusters";
    out << title.str() << endl;
    for (i = 0; i < (int)title.str().size(); i++)
        out << "=";
    out << endl << endl;

	// Set everything up
	vector<Scalar> measuredPotential, trialPotential, trialTime;
	if (measurementInterval == -1)
		measurementInterval = numIterations;
	Scalar bestMeasuredPotential = -1;
    Scalar maxTrialTime = 0, totalTrialTime = 0, maxTrialPot = 0, totalTrialPot = 0;
    Scalar minTrialTime = 1e20, minTrialPot = 1e20;
	int curTrial = 0, curIteration = 0;
	Scalar startTime, bonusTime;

	// Iterate as necessary
	vector<Point> thisClustering;
	while (curIteration < numIterations || curTrial < numTrials) {
		int stepSize = measurementInterval;

		// Iterate stepSize iterations
		do {
			// Initialize the clustering if appropriate
			if (thisClustering.size() == 0) {
				startTime = getTime();
				bonusTime = 0;
				thisClustering = clusterer->initClustering(data, numClusters);
				stepSize--;
			}

			// Do the clustering if appropriate
			if (stepSize > 0) {
				thisClustering = clusterer->doClustering(data, thisClustering, stepSize);
			}

			// Update our best results
			double tempStartTime = getTime();
			Scalar thisPotential = getKMeansPotential(data, thisClustering);
			bonusTime += (getTime() - tempStartTime);
			if (curIteration < numIterations) {
				if (bestMeasuredPotential < 0 || thisPotential < bestMeasuredPotential)
					bestMeasuredPotential = thisPotential;
			}

			// Reset the clustering if appropriate, and instrument this clustering trial
			if (stepSize != 0) {
				Scalar thisTime = (getTime() - startTime) - bonusTime; 
				if (curTrial < numTrials) {
					minTrialPot = min(thisPotential, minTrialPot);
					maxTrialPot = max(thisPotential, maxTrialPot);
					totalTrialPot += thisPotential;
					minTrialTime = min(thisTime, minTrialTime);
					maxTrialTime = max(thisTime, maxTrialTime);
					totalTrialTime += thisTime;
					curTrial++;
					cout << "Completed trial " << curTrial << " of " << numTrials 
						 << " (potential = " << thisPotential << ", time = " << thisTime << ")..." << endl;
					trialPotential.push_back(thisPotential);
					trialTime.push_back(thisTime);
				}
				thisClustering.clear();
			}
		} while (stepSize > 0);

		// Instrument the iteration data
		if (curIteration < numIterations) {
			measuredPotential.push_back(bestMeasuredPotential);
			curIteration += measurementInterval;
			cout << "Completed iteration " << curIteration << " of " << numIterations 
				 << " (potential = " << bestMeasuredPotential << ")..." << endl;
		}
	}

	// Output the summary over all trials
    out << "Potential average by trial: " << (totalTrialPot / numTrials) << endl;
    out << "Potential range by trial: " << minTrialPot << " to " << maxTrialPot << endl;
    out << "Time average by trial: " << (totalTrialTime / numTrials) << " seconds" << endl;
    out << "Time range by trial: " << minTrialTime << " to " << maxTrialTime << " seconds" << endl << endl;

	// Output the information for each trial
	for (i = 0; i < numTrials; i++)
        out << "Trial " << (i+1) << " of " << numTrials << ": "
            << trialPotential[i] << " potential, " << trialTime[i] << " seconds." << endl;
	out << endl;

	// Output the information for each iteration
	for (i = 0; i < numIterations/measurementInterval; i++)
		out << "Potential after " << (i+1)*measurementInterval << " iterations: " 
			<< measuredPotential[i] << endl;
	out << endl << endl;
}

/**
 * Generates data according to the given method, and then sees how various clustering
 * algorithms perform on the data according to the k-means potential. Data is aggregated
 * according to potential after a certain number of iterations.
 **/
void testAllMethods(const string &logFilename, const AbstractGen *dataGen, int numClusters) {
	const int numTrials = 20;
    const int numIterations = 200;
	const int measurementInterval = 5;

    // Open the log file
    cout << "Outputting this test case to log file: \"" << logFilename << "\"." << endl << endl;
    ofstream out(logFilename.c_str());
    if (out.fail()) {
        ostringstream error;
        error << "Could not write to log file \"" << logFilename << "\".";
        fatalError(error.str().c_str());
    }

    // Generate the inputs
    cout << "Generating data..." << endl;
    vector<Point> data = dataGen->generateData(out);

    // Run the algorithms
    testClusteringMethod(out, new UniformRandomClusterer(), data, numClusters, numTrials, numIterations, measurementInterval);
    testClusteringMethod(out, new UniformKMeans(), data, numClusters, numTrials, numIterations, measurementInterval);
    testClusteringMethod(out, new SmartKMeans(), data, numClusters, numTrials, numIterations, measurementInterval);
    testClusteringMethod(out, new SmartKMeansRetries(), data, numClusters, numTrials, numIterations, measurementInterval);
	testClusteringMethod(out, new SmartKMeansSeedingOnly(), data, numClusters, numTrials, numIterations, measurementInterval);
}

/** 
 * Runs a test case of our choice.
 **/
int main() {
    //testAllMethods("norm10(10).txt", new SeparateNormalGen(10000, 10, 5, 10, 1), 10);
	//testAllMethods("norm10(25).txt", new SeparateNormalGen(10000, 10, 5, 10, 1), 25);
	//testAllMethods("norm10(50).txt", new SeparateNormalGen(10000, 10, 5, 10, 1), 50);
    //testAllMethods("norm25(10).txt", new SeparateNormalGen(10000, 25, 15, 500, 1), 10);
	//testAllMethods("norm25(25).txt", new SeparateNormalGen(10000, 25, 15, 500, 1), 25);
	//testAllMethods("norm25(50).txt", new SeparateNormalGen(10000, 25, 15, 500, 1), 50);
    //testAllMethods("cloud(10).txt", new ReadFileGen("cloud_input.txt"), 10);
	//testAllMethods("cloud(25).txt", new ReadFileGen("cloud_input.txt"), 25);
	//testAllMethods("cloud(50).txt", new ReadFileGen("cloud_input.txt"), 50);
	//testAllMethods("intrusion(10).txt", new ReadFileGen("intrusion_input.txt"), 10);
	//testAllMethods("intrusion(25).txt", new ReadFileGen("intrusion_input.txt"), 25);
	//testAllMethods("intrusion(50).txt", new ReadFileGen("intrusion_input.txt"), 50);
	//testAllMethods("spam(10).txt", new ReadFileGen("spam_input.txt"), 10);
	//testAllMethods("spam(25).txt", new ReadFileGen("spam_input.txt"), 25);
	//testAllMethods("spam(50).txt", new ReadFileGen("spam_input.txt"), 50);
	//testAllMethods("test(10).txt", new ReadFileGen("test2.txt"), 10);
	testAllMethods("test2(10).txt", new ReadFileGen("test2.txt"), 10);
	//testAllMethods("test(10).txt", new ReadFileGen("test2.txt"), 10);

	//http://kdd.ics.uci.edu/databases/ipums/ipums.html
	return 0;
}
