#include <fstream>
#include <iostream>
#include <algorithm>

#include "OptiNLC_Data.h"
#include "OptiNLC_Options.h"
#include "OptiNLC_Solver.h"
#include <catch2/catch.hpp>

TEST_CASE( "OCP Test NEW FORMAT:" )
{
  std::cout << "OCP Test NEW FORMAT:" << std::endl;

  class TEST
  {
  public:

    TEST() {}

    void
    dummy()
    {}
  };

  OptiNLC_Options options;
  options.setDefaults();

  options.intermediateIntegration = 2;
  options.OptiNLC_ACC             = 1.e-7;
  options.maxNumberOfIteration    = 500;
  options.OSQP_verbose            = false;
  options.OSQP_max_iter           = 400;
  options.OptiNLC_time_limit      = 0.1;

  // options.OSQP_time_limit            = 0.1;
  enum STATES
  {
    X,
    Y,
    PSI,
    V,
    S,
    DELTA,
    dDELTA,
    L
  };

  enum CONTROLS
  {
    ddDELTA,
    F
  };

  // Define the state and input sizes
  constexpr int StateSize       = 8;
  constexpr int InputSize       = 2;
  constexpr int ConstraintsSize = 0;
  constexpr int ControlPoints   = 20;
  double        simTime         = 4.0;
  options.timeStep              = ( simTime / ( ControlPoints ) );

  // Create an instance of the OCP class with the appropriate horizon and time step
  OptiNLC_OCP<double, InputSize, StateSize, ConstraintsSize, ControlPoints> ocp( &options );


  VECTOR<double, StateSize> initialState = { 0.0, 0.0, -0.01, 0.1, 0.0, 0.0, 0.0, 0.0 };
  VECTOR<double, InputSize> initialInput = { 0.00, 0.0 };
  double                    SS[2]        = { 11.5, 15.5 };

  // Define a simple dynamic model as a lambda function
  ocp.setDynamicModel( [&]( const VECTOR<double, StateSize>& state, const VECTOR<double, InputSize>& input,
                            VECTOR<double, StateSize>& derivative, double currentTime, void* userData ) {
    TEST* testPtr = reinterpret_cast<TEST*>( userData );
    if( testPtr )
    {
      testPtr->dummy();
    }
    testPtr->dummy();
    // Populate the derivative vector
    const double l         = 4.2;
    const double tau       = 1.0;
    const double desired_v = 3.60;
    double       k         = desired_v / tau;
    derivative[X]          = state[V] * cos( state[PSI] );
    derivative[Y]          = state[V] * sin( state[PSI] );
    derivative[PSI]        = state[V] * tan( state[DELTA] ) / l;
    derivative[V]          = ( -( 1.0 / tau ) * state[V] ) + ( k * input[F] );
    derivative[S]          = state[V];
    derivative[DELTA]      = state[dDELTA];
    derivative[dDELTA]     = input[ddDELTA];
    derivative[L]          = ( state[V] - 3.250 ) * ( state[V] - 3.250 ) * 0.1 + ( state[Y] - 0.4 ) * ( state[Y] - 0.4 );
  } );

  // Define an objective function as a lambda function, states and input at tf
  ocp.setObjectiveFunction( [&]( const VECTOR<double, StateSize>& state, const VECTOR<double, InputSize>& input, double current_time ) {
    // Define your objective function here
    // For example, you can minimize the squared sum of state variables
    return state[L];
  } );


  // Define a simple input update method
  ocp.setInputUpdate(
    [&]( const VECTOR<double, StateSize>& state, const VECTOR<double, InputSize>& input, double currentTime, void* userData ) {
      VECTOR<double, InputSize> updatedInput = { 0.0, 0.90 };
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
    inputConstraints.setConstant( -1.0 );
    inputConstraints[F] = -2.0;
    return inputConstraints;
  } );
  // Define a inputs constraints method
  ocp.setUpdateInputUpperBounds( [&]( const VECTOR<double, StateSize>& state, const VECTOR<double, InputSize>& input ) {
    VECTOR<double, InputSize> inputConstraints;
    inputConstraints.setConstant( 1.0 );
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
  // user defined function for all constraints


  OptiNLC_Solver<double, InputSize, StateSize, ConstraintsSize, ControlPoints> _solver( ocp );
  _solver.solve( 0.0, initialState, initialInput );
  auto opt_x = _solver.get_optimal_states();
  auto time  = _solver.getTime();
  auto opt_u = _solver.get_optimal_inputs();
  // Open a file for writing

  std::ofstream dataFile( "eigen_data_02.txt" );
  //input shall be one smaller than values
  REQUIRE (opt_x.size() / StateSize == opt_u.size() / InputSize + 1);

  // Check if the file is open
  if( dataFile.is_open() )
  {
    for( int i = 0; i < time.size(); ++i )
    {
      dataFile << time[i] << " " << opt_x[StateSize * i + X] << " " << opt_x[StateSize * i + Y] << " " << opt_x[StateSize * i + PSI] << " "
               << opt_x[StateSize * i + V] << " " << opt_x[StateSize * i + DELTA];
      if (InputSize * i < opt_u.size())
      {
        dataFile << " " << opt_u[InputSize * i] << std::endl;
      } else {
        dataFile << std::endl;
      }
    }
    dataFile.close();
    std::cout << "Data saved to eigen_data_02.txt" << std::endl;
  }
  else
  {
    std::cerr << "Unable to open file for writing." << std::endl;
    REQUIRE( false );
  }


  for( int i = 2; i < time.size() - 1 ; ++i )
  {
    double minDiffPrev = std::abs(std::min(opt_u[InputSize * i - 4] - opt_u[InputSize * i - 2], opt_u[InputSize * i - 2] - opt_u[InputSize * i - 4]));
    double minDiffNow = std::abs(std::min(opt_u[InputSize * i - 2] - opt_u[InputSize * i], opt_u[InputSize * i] - opt_u[InputSize * i - 2]));

    //the optimization shall need an input, that is always smaller than before
    //or the difference shall not getting larger
    //no oscillation shall happen
    REQUIRE((std::abs(opt_u[InputSize * i - 2]) > std::abs(opt_u[InputSize * i]) || minDiffPrev > minDiffNow));
  }
}
