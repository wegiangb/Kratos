//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ \.
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:          BSD License
//  Original author:  Josep Maria Carbonell
//  coming from       SolidMechanicsApplication
//
//  Co-author:        Vicente Mataix Ferrandiz
//

#if !defined(KRATOS_RESIDUAL_BASED_BOSSAK_DISPLACEMENT_SCHEME )
#define  KRATOS_RESIDUAL_BASED_BOSSAK_DISPLACEMENT_SCHEME

/* System includes */

/* External includes */
#include "boost/smart_ptr.hpp"

/* Project includes */
#include "includes/define.h"
#include "includes/model_part.h"
#include "solving_strategies/schemes/scheme.h"
#include "includes/variables.h"
#include "containers/array_1d.h"
#include "includes/element.h"

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

/** @brief Bossak integration scheme (for dynamic problems)
 */
template<class TSparseSpace,  class TDenseSpace >
class ResidualBasedBossakDisplacementScheme: public Scheme<TSparseSpace,TDenseSpace>
{
public:
    ///@name Type Definitions
    ///@{
    KRATOS_CLASS_POINTER_DEFINITION( ResidualBasedBossakDisplacementScheme );

    typedef Scheme<TSparseSpace,TDenseSpace>                      BaseType;

    typedef typename BaseType::TDataType                         TDataType;

    typedef typename BaseType::DofsArrayType                 DofsArrayType;

    typedef typename Element::DofsVectorType                DofsVectorType;

    typedef typename BaseType::TSystemMatrixType         TSystemMatrixType;

    typedef typename BaseType::TSystemVectorType         TSystemVectorType;

    typedef typename BaseType::LocalSystemVectorType LocalSystemVectorType;

    typedef typename BaseType::LocalSystemMatrixType LocalSystemMatrixType;

    typedef ModelPart::NodesContainerType                   NodesArrayType;

    typedef ModelPart::ElementsContainerType             ElementsArrayType;

    typedef ModelPart::ConditionsContainerType         ConditionsArrayType;

    typedef typename BaseType::Pointer                     BaseTypePointer;

    ///@}
    ///@name Life Cycle
    ///@{

    /**
     * Constructor.
     * The bossak method
     */
    ResidualBasedBossakDisplacementScheme(double rAlpham = 0.0)
        :Scheme<TSparseSpace,TDenseSpace>()
    {
        // For pure Newmark Scheme
        mAlpha.f = 0.0;
        mAlpha.m = rAlpham;

        // Default values of the Newmark coefficients
        double beta  = 0.25;
        double gamma = 0.5;

        CalculateNewmarkCoefficients(beta, gamma);

        // std::cout << " MECHANICAL SCHEME: The Bossak Time Integration Scheme [alpha_m= " << mAlpha.m << " beta= " << mNewmark.beta << " gamma= " << mNewmark.gamma << "]" <<std::endl;

        // Allocate auxiliary memory
        const unsigned int NumThreads = OpenMPUtils::GetNumThreads();

        mMatrix.M.resize(NumThreads);
        mMatrix.D.resize(NumThreads);

        mVector.v.resize(NumThreads);
        mVector.a.resize(NumThreads);
        mVector.ap.resize(NumThreads);
    }

    /** Copy Constructor.
     */
    ResidualBasedBossakDisplacementScheme(ResidualBasedBossakDisplacementScheme& rOther)
        :BaseType(rOther)
        ,mAlpha(rOther.mAlpha)
        ,mNewmark(rOther.mNewmark)
        ,mMatrix(rOther.mMatrix)
        ,mVector(rOther.mVector)
    {
    }

    /**
     * Clone
     */
    BaseTypePointer Clone() override
    {
        return BaseTypePointer( new ResidualBasedBossakDisplacementScheme(*this) );
    }

    /** Destructor.
     */
    ~ResidualBasedBossakDisplacementScheme
    () override {}

    ///@}
    ///@name Operators
    ///@{

    ///@}
    ///@name Operations
    ///@{

    /**
     * Recalculates the Newmark coefficients, taking into account the alpha parameters
     * @param beta: The Newmark beta coefficient
     * @param gamma: The Newmark gamma coefficient
     */

    void CalculateNewmarkCoefficients(
            double beta,
            double gamma
            )
    {
        mNewmark.beta  = (1.0 + mAlpha.f - mAlpha.m) * (1.0 + mAlpha.f - mAlpha.m) * beta;
        mNewmark.gamma = gamma + mAlpha.f - mAlpha.m;
    }

    /**
     * Performing the update of the solution
     * Incremental update within newton iteration. It updates the state variables at the end of the time step: u_{n+1}^{k+1}= u_{n+1}^{k}+ \Delta u
     * @param rModelPart: The model of the problem to solve
     * @param rDofSet: Set of all primary variables
     * @param A: LHS matrix
     * @param Dx: incremental update of primary variables
     * @param b: RHS Vector
     */

    void Update(
        ModelPart& rModelPart,
        DofsArrayType& rDofSet,
        TSystemMatrixType& A,
        TSystemVectorType& Dx,
        TSystemVectorType& b 
        ) override
    {
        KRATOS_TRY;

        const unsigned int num_threads = OpenMPUtils::GetNumThreads();

        // Update of displacement (by DOF)
        const int num_dof = static_cast<int>(rDofSet.size());
        OpenMPUtils::PartitionVector dof_partition;
        OpenMPUtils::DivideInPartitions(num_dof, num_threads, dof_partition);

        #pragma omp parallel for
        for(int i = 0;  i < num_dof; ++i)
        {
            auto it_dof = rDofSet.begin() + i;

            if (it_dof->IsFree())
            {
                it_dof->GetSolutionStepValue() += TSparseSpace::GetValue(Dx,it_dof->EquationId());
            }
        }

        // Updating time derivatives (nodally for efficiency)
        const int num_nodes = static_cast<int>(rModelPart.Nodes().size());
        OpenMPUtils::PartitionVector node_partition;
        OpenMPUtils::DivideInPartitions(num_nodes, num_threads, node_partition);

        #pragma omp parallel for
        for(int i = 0;  i < num_nodes; ++i)
        {
            auto it_node = rModelPart.Nodes().begin() + i;
                        
            array_1d<double, 3 > delta_displacement;

            noalias(delta_displacement) = it_node->FastGetSolutionStepValue(DISPLACEMENT) - it_node->FastGetSolutionStepValue(DISPLACEMENT, 1);

            array_1d<double, 3 > & current_velocity = it_node->FastGetSolutionStepValue(VELOCITY);
            const array_1d<double, 3 > & previous_velocity = it_node->FastGetSolutionStepValue(VELOCITY, 1);

            array_1d<double, 3 > & current_aceleration = it_node->FastGetSolutionStepValue(ACCELERATION);
            const array_1d<double, 3 > & previous_aceleration = it_node->FastGetSolutionStepValue(ACCELERATION, 1);

            UpdateVelocity(current_velocity, delta_displacement, previous_velocity, previous_aceleration);
            UpdateAcceleration(current_aceleration, delta_displacement, previous_velocity, previous_aceleration);
        }

        KRATOS_CATCH( "" );
    }

    /**
     * Performing the prediction of the solution
     * It predicts the solution for the current step: x = xold + vold * Dt
     * @param rModelPart: The model of the problem to solve
     * @param rDofSet set of all primary variables
     * @param A: LHS matrix
     * @param Dx: Incremental update of primary variables
     * @param b: RHS Vector
     */

    void Predict(
        ModelPart& rModelPart,
        DofsArrayType& rDofSet,
        TSystemMatrixType& A,
        TSystemVectorType& Dx,
        TSystemVectorType& b
        ) override
    {
        KRATOS_TRY;

        const double delta_time = rModelPart.GetProcessInfo()[DELTA_TIME];

        // Updating time derivatives (nodally for efficiency)
        const unsigned int num_threads = OpenMPUtils::GetNumThreads();
        
        const int num_nodes = static_cast<int>( rModelPart.Nodes().size() );
        OpenMPUtils::PartitionVector node_partition;
        OpenMPUtils::DivideInPartitions(rModelPart.Nodes().size(), num_threads, node_partition);

        #pragma omp parallel for
        for(int i = 0;  i< num_nodes; ++i)
        {
            auto it_node = rModelPart.Nodes().begin() + i;

            //Predicting: NewDisplacement = previous_displacement + previous_velocity * delta_time;
            //ATTENTION::: the prediction is performed only on free nodes

            const array_1d<double, 3 > & previous_acceleration = it_node->FastGetSolutionStepValue(ACCELERATION, 1);
            const array_1d<double, 3 > & previous_velocity     = it_node->FastGetSolutionStepValue(VELOCITY,     1);
            const array_1d<double, 3 > & previous_displacement = it_node->FastGetSolutionStepValue(DISPLACEMENT, 1);
            array_1d<double, 3 > & current_acceleration = it_node->FastGetSolutionStepValue(ACCELERATION);
            array_1d<double, 3 > & current_velocity     = it_node->FastGetSolutionStepValue(VELOCITY);
            array_1d<double, 3 > & current_displacement = it_node->FastGetSolutionStepValue(DISPLACEMENT);

            if (it_node -> IsFixed(ACCELERATION_X))
            {
                current_displacement[0] = previous_displacement[0] + delta_time * previous_velocity[0] + std::pow(delta_time, 2) * ( 0.5 * (1.0 -  2.0 * mNewmark.beta) * previous_acceleration[0] + mNewmark.beta * current_acceleration[0]);
            }
            else if (it_node -> IsFixed(VELOCITY_X))
            {
                current_displacement[0] = previous_displacement[0] + 0.5 * delta_time * (previous_velocity[0] + current_velocity[0]) + 0.5 * std::pow(delta_time, 2) * previous_acceleration[0];
            }
            else if (it_node -> IsFixed(DISPLACEMENT_X) == false)
            {
                current_displacement[0] = previous_displacement[0] + delta_time * previous_velocity[0] + 0.5 * std::pow(delta_time, 2) * previous_acceleration[0];
            }

            if (it_node -> IsFixed(ACCELERATION_Y))
            {
                current_displacement[1] = previous_displacement[1] + delta_time * previous_velocity[1] + std::pow(delta_time, 2) * ( 0.5 * (1.0 -  2.0 * mNewmark.beta) * previous_acceleration[1] + mNewmark.beta * current_acceleration[1]);
            }
            else if (it_node -> IsFixed(VELOCITY_Y))
            {
                current_displacement[1] = previous_displacement[1] + 0.5 * delta_time * (previous_velocity[1] + current_velocity[1]) + 0.5 * std::pow(delta_time, 2) * previous_acceleration[1] ;
            }
            else if (it_node -> IsFixed(DISPLACEMENT_Y) == false)
            {
                current_displacement[1] = previous_displacement[1] + delta_time * previous_velocity[1] + 0.5 * std::pow(delta_time, 2) * previous_acceleration[1];
            }

            // For 3D cases
            if (it_node -> HasDofFor(DISPLACEMENT_Z))
            {
                if (it_node -> IsFixed(ACCELERATION_Z))
                {
                    current_displacement[2] = previous_displacement[2] + delta_time * previous_velocity[2] + std::pow(delta_time, 2) * ( 0.5 * (1.0 -  2.0 * mNewmark.beta) * previous_acceleration[2] + mNewmark.beta * current_acceleration[2]);
                }
                else if (it_node -> IsFixed(VELOCITY_Z))
                {
                    current_displacement[2] = previous_displacement[2] + 0.5 * delta_time * (previous_velocity[2] + current_velocity[2]) + 0.5 * std::pow(delta_time, 2) * previous_acceleration[2] ;
                }
                else if (it_node -> IsFixed(DISPLACEMENT_Z) == false)
                {
                    current_displacement[2] = previous_displacement[2] + delta_time * previous_velocity[2] + 0.5 * std::pow(delta_time, 2) * previous_acceleration[2];
                }
            }

            // Updating time derivatives ::: Please note that displacements and its time derivatives can not be consistently fixed separately
            const array_1d<double, 3 > delta_displacement = current_displacement - previous_displacement;

            UpdateVelocity(current_velocity, delta_displacement, previous_velocity, previous_acceleration);

            UpdateAcceleration(current_acceleration, delta_displacement, previous_velocity, previous_acceleration);
        }

        KRATOS_CATCH( "" );
    }

    /**
     * It initializes time step solution. Only for reasons if the time step solution is restarted
     * @param rModelPart: The model of the problem to solve
     * @param A: LHS matrix
     * @param Dx: Incremental update of primary variables
     * @param b: RHS Vector
     *
     */
    
    void InitializeSolutionStep(
        ModelPart& rModelPart,
        TSystemMatrixType& A,
        TSystemVectorType& Dx,
        TSystemVectorType& b
    ) override
    {
        KRATOS_TRY;

        ProcessInfo current_process_info= rModelPart.GetProcessInfo();

        BaseType::InitializeSolutionStep(rModelPart, A, Dx, b);

        double delta_time = current_process_info[DELTA_TIME];

        double beta = 0.25;
        if (current_process_info.Has(NEWMARK_BETA))
        {
            beta = current_process_info[NEWMARK_BETA];
        }
        double gamma = 0.5;
        if (current_process_info.Has(NEWMARK_GAMMA))
        {
            gamma = current_process_info[NEWMARK_GAMMA];
        }

        CalculateNewmarkCoefficients(beta, gamma);

        if (delta_time < 1.0e-24)
        {
            KRATOS_ERROR << " ERROR: detected delta_time = 0 in the Solution Scheme DELTA_TIME. PLEASE : check if the time step is created correctly for the current model part ";
        }

        // Initializing Newmark constants
        mNewmark.c0 = ( 1.0 / (mNewmark.beta * delta_time * delta_time) );
        mNewmark.c1 = ( mNewmark.gamma / (mNewmark.beta * delta_time) );
        mNewmark.c2 = ( 1.0 / (mNewmark.beta * delta_time) );
        mNewmark.c3 = ( 0.5 / (mNewmark.beta) - 1.0 );
        mNewmark.c4 = ( (mNewmark.gamma / mNewmark.beta) - 1.0  );
        mNewmark.c5 = ( delta_time * 0.5 * ( ( mNewmark.gamma / mNewmark.beta ) - 2.0 ) );

        KRATOS_CATCH( "" );
    }

    /**
     * It initializes a non-linear iteration (for the element)
     * @param rModelPart The model of the problem to solve
     * @param A LHS matrix
     * @param Dx Incremental update of primary variables
     * @param b RHS Vector
     */

    void InitializeNonLinIteration(
        ModelPart& rModelPart,
        TSystemMatrixType& A,
        TSystemVectorType& Dx,
        TSystemVectorType& b
    ) override
    {
        KRATOS_TRY;

        ProcessInfo& current_process_info = rModelPart.GetProcessInfo();
        
        #pragma omp parallel for
        for(int i=0; i<static_cast<int>(rModelPart.Elements().size()); ++i)
        {
            auto itElem = rModelPart.ElementsBegin() + i;
            itElem->InitializeNonLinearIteration(current_process_info);
        }
        
        #pragma omp parallel for
        for(int i=0; i<static_cast<int>(rModelPart.Conditions().size()); ++i)
        {
            auto itElem = rModelPart.ConditionsBegin() + i;
            itElem->InitializeNonLinearIteration(current_process_info);
        }     
        
        KRATOS_CATCH( "" );
    }

    /**
     * It initializes a non-linear iteration (for an individual condition)
     * @param rCurrentConditiont The condition to compute
     * @param CurrentProcessInfo The current process info instance
     */

    void InitializeNonLinearIteration(
        Condition::Pointer rCurrentCondition,
        ProcessInfo& CurrentProcessInfo
        ) override
    {
        (rCurrentCondition) -> InitializeNonLinearIteration(CurrentProcessInfo);
    }

    /**
     * It initializes a non-linear iteration (for an individual element)
     * @param rCurrentElement The element to compute
     * @param CurrentProcessInfo The current process info instance
     */

    void InitializeNonLinearIteration(
        Element::Pointer rCurrentElement,
        ProcessInfo& CurrentProcessInfo
        ) override
    {
        (rCurrentElement) -> InitializeNonLinearIteration(CurrentProcessInfo);
    }

    /**
     * This function is designed to be called in the builder and solver to introduce
     * @param rCurrentElement The element to compute
     * @param LHS_Contribution The LHS matrix contribution
     * @param RHS_Contribution The RHS vector contribution
     * @param EquationId The ID's of the element degrees of freedom
     * @param CurrentProcessInfo The current process info instance
     */

    void CalculateSystemContributions(
        Element::Pointer rCurrentElement,
        LocalSystemMatrixType& LHS_Contribution,
        LocalSystemVectorType& RHS_Contribution,
        Element::EquationIdVectorType& EquationId,
        ProcessInfo& CurrentProcessInfo
        ) override
    {
        KRATOS_TRY;

        const int thread = OpenMPUtils::ThisThread();

        //(rCurrentElement) -> InitializeNonLinearIteration(CurrentProcessInfo);

        (rCurrentElement) -> CalculateLocalSystem(LHS_Contribution,RHS_Contribution,CurrentProcessInfo);

        (rCurrentElement) -> EquationIdVector(EquationId,CurrentProcessInfo);

        (rCurrentElement) -> CalculateMassMatrix(mMatrix.M[thread],CurrentProcessInfo);

        (rCurrentElement) -> CalculateDampingMatrix(mMatrix.D[thread],CurrentProcessInfo);

        AddDynamicsToLHS (LHS_Contribution, mMatrix.D[thread], mMatrix.M[thread], CurrentProcessInfo);

        AddDynamicsToRHS (rCurrentElement, RHS_Contribution, mMatrix.D[thread], mMatrix.M[thread], CurrentProcessInfo);

        //AssembleTimeSpaceLHS(rCurrentElement, LHS_Contribution, DampMatrix, MassMatrix,CurrentProcessInfo);

        KRATOS_CATCH( "" );
    }

    /**
     * This function is designed to calculate just the RHS contribution
     * @param rCurrentElemen The element to compute
     * @param RHS_Contribution The RHS vector contribution
     * @param EquationId The ID's of the element degrees of freedom
     * @param CurrentProcessInfo The current process info instance
     */

    void Calculate_RHS_Contribution(
        Element::Pointer rCurrentElement,
        LocalSystemVectorType& RHS_Contribution,
        Element::EquationIdVectorType& EquationId,
        ProcessInfo& CurrentProcessInfo
        ) override
    {

        KRATOS_TRY;

        const int thread = OpenMPUtils::ThisThread();

        // Initializing the non linear iteration for the current element
        // (rCurrentElement) -> InitializeNonLinearIteration(CurrentProcessInfo);

        // Basic operations for the element considered
        (rCurrentElement) -> CalculateRightHandSide(RHS_Contribution,CurrentProcessInfo);

        (rCurrentElement) -> CalculateMassMatrix(mMatrix.M[thread], CurrentProcessInfo);

        (rCurrentElement) -> CalculateDampingMatrix(mMatrix.D[thread],CurrentProcessInfo);

        (rCurrentElement) -> EquationIdVector(EquationId,CurrentProcessInfo);

        AddDynamicsToRHS (rCurrentElement, RHS_Contribution, mMatrix.D[thread], mMatrix.M[thread], CurrentProcessInfo);

        KRATOS_CATCH( "" );
    }

    /**
     * Functions totally analogous to the precedent but applied to the "condition" objects
     * @param rCurrentCondition The condition to compute
     * @param LHS_Contribution The LHS matrix contribution
     * @param RHS_Contribution The RHS vector contribution
     * @param EquationId The ID's of the element degrees of freedom
     * @param CurrentProcessInfo The current process info instance
     */

    void Condition_CalculateSystemContributions(
        Condition::Pointer rCurrentCondition,
        LocalSystemMatrixType& LHS_Contribution,
        LocalSystemVectorType& RHS_Contribution,
        Element::EquationIdVectorType& EquationId,
        ProcessInfo& CurrentProcessInfo
        ) override
    {
        KRATOS_TRY;

        const int thread = OpenMPUtils::ThisThread();

        // Initializing the non linear iteration for the current condition
        //(rCurrentCondition) -> InitializeNonLinearIteration(CurrentProcessInfo);

        // Basic operations for the condition considered
        (rCurrentCondition) -> CalculateLocalSystem(LHS_Contribution,RHS_Contribution,CurrentProcessInfo);

        (rCurrentCondition) -> EquationIdVector(EquationId,CurrentProcessInfo);

        (rCurrentCondition) -> CalculateMassMatrix(mMatrix.M[thread], CurrentProcessInfo);

        (rCurrentCondition) -> CalculateDampingMatrix(mMatrix.D[thread],CurrentProcessInfo);

        AddDynamicsToLHS  (LHS_Contribution, mMatrix.D[thread], mMatrix.M[thread], CurrentProcessInfo);

        AddDynamicsToRHS  (rCurrentCondition, RHS_Contribution, mMatrix.D[thread], mMatrix.M[thread], CurrentProcessInfo);

        // AssembleTimeSpaceLHS_Condition(rCurrentCondition, LHS_Contribution,DampMatrix, MassMatrix,CurrentProcessInfo);

        KRATOS_CATCH( "" );
    }

    /**
     * Functions that calculates the RHS of a "condition" object
     * @param rCurrentCondition The condition to compute
     * @param RHS_Contribution The RHS vector contribution
     * @param EquationId The ID's of the condition degrees of freedom
     * @param CurrentProcessInfo The current process info instance
     */

    void Condition_Calculate_RHS_Contribution(
        Condition::Pointer rCurrentCondition,
        LocalSystemVectorType& RHS_Contribution,
        Element::EquationIdVectorType& EquationId,
        ProcessInfo& CurrentProcessInfo
        ) override
    {
        KRATOS_TRY;

        const int thread = OpenMPUtils::ThisThread();

        // Initializing the non linear iteration for the current condition
        //(rCurrentCondition) -> InitializeNonLinearIteration(CurrentProcessInfo);

        // Basic operations for the condition considered
        (rCurrentCondition) -> CalculateRightHandSide(RHS_Contribution, CurrentProcessInfo);

        (rCurrentCondition) -> EquationIdVector(EquationId, CurrentProcessInfo);

        (rCurrentCondition) -> CalculateMassMatrix(mMatrix.M[thread], CurrentProcessInfo);

        (rCurrentCondition) -> CalculateDampingMatrix(mMatrix.D[thread], CurrentProcessInfo);

        // Adding the dynamic contributions (static is already included)
        AddDynamicsToRHS  (rCurrentCondition, RHS_Contribution, mMatrix.D[thread], mMatrix.M[thread], CurrentProcessInfo);

        KRATOS_CATCH( "" );
    }

    /**
     * This function is designed to be called once to perform all the checks needed
     * on the input provided. Checks can be "expensive" as the function is designed
     * to catch user's errors.
     * @param rModelPart The model of the problem to solve
     * @return Zero means  all ok
     */

    int Check(ModelPart& rModelPart) override
    {
        KRATOS_TRY;

        const int err = Scheme<TSparseSpace, TDenseSpace>::Check(rModelPart);
        if(err != 0) return err;

        // Check for variables keys
        // Verify that the variables are correctly initialized
        if(DISPLACEMENT.Key() == 0)
        {
            KRATOS_ERROR << "DISPLACEMENT has Key zero! (check if the application is correctly registered" << std::endl;
        }
        if(VELOCITY.Key() == 0)
        {
            KRATOS_ERROR << "VELOCITY has Key zero! (check if the application is correctly registered" << std::endl;
        }
        if(ACCELERATION.Key() == 0)
        {
            KRATOS_ERROR << "ACCELERATION has Key zero! (check if the application is correctly registered" << std::endl;
        }

        // Check that variables are correctly allocated
        for(ModelPart::NodesContainerType::iterator it=rModelPart.NodesBegin();
                it!=rModelPart.NodesEnd(); ++it)
        {
            if (it->SolutionStepsDataHas(DISPLACEMENT) == false)
            {
                KRATOS_ERROR << "DISPLACEMENT variable is not allocated for node " << it->Id() << std::endl;
            }
            if (it->SolutionStepsDataHas(VELOCITY) == false)
            {
                KRATOS_ERROR << "VELOCITY variable is not allocated for node " << it->Id() << std::endl;
            }
            if (it->SolutionStepsDataHas(ACCELERATION) == false)
            {
                KRATOS_ERROR << "ACCELERATION variable is not allocated for node " << it->Id() << std::endl;
            }
        }

        // Check that dofs exist
        for(ModelPart::NodesContainerType::iterator it=rModelPart.NodesBegin();
                it!=rModelPart.NodesEnd(); ++it)
        {
            if(it->HasDofFor(DISPLACEMENT_X) == false)
            {
                KRATOS_ERROR << "missing DISPLACEMENT_X dof on node " << it->Id() << std::endl;
            }
            if(it->HasDofFor(DISPLACEMENT_Y) == false)
            {
                KRATOS_ERROR << "missing DISPLACEMENT_Y dof on node " << it->Id() << std::endl;
            }
            if(it->HasDofFor(DISPLACEMENT_Z) == false)
            {
                KRATOS_ERROR << "missing DISPLACEMENT_Z dof on node " << it->Id() << std::endl;
            }
        }

        // Check for admissible value of the AlphaBossak
        if(mAlpha.m > 0.0 || mAlpha.m < -0.3)
        {
            KRATOS_ERROR << "Value not admissible for AlphaBossak. Admissible values should be between 0.0 and -0.3. Current value is " << mAlpha.m << std::endl;
        }

        // Check for minimum value of the buffer index
        // Verify buffer size
        if (rModelPart.GetBufferSize() < 2)
        {
            KRATOS_ERROR << "insufficient buffer size. Buffer size should be greater than 2. Current size is" << rModelPart.GetBufferSize() << std::endl;
        }

        return 0;
        KRATOS_CATCH( "" );
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

protected:

    ///@name Protected static Member Variables
    ///@{

    ///@}
    ///@name Protected member Variables
    ///@{

    struct GeneralAlphaMethod
    {
        double f;  // Alpha Hilbert
        double m;  // Alpha Bosssak
    };

    struct NewmarkMethod
    {
        double beta;
        double gamma;

        // System constants
        double c0;
        double c1;
        double c2;
        double c3;
        double c4;
        double c5;
        double c6;
    };

    struct  GeneralMatrices
    {
        std::vector< Matrix > M;     // First derivative matrix  (usually mass matrix)
        std::vector< Matrix > D;     // Second derivative matrix (usually damping matrix)
    };

    struct GeneralVectors
    {
        std::vector< Vector > v;    // Velocity
        std::vector< Vector > a;    // Acceleration
        std::vector< Vector > ap;   // Previous acceleration
    };

    GeneralAlphaMethod  mAlpha;
    NewmarkMethod       mNewmark;

    GeneralMatrices     mMatrix;
    GeneralVectors      mVector;

    ///@}
    ///@name Protected Operators
    ///@{

    ///@}
    ///@name Protected Operations
    ///@{

    /**
     * Updating first time Derivative
     * @param CurrentVelocity The current velocity
     * @param DeltaDisplacement The increment of displacement
     * @param PreviousVelocity The previous velocity
     * @param PreviousAcceleration The previous acceleration
     */

    inline void UpdateVelocity(
        array_1d<double, 3 > & CurrentVelocity,
        const array_1d<double, 3 > & DeltaDisplacement,
        const array_1d<double, 3 > & PreviousVelocity,
        const array_1d<double, 3 > & PreviousAcceleration
    )
    {
        noalias(CurrentVelocity) =  (mNewmark.c1 * DeltaDisplacement - mNewmark.c4 * PreviousVelocity
                                     - mNewmark.c5 * PreviousAcceleration);
    }

    /**
     * Updating second time Derivative
     * @param CurrentVelocity The current velocity
     * @param DeltaDisplacement The increment of displacement
     * @param PreviousVelocity The previous velocity
     * @param PreviousAcceleration The previous acceleration
     */

    inline void UpdateAcceleration(
        array_1d<double, 3 > & CurrentAcceleration,
        const array_1d<double, 3 > & DeltaDisplacement,
        const array_1d<double, 3 > & PreviousVelocity,
        const array_1d<double, 3 > & PreviousAcceleration
    )
    {
        noalias(CurrentAcceleration) =  (mNewmark.c0 * DeltaDisplacement - mNewmark.c2 * PreviousVelocity
                                         -  mNewmark.c3 * PreviousAcceleration);
    }

    /**
     * It adds the dynamic LHS contribution of the elements M*c0 + D*c1 + K
     * @param LHS_Contribution The dynamic contribution for the LHS
     * @param D The damping matrix
     * @param M The mass matrix
     * @param CurrentProcessInfo The current process info instance
     */

    void AddDynamicsToLHS(
        LocalSystemMatrixType& LHS_Contribution,
        LocalSystemMatrixType& D,
        LocalSystemMatrixType& M,
        ProcessInfo& CurrentProcessInfo)
    {
        // Adding mass contribution to the dynamic stiffness
        if (M.size1() != 0) // if M matrix declared
        {
            noalias(LHS_Contribution) += M * (1.0 - mAlpha.m) * mNewmark.c0;

            // std::cout<<" Mass Matrix "<<M<<" coeficient "<<(1-mAlpha.m)*mNewmark.c0<<std::endl;
        }

        // Adding  damping contribution
        if (D.size1() != 0) // if D matrix declared
        {
            noalias(LHS_Contribution) += D * (1.0 - mAlpha.f) * mNewmark.c1;
        }
    }

    /**
     * It adds the dynamic RHS contribution of the elements b - M*a - D*v
     * @param rCurrentElement The element to compute
     * @param RHS_Contribution The dynamic contribution for the RHS
     * @param D The damping matrix
     * @param M The mass matrix
     * @param CurrentProcessInfo The current process info instance
     */

    void AddDynamicsToRHS(
        Element::Pointer rCurrentElement,
        LocalSystemVectorType& RHS_Contribution,
        LocalSystemMatrixType& D,
        LocalSystemMatrixType& M,
        ProcessInfo& CurrentProcessInfo
        )
    {
        const int thread = OpenMPUtils::ThisThread();

        // Adding inertia contribution
        if (M.size1() != 0)
        {
            rCurrentElement->GetSecondDerivativesVector(mVector.a[thread], 0);

            (mVector.a[thread]) *= (1.00 - mAlpha.m);

            rCurrentElement->GetSecondDerivativesVector(mVector.ap[thread], 1);

            noalias(mVector.a[thread]) += mAlpha.m * mVector.ap[thread];

            noalias(RHS_Contribution)  -= prod(M, mVector.a[thread]);
            //KRATOS_WATCH( prod(M, mVector.a[thread] ) )

        }

        // Adding damping contribution
        if (D.size1() != 0)
        {
            rCurrentElement->GetFirstDerivativesVector(mVector.v[thread], 0);

            noalias(RHS_Contribution) -= prod(D, mVector.v[thread]);
        }
    }

    /**
     * It adds the dynamic RHS contribution of the condition b - M*a - D*v
     * @param rCurrentCondition The condition to compute
     * @param RHS_Contribution The dynamic contribution for the RHS
     * @param D The damping matrix
     * @param M The mass matrix
     * @param CurrentProcessInfo The current process info instance
     */

    void AddDynamicsToRHS(
        Condition::Pointer rCurrentCondition,
        LocalSystemVectorType& RHS_Contribution,
        LocalSystemMatrixType& D,
        LocalSystemMatrixType& M,
        ProcessInfo& CurrentProcessInfo)
    {
        const int thread = OpenMPUtils::ThisThread();

        // Adding inertia contribution
        if (M.size1() != 0)
        {
            rCurrentCondition->GetSecondDerivativesVector(mVector.a[thread], 0);

            (mVector.a[thread]) *= (1.00 - mAlpha.m);

            rCurrentCondition->GetSecondDerivativesVector(mVector.ap[thread], 1);

            noalias(mVector.a[thread]) += mAlpha.m * mVector.ap[thread];

            noalias(RHS_Contribution)  -= prod(M, mVector.a[thread]);
        }

        // Adding damping contribution
        // Damping contribution
        if (D.size1() != 0)
        {
            rCurrentCondition->GetFirstDerivativesVector(mVector.v[thread], 0);

            noalias(RHS_Contribution) -= prod(D, mVector.v [thread]);
        }
    }

    ///@}
    ///@name Protected  Access
    ///@{

    ///@}
    ///@name Protected Inquiry
    ///@{

    ///@}
    ///@name Protected LifeCycle
    ///@{
    ///@{

private:

    ///@name Static Member Variables
    ///@{
    ///@}
    ///@name Member Variables
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

    ///@}
    ///@name Serialization
    ///@{

    ///@name Private Inquiry
    ///@{
    ///@}
    ///@name Un accessible methods
    ///@{
    ///@}
}; /* Class ResidualBasedBossakDisplacementScheme */
///@}
///@name Type Definitions
///@{
///@}
///@name Input and output
///@{
///@}
}  /* namespace Kratos.*/

#endif /* KRATOS_RESIDUAL_BASED_BOSSAK_DISPLACEMENT_SCHEME defined */
