#pragma once

#ifndef MESH_ENTITIES_H
#define MESH_ENTITIES_H

#include <vector>
#include <array>

// parent element struct
struct element
{
  std::array<int, 8> nodes{ -1, -1, -1, -1, -1, -1, -1, -1 };
};

// point in 3D struct
struct point3D {
  double x, y, z;
};

// face (element edge) struct
struct elementFace {
  int directionID = 0;
  int solidElemID = -1;
  std::array<int, 4> nodes{ -1, -1, -1, -1 };  // each element face in hex mesh has 4 nodes
};

// struct for a solid element
struct solidElement : element {
  std::array<elementFace, 6> faces{ -1, -1, -1, -1, -1, -1 }; // hex elemenet has 6 faces
};

// struct for an interface element
struct interfaceElement : element {
  std::array<elementFace, 2> faces{ -1 };  // interface element has two faces
  int solidID1 = -1, solidID2 = -1;
  int faceID1 = -1, faceID2 = -1;
};

// struct for a solid mesh
struct solidMesh {
  std::vector<point3D> nodes;
  std::vector<solidElement> elements;
  std::vector<interfaceElement> interfaces;
  std::vector<elementFace> boundaries;
};

// node origin struct
struct nodeOrigin {
  int originalNode;    // original node of a duplicated node
  int Originalelement; // original element the original node belonged to
  int localNodeIndex;  // local index of the original node in the original element
};

#endif MESH_ENTITIES_H