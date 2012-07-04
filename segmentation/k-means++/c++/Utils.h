#ifndef UTILS_H
#define UTILS_H

/**
 * Includes
 **/
#include <iostream>
#include <vector>
using namespace std;

/**
 * Type definitions
 **/
typedef double Scalar;

/**
 * General utilities
 **/
//#define DEBUG_MODE
#ifdef DEBUG_MODE
#define ASSERT(condition) if (!(condition)) assertionFailure(__FILE__, __LINE__);
#else
#define ASSERT(condition)
#endif
void fatalError(const char *message);
void assertionFailure(const char *filename, int lineNumber);
double getTime();

/**
 * Point class
 **/
class Point {
public:
	Point() {}
	Point(int numDimensions) {myData = vector<Scalar>(numDimensions);}
	Point(const Point &rhs) {myData = rhs.myData;}

	inline int getNumDimensions() const {return (int)myData.size();}
	inline Scalar operator[](int index) const {ASSERT(index >= 0 && index < getNumDimensions()); return myData[index];}
	inline Scalar &operator[](int index) {ASSERT(index >= 0 && index < getNumDimensions()); return myData[index];}

private:
	vector<Scalar> myData;
};

/**
 * Point utilities
 **/
ostream &operator<<(ostream &out, const Point &rhs);
ostream &operator<<(ostream &out, const vector<Point> &rhs);
inline Scalar distSq(const Point &lhs, const Point &rhs);
Point operator-(const Point &x);
Point operator+(const Point &lhs, const Point &rhs);
Point operator-(const Point &lhs, const Point &rhs);
Point operator*(const Point &lhs, Scalar f);
Point operator*(Scalar f, const Point &lhs);
Point operator/(const Point &lhs, Scalar f);

/**
 * Randomization utilities
 **/
Scalar getRandomScalar();
Point getRandomPoint(int numDimensions);
Point getByNormDist(const Point &center, Scalar variance);

/**
 * K-means utilities
 **/
vector<Point> chooseUniformCenters(const vector<Point> &data, int numCenters);
vector<Point> chooseSmartCenters(const vector<Point> &data, int numCenters, int numLocalTries);
vector<Point> runKMeans(const vector<Point> &data, const vector<Point> &initCenters, int &numIterations);
Scalar getKMeansPotential(const vector<Point> &data, const vector<Point> &centers);

#endif