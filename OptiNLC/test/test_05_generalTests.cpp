#include <catch2/catch.hpp>
#include <iostream>
#include <memory>
#include <vector>

#include <Eigen/Dense>

#include "osqp.h"

using Eigen::MatrixXd;

// Test objective function
double
testObjectiveFunction( const Eigen::VectorXd& x )
{
  // Example: f(x) = x1^2 + x2^2
  return x( 0 ) * x( 0 ) + x( 1 ) * x( 1 );
}

// Helper function to create and populate CSC matrices
std::unique_ptr<OSQPCscMatrix>
createCscMatrix( OSQPInt rows, OSQPInt cols, OSQPInt nnz, const std::vector<OSQPFloat>& values, const std::vector<OSQPInt>& rowIndices,
                 const std::vector<OSQPInt>& colPointers )
{
  auto matrix = std::make_unique<OSQPCscMatrix>();
  csc_set_data( matrix.get(), rows, cols, nnz,
                const_cast<OSQPFloat*>( values.data() ), // Safe because data is not const
                const_cast<OSQPInt*>( rowIndices.data() ), const_cast<OSQPInt*>( colPointers.data() ) );
  return matrix;
}

TEST_CASE("Eigen Test")
{
  MatrixXd m_eigen( 2, 2 );
  m_eigen << 3, -1, 2.5, 1.5;
  std::cout << m_eigen << std::endl;
}

TEST_CASE("OSQP Test"){
  // Define problem data
  std::vector<OSQPFloat> P_x = { 4.0, 1.0, 2.0 };
  std::vector<OSQPInt>   P_i = { 0, 0, 1 };
  std::vector<OSQPInt>   P_p = { 0, 1, 3 };

  std::vector<OSQPFloat> q = { 1.0, 1.0 };

  std::vector<OSQPFloat> A_x = { 1.0, 1.0, 1.0, 1.0 };
  std::vector<OSQPInt>   A_i = { 0, 1, 0, 2 };
  std::vector<OSQPInt>   A_p = { 0, 2, 4 };

  std::vector<OSQPFloat> l = { 1.0, 0.0, 0.0 };
  std::vector<OSQPFloat> u = { 1.0, 0.7, 0.7 };

  OSQPInt n = 2;
  OSQPInt m = 3;

  // Create and populate CSC matrices
  auto P = createCscMatrix( n, n, static_cast<OSQPInt>( P_x.size() ), P_x, P_i, P_p );
  auto A = createCscMatrix( m, n, static_cast<OSQPInt>( A_x.size() ), A_x, A_i, A_p );

  // Configure OSQP settings
  std::unique_ptr<OSQPSettings> settings = std::make_unique<OSQPSettings>();
  if( !settings )
  {
    CAPTURE("Failed to allocate memory for OSQP settings.");
    REQUIRE(false);
  }
  osqp_set_default_settings( settings.get() );
  settings->alpha = 1.0; // Change alpha parameter

  // Solver setup
  OSQPSolver* solver   = nullptr;
  OSQPInt     exitflag = osqp_setup( &solver, P.get(), q.data(), A.get(), l.data(), u.data(), m, n, settings.get() );
  
  if (exitflag) {
    CAPTURE(exitflag, "OSQP setup failed with exit code: ");
    REQUIRE(exitflag == 0);
  }

  // Solve the problem
  exitflag = osqp_solve( solver );
  if( exitflag == 0 )
  {
    std::cout << "\nOSQP solution:" << std::endl;
    for( OSQPInt i = 0; i < n; ++i )
    {
      std::cout << "x[" << i << "] = " << solver->solution->x[i] << std::endl;
    }
  }
  else
  {
    CAPTURE("OSQP solve failed with exit code: ", exitflag);
    REQUIRE(false);
  }

  // Clean up
  if( solver )
  {
    osqp_cleanup( solver );
  }

  REQUIRE(exitflag == 0);
}
