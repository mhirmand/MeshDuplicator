#include "SeperateNodes.h"
#include "VTKWriter.h"
#include <iostream>


int testCaseCube()
{
  // Create a simple cube mesh with 2 elements
  std::vector<solidElement> originalElements(2); // 2 elements
  std::vector<point3D> originalPos(12); // 12 total nodes

  // Define node coordinates for a cube
  originalPos =
  {
    {0,	0, 0},
    {1,	0, 0},
    {1,	1, 0},
    {0,	1, 0},
    {0,	0, 1},
    {1,	0, 1},
    {1,	1, 1},
    {0,	1, 1},
    {2,	0, 0},
    {2,	1, 0},
    {2,	0, 1},
    {2,	1, 1}
  };

  // Define element connectivity
  originalElements[0].nodes = { 0, 1, 2, 3, 4, 5, 6, 7 };
  originalElements[1].nodes = { 1, 8,	9, 2,	5, 10, 11, 6 };

  MeshDuplicator duplicator;
  solidMesh originalMesh;
  originalMesh.elements = originalElements;
  originalMesh.nodes = originalPos;

  // prepare nodeOrigin data structure
  std::vector<nodeOrigin> nodeOrigin;

  // evluate new mesh with seperated elements
  solidMesh newMesh = duplicator.duplicateNodesHexa(originalMesh, nodeOrigin);

  // Export results for visualization
  exportElementsToVTK(originalMesh.nodes, originalMesh.elements, "test_cube_original_mesh.vtk", 1.0, 2.0);
  exportElementsToVTK(newMesh.nodes, newMesh.elements, "test_cube_duplicated_mesh.vtk", 0.75, 0.0);
  exportElementsToVTK(newMesh.nodes, newMesh.interfaces, "test_cube_interfaces.vtk", 1.0, 1.0);

  std::cout << "Unit Cube Test Case: Generated VTK files:\n";
  std::cout << "  - " << "test_cube_original_mesh.vtk" << "\n";
  std::cout << "  - " << "test_cube_duplicated_mesh.vtk" << "\n";
  std::cout << "  - " << "test_cube_interfaces.vtk" << "\n";

  return 0;
}

int testCaseGeneral(const std::string& filename)
{

  // Extract the base name of the input file
  std::string baseName = getBaseName(filename);

  // Read mesh data from file
  solidMesh originalMesh;
  try {
    originalMesh = readMeshFromFile(filename);
  }
  catch (const std::exception& e) {
    std::cerr << "Error reading mesh file: " << e.what() << "\n";
    return 1;
  }

  // Prepare nodeOrigin data structure
  std::vector<nodeOrigin> nodeOrigin;

  // Evaluate new mesh with separated elements
  MeshDuplicator duplicator;
  solidMesh newMesh = duplicator.duplicateNodesHexa(originalMesh, nodeOrigin);

  // Construct output file names based on the input file name
  std::string originalMeshFile = baseName + "_original_mesh.vtk";
  std::string duplicatedMeshFile = baseName + "_duplicated_mesh.vtk";
  std::string interfacesFile = baseName + "_interfaces.vtk";

  // Export results for visualization
  exportElementsToVTK(originalMesh.nodes, originalMesh.elements, originalMeshFile, 1.0, 2.0);
  exportElementsToVTK(newMesh.nodes, newMesh.elements, duplicatedMeshFile, 0.75, 0.0);
  exportElementsToVTK(newMesh.nodes, newMesh.interfaces, interfacesFile, 1.0, 1.0);

  std::cout << "Test Case" << baseName << ": Generated VTK files: \n";
  std::cout << "  - " << originalMeshFile << "\n";
  std::cout << "  - " << duplicatedMeshFile << "\n";
  std::cout << "  - " << interfacesFile << "\n";

  return 0;
}

// main entry point
int main() {


  int test1 = testCaseCube();
  int test2 = testCaseGeneral("test_1.txt");
  int test3 = testCaseGeneral("test_2.txt");

  std::cout << "Use ParaView or similar software to visualize the generated VTK files\n";

  return 0;
}