#pragma once

#ifndef MESH_ENTITIES_H
#define MESH_ENTITIES_H

#include <vector>
#include <array>

struct Point3D {
  double x, y, z;
};

struct NodeOrigin {
  int originalNodeID;
  int elementID;
  int localIndex;
};

struct Face {
  int directionID = 0;
  std::array<int, 4> nNode{ -1 };
};

struct Element {
  std::array<int, 8> nNode{ -1 };
  std::array<Face, 6> faces{ -1 };
};

struct InterfaceElem {
  int solidID1 = -1, solidID2 = -1;
  int faceID1 = -1, faceID2 = -1;
  std::array<int, 8> nNode{ -1 };
};

struct BoundaryFace {
  std::array<int, 4> nNode{ -1 };
};

struct Mesh {
  std::vector<Point3D> nodes;
  std::vector<Element> elements;
  std::vector<InterfaceElem> interfaces;
  std::vector<BoundaryFace> boundaries;
};

#endif MESH_ENTITIES_H