#include <iostream>
#include <math.h>
#include <time.h>
#include <iomanip>
using namespace std;

//function declaration
double calculusOne (int n, double xDelta);
double calculusTwo (int n, double xDelta);

int main () {
    clock_t t;
    int precision [] =  {2, 20, 200, 2000, 20000, 200000};
    double upperLimit = 1.0;
    double lowerLimit = 0.0;
    double integralInterval = upperLimit - lowerLimit;
    cout << "Calculus One" <<endl;
    for (int i = 0; i <= 5; i++) {
        double xDelta = integralInterval / precision[i];
        t = clock();
        double piValue = calculusOne(precision[i], xDelta);
        cout << "Time: "<< (((float)(clock() - t))/CLOCKS_PER_SEC) * 1000.0 <<" milliseconds" <<endl;
        cout << setprecision(12)<< piValue <<endl;
    }
    cout << "Calculus Two" <<endl;
    for (int i = 0; i <= 5; i++) {
        double xDelta = integralInterval / precision[i];
        t = clock();
        double piValue = calculusTwo(precision[i], xDelta);
        cout << "Time: "<< (((float)(clock() - t))/CLOCKS_PER_SEC) * 1000.0 <<" milliseconds" <<endl;
        cout << piValue <<endl;
    }
    return 0;
}

double calculusOne (int n, double xDelta) {
    double xValue = 0.0;
    double pi = 0.0;
    for (int i = 1; i <= n; i++) {
        //find height using f(x)
        double height = 4.0 / (1.0 + (xValue * xValue));
        double area = xDelta * height;
        pi += area;
        xValue += xDelta;
    }
    return pi;
}

double calculusTwo (int n, double xDelta) {
    double xValue = 0.0;
    double pi = 0.0;
    for (int i = 1; i <= n; i++) {
        //find height using f(x)
        double height = 4.0 * sqrt(1.0 - (xValue * xValue));
        double area = xDelta * height;
        pi += area;
        xValue += xDelta;
    }
    return pi;
}