#include "SeperateNodes.h"
#include "VTKWriter.h"
#include <iostream>


int main() {

  // Create a simple cube mesh with 2 elements
  std::vector<Element> originalElements(2); // 2 elements
  std::vector<Point3D> originalPos(12); // 12 total nodes

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
  originalElements[0].nNode = {0, 1, 2, 3, 4, 5, 6, 7};
  originalElements[1].nNode = {1, 8,	9, 2,	5, 10, 11, 6};

  MeshDuplicator duplicator;
  Mesh originalMesh;
  originalMesh.elements = originalElements;
  originalMesh.nodes = originalPos;

  Mesh newMesh;
  std::vector<NodeOrigin> nodeOrigin;

  duplicator.duplicateNodesHexa(originalMesh, newMesh, nodeOrigin);

  // Export results for visualization
  exportToVTK(originalPos, originalElements, "original_mesh.vtk");
  exportToVTK(newMesh.nodes, newMesh.elements, "duplicated_mesh.vtk", 0.75);
  exportToVTK(newMesh.nodes, newMesh.interfaces, "interfaces.vtk", 0.75);
  std::cout << "Use ParaView or similar software to visualize the generated VTK files\n";

  return 0;
}