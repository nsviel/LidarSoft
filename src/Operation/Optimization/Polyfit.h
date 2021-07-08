#ifndef _POLYNOMIAL_REGRESSION_H
#define _POLYNOMIAL_REGRESSION_H  __POLYNOMIAL_REGRESSION_H

#include <cmath>
#include <stdexcept>
#include "Eigen/Dense"
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/lu.hpp>

//https://gist.github.com/chrisengelsma/108f7ab0a746323beaaf7d6634cf4add
template <class TYPE>
std::vector<TYPE> polyfit(
  const std::vector<TYPE> & x,
  const std::vector<TYPE> & y,
  const int &               order)
{

  // The size of xValues and yValues should be same
  if (x.size() != y.size()) {
    throw std::invalid_argument( "The size of x & y arrays are different" );
  }
  // The size of xValues and yValues cannot be 0, should not happen
  if (x.size() == 0 || y.size() == 0) {
    throw std::invalid_argument( "The size of x or y arrays is 0" );
  }

  size_t N = x.size();
  int n = order;
  int np1 = n + 1;
  int np2 = n + 2;
  int tnp1 = 2 * n + 1;
  TYPE tmp;
  //-------------------------

  // X = vector that stores values of sigma(xi^2n)
  std::vector<TYPE> X(tnp1);
  for (int i = 0; i < tnp1; ++i) {
    X[i] = 0;
    for (int j = 0; j < N; ++j)
      X[i] += (TYPE)pow(x[j], i);
  }

  // a = vector to store final coefficients.
  std::vector<TYPE> a(np1);

  // B = normal augmented matrix that stores the equations.
  std::vector<std::vector<TYPE> > B(np1, std::vector<TYPE> (np2, 0));

  for (int i = 0; i <= n; ++i)
    for (int j = 0; j <= n; ++j)
      B[i][j] = X[i + j];

  // Y = vector to store values of sigma(xi^n * yi)
  std::vector<TYPE> Y(np1);
  for (int i = 0; i < np1; ++i) {
    Y[i] = (TYPE)0;
    for (int j = 0; j < N; ++j) {
      Y[i] += (TYPE)pow(x[j], i)*y[j];
    }
  }

  // Load values of Y as last column of B
  for (int i = 0; i <= n; ++i)
    B[i][np1] = Y[i];

  n += 1;
  int nm1 = n-1;

  // Pivotisation of the B matrix.
  for (int i = 0; i < n; ++i)
    for (int k = i+1; k < n; ++k)
      if (B[i][i] < B[k][i])
        for (int j = 0; j <= n; ++j) {
          tmp = B[i][j];
          B[i][j] = B[k][j];
          B[k][j] = tmp;
        }

  // Performs the Gaussian elimination.
  // (1) Make all elements below the pivot equals to zero
  //     or eliminate the variable.
  for (int i=0; i<nm1; ++i)
    for (int k =i+1; k<n; ++k) {
      TYPE t = B[k][i] / B[i][i];
      for (int j=0; j<=n; ++j)
        B[k][j] -= t*B[i][j];         // (1)
    }

  // Back substitution.
  // (1) Set the variable as the rhs of last equation
  // (2) Subtract all lhs values except the target coefficient.
  // (3) Divide rhs by coefficient of variable being calculated.
  for (int i=nm1; i >= 0; --i) {
    a[i] = B[i][n];                   // (1)
    for (int j = 0; j<n; ++j)
      if (j != i)
        a[i] -= B[i][j] * a[j];       // (2)
    a[i] /= B[i][i];                  // (3)
  }

  return a;
}

//Homemade function
template <class TYPE>
vector<TYPE> polyfit_homemade(
  const vector<TYPE>& vec_x,
  const vector<TYPE>& vec_y,
  const int& n)
{
  //Checking
  if(vec_x.size() != vec_y.size()) {
    throw std::invalid_argument( "The size of x & y arrays are different" );
  }
  if(vec_x.size() == 0 || vec_y.size() == 0) {
    throw std::invalid_argument( "The size of x or y arrays is 0" );
  }
  //------------------

  //Calcule du jacobien
  MatrixXf J = MatrixXf::Zero(vec_x.size(), n+1);
  for(int i=0; i<vec_x.size(); i++){
    float x = vec_x[i];

    for(int j=0; j<=n; j++){
      J(i, j) = pow(x,j);
    }
  }
  //------------------

  //Optimization
  VectorXf E = VectorXf::Zero(vec_x.size());
  VectorXf P = VectorXf::Zero(n+1);
  int iter = 10;

  for(int i=0; i<iter; i++){
    for(int j=0; j<vec_x.size(); j++){
      float x = vec_x[j];
      float y = vec_y[j];

      //Fitting
      float fit = 0;
      for(int k=0; k<=n; k++){
        fit += P(k)*pow(x,k);
      }

      //Error vectors
      E(j) = fit - y;
    }

    //Optimization
    arma::mat A(J.rows(), J.cols());
    for(int i=0; i<J.rows(); i++){
      for(int j=0; j<J.cols(); j++){
        A(i,j) = J(i,j);
      }
    }

    arma::mat Ji = arma::pinv(A);

    MatrixXf Jinv(Ji.n_rows, Ji.n_cols);
    for(int i=0; i<Ji.n_rows; i++){
      for(int j=0; j<Ji.n_cols; j++){
        Jinv(i,j) = Ji(i,j);
      }
    }
    //MatrixXf Jinv = J.completeOrthogonalDecomposition().pseudoInverse();

    P = P - Jinv*E;
  }

  VectorXf a = VectorXf::Zero(vec_x.size());
  VectorXf b = VectorXf::Zero(vec_y.size());
  for(int i=0; i<vec_y.size(); i++){
    a(i) = vec_x[i];
    b(i) = vec_y[i];
  }

  say("-------lin:");
  MatrixXf Jinv = J.completeOrthogonalDecomposition().pseudoInverse();
  VectorXf coe = Jinv*b;
  coe = J.completeOrthogonalDecomposition().solve(b);
  say(coe);

  say("-------solver:");
  coe = J.colPivHouseholderQr().solve(b);
  coe = (J.transpose() * J).ldlt().solve(J.transpose() * b);

  coe = J.bdcSvd(ComputeThinU | ComputeThinV).solve(b);
  say(coe);
  //------------------

  //Convert into std vector
  vector<TYPE> coeffs;
  for(int i=0; i<P.size(); i++){
    coeffs.push_back(P(i));
  }

  return coeffs;
}

//https://github.com/patLoeber/Polyfit/tree/8ebea0ee4480c293d556e4061fa549c98218693c
template <typename T>
std::vector<T> polyfit_boost(
  const std::vector<T> &xValues,
  const std::vector<T> &yValues,
  const int degree,
  const std::vector<T>& weights = std::vector<T>()){
    using namespace boost::numeric::ublas;

    if (xValues.size() != yValues.size())
        throw std::invalid_argument("X and Y vector sizes do not match");

    bool useWeights = weights.size() > 0 && weights.size() == xValues.size();

    // one more because of c0 coefficient
    int numCoefficients = degree + 1;

    size_t nCount = xValues.size();
    matrix<T> X(nCount, numCoefficients);
    matrix<T> Y(nCount, 1);

    // fill Y matrix
    for (size_t i = 0; i < nCount; i++)
    {
        if (useWeights)
            Y(i, 0) = yValues[i] * weights[i];
        else
            Y(i, 0) = yValues[i];
    }

    // fill X matrix (Vandermonde matrix)
    for (size_t nRow = 0; nRow < nCount; nRow++)
    {
        T nVal = 1.0f;
        for (int nCol = 0; nCol < numCoefficients; nCol++)
        {
            if (useWeights)
                X(nRow, nCol) = nVal * weights[nRow];
            else
                X(nRow, nCol) = nVal;
            nVal *= xValues[nRow];
        }
    }

    // transpose X matrix
    matrix<T> Xt(trans(X));
    // multiply transposed X matrix with X matrix
    matrix<T> XtX(prec_prod(Xt, X));
    // multiply transposed X matrix with Y matrix
    matrix<T> XtY(prec_prod(Xt, Y));

    // lu decomposition
    permutation_matrix<int> pert(XtX.size1());
    const std::size_t singular = lu_factorize(XtX, pert);
    // must be singular
    assert(singular == 0);

    // backsubstitution
    lu_substitute(XtX, pert, XtY);

    // copy the result to coeff
    return std::vector<T>(XtY.data().begin(), XtY.data().end());
}

//Eigen based
template<typename T>
std::vector<T> polyfit_Eigen(
  const vector<T>& X,
  const vector<T>& Y,
  const int n){
  int size = X.size();
  MatrixXf mat_X(size, n+1);
  MatrixXf mat_Y(size, 1);
  //------------------------

  // fill Y matrix
  for(int i=0; i<size; i++){
    mat_Y(i, 0) = Y[i];
  }

  //Jacobian
  MatrixXf J = MatrixXf::Zero(X.size(), n+1);
  for(int i=0; i<X.size(); i++){
    float x = X[i];

    for(int j=0; j<=n; j++){
      mat_X(i, j) = pow(x,j);
    }
  }

  VectorXf coefficients;
  /*coefficients = mat_X.jacobiSvd(ComputeThinU | ComputeThinV).solve(mat_Y);
  say(coefficients);*/
  /*coefficients = mat_X.fullPivLu().solve(mat_Y);
  say(coefficients);
  coefficients = mat_X.colPivHouseholderQr().solve(mat_Y);
  say(coefficients);
  coefficients = mat_X.fullPivHouseholderQr().solve(mat_Y);
  say(coefficients);*/
  coefficients = mat_X.bdcSvd(ComputeThinU | ComputeThinV).solve(mat_Y);
  //say(coefficients);
  /*coefficients = mat_X.completeOrthogonalDecomposition().solve(mat_Y);
  say(coefficients);*/


  //------------------------
  vector<T> coeffs(coefficients.data(), coefficients.data() + (n+1));
  return coeffs;
}

//Polyval function
template<typename T>
std::vector<T> polyval(std::vector<T>& data_X, std::vector<T>& coeffs, int n){
  std::vector<T> px_out;
  int size_in = data_X.size();
  //-------------------

  for(int i=0; i<size_in; i++){
    T px = 0.0f;

    for(int j=0; j<n+1; j++){
      px += coeffs[j]*pow(data_X[i],j);
    }

    px_out.push_back(px);
  }

  //-------------------
  return px_out;
}

#endif
