#pragma once

#ifndef MESH_DUPLICATOR_H
#define MESH_DUPLICATOR_H

#include <vector>
#include <array>
#include <unordered_map>
#include <stdexcept>

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
  std::array<int, 4> nNode = { 0, 0, 0, 0 };
};

struct Element {
  std::array<int, 8> nNode;
  std::array<Face, 6> faces;
};

struct InterfaceElem {
  int solidID1, solidID2;
  int faceID1, faceID2;
  std::array<int, 8> nNode;
};

struct BoundaryFace {
  std::array<int, 4> nNode;
};

// Hash function for std::array<int, 4> to use in unordered_map
namespace std {
  template<> struct hash<std::array<int, 4>> {
    size_t operator()(const std::array<int, 4>& arr) const {
      size_t h = 0;
      for (int val : arr) h ^= hash<int>()(val) + 0x9e3779b9 + (h << 6) + (h >> 2);
      return h;
    }
  };
}

class MeshDuplicator {
public:
  void duplicateNodesHexa(
    const std::vector<Element>& originalElements,
    const std::vector<Point3D>& originalPos,
    std::vector<Element>& newElements,
    std::vector<Point3D>& newPos,
    std::vector<InterfaceElem>& interfaceList,
    std::vector<BoundaryFace>& boundaryList,
    std::vector<NodeOrigin>& nodeOrigin
  );

private:
  static constexpr std::array<std::array<int, 4>, 6> faceNodes = { {
      {{0, 3, 2, 1}}, {{4, 5, 6, 7}}, {{0, 1, 5, 4}},
      {{3, 7, 6, 2}}, {{0, 4, 7, 3}}, {{1, 2, 6, 5}}
  } };

  void buildFaceLookup(
    const std::vector<Element>& elements,
    std::unordered_map<std::array<int, 4>, std::pair<int, int>>& lookup
  );

  void processFace(
    int e,
    int face,
    const std::vector<Element>& originalElements,
    const std::unordered_map<std::array<int, 4>, std::pair<int, int>>& faceLookup,
    std::vector<std::vector<bool>>& paired,
    std::vector<Element>& newElements,
    std::vector<InterfaceElem>& interfaceList,
    std::vector<BoundaryFace>& boundaryList
  );

  void handleInternalFace(
    int e,
    int face,
    const std::pair<int, int>& foundPair,
    const std::vector<Element>& originalElements,
    std::vector<std::vector<bool>>& paired,
    std::vector<Element>& newElements,
    std::vector<InterfaceElem>& interfaceList
  );
};

#endif