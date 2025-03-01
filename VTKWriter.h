#pragma once

#ifndef VTKWRITER_H
#define VTKWRITER_H

#include <iostream>
#include <fstream>

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

#endif
