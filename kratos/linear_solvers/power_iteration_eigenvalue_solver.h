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
//  Collaborator:    Vicente Mataix Ferrandiz
//
//

#if !defined(KRATOS_POWER_ITERATION_EIGENVALUE_SOLVER_H_INCLUDED )
#define  KRATOS_POWER_ITERATION_EIGENVALUE_SOLVER_H_INCLUDED

// System includes
#include <string>
#include <iostream>
#include <numeric>
#include <vector>
#include <random>
#include <boost/range/algorithm.hpp>

// External includes

// Project includes
#include "spaces/ublas_space.h"
#include "includes/ublas_interface.h"
#include "processes/process.h"
#include "includes/define.h"
#include "linear_solvers/iterative_solver.h"

namespace Kratos
{

///@name Kratos Globals
///@{

///@}
///@name Type Definitions
///@{

///@}
///@name  Enum's
///@{

///@}
///@name  Functions
///@{

///@}
///@name Kratos Classes
///@{
    
/// Utility to initialize a random vector
/**
 * Defines several utility functions
 */
template<class TDataType>
class RandomInitializeUtil
{
public:

    ///@name Type Definitions
    ///@{

    typedef UblasSpace<TDataType, CompressedMatrix, Vector> SparseSpaceType;
    
    typedef UblasSpace<TDataType, Matrix, Vector> LocalSpaceType;
    
    typedef typename SparseSpaceType::MatrixType SparseMatrixType;

    typedef typename SparseSpaceType::VectorType VectorType;

    typedef typename LocalSpaceType::MatrixType DenseMatrixType;

    typedef typename LocalSpaceType::VectorType DenseVectorType;
    
    typedef std::size_t SizeType;

    ///@}
    ///@name Life Cycle
    ///@{

    ///@}
    ///@name Operators
    ///@{


    ///@}
    ///@name Operations
    ///@{
    
    static inline void RandomInitialize(
        const SparseMatrixType& K,
        DenseVectorType& R,
        const bool Inverse = false 
        )
    {
        // We create a random vector as seed of the method
        std::random_device this_random_device;
        std::mt19937 generator(this_random_device());
        
        const SizeType size = K.size1();
        const TDataType normK = SparseSpaceType::TwoNorm(K);
        const TDataType aux_value = (Inverse == false) ? normK : 1.0/normK;
        std::normal_distribution<> normal_distribution(aux_value, 0.25 * aux_value);
        
        for (SizeType i = 0; i < size; i++)
        {
            R[i] = normal_distribution(generator);
        }
    }
    
    ///@}
    ///@name Access
    ///@{


    ///@}
    ///@name Inquiry
    ///@{


    ///@}
    ///@name Input and output
    ///@{

    ///@}
    ///@name Friends
    ///@{

private:
    
    ///@name Private static Member Variables
    ///@{

    ///@}
    ///@name Private member Variables
    ///@{

    ///@}
    ///@name Private Operators
    ///@{

    ///@}
    ///@name Private Operations
    ///@{
    
    ///@}
    ///@name Private  Access
    ///@{

    ///@}
    ///@name Private Inquiry
    ///@{

    ///@}
    ///@name Private LifeCycle
    ///@{

    ///@}
    ///@name Unaccessible methods
    ///@{

    RandomInitializeUtil(void);

    RandomInitializeUtil(RandomInitializeUtil& rSource);

}; /* Class RandomInitializeUtil */

/// This class uses the inverted power iteration method to obtain the lowest eigenvalue of a system
/** Basically that
*/
template<class TSparseSpaceType, class TDenseSpaceType, class TLinearSolverType,
         class TPreconditionerType = Preconditioner<TSparseSpaceType, TDenseSpaceType>,
         class TReordererType = Reorderer<TSparseSpaceType, TDenseSpaceType> >
class PowerIterationEigenvalueSolver : public IterativeSolver<TSparseSpaceType, TDenseSpaceType, TPreconditionerType, TReordererType>
{
public:
    ///@name Type Definitions
    ///@{

    /// Pointer definition of PowerIterationEigenvalueSolver
    KRATOS_CLASS_POINTER_DEFINITION(PowerIterationEigenvalueSolver);

    typedef IterativeSolver<TSparseSpaceType, TDenseSpaceType, TPreconditionerType, TReordererType> BaseType;

    typedef typename TSparseSpaceType::MatrixType SparseMatrixType;

    typedef typename TSparseSpaceType::VectorType VectorType;

    typedef typename TDenseSpaceType::MatrixType DenseMatrixType;

    typedef typename TDenseSpaceType::VectorType DenseVectorType;

    typedef std::size_t SizeType;

    typedef std::size_t IndexType;

    ///@}
    ///@name Life Cycle
    ///@{

    /// Default constructor.
    PowerIterationEigenvalueSolver() {}

    PowerIterationEigenvalueSolver(
        double MaxTolerance,
        unsigned int MaxIterationNumber,
        unsigned int RequiredEigenvalueNumber,
        typename TLinearSolverType::Pointer pLinearSolver
    ): BaseType(MaxTolerance, MaxIterationNumber),   
       mRequiredEigenvalueNumber(RequiredEigenvalueNumber),
       mpLinearSolver(pLinearSolver)
    {

    }

    PowerIterationEigenvalueSolver(
        Parameters ThisParameters,
        typename TLinearSolverType::Pointer pLinearSolver
        ): mpLinearSolver(pLinearSolver)
    {
        Parameters DefaultParameters = Parameters(R"(
        {
            "solver_type"             : "PowerIterationEigenvalueSolver",
            "max_iteration"           : 500,
            "tolerance"               : 1e-9,
            "required_eigen_number"   : 1,
            "shifting_convergence"    : 0.25,
            "verbosity"               : 1
        })" );

        ThisParameters.ValidateAndAssignDefaults(DefaultParameters);

        mRequiredEigenvalueNumber = ThisParameters["required_eigen_number"].GetInt();
        mEchoLevel = ThisParameters["verbosity"].GetInt();
        BaseType::SetTolerance( ThisParameters["tolerance"].GetDouble() );
        BaseType::SetMaxIterationsNumber( ThisParameters["max_iteration"].GetInt() );
    }

    /// Copy constructor.
    PowerIterationEigenvalueSolver(const PowerIterationEigenvalueSolver& Other) : BaseType(Other)
    {

    }


    /// Destructor.
    ~PowerIterationEigenvalueSolver() override
    {

    }


    ///@}
    ///@name Operators
    ///@{

    /// Assignment operator.
    PowerIterationEigenvalueSolver& operator=(const PowerIterationEigenvalueSolver& Other)
    {
        BaseType::operator=(Other);
        return *this;
    }

    ///@}
    ///@name Operations
    ///@{

    /**
     * The power iteration algorithm
     * @param K: The stiffness matrix
     * @param M: The mass matrix
     * @param Eigenvalues: The vector containing the eigen values
     * @param Eigenvectors: The matrix containing the eigen vectors
     */
    void Solve(
        SparseMatrixType& K,
        SparseMatrixType& M,
        DenseVectorType& Eigenvalues,
        DenseMatrixType& Eigenvectors
        ) override
    {

        using boost::numeric::ublas::trans;

        const SizeType size = K.size1();
        const SizeType max_iteration = BaseType::GetMaxIterationsNumber();
        const double tolerance = BaseType::GetTolerance();

        VectorType x = ZeroVector(size);
        VectorType y = ZeroVector(size);

        RandomInitializeUtil<double>::RandomInitialize(K, y);

        if(Eigenvalues.size() < 1)
        {
            Eigenvalues.resize(1, 0.0);
        }

        // Starting with first step
        double beta = 0.0;
        double ro = 0.0;
        double old_ro = Eigenvalues[0];
        VectorType y_old = ZeroVector(size);

        if (mEchoLevel > 1)
        {
            std::cout << "Iteration  beta \t\t ro \t\t convergence norm" << std::endl;
        }

        for(SizeType i = 0 ; i < max_iteration ; i++)
        {
            // K*x = y
            mpLinearSolver->Solve(K, x, y);
            
            ro = inner_prod(y, x);
            
            // y = M*x
            TSparseSpaceType::Mult(M, x, y);
            beta = inner_prod(x, y);
            
            if(beta <= 0.0)
            {
                KRATOS_ERROR << "M is not Positive-definite. beta = " << beta << std::endl;
            }

            ro /= beta;
            beta = std::sqrt(beta);
            TSparseSpaceType::InplaceMult(y, 1.0/beta);

            if(ro == 0.0)
            {
                KRATOS_ERROR << "Perpendicular eigenvector to M" << std::endl;
            }

            const double convergence_ro = std::abs((ro - old_ro) / ro);
            const double convergence_norm = TSparseSpaceType::TwoNorm(y - y_old)/TSparseSpaceType::TwoNorm(y);

            if (mEchoLevel > 1)
            {
                std::cout << "Iteration: " << i << " \t beta: " << beta << "\tro: " << ro << " \tConvergence norm: " << convergence_norm << " \tConvergence ro: " << convergence_ro << std::endl;
            }
            
            if(convergence_norm < tolerance || convergence_ro < tolerance)
            {
                break;
            }

            old_ro = ro;
            TSparseSpaceType::Assign(y_old, 1.0, y);
        }

        if (mEchoLevel > 0)
        {
            KRATOS_WATCH(ro);
            KRATOS_WATCH(y);
        }

        Eigenvalues[0] = ro;

        if((Eigenvectors.size1() < 1) || (Eigenvectors.size2() < size))
        {
            Eigenvectors.resize(1,size);
        }

        for(SizeType i = 0 ; i < size ; i++)
        {
            Eigenvectors(0,i) = y[i];
        }
    }
    
    /**
     * This method returns directly the first eigen value obtained
     * @param K: The stiffness matrix
     * @param M: The mass matrix
     * @return The first eigenvalue
     */
    double GetEigenValue(
        SparseMatrixType& K,
        SparseMatrixType& M
        )
    {
        DenseVectorType eigen_values;
        DenseMatrixType eigen_vectors;
        
        Solve(K, M, eigen_values, eigen_vectors);
        
        return eigen_values[0];
    }
    
    ///@}
    ///@name Access
    ///@{


    ///@}
    ///@name Inquiry
    ///@{


    ///@}
    ///@name Input and output
    ///@{

    /// Turn back information as a string.
    std::string Info() const override
    {
        std::stringstream buffer;
        buffer << "Power iteration eigenvalue solver with " << BaseType::GetPreconditioner()->Info();
        return  buffer.str();
    }

    /// Print information about this object.
    void PrintInfo(std::ostream& rOStream) const override
    {
        rOStream << Info();
    }

    /// Print object's data.
    void PrintData(std::ostream& rOStream) const override
    {
        BaseType::PrintData(rOStream);
    }


    ///@}
    ///@name Friends
    ///@{


    ///@}

protected:
    ///@name Protected static Member Variables
    ///@{


    ///@}
    ///@name Protected member Variables
    ///@{


    ///@}
    ///@name Protected Operators
    ///@{


    ///@}
    ///@name Protected Operations
    ///@{


    ///@}
    ///@name Protected  Access
    ///@{


    ///@}
    ///@name Protected Inquiry
    ///@{


    ///@}
    ///@name Protected LifeCycle
    ///@{


    ///@}

private:
    ///@name Static Member Variables
    ///@{


    ///@}
    ///@name Member Variables
    ///@{

    unsigned int mRequiredEigenvalueNumber;

    unsigned int mEchoLevel;

    typename TLinearSolverType::Pointer mpLinearSolver;

    ///@}
    ///@name Private Operators
    ///@{


    ///@}
    ///@name Private Operations
    ///@{


    ///@}
    ///@name Private  Access
    ///@{


    ///@}
    ///@name Private Inquiry
    ///@{


    ///@}
    ///@name Un accessible methods
    ///@{


    ///@}

}; // Class PowerIterationEigenvalueSolver

/// This class uses the inverted power iteration method to obtain the lowest eigenvalue of a system
/** Basically that
*/
template<class TSparseSpaceType, class TDenseSpaceType, class TLinearSolverType,
         class TPreconditionerType = Preconditioner<TSparseSpaceType, TDenseSpaceType>,
         class TReordererType = Reorderer<TSparseSpaceType, TDenseSpaceType> >
class PowerIterationHighestEigenvalueSolver : public IterativeSolver<TSparseSpaceType, TDenseSpaceType, TPreconditionerType, TReordererType>
{
public:
    ///@name Type Definitions
    ///@{

    /// Pointer definition of PowerIterationHighestEigenvalueSolver
    KRATOS_CLASS_POINTER_DEFINITION(PowerIterationHighestEigenvalueSolver);

    typedef IterativeSolver<TSparseSpaceType, TDenseSpaceType, TPreconditionerType, TReordererType> BaseType;

    typedef typename TSparseSpaceType::MatrixType SparseMatrixType;

    typedef typename TSparseSpaceType::VectorType VectorType;

    typedef typename TDenseSpaceType::MatrixType DenseMatrixType;

    typedef typename TDenseSpaceType::VectorType DenseVectorType;

    typedef std::size_t SizeType;

    typedef std::size_t IndexType;

    ///@}
    ///@name Life Cycle
    ///@{

    /// Default constructor.
    PowerIterationHighestEigenvalueSolver() {}

    PowerIterationHighestEigenvalueSolver(
        double MaxTolerance,
        unsigned int MaxIterationNumber,
        unsigned int RequiredEigenvalueNumber,
        typename TLinearSolverType::Pointer pLinearSolver
    ): BaseType(MaxTolerance, MaxIterationNumber),   
       mRequiredEigenvalueNumber(RequiredEigenvalueNumber),
       mpLinearSolver(pLinearSolver)
    {

    }

    PowerIterationHighestEigenvalueSolver(
        Parameters ThisParameters,
        typename TLinearSolverType::Pointer pLinearSolver
        ): mpLinearSolver(pLinearSolver)
    {
        Parameters DefaultParameters = Parameters(R"(
        {
            "solver_type"             : "PowerIterationHighestEigenvalueSolver",
            "max_iteration"           : 500,
            "tolerance"               : 1e-9,
            "required_eigen_number"   : 1,
            "shifting_convergence"    : 0.25,
            "verbosity"               : 1
        })" );

        ThisParameters.ValidateAndAssignDefaults(DefaultParameters);

        mRequiredEigenvalueNumber = ThisParameters["required_eigen_number"].GetInt();
        mEchoLevel = ThisParameters["verbosity"].GetInt();
        BaseType::SetTolerance( ThisParameters["tolerance"].GetDouble() );
        BaseType::SetMaxIterationsNumber( ThisParameters["max_iteration"].GetInt() );
    }

    /// Copy constructor.
    PowerIterationHighestEigenvalueSolver(const PowerIterationHighestEigenvalueSolver& Other) : BaseType(Other)
    {

    }


    /// Destructor.
    ~PowerIterationHighestEigenvalueSolver() override
    {

    }


    ///@}
    ///@name Operators
    ///@{

    /// Assignment operator.
    PowerIterationHighestEigenvalueSolver& operator=(const PowerIterationHighestEigenvalueSolver& Other)
    {
        BaseType::operator=(Other);
        return *this;
    }

    ///@}
    ///@name Operations
    ///@{

    /**
     * The power iteration algorithm
     * @param K: The stiffness matrix
     * @param M: The mass matrix
     * @param Eigenvalues: The vector containing the eigen values
     * @param Eigenvectors: The matrix containing the eigen vectors
     */
    void Solve(
        SparseMatrixType& K,
        SparseMatrixType& M,
        DenseVectorType& Eigenvalues,
        DenseMatrixType& Eigenvectors
        ) override
    {
        using boost::numeric::ublas::trans;

        const SizeType size = K.size1();
        const SizeType max_iteration = BaseType::GetMaxIterationsNumber();
        const double tolerance = BaseType::GetTolerance();

        VectorType x = ZeroVector(size);
        VectorType y = ZeroVector(size);

        RandomInitializeUtil<double>::RandomInitialize(K, y);

        if(Eigenvalues.size() < 1)
        {
            Eigenvalues.resize(1, 0.0);
        }

        // Starting with first step
        double ro = 0.0;
        double old_ro = Eigenvalues[0];
        VectorType y_old = ZeroVector(size);

        if (mEchoLevel > 1)
        {
            std::cout << "Iteration ro \t\t convergence norm" << std::endl;
        }

        for(SizeType i = 0 ; i < max_iteration ; i++)
        {
            // x = K*y
            TSparseSpaceType::Mult(K, y, x);
            
            // y = M*x
            TSparseSpaceType::Mult(M, x, y);
            
            ro = static_cast<double>(*boost::max_element(y));
            
            TSparseSpaceType::InplaceMult(y, 1.0/ro);

            if(ro == 0.0)
            {
                KRATOS_ERROR << "Perpendicular eigenvector to M" << std::endl;
            }

            const double convergence_ro = std::abs((ro - old_ro) / ro);
            const double convergence_norm = TSparseSpaceType::TwoNorm(y - y_old)/TSparseSpaceType::TwoNorm(y);

            if (mEchoLevel > 1)
            {
                std::cout << "Iteration: " << i << "\tro: " << ro << " \tConvergence norm: " << convergence_norm << " \tConvergence ro: " << convergence_ro << std::endl;
            }
            
            if(convergence_norm < tolerance || convergence_ro < tolerance)
            {
                break;
            }

            old_ro = ro;
            TSparseSpaceType::Assign(y_old, 1.0, y);
        }

        if (mEchoLevel > 0)
        {
            KRATOS_WATCH(ro);
            KRATOS_WATCH(y);
        }

        Eigenvalues[0] = ro;

        if((Eigenvectors.size1() < 1) || (Eigenvectors.size2() < size))
        {
            Eigenvectors.resize(1,size);
        }

        for(SizeType i = 0 ; i < size ; i++)
        {
            Eigenvectors(0,i) = y[i];
        }
    }
    
    /**
     * This method returns directly the first eigen value obtained
     * @param K: The stiffness matrix
     * @param M: The mass matrix
     * @return The first eigenvalue
     */
    double GetEigenValue(
        SparseMatrixType& K,
        SparseMatrixType& M
        )
    {
        DenseVectorType eigen_values;
        DenseMatrixType eigen_vectors;
        
        Solve(K, M, eigen_values, eigen_vectors);
        
        return eigen_values[0];
    }
    
    ///@}
    ///@name Access
    ///@{


    ///@}
    ///@name Inquiry
    ///@{


    ///@}
    ///@name Input and output
    ///@{

    /// Turn back information as a string.
    std::string Info() const override
    {
        std::stringstream buffer;
        buffer << "Power iteration eigenvalue solver with " << BaseType::GetPreconditioner()->Info();
        return  buffer.str();
    }

    /// Print information about this object.
    void PrintInfo(std::ostream& rOStream) const override
    {
        rOStream << Info();
    }

    /// Print object's data.
    void PrintData(std::ostream& rOStream) const override
    {
        BaseType::PrintData(rOStream);
    }


    ///@}
    ///@name Friends
    ///@{


    ///@}

protected:
    ///@name Protected static Member Variables
    ///@{


    ///@}
    ///@name Protected member Variables
    ///@{


    ///@}
    ///@name Protected Operators
    ///@{


    ///@}
    ///@name Protected Operations
    ///@{


    ///@}
    ///@name Protected  Access
    ///@{


    ///@}
    ///@name Protected Inquiry
    ///@{


    ///@}
    ///@name Protected LifeCycle
    ///@{


    ///@}

private:
    ///@name Static Member Variables
    ///@{


    ///@}
    ///@name Member Variables
    ///@{

    unsigned int mRequiredEigenvalueNumber;

    unsigned int mEchoLevel;

    typename TLinearSolverType::Pointer mpLinearSolver;

    ///@}
    ///@name Private Operators
    ///@{


    ///@}
    ///@name Private Operations
    ///@{


    ///@}
    ///@name Private  Access
    ///@{


    ///@}
    ///@name Private Inquiry
    ///@{


    ///@}
    ///@name Un accessible methods
    ///@{


    ///@}

}; // Class PowerIterationHighestEigenvalueSolver

///@}

///@name Type Definitions
///@{


///@}
///@name Input and output
///@{


/// input stream function
template<class TSparseSpaceType, class TDenseSpaceType,
         class TPreconditionerType,
         class TReordererType>
inline std::istream& operator >> (std::istream& IStream,
                                  PowerIterationEigenvalueSolver<TSparseSpaceType, TDenseSpaceType,
                                  TPreconditionerType, TReordererType>& rThis)
{
    return IStream;
}

/// output stream function
template<class TSparseSpaceType, class TDenseSpaceType,
         class TPreconditionerType,
         class TReordererType>
inline std::ostream& operator << (std::ostream& OStream,
                                  const PowerIterationEigenvalueSolver<TSparseSpaceType, TDenseSpaceType,
                                  TPreconditionerType, TReordererType>& rThis)
{
    rThis.PrintInfo(OStream);
    OStream << std::endl;
    rThis.PrintData(OStream);

    return OStream;
}
///@}


}  // namespace Kratos.

#endif // KRATOS_POWER_ITERATION_EIGENVALUE_SOLVER_H_INCLUDED defined
