#pragma once

#ifndef VTKWRITER_H
#define VTKWRITER_H

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <filesystem>
#include <sstream>

// Function to calculate the center of an element
template<typename T>
point3D calculateElementCenter(const std::vector<point3D>& points, T& element) {
  point3D center = { 0.0, 0.0, 0.0 };
  for (int node : element.nodes) {
    center.x += points[node].x;
    center.y += points[node].y;
    center.z += points[node].z;
  }
  center.x /= 8.0;
  center.y /= 8.0;
  center.z /= 8.0;
  return center;
}

// Function to calculate the normal to an element face
point3D calculateFaceNormal(const std::vector<point3D>& points, const elementFace& face) {
  // Get the coordinates of the first three nodes of the face
  const point3D& p0 = points[face.nodes[0]];
  const point3D& p1 = points[face.nodes[1]];
  const point3D& p2 = points[face.nodes[2]];

  // Calculate two edge vectors of the face
  point3D edge1 = { p1.x - p0.x, p1.y - p0.y, p1.z - p0.z };
  point3D edge2 = { p2.x - p0.x, p2.y - p0.y, p2.z - p0.z };

  // Calculate the cross product of the two edge vectors to get the normal
  point3D normal;
  normal.x = edge1.y * edge2.z - edge1.z * edge2.y; // x-component
  normal.y = edge1.z * edge2.x - edge1.x * edge2.z; // y-component
  normal.z = edge1.x * edge2.y - edge1.y * edge2.x; // z-component

  // Normalize the normal vector
  double magnitude = std::sqrt(normal.x* normal.x + normal.y * normal.y + normal.z * normal.z);
  if (magnitude > 0) {
    normal.x /= magnitude;
    normal.y /= magnitude;
    normal.z /= magnitude;
  }
  else {
    normal = { 0.0, 0.0, 0.0 };
  }

  return normal;
}

// Function to calculate the normal to an element face
template<typename T>
double calculateElementSize(const std::vector<point3D>& points, T& element) {

  // Define the 12 edges of a hexahedral element
  const std::array<std::pair<int, int>, 12> edges = { 
    {
      {0, 1}, {1, 2}, {2, 3}, {3, 0}, // Bottom face
      {4, 5}, {5, 6}, {6, 7}, {7, 4}, // Top face
      {0, 4}, {1, 5}, {2, 6}, {3, 7}  // Vertical edges
    } 
  };

  // Calculate the length of each edge and accumulate the total length
  double totalLength = 0.0;
  for (const auto& edge : edges) {
    const point3D& p1 = points[element.nodes[edge.first]];
    const point3D& p2 = points[element.nodes[edge.second]];
    double dx = p2.x - p1.x;
    double dy = p2.y - p1.y;
    double dz = p2.z - p1.z;
    totalLength += std::sqrt(dx * dx + dy * dy + dz * dz);
  }

  // Return the average edge length
  return totalLength / 12.0;
}


// Function to export solid elemenets to VTK format for visualization
void exportSolidElementsToVTK(solidMesh mesh, const std::string filename, double shrinkFactor = 1.0) {
  std::ofstream file(filename);
  file << "# vtk DataFile Version 3.0\n";
  file << "Hexahedral Mesh\n";
  file << "ASCII\n";
  file << "DATASET UNSTRUCTURED_GRID\n";

  // make a copy of the nodes
  std::vector<point3D> newPoints = mesh.nodes;

  // adjust the position of the nodes for shrinkage
  if (shrinkFactor != 1.0 && mesh.interfaces.size() > 0)
  {
    for (const auto& e : mesh.elements) {
      point3D center = calculateElementCenter(mesh.nodes, e);
      for (int node : e.nodes) {
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
  int totalSize = static_cast<int>(mesh.elements.size() * (1 + 8)); // 8 nodes per hex + cell type
  file << "CELLS " << mesh.elements.size() << " " << totalSize << "\n";
  for (const auto& e : mesh.elements) {
    file << "8 ";
    for (int node : e.nodes) file << node << " ";
    file << "\n";
  }

  // Write cell types
  file << "CELL_TYPES " << mesh.elements.size() << "\n";
  for (size_t i = 0; i < mesh.elements.size(); ++i) {
    file << "12\n"; // VTK_HEXAHEDRON
  }

  // Write scalar data (e.g., a double value for each node)
  file << "POINT_DATA " << newPoints.size() << "\n";
  file << "SCALARS ele_type int 1\n";
  file << "LOOKUP_TABLE default\n";
  for (const auto& p : newPoints) {
    double scalarValue = 0.0; // Example: Assume each Point3D has a `scalarValue` member
    file << scalarValue << "\n";
  }
}

// Function to export solid elemenets to VTK format for visualization
void exportInterfaceElementsToVTK(solidMesh mesh, const std::string filename, double shrinkFactor = 1.0, double expansionFactor = 0.0) {
  std::ofstream file(filename);
  file << "# vtk DataFile Version 3.0\n";
  file << "Hexahedral Mesh\n";
  file << "ASCII\n";
  file << "DATASET UNSTRUCTURED_GRID\n";

  // make a copy of the nodes
  std::vector<point3D> newPoints = mesh.nodes;

  // adjust the position of the nodes for interface expansion
  if (mesh.interfaces.size() > 0) {
    for (const auto& e : mesh.interfaces) {
      auto center = calculateElementCenter(newPoints, e);

			// apply shirnkage.
			if (shrinkFactor != 1.0) {
				for (int node : e.nodes) {
					newPoints[node].x = center.x + (newPoints[node].x - center.x) * shrinkFactor;
					newPoints[node].y = center.y + (newPoints[node].y - center.y) * shrinkFactor;
					newPoints[node].z = center.z + (newPoints[node].z - center.z) * shrinkFactor;
				}
			}

      // apply expansion.
      if (expansionFactor > 0.0) {
        for (const auto& f : e.faces) {
          auto normal = calculateFaceNormal(newPoints, f);
          double lSolidEle = calculateElementSize(newPoints, mesh.elements[f.solidElemID]);
          for (int node : f.nodes) {
            newPoints[node].x += normal.x * (double)(f.directionID) * 0.5 * lSolidEle * expansionFactor;
            newPoints[node].y += normal.y * (double)(f.directionID) * 0.5 * lSolidEle * expansionFactor;
            newPoints[node].z += normal.z * (double)(f.directionID) * 0.5 * lSolidEle * expansionFactor;
          }
        }
      }
    }
  }

  // Write points
  file << "POINTS " << newPoints.size() << " double\n";
  for (const auto& p : newPoints) {
    file << p.x << " " << p.y << " " << p.z << "\n";
  }

  // Write cells
  int totalSize = static_cast<int>(mesh.interfaces.size() * (1 + 8)); // 8 nodes per interface + cell type
  file << "CELLS " << mesh.interfaces.size() << " " << totalSize << "\n";
  for (const auto& e : mesh.interfaces) {
    file << "8 ";
    for (int node : e.nodes) file << node << " ";
    file << "\n";
  }

  // Write cell types
  file << "CELL_TYPES " << mesh.interfaces.size() << "\n";
  for (size_t i = 0; i < mesh.interfaces.size(); ++i) {
    file << "12\n"; // VTK_HEXAHEDRON
  }

  // Write scalar data (e.g., a double value for each node)
  file << "POINT_DATA " << newPoints.size() << "\n"; 
  file << "SCALARS ele_type int 1\n";         
  file << "LOOKUP_TABLE default\n";
  for (const auto& p : newPoints) {
    double scalarValue = 1.0; // Example: Assume each Point3D has a `scalarValue` member
    file << scalarValue << "\n";
  }
}


// Function to read mesh data from a text file
solidMesh readMeshFromFile(const std::string& filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + filename);
  }

  solidMesh mesh;

  // Read number of nodes
  int numNodes;
  file >> numNodes;
  mesh.nodes.resize(numNodes);

  // Read node coordinates
  for (int i = 0; i < numNodes; ++i) {
    file >> mesh.nodes[i].x >> mesh.nodes[i].y >> mesh.nodes[i].z;
  }

  // Read number of elements
  int numElements;
  file >> numElements;
  mesh.elements.resize(numElements);

  // Read element connectivity
  for (int i = 0; i < numElements; ++i) {
    for (int j = 0; j < 8; ++j) {
      file >> mesh.elements[i].nodes[j];
      mesh.elements[i].nodes[j]--;
    }
  }

  return mesh;
}

// Function to extract the base name of a file (without extension)
std::string getBaseName(const std::string& filename) {
  std::filesystem::path path(filename);
  return path.stem().string(); 
}

#endif
