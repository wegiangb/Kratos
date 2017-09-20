from __future__ import print_function, absolute_import, division  # makes KratosMultiphysics backward compatible with python 2.6 and 2.7

from KratosMultiphysics import *
from KratosMultiphysics.ExternalSolversApplication  import *
from KratosMultiphysics.StructuralMechanicsApplication  import *

## Import define_output
parameter_file = open("ProjectParametersCompositeUtility.json",'r')
ProjectParameters = Parameters( parameter_file.read())

## Get echo level and parallel type
echo_level = ProjectParameters["problem_data"]["echo_level"].GetInt()
parallel_type = ProjectParameters["problem_data"]["parallel_type"].GetString()

## Import parallel modules if needed
if (parallel_type == "MPI"):
    from KratosMultiphysics.mpi import *
    from KratosMultiphysics.MetisApplication import *
    from KratosMultiphysics.TrilinosApplication import *

## Structure model part definition
main_model_part = ModelPart(ProjectParameters["problem_data"]["model_part_name"].GetString())
main_model_part.ProcessInfo.SetValue(DOMAIN_SIZE, ProjectParameters["problem_data"]["domain_size"].GetInt())

## Solver construction
import python_solvers_wrapper_structural
solver = python_solvers_wrapper_structural.CreateSolver(main_model_part, ProjectParameters)

solver.AddVariables()
main_model_part.AddNodalSolutionStepVariable(FIBER_ANGLE)

## Read the model - note that SetBufferSize is done here
solver.ImportModelPart()

composite_property_alignment_settings_11 = Parameters("""
{
    "method": "simple",
    "method_specific_settings" : {
        "global_fiber_direction" : [0,0,1],
        "normal_vector"   : [1,0,0]
    }
}
""")
composite_property_alignment_settings_12 = Parameters("""
{
    "method": "simple",
    "method_specific_settings" : {
        "cs_axis_1" : [0,0,1],
        "cs_axis_2" : [1,0,0],
        "cs_normal_axis" : 3
    }
}
""")
composite_property_alignment_settings_13 = Parameters("""
{
    "method": "simple",
    "method_specific_settings" : {
        "cs_axis_1" : [0,0,1],
        "cs_axis_2" : [1,0,0],
        "cs_rotation_angle" : 15,
        "cs_normal_axis" : 3        
    }
}
""")

composite_property_alignment_settings_21 = Parameters("""
{
    "method": "advanced",
    "method_specific_settings" : {
        "global_fiber_direction" : [0,0,1],
        "normal_vector"   : [1,0,0],
        "smoothness_level" : 1
    }
}
""")

composite_property_alignment_settings_22 = Parameters("""
{
    "method": "advanced",
    "method_specific_settings" : {
        "global_fiber_direction" : [0,0,1],
        "normal_vector"   : [1,0,0]
    }
}
""")

composite_property_alignment_settings_23 = Parameters("""
{
    "method": "advanced",
    "method_specific_settings" : {
        "global_fiber_direction" : [0,0,1],
        "normal_vector"   : [1,0,0],
        "smoothness_level" : 3        
    }
}
""")
comp_assignment_utility = CompositePropertyAssignment(main_model_part.GetSubModelPart("main_mesh"))
comp_assignment_utility.Execute(composite_property_alignment_settings_11)
comp_assignment_utility.WriteFiberAngles()


## Example of CompositePropertyAssignment utility 1
#fiberVector = [0.0,0.0,1.0]
#normalVector = [0,1,0]
#CompositePropertyAssignment().Execute(main_model_part.GetSubModelPart("main_mesh"),fiberVector,normalVector,main_model_part.ProcessInfo)

## Example of CompositePropertyAssignment utility 2
# lcs1 = [0,0,1]
# lcs2 = [1,0,0]
# userAxisNormalToShell = 3
# rotationDegreesAboutUserNormalAxis = 45.0
# suppressOrthogonalError = True
# CompositePropertyAssignment().ExecuteCustomCS(main_model_part.GetSubModelPart("main_mesh"),lcs1,lcs2,userAxisNormalToShell,rotationDegreesAboutUserNormalAxis,suppressOrthogonalError,main_model_part.ProcessInfo)



## Add AddDofs
solver.AddDofs()

## Initialize GiD  I/O
output_post  = ProjectParameters.Has("output_configuration")
if (output_post == True):
    if (parallel_type == "OpenMP"):
        from gid_output_process import GiDOutputProcess
        gid_output = GiDOutputProcess(solver.GetComputingModelPart(),
                                      ProjectParameters["problem_data"]["problem_name"].GetString() ,
                                      ProjectParameters["output_configuration"])
    elif (parallel_type == "MPI"):
        from gid_output_process_mpi import GiDOutputProcessMPI
        gid_output = GiDOutputProcessMPI(solver.GetComputingModelPart(),
                                         ProjectParameters["problem_data"]["problem_name"].GetString() ,
                                         ProjectParameters["output_configuration"])

    gid_output.ExecuteInitialize()

## Creation of the Kratos model (build sub_model_parts or submeshes)
StructureModel = {ProjectParameters["problem_data"]["model_part_name"].GetString(): main_model_part}

## Get the list of the sub_model_parts in where the processes are to be applied
for i in range(ProjectParameters["solver_settings"]["processes_sub_model_part_list"].size()):
    part_name = ProjectParameters["solver_settings"]["processes_sub_model_part_list"][i].GetString()
    StructureModel.update({part_name: main_model_part.GetSubModelPart(part_name)})

## Print model_part and properties
if ((parallel_type == "OpenMP") or (mpi.rank == 0)) and (echo_level > 1):
    print("")
    print(main_model_part)
    for properties in main_model_part.Properties:
        print(properties)

## Processes construction
import process_factory
list_of_processes = process_factory.KratosProcessFactory(StructureModel).ConstructListOfProcesses(ProjectParameters["constraints_process_list"])
list_of_processes += process_factory.KratosProcessFactory(StructureModel).ConstructListOfProcesses(ProjectParameters["loads_process_list"])
if (ProjectParameters.Has("list_other_processes") == True):
    list_of_processes += process_factory.KratosProcessFactory(StructureModel).ConstructListOfProcesses(ProjectParameters["list_other_processes"])
if (ProjectParameters.Has("json_output_process") == True):
    list_of_processes += process_factory.KratosProcessFactory(StructureModel).ConstructListOfProcesses(ProjectParameters["json_output_process"])

if ((parallel_type == "OpenMP") or (mpi.rank == 0)) and (echo_level > 1):
    for process in list_of_processes:
        print(process)

## Processes initialization
for process in list_of_processes:
    process.ExecuteInitialize()

## Solver initialization
solver.Initialize()
solver.SetEchoLevel(echo_level)

if (output_post == True):
    gid_output.ExecuteBeforeSolutionLoop()

for process in list_of_processes:
    process.ExecuteBeforeSolutionLoop()

## Writing the full ProjectParameters file before solving
if ((parallel_type == "OpenMP") or (mpi.rank == 0)) and (echo_level > 0):
    f = open("ProjectParametersOutput.json", 'w')
    f.write(ProjectParameters.PrettyPrintJsonString())
    f.close()

## Stepping and time settings
delta_time = ProjectParameters["problem_data"]["time_step"].GetDouble()
start_time = ProjectParameters["problem_data"]["start_time"].GetDouble()
end_time = ProjectParameters["problem_data"]["end_time"].GetDouble()

time = start_time
main_model_part.ProcessInfo[TIME_STEPS] = 0




# Solving the problem (time integration)
while(time <= end_time):

    time = time + delta_time
    main_model_part.ProcessInfo[TIME_STEPS] += 1
    main_model_part.CloneTimeStep(time)

    if (parallel_type == "OpenMP") or (mpi.rank == 0):
        print("")
        print("STEP = ", main_model_part.ProcessInfo[TIME_STEPS])
        print("TIME = ", time)

    for process in list_of_processes:
        process.ExecuteInitializeSolutionStep()

    if (output_post == True):
        gid_output.ExecuteInitializeSolutionStep()

    solver.Solve()

    for process in list_of_processes:
        process.ExecuteFinalizeSolutionStep()

    if (output_post == True):
        gid_output.ExecuteFinalizeSolutionStep()

    for process in list_of_processes:
        process.ExecuteBeforeOutputStep()

    if (output_post == True) and (gid_output.IsOutputStep()):
        gid_output.PrintOutput()

    for process in list_of_processes:
        process.ExecuteAfterOutputStep()
        

for process in list_of_processes:
    process.ExecuteFinalize()

if (output_post == True):
    gid_output.ExecuteFinalize()

if (parallel_type == "OpenMP") or (mpi.rank == 0):
    print(" ")
    print("::[KSM Simulation]:: Analysis -END- ")
    print(" ")
