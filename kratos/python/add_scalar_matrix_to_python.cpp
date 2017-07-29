//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Pooyan Dadvand
//                   Riccardo Rossi
//

// System includes

// External includes
#include <boost/python.hpp>
#include <cstring>

// Project includes
#include "includes/define.h"
#include "includes/ublas_interface.h"
#include "python/matrix_python_interface.h"
#include "python/matrix_scalar_operator_python.h"
#include "python/matrix_vector_operator_python.h"

namespace Kratos
{
namespace Python
{

using namespace boost::python;


void  AddScalarMatrixToPython()
{
    /*    	  ReadonlyMatrixPythonInterface<scalar_matrix<double> >::CreateInterface("ScalarMatrix")
    	  .def(init<scalar_matrix<double>::size_type, scalar_matrix<double>::size_type>())
    	  .def(init<scalar_matrix<double>::size_type, scalar_matrix<double>::size_type, scalar_matrix<double>::value_type>())
              		  .def(MatrixScalarOperatorPython<scalar_matrix<double>, double, matrix<double>, matrix<double> >())
    	  .def(MatrixVectorOperatorPython<scalar_matrix<double>, vector<double> >())
    	  .def(MatrixVectorOperatorPython<scalar_matrix<double>, zero_vector<double>, vector<double> >())
    	  .def(MatrixVectorOperatorPython<scalar_matrix<double>, unit_vector<double>, vector<double> >())
    	  .def(MatrixVectorOperatorPython<scalar_matrix<double>, scalar_vector<double>, vector<double> >())
    	  .def(MatrixVectorOperatorPython<scalar_matrix<double>, mapped_vector<double>  >())
    	  .def(MatrixVectorOperatorPython<scalar_matrix<double>, compressed_vector<double> >())
    	  .def(MatrixVectorOperatorPython<scalar_matrix<double>, coordinate_vector<double> >())
              .def(MatrixMatrixOperatorPython<scalar_matrix<double>, zero_matrix<double>, matrix<double> >())
              .def(MatrixMatrixOperatorPython<scalar_matrix<double>, identity_matrix<double>, matrix<double> >())
              .def(MatrixMatrixOperatorPython<scalar_matrix<double>, scalar_matrix<double>, matrix<double> >())
              .def(MatrixMatrixOperatorPython<scalar_matrix<double>, matrix<double>, matrix<double> >())
              .def(MatrixMatrixOperatorPython<scalar_matrix<double>, banded_matrix<double>, matrix<double> >())
              .def(MatrixMatrixOperatorPython<scalar_matrix<double>, triangular_matrix<double, upper>, matrix<double> >())
              .def(MatrixMatrixOperatorPython<scalar_matrix<double>, triangular_matrix<double, lower>, matrix<double> >())
              .def(MatrixMatrixOperatorPython<scalar_matrix<double>, symmetric_matrix<double, upper>, matrix<double> >())
    #if defined KRATOS_ADD_HERMITIAN_MATRIX_INTERFACE
              .def(MatrixMatrixOperatorPython<scalar_matrix<double>, hermitian_matrix<double, upper>, matrix<double> >())
    #endif
              .def(MatrixMatrixOperatorPython<scalar_matrix<double>, mapped_matrix<double>, mapped_matrix<double> >())
              .def(MatrixMatrixOperatorPython<scalar_matrix<double>, compressed_matrix<double>, compressed_matrix<double> >())
    #if defined KRATOS_ADD_COORDINATE_MATRIX_INTERFACE
              .def(MatrixMatrixOperatorPython<scalar_matrix<double>, coordinate_matrix<double>, coordinate_matrix<double> >())
    #endif
    	  ;
    */
}

}  // namespace Python.

} // Namespace Kratos

