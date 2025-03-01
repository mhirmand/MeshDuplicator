#include "SeperateNodes.h"
#include <fstream>
#include <iostream>

// Function to calculate the center of an element
template<typename T>
Point3D calculateElementCenter(const std::vector<Point3D>& points, T& element) {
  Point3D center = { 0.0, 0.0, 0.0 };
  for (int node : element.nNode) {
    center.x += points[node].x;
    center.y += points[node].y;
    center.z += points[node].z;
  }
  center.x /= 8.0;
  center.y /= 8.0;
  center.z /= 8.0;
  return center;
}

// Function to export mesh to VTK format for visualization
template<typename T>
void exportToVTK(const std::vector<Point3D>& points, T& elements, const char* filename, double shrinkFactor = 1.0) {
  std::ofstream file(filename);
  file << "# vtk DataFile Version 3.0\n";
  file << "Hexahedral Mesh\n";
  file << "ASCII\n";
  file << "DATASET UNSTRUCTURED_GRID\n";

  // make a copy of the nodes
  std::vector<Point3D> newPoints = points;

  // adjust the position of the nodes for shrinkage

  if (shrinkFactor != 1.0)
  {
    for (const auto& e : elements) {
      Point3D center = calculateElementCenter(points, e);
      for (int node : e.nNode) {
        newPoints[node].x = center.x + (newPoints[node].x - center.x) * shrinkFactor;
        newPoints[node].y = center.y + (newPoints[node].y - center.y) * shrinkFactor;
        newPoints[node].z = center.z + (newPoints[node].z - center.z) * shrinkFactor;
      }

    }
  }

  // Write points
  file << "POINTS " << newPoints.size() << " double\n";
  for (const auto& p : newPoints) {
    file << p.x << " " << p.y << " " << p.z << "\n";
  }

  // Write cells
  int totalSize = static_cast<int>(elements.size() * (1 + 8)); // 8 nodes per hex + cell type
  file << "CELLS " << elements.size() << " " << totalSize << "\n";
  for (const auto& e : elements) {
    file << "8 ";
    for (int node : e.nNode) file << node << " ";
    file << "\n";
  }

  // Write cell types
  file << "CELL_TYPES " << elements.size() << "\n";
  for (size_t i = 0; i < elements.size(); ++i) {
    file << "12\n"; // VTK_HEXAHEDRON
  }
}

int main() {

  // Create a simple cube mesh with 2 elements
  std::vector<Element> originalElements(2); // 2 elements
  std::vector<Point3D> originalPos(12); // 12 total nodes

  // Define node coordinates for a cube
	originalPos = 
  {
		{0,	0, 0 },
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
  std::vector<Element> newElements;
  std::vector<Point3D> newPos;
  std::vector<InterfaceElem> interfaces;
  std::vector<BoundaryFace> boundaries;
  std::vector<NodeOrigin> nodeOrigin;

  duplicator.duplicateNodesHexa(originalElements, originalPos, newElements, newPos, interfaces, boundaries, nodeOrigin);

  // Export results for visualization
  exportToVTK(originalPos, originalElements, "original_mesh.vtk");
  exportToVTK(newPos, newElements, "duplicated_mesh.vtk", 0.75);
  exportToVTK(newPos, interfaces, "interfaces.vtk", 0.75);
  std::cout << "Use ParaView or similar software to visualize the generated VTK files\n";

  return 0;
}