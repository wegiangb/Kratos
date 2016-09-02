import os

# Import Kratos
from KratosMultiphysics import *

# Import KratosUnittest
import KratosMultiphysics.KratosUnittest as KratosUnittest
import KratosExecuteMapperTest as ExecuteMapperTest

# This utiltiy will control the execution scope in case we need to acces files or we depend
# on specific relative locations of the files.

# TODO: Should we move this to KratosUnittest?
class controlledExecutionScope:
    def __init__(self, scope):
        self.currentPath = os.getcwd()
        self.scope = scope

    def __enter__(self):
        os.chdir(self.scope)

    def __exit__(self, type, value, traceback):
        os.chdir(self.currentPath)

class MapperTestFactory(KratosUnittest.TestCase):

    def setUp(self):
        # Within this location context:
        with controlledExecutionScope(os.path.dirname(os.path.realpath(__file__))):
            # Initialize GiD  I/O
            parameter_file = open(self.file_name + "_parameters.json", 'r')
            ProjectParameters = Parameters(parameter_file.read())

            # Creating the model part
            self.test = ExecuteMapperTest.KratosExecuteMapperTest(ProjectParameters)

    def test_execution(self):
        # Within this location context:
        with controlledExecutionScope(os.path.dirname(os.path.realpath(__file__))):
            self.test.Solve()

    def tearDown(self):
        pass
        

class NonConformantOneSideMap2D_test1(MapperTestFactory):
    file_name = "NonConformantOneSideMap2D_test1/NonConformantOneSideMap2D_test1"


#~ class NonConformantOneSideMap2D_test1(MapperTestFactory):
    #~ file_name = "NonConformantOneSideMap2D_test2/NonConformantOneSideMap2D_test2"


#~ class NonConformantOneSideMap2D_test1(MapperTestFactory):
    #~ file_name = "NonConformantOneSideMap3D_test1/NonConformantOneSideMap3D_test1"


#~ class NonConformantOneSideMap2D_test1(MapperTestFactory):
    #~ file_name = "NonConformantOneSideMap3D_test2/NonConformantOneSideMap3D_test2"
