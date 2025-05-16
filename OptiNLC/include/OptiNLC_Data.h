/*
 * OptiNLC_Data.h
 *
 * This file defines the data structures for the OptiNLC toolbox, which is specialized for solving nonlinear optimal control problems using
 * the line search Sequential Quadratic Programming (SQP) method.
 *
 * Author: Reza Dariani
 * Email: reza.dariani@dlr.de
 */

#pragma once

#include <cstring> // For std::memcpy
#include <iomanip>
#include <iostream>

#include <Eigen/Dense>

// Forward declare MATRIX
template<int Rows, int Cols>
struct MATRIX;

template<typename Scalar, int Size,
typename = std::enable_if_t<std::is_arithmetic_v<Scalar>>>
struct VECTOR
{
  Scalar vector[Size];

  Scalar*
  data()
  {
    return vector;
  } // This will provide access to the underlying data

  const Scalar*
  data() const
  {
    return vector;
  } // Const version

  VECTOR()
  {
    for( int i = 0; i < Size; i++ )
    {
      vector[i] = 0.0;
    }
  }

  VECTOR( std::initializer_list<Scalar> values )
  {
    int i = 0;
    for( auto it = values.begin(); it != values.end(); ++it )
    {
      if( i >= Size )
        break;
      vector[i++] = *it;
    }
  }

  int
  size() const
  {
    return Size;
  }

  void
  clear()
  {
    for( int i = 0; i < Size; i++ )
    {
      vector[i] = 0.0;
    }
  }

  void
  setConstant( const double value )
  {
    for( int i = 0; i < Size; i++ )
    {
      vector[i] = value;
    }
  }

  void
  set( const VECTOR& other )
  {
    std::memcpy( vector, other.vector, Size * sizeof( double ) );
  }

  void
  set( const double* values )
  {
    for( int i = 0; i < Size; i++ )
    {
      vector[i] = values[i];
    }
  }

  void
  set( const double value, int index )
  {
    vector[index] = value;
  }

  template<int SubSize>
  void
  setSubVector( const VECTOR<Scalar, SubSize>& subVector, int startIndex )
  {
    if( startIndex < 0 || startIndex + SubSize > Size )
    {
      std::cout << "Invalid start index or subvector size!" << std::endl;
      return;
    }

    std::memcpy( &vector[startIndex], &subVector[0], subVector.size() * sizeof( Scalar ) );
  }

  Eigen::Matrix<Scalar, Size, 1>
  getDataAsEigen() const
  {
    Eigen::Matrix<Scalar, Size, 1> eigenMatrix;
    for( int i = 0; i < Size; ++i )
    {
      eigenMatrix( i ) = ( vector[i] );
    }
    return eigenMatrix;
  }

  void
  getDataAsEigen( Eigen::Matrix<Scalar, Size, 1>& eigenMatrix ) const
  {
    for( int i = 0; i < Size; ++i )
    {
      eigenMatrix( i ) = ( vector[i] );
    }
  }

  void
  print() const
  {
    int textWidth = 15;
    std::cerr << std::fixed << std::defaultfloat << std::left;
    for( int i = 0; i < Size; ++i )
    {
      std::cerr << std::setw( textWidth ) << vector[i];
    }
    std::cerr << std::endl;
  }

  void
  print_T() const
  {
    int textWidth = 15;
    std::cerr << std::fixed << std::defaultfloat << std::left;
    for( int i = 0; i < Size; ++i )
    {
      std::cerr << std::setw( textWidth ) << vector[i];
      std::cerr << std::endl;
    }
  }

  void
  print( int row, int column ) const
  {
    int textWidth = 15;
    std::cout << std::fixed << std::defaultfloat << std::left;

    if( row * column != Size )
    {
      std::cout << "Invalid row and column configuration for the given vector size!" << std::endl;
      return;
    }

    for( int i = 0; i < row; ++i )
    {
      for( int j = 0; j < column; ++j )
      {
        std::cout << std::setw( textWidth ) << vector[i * column + j];
      }
      std::cout << std::endl;
    }
  }

  // Overloading the assignment operator '='
  VECTOR&
  operator=( const VECTOR& other )
  {
    if( this != &other )
    {
      std::memcpy( vector, other.vector, Size * sizeof( Scalar ) );
    }
    return *this;
  }

  Scalar&
  operator[]( int index )
  {
    return vector[index];
  }

  const Scalar&
  operator[]( int index ) const
  {
    return vector[index];
  }

  friend VECTOR<Scalar, Size>
  operator+( const VECTOR<Scalar, Size>& vec1, const VECTOR<Scalar, Size>& vec2 )
  {
    VECTOR<Scalar, Size> result;
    for( int i = 0; i < Size; ++i )
    {
      result[i] = vec1[i] + vec2[i];
    }
    return result;
  }

  friend VECTOR<Scalar, Size>
  operator-( const VECTOR<Scalar, Size>& vec1, const VECTOR<Scalar, Size>& vec2 )
  {
    VECTOR<Scalar, Size> result;
    for( int i = 0; i < Size; ++i )
    {
      result[i] = vec1[i] - vec2[i];
    }
    return result;
  }

  VECTOR<Scalar, Size>&
  operator-=( const VECTOR<Scalar, Size>& rhs )
  {
    for( int i = 0; i < Size; ++i )
    {
      ( *this )[i] -= rhs[i]; // Assuming operator[] is defined
    }
    return *this;
  }

  // Scalar multiplication
  friend VECTOR<Scalar, Size>
  operator*( const VECTOR<Scalar, Size>& vec, const Scalar& scalar )
  {
    VECTOR<Scalar, Size> result;
    for( int i = 0; i < Size; ++i )
    {
      result[i] = vec[i] * scalar;
    }
    return result;
  }

  // Scalar multiplication (reverse order)
  friend VECTOR<Scalar, Size>
  operator*( const Scalar& scalar, const VECTOR<Scalar, Size>& vec )
  {
    return vec * scalar;
  }

  Scalar
  dotProduct( const VECTOR<Scalar, Size>& vec1 ) const
  {
    Scalar result = 0;
    for( int i = 0; i < Size; ++i )
    {
      result += this->vector[i] * vec1.vector[i];
    }
    return result;
  }

  Scalar
  maxCoeff() const
  {
    if( Size == 0 )
    {
      std::cout << "Error: Vector has zero size." << std::endl;
      return Scalar( 0 ); // Return a default value or handle the error as needed
    }

    Scalar maxVal = vector[0];
    for( int i = 1; i < Size; ++i )
    {
      if( vector[i] > maxVal )
      {
        maxVal = vector[i];
      }
    }
    return maxVal;
  }

  // Norms calculation
  Scalar
  norm( int p = 2 ) const
  {
    Scalar result = 0;

    if( p == 1 )
    { // First norm (Manhattan norm)
      for( int i = 0; i < Size; ++i )
      {
        result += std::abs( vector[i] );
      }
    }
    else if( p == 2 )
    { // Second norm (Euclidean norm)
      for( int i = 0; i < Size; ++i )
      {
        result += vector[i] * vector[i];
      }
      result = std::sqrt( result );
    }
    else
    { // General p-norm
      for( int i = 0; i < Size; ++i )
      {
        result += std::pow( std::abs( vector[i] ), p );
      }
      result = std::pow( result, 1.0 / p );
    }

    return result;
  }

  Scalar
  norm_inf() const
  {
    Scalar result = 0;
    for( int i = 0; i < Size; ++i )
    {
      result = std::max( result, std::abs( vector[i] ) );
    }
    return result;
  }

  // Element-wise maximum operation with a scalar value
  void
  cwiseMax( const Scalar& value )
  {
    for( int i = 0; i < Size; ++i )
    {
      vector[i] = std::max( vector[i], value );
    }
  }

  // Element-wise maximum operation with another vector
  void
  cwiseMax( const VECTOR<Scalar, Size>& other )
  {
    for( int i = 0; i < Size; ++i )
    {
      vector[i] = std::max( vector[i], other[i] );
    }
  }

  // Element-wise minimum operation with a scalar value
  void
  cwiseMin( const Scalar& value )
  {
    for( int i = 0; i < Size; ++i )
    {
      vector[i] = std::min( vector[i], value );
    }
  }

  // Element-wise minimum operation with another vector
  void
  cwiseMin( const VECTOR<Scalar, Size>& other )
  {
    for( int i = 0; i < Size; ++i )
    {
      vector[i] = std::min( vector[i], other[i] );
    }
  }

  // Vector division by a scalar
  friend VECTOR<Scalar, Size>
  operator/( const VECTOR<Scalar, Size>& vec, const Scalar& scalar )
  {
    VECTOR<Scalar, Size> result;
    for( int i = 0; i < Size; ++i )
    {
      result[i] = vec[i] / scalar;
    }
    return result;
  }

  // Vector division by another vector
  friend VECTOR<Scalar, Size>
  operator/( const VECTOR<Scalar, Size>& vec1, const VECTOR<Scalar, Size>& vec2 )
  {
    VECTOR<Scalar, Size> result;
    for( int i = 0; i < Size; ++i )
    {
      result[i] = vec1[i] / vec2[i];
    }
    return result;
  }

  // Equality comparison operator
  bool
  operator==( const VECTOR<Scalar, Size>& other ) const
  {
    for( int i = 0; i < Size; ++i )
    {
      if( vector[i] != other[i] )
      {
        return false;
      }
    }
    return true;
  }

  // Inequality comparison operator
  bool
  operator!=( const VECTOR<Scalar, Size>& other ) const
  {
    return !( *this == other );
  }

  // Less than comparison operator (Compare magnitudes)
  bool
  operator<( const VECTOR<Scalar, Size>& other ) const
  {
    return this->norm() < other.norm();
  }

  // Greater than comparison operator (Compare magnitudes)
  bool
  operator>( const VECTOR<Scalar, Size>& other ) const
  {
    return this->norm() > other.norm();
  }

  // Less than or equal to comparison operator (Compare magnitudes)
  bool
  operator<=( const VECTOR<Scalar, Size>& other ) const
  {
    return this->norm() <= other.norm();
  }

  // Greater than or equal to comparison operator (Compare magnitudes)
  bool
  operator>=( const VECTOR<Scalar, Size>& other ) const
  {
    return this->norm() >= other.norm();
  }

  double
  sum()
  {
    double result = 0.0;
    for( int i = 0; i < Size; i++ )
    {
      result += vector[i];
    }
    return result;
  }

  template<int Rows, int Cols>
  VECTOR<double, Cols>
  operator*( const MATRIX<Rows, Cols>& matrix ) const
  {
    VECTOR<double, Cols> resultVector;
    for( int i = 0; i < Rows; ++i )
    {
      double sum = 0.0;
      for( int j = 0; j < Cols; ++j )
      {
        sum += this->vector[j] * matrix.matrix[i][j];
      }
      resultVector[i] = sum;
    }
    return resultVector;
  }

  // only for square matrix
  VECTOR<double, Size>
  operator*( const Eigen::MatrixXd& eigenMatrix ) const
  {
    VECTOR<double, Size> result;
    for( int i = 0; i < Size; ++i )
    {
      Scalar sum = Scalar( 0 );
      for( int j = 0; j < Size; ++j )
      {
        sum += eigenMatrix( j, i ) * vector[j];
      }
      result[i] = sum;
    }
    return result;
  }
};

template<typename Scalar, int SizeVector, int SizeSequence>
struct SEQUENCE
{
  VECTOR<Scalar, SizeVector> vector;
  VECTOR<Scalar, SizeVector> sequence[SizeSequence];

  SEQUENCE()
  {
    vector.clear();
    for( int i = 0; i < SizeSequence; ++i )
    {
      sequence[i].clear(); // Initialize each vector in the sequence to zero
    }
  }

  void
  clear()
  {
    vector.clear();
    for( int i = 0; i < SizeSequence; ++i )
    {
      sequence[i].clear();
    }
  }

  void
  set( const VECTOR<Scalar, SizeVector>& data, int index )
  {
    sequence[index].set( data );
  }

  void
  set( const VECTOR<Scalar, SizeVector> data[SizeSequence] )
  {
    for( int i = 0; i < SizeSequence; ++i )
    {
      sequence[i].set( data[i] );
    }
  }

  void
  setColumn( const VECTOR<Scalar, SizeVector>& columnData, int columnIndex )
  {
    // if (columnIndex < 0 || columnIndex >= SizeVector) {
    //     std::cout << "Invalid column index!" << std::endl;
    //    return;
    // }

    for( int i = 0; i < SizeVector; ++i )
    {
      sequence[i][columnIndex] = columnData[i];
    }
  }

  VECTOR<Scalar, SizeVector>
  get( int index )
  {
    return sequence[index];
  }

  VECTOR<Scalar, SizeVector>
  get()
  {
    return sequence[0]; // Return the first vector for simplicity
  }

  void
  print() const
  {
    std::cout << "\nSequence:\n";
    for( int i = 0; i < SizeSequence; ++i )
    {
      sequence[i].print();
    }
  }

  VECTOR<Scalar, SizeVector * SizeSequence>
  convertToVector()
  {
    VECTOR<Scalar, SizeVector * SizeSequence> result;
    int                                       index = 0;
    for( int i = 0; i < SizeSequence; ++i )
    {
      for( int j = 0; j < SizeVector; ++j )
      {
        result[index++] = sequence[i][j];
      }
    }
    return result;
  }

  void
  identity( double factor = 1.0 )
  {
    if( SizeVector == SizeSequence )
    {
      for( int i = 0; i < SizeSequence; ++i )
      {
        sequence[i].clear();
        sequence[i][i] = 1.0 * factor; // Set diagonal elements to 1.0 for identity matrix
      }
    }
    else
    {
      std::cout << "Sequence is not square; cannot create an identity matrix." << std::endl;
    }
  }

  SEQUENCE<Scalar, SizeVector, SizeSequence>
  operator+( const SEQUENCE<Scalar, SizeVector, SizeSequence>& other ) const
  {
    SEQUENCE<Scalar, SizeVector, SizeSequence> result;

    for( int i = 0; i < SizeSequence; ++i )
    {
      for( int j = 0; j < SizeVector; ++j )
      {
        result.sequence[i][j] = sequence[i][j] + other.sequence[i][j];
      }
    }

    return result;
  }

  SEQUENCE<Scalar, SizeVector, SizeSequence>
  operator-( const SEQUENCE<Scalar, SizeVector, SizeSequence>& other ) const
  {
    SEQUENCE<Scalar, SizeVector, SizeSequence> result;

    for( int i = 0; i < SizeSequence; ++i )
    {
      for( int j = 0; j < SizeVector; ++j )
      {
        result.sequence[i][j] = sequence[i][j] - other.sequence[i][j];
      }
    }

    return result;
  }

  int
  getSizeSequence() const
  {
    return SizeSequence;
  }

  int
  getSizeVector() const
  {
    return SizeVector;
  }
};

template<int Rows, int Cols>
struct MATRIX
{
  double matrix[Rows][Cols];

  MATRIX()
  {
    for( int i = 0; i < Rows; ++i )
    {
      for( int j = 0; j < Cols; ++j )
      {
        matrix[i][j] = 0.0;
      }
    }
  }

  void
  setConstant( const double value )
  {
    for( int i = 0; i < Rows; ++i )
    {
      for( int j = 0; j < Cols; ++j )
      {
        matrix[i][j] = value;
      }
    }
  }

  void
  set( const MATRIX& other )
  {
    for( int i = 0; i < Rows; ++i )
    {
      for( int j = 0; j < Cols; ++j )
      {
        matrix[i][j] = other.matrix[i][j];
      }
    }
  }

  void
  set( const double value, int row, int col )
  {
    matrix[row][col] = value;
  }

  void
  setIdentity( double factor = 1.0 )
  {
    if( Rows != Cols )
    {
      std::cout << "Error: Identity matrix only defined for square matrices!" << std::endl;
      return;
    }

    for( int i = 0; i < Rows; ++i )
    {
      for( int j = 0; j < Cols; ++j )
      {
        if( i == j )
        {
          matrix[i][j] = 1.0 * factor;
        }
        else
        {
          matrix[i][j] = 0.0;
        }
      }
    }
  }

  void
  setCol( int colIndex, const VECTOR<double, Rows>& colData )
  {
    if( colIndex < 0 || colIndex >= Cols )
    {
      std::cout << "Error: Invalid column index!" << std::endl;
      return;
    }

    for( int i = 0; i < Rows; ++i )
    {
      matrix[i][colIndex] = colData[i];
    }
  }

  void
  setRow( int rowIndex, const VECTOR<double, Cols>& rowVector )
  {
    if( rowIndex >= 0 && rowIndex < Rows )
    {
      for( int i = 0; i < Cols; ++i )
      {
        matrix[rowIndex][i] = rowVector[i];
      }
    }
    else
    {
      std::cout << "Invalid row index." << std::endl;
    }
  }

  void
  print() const
  {
    int textWidth = 15;
    std::cout << std::fixed << std::defaultfloat << std::left;
    for( int i = 0; i < Rows; ++i )
    {
      for( int j = 0; j < Cols; ++j )
      {
        std::cout << std::setw( textWidth ) << matrix[i][j] << " ";
      }
      std::cout << std::endl;
    }
  }

  MATRIX<Rows, Cols>
  operator+( const MATRIX<Rows, Cols>& other ) const
  {
    MATRIX<Rows, Cols> result;
    for( int i = 0; i < Rows; ++i )
    {
      for( int j = 0; j < Cols; ++j )
      {
        result.matrix[i][j] = matrix[i][j] + other.matrix[i][j];
      }
    }
    return result;
  }

  MATRIX<Rows, Cols>
  operator-( const MATRIX<Rows, Cols>& other ) const
  {
    MATRIX<Rows, Cols> result;
    for( int i = 0; i < Rows; ++i )
    {
      for( int j = 0; j < Cols; ++j )
      {
        result.matrix[i][j] = matrix[i][j] - other.matrix[i][j];
      }
    }
    return result;
  }

  MATRIX<Rows, Cols>
  operator*( const double& scalar ) const
  {
    MATRIX<Rows, Cols> result;
    for( int i = 0; i < Rows; ++i )
    {
      for( int j = 0; j < Cols; ++j )
      {
        result.matrix[i][j] = matrix[i][j] * scalar;
      }
    }
    return result;
  }

  bool
  operator==( const MATRIX<Rows, Cols>& other ) const
  {
    for( int i = 0; i < Rows; ++i )
    {
      for( int j = 0; j < Cols; ++j )
      {
        if( matrix[i][j] != other.matrix[i][j] )
        {
          return false;
        }
      }
    }
    return true;
  }

  double*
  operator[]( int index )
  {
    return matrix[index];
  }

  const double*
  operator[]( int index ) const
  {
    return matrix[index]; // This returns a const pointer, so the data cannot be modified
  }

  bool
  operator!=( const MATRIX<Rows, Cols>& other ) const
  {
    return !( *this == other );
  }

  int
  getNR() const
  {
    return Rows;
  }

  int
  getNC() const
  {
    return Cols;
  }

  // Operator to multiply a matrix by a vector
  VECTOR<double, Rows>
  operator*( const VECTOR<double, Cols>& vec ) const
  {
    VECTOR<double, Rows> resultVector;

    for( int i = 0; i < Rows; ++i )
    {
      double sum = 0.0;
      for( int j = 0; j < Cols; ++j )
      {
        sum += matrix[i][j] * vec[j];
      }
      resultVector[i] = sum;
    }

    return resultVector;
  }

  VECTOR<double, Rows>
  getColumn( int colIndex ) const
  {
    VECTOR<double, Rows> columnVector;
    for( int i = 0; i < Rows; ++i )
    {
      columnVector[i] = matrix[i][colIndex];
    }
    return columnVector;
  }

  VECTOR<double, Cols>
  getRow( int rowIndex ) const
  {
    VECTOR<double, Cols> rowVector;
    for( int i = 0; i < Cols; ++i )
    {
      rowVector[i] = matrix[rowIndex][i];
    }
    return rowVector;
  }

  void
  copyToDoubleArray( double array[Rows * Cols] ) const
  {
    int index = 0;
    for( int i = 0; i < Cols; ++i )
    {
      for( int j = 0; j < Rows; ++j )
      {
        array[index] = matrix[j][i];
        index++;
      }
    }
  }

  Eigen::MatrixXd
  ToEigen() const
  {
    Eigen::MatrixXd eigenMatrix( Rows, Cols );
    eigenMatrix.setZero(); // Set all values to zero

    for( int i = 0; i < Rows; ++i )
    {
      for( int j = 0; j < Cols; ++j )
      {
        eigenMatrix( i, j ) = matrix[i][j];
      }
    }
    return eigenMatrix;
  }

  // Define a structure for a sparse matrix representation within the MATRIX structure
  struct SparseMatrix
  {
    std::vector<double>        values;       // Store non-zero values
    std::vector<long long int> innerIndices; // Store inner indices
    std::vector<long long int> outerIndices; // Store outer indices
    int                        innerSize;    // Inner size of the sparse matrix
    int                        otterSize;    // Inner size of the sparse matrix
    int                        nonZeros;     // Number of non-zero elements

    // Function to convert the dense matrix to a sparse representation
    void
    createSparseView( const MATRIX<Rows, Cols>& denseMatrix )
    {
      // Iterate through the dense matrix
      for( int i = 0; i < Cols; ++i )
      {
        outerIndices.push_back( values.size() );
        for( int j = 0; j < Rows; ++j )
        {
          if( denseMatrix[j][i] != 0.00 )
          {
            values.push_back( denseMatrix[j][i] );
            innerIndices.push_back( j );
          }
        }
      }
      outerIndices.push_back( values.size() ); // The end of the last column
      nonZeros  = values.size();
      innerSize = Rows;
      otterSize = Cols;
    }

    // Function to retrieve the number of non-zero elements
    int
    getNonZeros() const
    {
      return nonZeros;
    }

    // Function to retrieve the inner size of the sparse matrix
    int
    getInnerSize() const
    {
      return innerSize;
    }

    // Function to retrieve the inner size of the sparse matrix
    int
    getOutterSize() const
    {
      return otterSize;
    }

    // Functions to return pointers to values, inner indices, and outer indices
    double*
    valuePtr()
    {
      return values.data();
    }

    long long int*
    innerIndexPtr()
    {
      return innerIndices.data();
    }

    long long int*
    outerIndexPtr()
    {
      return outerIndices.data();
    }
  };

  SparseMatrix sparse; // Instance of the SparseMatrix structure

  // Function to get the sparse view of the matrix
  void
  getSparsity()
  {
    sparse.createSparseView( *this );
  }
};
