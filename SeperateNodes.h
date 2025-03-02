#pragma once

#ifndef SEPERATENODES_H
#define SEPERATENODES_H

#include "mesh_entities.h"

#include <vector>
#include <array>
#include <unordered_map>
#include <stdexcept>

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
  solidMesh duplicateNodesHexa(
    const solidMesh& originalMesh,
    // solidMesh& newMesh,
    std::vector<nodeOrigin>& nodeOrigin
  );

private:
  static constexpr std::array<std::array<int, 4>, 6> faceNodes = { {
      {{0, 3, 2, 1}}, {{4, 5, 6, 7}}, {{0, 1, 5, 4}},
      {{3, 7, 6, 2}}, {{0, 4, 7, 3}}, {{1, 2, 6, 5}}
  } };

  void buildFaceLookup(
    const std::vector<solidElement>& elements,
    std::unordered_map<std::array<int, 4>, std::pair<int, int>>& lookup
  );

  void processFace(
    int e,
    int face,
    const std::vector<solidElement>& originalElements,
    const std::unordered_map<std::array<int, 4>, std::pair<int, int>>& faceLookup,
    std::vector<std::vector<bool>>& paired,
    std::vector<solidElement>& newElements,
    std::vector<interfaceElement>& interfaceList,
    std::vector<elementFace>& boundaryList
  );

  void handleInternalFace(
    int e,
    int face,
    const std::pair<int, int>& foundPair,
    const std::vector<solidElement>& originalElements,
    std::vector<std::vector<bool>>& paired,
    std::vector<solidElement>& newElements,
    std::vector<interfaceElement>& interfaceList
  );
};

#endif