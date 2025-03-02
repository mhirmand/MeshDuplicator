#include "SeperateNodes.h"
#include "VTKWriter.h"
#include <iostream>


int main() {

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
  originalElements[0].nodes = {0, 1, 2, 3, 4, 5, 6, 7};
  originalElements[1].nodes = {1, 8,	9, 2,	5, 10, 11, 6};

  MeshDuplicator duplicator;
  solidMesh originalMesh;
  originalMesh.elements = originalElements;
  originalMesh.nodes = originalPos;

  // prepare nodeOrigin data structure
  std::vector<nodeOrigin> nodeOrigin;

  // evluate new mesh with seperated elements
  solidMesh newMesh = duplicator.duplicateNodesHexa(originalMesh, nodeOrigin);

  // Export results for visualization
  exportSolidElementsToVTK(originalMesh, "original_mesh.vtk");
  exportSolidElementsToVTK(newMesh, "duplicated_mesh.vtk", 0.85);
  exportInterfaceElementsToVTK(newMesh, "interfaces.vtk", 0.85, 0.02);
  std::cout << "Use ParaView or similar software to visualize the generated VTK files\n";

  return 0;
}