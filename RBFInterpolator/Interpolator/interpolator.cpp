#include "interpolator.h"

#include <cmath>
#include <fstream>
#include <sstream>
#include <Eigen/Core>
#include <Eigen/LU>

using namespace std;
using namespace Eigen;

extern VectorXd solveLinearSystem(MatrixXd A, VectorXd y);
template<class T> extern string toString(T x);

Interpolator::Interpolator() :
    functionType(BIHARMONICSPLINE),
    epsilon(2.0),
    lambda(0.1),
    useRegularization(true),
    readyForUse(false)
{
}

void Interpolator::resetAll()
{
    ys.clear();
    xs.clear();

    w.clear();

    readyForUse = false;
}

void Interpolator::addCenterPoint(double y, vector<double> x)
{
    ys.push_back(y);
    xs.push_back(x);

    readyForUse = false;
}

void Interpolator::computeWeights()
{
    assert(ys.size() == xs.size());

    int dim = ys.size();

    MatrixXd O = MatrixXd::Zero(dim, dim);
    VectorXd y = Map<VectorXd>(&ys[0], dim);

    for (int i = 0; i < dim; ++ i) {
        for (int j = 0; j < dim; ++ j) {
            O(i, j) = getRBFValue(xs[i], xs[j]);
        }
    }

    MatrixXd A;
    VectorXd b;
    if (useRegularization) {
        MatrixXd O2 = MatrixXd::Zero(dim * 2, dim);
        for (int i = 0; i < dim; ++ i) {
            for (int j = 0; j < dim; ++ j) {
                O2(i, j) = O(i, j);
            }
        }
        double coef = 0.5 * lambda;
        for (int i = 0; i < dim; ++ i) {
            O2(i + dim, i) = coef;
        }

        VectorXd y2 = VectorXd::Zero(dim * 2);
        for (int i = 0; i < dim; ++ i) {
            y2(i) = y(i);
        }

        A = O2.transpose() * O2;
        b = O2.transpose() * y2;
    } else {
        A = O;
        b = y;
    }

    VectorXd x = solveLinearSystem(A, b);
    assert(x.rows() == dim);

    w.clear();
    for (int i = 0; i < dim; ++ i) {
        w.push_back(x(i));
    }

    readyForUse = true;
}

double Interpolator::getInterpolatedValue(vector<double> x)
{
    if (!readyForUse) {
        return 0.0;
    }

    int    dim    = w.size();
    double result = 0.0;

    for (int i = 0; i < dim; ++ i) {
        result += w[i] * getRBFValue(x, xs[i]);
    }

    return result;
}

void Interpolator::exportToCSV(const string& filePath) {
    if (!readyForUse) {
        return;
    }

    ofstream ofs(filePath.c_str());
    string csv;

    int dim     = xs[0].size();
    int nPoints = ys.size();

    // the first line: <dim>, <nPoints>, <functionType>, <epsilon>, <useRegularization>, <lambda>
    csv += toString(dim) + ","
            + toString(nPoints) + ","
            + toString(functionType) + ","
            + toString(epsilon) + ","
            + toString(useRegularization) + ","
            + toString(lambda) + "\n";

    // the other lines: <w>, <y>, <x1>, ..., <xn>
    for (int i = 0; i < nPoints; ++ i) {
        csv += toString(w[i]) + ",";
        csv += toString(ys[i]) + ",";
        for (int j = 0; j < dim; ++ j) {
            csv += toString(xs[i][j]);
            if (j != dim - 1) {
                csv += ",";
            } else {
                csv += "\n";
            }
        }
    }

    ofs << csv;
}

vector<double> Interpolator::getYs() {
    return ys;
}

vector<vector<double> > Interpolator::getXs() {
    return xs;
}

vector<double> Interpolator::getW() {
    return w;
}

double Interpolator::getRBFValue(double r)
{
    double result;
    switch (functionType) {
    case GAUSSIAN:
        result = exp(- pow((epsilon * r), 2.0));
        break;
    case THINPLATESPLINE:
        result = r * r * log(r);
        if (isnan(result)) {
            result = 0.0;
        }
        break;
    case INVERSEQUADRATIC:
        result = 1.0 / (1.0 + pow((epsilon * r), 2.0));
        break;
    case BIHARMONICSPLINE:
        result = r;
        break;
    default:
        break;
    }
    return result;
}

double Interpolator::getRBFValue(vector<double> xi, vector<double> xj)
{
    assert (xi.size() == xj.size());

    VectorXd xiVec = Map<VectorXd>(&xi[0], xi.size());
    VectorXd xjVec = Map<VectorXd>(&xj[0], xj.size());

    return getRBFValue((xjVec - xiVec).norm());
}

VectorXd solveLinearSystem(MatrixXd A, VectorXd y)
{
    FullPivLU<MatrixXd> lu(A);
    return lu.solve(y);
}

template<class T> inline string toString(T x)
{
    ostringstream sout;
    sout << x;
    return sout.str();
}
