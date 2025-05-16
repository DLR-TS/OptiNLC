/*
* Author: Björn Bahn
* Email: bjoern.bahn@dlr.de
*/

#include <catch2/catch.hpp>
#include "OptiNLC_Data.h"

TEST_CASE("VECTOR Datastructure")
{
    int SIZE = 10;
    VECTOR<double, 10> input = { 1.0, 0.0};
    
    SECTION("Size")
    {
        REQUIRE(input.data()[0] == 1.0);
        REQUIRE(input.data()[1] == 0.0);
        REQUIRE(input.size() == SIZE);
    }
    SECTION("Setter")
    {
        REQUIRE(input.data()[0] == 1.0);
        REQUIRE(input.data()[1] == 0.0);
        input.setConstant(30.0);
        REQUIRE(input.data()[0] == 30.0);
        REQUIRE(input.data()[1] == 30.0);
        REQUIRE(input.data()[SIZE-1] == 30.0);
        input.set(10.0, 1);
        REQUIRE(input.data()[0] == 30.0);
        REQUIRE(input.data()[1] == 10.0);
        REQUIRE(input.data()[SIZE-1] == 30.0);
    }
    SECTION("Vector Setter")
    {
        VECTOR<double, 10> input2 = { 2.0 };
        input.set(input2);
        REQUIRE(input.data()[0] == 2.0);
        REQUIRE(input.data()[1] == 0.0);
        VECTOR<double, 10> input3 = { 2.0 , 2.0, 2.0 };
        VECTOR<double, 10> input4 = { 4.0 };
        input3.set(input4);
        REQUIRE(input3.data()[0] == 4.0);
        REQUIRE(input3.data()[1] > -0.1);
        REQUIRE(input3.data()[1] < 0.1);
    }
    SECTION("Eigen Matrix")
    {
        auto eigenMatrix = input.getDataAsEigen();
        REQUIRE(eigenMatrix.size() == 10);
    }
    SECTION("Printer")
    {
        input.print();
        input.print_T();
        //input.print(0,0);
        REQUIRE(true);
    }
    SECTION("Operators")
    {
        VECTOR<double, 3> vec1 = {1.0, 2.0, 3.0};
        VECTOR<double, 3> vec2 = {4.0, 5.0, 6.0};

        VECTOR<double, 3> resultPlus = vec1 + vec2;
        VECTOR<double, 3> resultMinus = vec1 - vec2;
        REQUIRE(resultPlus[0] == 1 + 4);
        REQUIRE(resultPlus[1] == 2 + 5);
        REQUIRE(resultPlus[2] == 3 + 6);
        REQUIRE(resultMinus[0] == 1 - 4);
        REQUIRE(resultMinus[1] == 2 - 5);
        REQUIRE(resultMinus[2] == 3 - 6);

        VECTOR<double, 3> init = {10.0, 10.0, 10.0};
        init = init + vec1;//why is no += defined?
        REQUIRE(init[0] == 11);
        REQUIRE(init[1] == 12);
        REQUIRE(init[2] == 13);
        init -= vec1;
        REQUIRE(init[0] == 10);
        REQUIRE(init[1] == 10);
        REQUIRE(init[2] == 10);
        init = init * 5;
        REQUIRE(init[0] == 50);
        REQUIRE(init[1] == 50);
        REQUIRE(init[2] == 50);
    }
    SECTION("Methods")
    {
        VECTOR<double, 10> input2 = { 1.0, 0.0, 10.6};
        REQUIRE(input2.sum() == 11.6);
        REQUIRE(input2.norm_inf() == 10.6);
    }
    SECTION("minMAX")
    {
        VECTOR<double, 10> input2 = { 1.0, 0.0, 10.6};
        input2.cwiseMin(5.0);
        REQUIRE(input2[0] == 1.0);
        REQUIRE(input2[2] == 5.0);
        input2.cwiseMax(3.0);
        REQUIRE(input2[0] == 3.0);
        REQUIRE(input2[2] == 5.0);
    }
    SECTION("minMAXVector")
    {
        VECTOR<double, 10> input2 = { 0.0, 10.6};
        input.cwiseMin(input2);
        REQUIRE(input[0] == 0.0);
        REQUIRE(input[1] == 0.0);
        input2.cwiseMax(input);
        REQUIRE(input2[0] == 0.0);
        REQUIRE(input2[1] == 10.6);
    }
    SECTION("norm"){
        //positive
        VECTOR<double, 2> input2 = { 3.0, 4.0};
        REQUIRE(input2.norm(1) == 7.0);
        REQUIRE(input2.norm() == 5.0);
        REQUIRE(input2.norm(2) == 5.0);
        VECTOR<double, 3> input3 = { 3.0, 4.0, 5.0};
        REQUIRE(input3.norm(3) > 5.9);
        REQUIRE(input3.norm(3) < 6.1);

        //zero
        VECTOR<double, 2> input4 = { 0, 0};
        REQUIRE(input4.norm(1) == 0);
        REQUIRE(input4.norm() == 0);
        REQUIRE(input4.norm(2) == 0);
        VECTOR<double, 10> input5 = { 0, 0, 0};
        REQUIRE(input5.norm(3) == 0);

        //negative
        VECTOR<double, 2> input6 = { -3.0, -4.0};
        REQUIRE(input6.norm(1) == 7.0);
        REQUIRE(input6.norm() == 5.0);
        REQUIRE(input6.norm(2) == 5.0);
        VECTOR<double, 3> input7 = { -3, -4, 5};
        REQUIRE(input7.norm(3) > 5.9);
        REQUIRE(input7.norm(3) < 6.1);
    }
}

TEST_CASE("MATRIX Datastructure")
{
    MATRIX<2, 2> matrix1;
    MATRIX<2, 2> matrix2;
    MATRIX<2, 3> illFormedMatrix;
    
    SECTION("Setter")
    {
        REQUIRE(matrix1[0][0] == 0.0);
        REQUIRE(matrix1[0][1] == 0.0);
        REQUIRE(matrix1[1][0] == 0.0);
        REQUIRE(matrix1[1][1] == 0.0);
        matrix1.setConstant(1.0);
        REQUIRE(matrix1[0][0] == 1.0);
        REQUIRE(matrix1[0][1] == 1.0);
        REQUIRE(matrix1[1][0] == 1.0);
        REQUIRE(matrix1[1][1] == 1.0);
        matrix2.set(matrix1);
        REQUIRE(matrix2[0][0] == 1.0);
        REQUIRE(matrix2[0][1] == 1.0);
        REQUIRE(matrix2[1][0] == 1.0);
        REQUIRE(matrix2[1][1] == 1.0);
        matrix2.set(10.0 , 1, 1);
        REQUIRE(matrix2[1][1] == 10.0);

        matrix1.setIdentity();
        illFormedMatrix.setIdentity();
    }
    SECTION("Identitity")
    {
        matrix1.setIdentity();
        REQUIRE(matrix1[0][0] == 1.0);
        REQUIRE(matrix1[0][1] == 0.0);
        REQUIRE(matrix1[1][0] == 0.0);
        REQUIRE(matrix1[1][1] == 1.0);
        illFormedMatrix.setIdentity();
        REQUIRE(illFormedMatrix[0][0] == 0.0);
        REQUIRE(illFormedMatrix[0][1] == 0.0);
        REQUIRE(illFormedMatrix[0][2] == 0.0);
        REQUIRE(illFormedMatrix[1][0] == 0.0);
        REQUIRE(illFormedMatrix[1][1] == 0.0);
        REQUIRE(illFormedMatrix[1][2] == 0.0);
    }
    SECTION("SetColAndRow"){
        VECTOR<double, 2> input = { 1.0, 1.0};
        matrix1.setCol(1, input);
        REQUIRE(matrix1[0][0] == 0.0);
        REQUIRE(matrix1[0][1] == 1.0);
        REQUIRE(matrix1[1][0] == 0.0);
        REQUIRE(matrix1[1][1] == 1.0);
        matrix2.setRow(1, input);
        REQUIRE(matrix2[0][0] == 0.0);
        REQUIRE(matrix2[0][1] == 0.0);
        REQUIRE(matrix2[1][0] == 1.0);
        REQUIRE(matrix2[1][1] == 1.0);

        //handle wrong row/column
        matrix1.setCol(2, input);
        matrix1.setRow(2, input);
        matrix1.print();
    }
    SECTION("Operators"){
        matrix1.setConstant(10.0);
        matrix2.setConstant(5.0);
        MATRIX<2, 2> matrix3;

        matrix3 = matrix2 + matrix1;
        REQUIRE(matrix3[0][0] == 15);
        REQUIRE(matrix3[0][1] == 15);
        REQUIRE(matrix3[1][0] == 15);
        REQUIRE(matrix3[1][1] == 15);
        matrix3 = matrix3 - matrix1;
        REQUIRE(matrix3[0][0] == 5);
        REQUIRE(matrix3[0][1] == 5);
        REQUIRE(matrix3[1][0] == 5);
        REQUIRE(matrix3[1][1] == 5);
        REQUIRE(matrix3 == matrix2);//test of ==
        REQUIRE(matrix3 != matrix1);//test of !=
        matrix1 = matrix1 * 5;
        REQUIRE(matrix1[0][0] == 50);
        REQUIRE(matrix1[0][1] == 50);
        REQUIRE(matrix1[1][0] == 50);
        REQUIRE(matrix1[1][1] == 50);

        VECTOR<double, 2> vec = { 2.0, 5.0};
        VECTOR<double, 2> result = matrix2 * vec;
        REQUIRE(result[0] == 35);
        REQUIRE(result[1] == 35);
        VECTOR<double, 2> vec2 = { 0.0, 0.0};
        result = matrix2 * vec2;
        REQUIRE(result[0] == 0);
        REQUIRE(result[1] == 0);
    }
    SECTION("Getter")
    {
        REQUIRE(matrix1.getNR() == 2);
        REQUIRE(matrix1.getNC() == 2);

        VECTOR<double, 2> input = { 1.0, 9.0};
        matrix1.setCol(1, input);
        matrix2.setRow(1, input);
        VECTOR<double, 2> output1 = matrix1.getColumn(1);
        VECTOR<double, 2> output2 = matrix2.getRow(1);
        REQUIRE(input == output1);
        REQUIRE(input == output2);

        matrix1.setCol(0, input);
        double array[4];
        matrix1.copyToDoubleArray(array);
        REQUIRE(array[0] == 1.0);
        REQUIRE(array[1] == 9.0);
        REQUIRE(array[2] == 1.0);
        REQUIRE(array[3] == 9.0);
    }
    SECTION("Eigen")
    {
        matrix1.setConstant(10);
        Eigen::MatrixXd m = matrix1.ToEigen();
        REQUIRE(m(0,0) == 10);
        REQUIRE(m(0,1) == 10);
        REQUIRE(m(1,0) == 10);
        REQUIRE(m(1,1) == 10);
    }
}
