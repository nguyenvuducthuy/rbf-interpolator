#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

#include <vector>

namespace RBF {
   
enum class FunctionType
{
    Gaussian,         // f(r) = exp(-(epsilon*r)^2)
    ThinPlateSpline,  // f(r) = (r^2)*log(r)
    InverseQuadratic, // f(r) = (1 + (epsilon*r)^2)^(-1)
    BiharmonicSpline, // f(r) = r
};

class Interpolator
{
public:
    Interpolator();

    // Function type
    FunctionType functionType;

    double epsilon;
    double lambda;

    bool useRegularization;

    // API
    void    resetAll();
    void    addCenterPoint(const double y, const std::vector<double>& x);
    void    computeWeights();
    double  getInterpolatedValue(const std::vector<double>& x) const;

    // getters
    const std::vector<double>&              getYs() const { return ys; }
    const std::vector<std::vector<double>>& getXs() const { return xs; }
    const std::vector<double>&              getW()  const { return w;  }

private:

    // registered center points
    std::vector<double>              ys;
    std::vector<std::vector<double>> xs;

    // computed weights
    std::vector<double>              w;

    // for error check
    bool readyForUse;

    // returns f(r)
    double getRBFValue(const double r) const;
    // returns f(||xj - xi||)
    double getRBFValue(const std::vector<double>& xi, const std::vector<double>& xj) const;
};

}

#endif // INTERPOLATOR_H
