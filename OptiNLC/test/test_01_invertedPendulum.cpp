#include <fstream>
#include <iostream>

#include "OptiNLC_Solver.h"
#include <catch2/catch.hpp>

TEST_CASE( "TEST Inverted Pendulum:" )
{

  OptiNLC_Options options;
  options.setDefaults();
  options.intermediateIntegration = 6;
  options.OSQP_max_iter           = 10000;
  options.maxNumberOfIteration    = 10000;
  options.OSQP_time_limit         = 1.;
  options.OptiNLC_ACC             = 1.e-10;
  options.OSQP_eps_abs            = 1.e-12;
  options.OSQP_eps_rel            = 1.e-12;
  options.timeStep                = 0.09;
  // Define the state and input sizes
  constexpr int StateSize       = 4;
  constexpr int InputSize       = 1;
  constexpr int ConstraintsSize = 0;
  constexpr int ControlPoints   = 40;

  // Create an instance of the OCP class with the appropriate horizon and time step

  OptiNLC_OCP<double, InputSize, StateSize, ConstraintsSize, ControlPoints> ocp( &options );


  VECTOR<double, StateSize> initialState = { 0.0, 0.0, -1.0, 0.0 };
  VECTOR<double, InputSize> initialInput = { 0.0 };

  // Define a simple dynamic model as a lambda function
  ocp.setDynamicModel( [&]( const VECTOR<double, StateSize>& state, const VECTOR<double, InputSize>& input,
                            VECTOR<double, StateSize>& derivative, double currentTime, void* userData ) {
    const double m2a     = 1.0;
    double       epsilon = m2a / ( m2a + 10. );
    derivative[0]        = state[1];
    derivative[1]        = -epsilon * state[2] + input[0];
    derivative[2]        = state[3];
    derivative[3]        = state[2] - input[0];
  } );

  // Define an objective function as a lambda function, states and input at tf
  ocp.setObjectiveFunction( [&]( const VECTOR<double, StateSize>& state, const VECTOR<double, InputSize>& input, double current_time ) {
    // Define your objective function here
    // For example, you can minimize the squared sum of state variables
    // Compute squared norm manually
    double stateSquaredNorm = 0.0;
    for( int i = 0; i < StateSize; ++i )
    {
      stateSquaredNorm += state[i] * state[i];
    }

    double inputSquaredNorm = 0.0;
    for( int i = 0; i < InputSize; ++i )
    {
      inputSquaredNorm += input[i] * input[i];
    }

    return stateSquaredNorm + 0.001 * inputSquaredNorm;
  } );


  // Define a simple input update method
  ocp.setInputUpdate(
    [&]( const VECTOR<double, StateSize>& state, const VECTOR<double, InputSize>& input, double currentTime, void* userData ) {
      VECTOR<double, InputSize> updatedInput = { 0.0 };
      return updatedInput;
    } );
  // Define a states constraints method
  ocp.setUpdateStateLowerBounds( [&]( const VECTOR<double, StateSize>& state, const VECTOR<double, InputSize>& input ) {
    VECTOR<double, StateSize> stateConstraints;
    stateConstraints.setConstant( -1e3 );
    return stateConstraints;
  } );
  // Define a states constraints method
  ocp.setUpdateStateUpperBounds( [&]( const VECTOR<double, StateSize>& state, const VECTOR<double, InputSize>& input ) {
    VECTOR<double, StateSize> stateConstraints;
    stateConstraints.setConstant( 1e3 );
    return stateConstraints;
  } );
  // Define a inputs constraints method
  ocp.setUpdateInputLowerBounds( [&]( const VECTOR<double, StateSize>& state, const VECTOR<double, InputSize>& input ) {
    VECTOR<double, InputSize> inputConstraints;
    inputConstraints.setConstant( -1e3 );
    return inputConstraints;
  } );
  // Define a inputs constraints method
  ocp.setUpdateInputUpperBounds( [&]( const VECTOR<double, StateSize>& state, const VECTOR<double, InputSize>& input ) {
    VECTOR<double, InputSize> inputConstraints;
    inputConstraints.setConstant( 1e3 );
    return inputConstraints;
  } );

  // Define a functions constraints method
  //
  ocp.setUpdateFunctionConstraints( [&]( const VECTOR<double, StateSize>& state, const VECTOR<double, InputSize>& input ) {
    VECTOR<double, ConstraintsSize> functionsConstraints;
    functionsConstraints.setConstant( 0.0 );
    return functionsConstraints;
  } );
  ocp.setUpdateFunctionConstraintsLowerBounds( [&]( const VECTOR<double, StateSize>& state, const VECTOR<double, InputSize>& input ) {
    VECTOR<double, ConstraintsSize> functionsConstraints;
    functionsConstraints.setConstant( -std::numeric_limits<double>::infinity() );
    return functionsConstraints;
  } );
  ocp.setUpdateFunctionConstraintsUpperBounds( [&]( const VECTOR<double, StateSize>& state, const VECTOR<double, InputSize>& input ) {
    VECTOR<double, ConstraintsSize> functionsConstraints;
    functionsConstraints.setConstant( std::numeric_limits<double>::infinity() );
    return functionsConstraints;
  } );
  // user defined function for all constreaintzs


  OptiNLC_Solver<double, InputSize, StateSize, ConstraintsSize,ControlPoints> _solver(ocp);
  _solver.solve(0.0, initialState, initialInput);

  auto opt_x =  _solver.get_optimal_states();
  auto time = _solver.getTime();

  std::ofstream dataFile("eigen_data_01.txt");

  // Check if the file is open
  if (dataFile.is_open()) {
      for (int i = 0; i < time.size(); ++i) {
          dataFile << time[i]<< " " << opt_x[StateSize*i + 0] << " " << opt_x[StateSize*i + 1]
                  << " " << opt_x[StateSize*i + 2]
                  << " " << opt_x[StateSize*i + 3] << std::endl;
      }
      dataFile.close();
      std::cout << "Data saved to eigen_data_01.txt" << std::endl;
  } else {
      std::cerr << "Unable to open file for writing." << std::endl;
      REQUIRE( false );
  }

  double sum0=0, sum1=0, sum2=0, sum3=0, sum00=0, sum11=0, sum22=0, sum33=0;

  //first half of pendulum simulation
  for (int i = 0; i < time.size() / 2; ++i) {
    sum0 += std::abs(opt_x[StateSize * i]);
    sum1 += std::abs(opt_x[StateSize * i + 1]);
    sum2 += std::abs(opt_x[StateSize * i + 2]);
    sum3 += std::abs(opt_x[StateSize * i + 3]);
  }

  //second half of pendulum simulation
  for (int i = time.size() / 2; i < time.size(); ++i) {
    sum00 += std::abs(opt_x[StateSize * i]);
    sum11 += std::abs(opt_x[StateSize * i + 1]);
    sum22 += std::abs(opt_x[StateSize * i + 2]);
    sum33 += std::abs(opt_x[StateSize * i + 3]);
  }
  
  //in general the pendulum values should be smaller than at the beginning
  REQUIRE( sum0 > sum00 );
  REQUIRE( sum1 > sum11 );
  REQUIRE( sum2 > sum22 );
  REQUIRE( sum3 > sum33 );
}
