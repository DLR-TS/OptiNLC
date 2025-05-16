/*
* Author: Björn Bahn
* Email: bjoern.bahn@dlr.de
*/

#include <catch2/catch.hpp>
#include "OptiNLC_Options.h"

TEST_CASE("Options")
{
    OptiNLC_Options opt1;
    OptiNLC_Options opt2;
    opt1.setDefaults();

    REQUIRE(opt1.timeStep == opt2.timeStep);
    REQUIRE(opt1.intermediateIntegration == opt2.intermediateIntegration);
    REQUIRE(opt1.perturbation == opt2.perturbation);
    REQUIRE(opt1.convergenceThreshold == opt2.convergenceThreshold);
    REQUIRE(opt1.accuracy == opt2.accuracy);
    REQUIRE(opt1.timeLimit == opt2.timeLimit);
    REQUIRE(opt1.maxNumberOfIteration == opt2.maxNumberOfIteration);
    REQUIRE(opt1.OSQP_verbose == opt2.OSQP_verbose);
    REQUIRE(opt1.OSQP_warm_starting == opt2.OSQP_warm_starting);
    REQUIRE(opt1.OSQP_max_iter == opt2.OSQP_max_iter);
    REQUIRE(opt1.OSQP_time_limit == opt2.OSQP_time_limit);
    REQUIRE(opt1.OSQP_time_limit == opt2.OSQP_time_limit);
    REQUIRE(opt1.OSQP_eps_abs == opt2.OSQP_eps_abs);
    REQUIRE(opt1.OSQP_eps_rel == opt2.OSQP_eps_rel);
    REQUIRE(opt1.SQP_ACC == opt2.SQP_ACC);
    REQUIRE(opt1.OptiNLC_ACC == opt2.OptiNLC_ACC);
    REQUIRE(opt1.OptiNLC_time_limit == opt2.OptiNLC_time_limit);
    REQUIRE(opt1.debugPrint == opt2.debugPrint);
}