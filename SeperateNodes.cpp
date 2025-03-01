#include "SeperateNodes.h"
#include <algorithm>

void MeshDuplicator::duplicateNodesHexa(
  const std::vector<Element>& originalElements,
  const std::vector<Point3D>& originalPos,
  std::vector<Element>& newElements,
  std::vector<Point3D>& newPos,
  std::vector<InterfaceElem>& interfaceList,
  std::vector<BoundaryFace>& boundaryList,
  std::vector<NodeOrigin>& nodeOrigin
) {
  // Reset output containers
  newElements.clear();
  nodeOrigin.clear();
  newPos.clear();
  interfaceList.clear();
  boundaryList.clear();

  // Pre-allocate memory
  newElements.reserve(originalElements.size());
  nodeOrigin.reserve(8 * originalElements.size());
  newPos.resize(8 * originalElements.size());

  // Step 1: Duplicate nodes
  int newnnode = 0;
  for (const auto& elem : originalElements) {
    Element newElem;
    for (int i = 0; i < 8; ++i) {
      nodeOrigin.push_back({ elem.nNode[i], static_cast<int>(newElements.size()), i });
      newElem.nNode[i] = newnnode++;
    }
    newElements.push_back(newElem);
  }

  // Step 2: Build face lookup
  std::unordered_map<std::array<int, 4>, std::pair<int, int>> faceLookup;
  buildFaceLookup(originalElements, faceLookup);

  // Step 3: Process faces
  std::vector<std::vector<bool>> paired(originalElements.size(), std::vector<bool>(6, false));
  for (int e = 0; e < originalElements.size(); ++e) {
    for (int face = 0; face < 6; ++face) {
      if (!paired[e][face]) {
        processFace(e, face, originalElements, faceLookup, paired, newElements, interfaceList, boundaryList);
      }
    }
  }

  // Step 4: Assign coordinates
  for (int i = 0; i < nodeOrigin.size(); ++i) {
    newPos[i] = originalPos[nodeOrigin[i].originalNodeID];
  }
}

void MeshDuplicator::buildFaceLookup(
  const std::vector<Element>& elements,
  std::unordered_map<std::array<int, 4>, std::pair<int, int>>& lookup) {
  for (int e = 0; e < elements.size(); ++e) {
    for (int face = 0; face < 6; ++face) {
      std::array<int, 4> key;
      for (int i = 0; i < 4; ++i) {
        key[i] = elements[e].nNode[faceNodes[face][i]];
      }
      if (lookup.find(key) != lookup.end()) {
        throw std::runtime_error("Duplicate face key in face lookup");
      }
      lookup[key] = { e, face };
    }
  }
}

void MeshDuplicator::processFace(
  int e,
  int face,
  const std::vector<Element>& originalElements,
  const std::unordered_map<std::array<int, 4>, std::pair<int, int>>& faceLookup,
  std::vector<std::vector<bool>>& paired,
  std::vector<Element>& newElements,
  std::vector<InterfaceElem>& interfaceList,
  std::vector<BoundaryFace>& boundaryList
) {
  const auto& elem = originalElements[e];
  std::array<int, 4> originalKey;
  for (int i = 0; i < 4; ++i) {
    originalKey[i] = elem.nNode[faceNodes[face][i]];
  }

  // Check permutations
  const std::array<std::array<int, 4>, 4> otherKeys = { {
      {originalKey[3], originalKey[2], originalKey[1], originalKey[0]},
      {originalKey[2], originalKey[1], originalKey[0], originalKey[3]},
      {originalKey[1], originalKey[0], originalKey[3], originalKey[2]},
      {originalKey[0], originalKey[3], originalKey[2], originalKey[1]}
  } };

  std::pair<int, int> foundPair(-1, -1);
  for (const auto& key : otherKeys) {
    auto it = faceLookup.find(key);
    if (it != faceLookup.end() && (it->second.first != e || it->second.second != face)) {
      foundPair = it->second;
      break;
    }
  }

  if (foundPair.first == -1) {
    // Boundary face
    BoundaryFace bf;
    for (int i = 0; i < 4; ++i) {
      bf.nNode[i] = newElements[e].nNode[faceNodes[face][i]];
    }
    boundaryList.push_back(bf);
    newElements[e].faces[face].directionID = 0;
  }
  else {
    handleInternalFace(e, face, foundPair, originalElements, paired, newElements, interfaceList);
  }
}

void MeshDuplicator::handleInternalFace(
  int e,
  int face,
  const std::pair<int, int>& foundPair,
  const std::vector<Element>& originalElements,
  std::vector<std::vector<bool>>& paired,
  std::vector<Element>& newElements,
  std::vector<InterfaceElem>& interfaceList
) {
  int e2 = foundPair.first;
  int face2 = foundPair.second;
  if (paired[e2][face2]) return;

  paired[e][face] = true;
  paired[e2][face2] = true;

  InterfaceElem ie;
  ie.solidID1 = e;
  ie.solidID2 = e2;
  ie.faceID1 = face;
  ie.faceID2 = face2;

  // Determine permutation
  const auto& otherFaceNodes = faceNodes[face2];
  std::array<int, 4> otherOriginalNodes;
  for (int i = 0; i < 4; ++i) {
    otherOriginalNodes[i] = originalElements[e2].nNode[otherFaceNodes[i]];
  }

  std::array<int, 4> permutation;
  const auto& originalKey = originalElements[e].nNode;
  if (originalKey[faceNodes[face][0]] == otherOriginalNodes[3] &&
    originalKey[faceNodes[face][1]] == otherOriginalNodes[2]) {
    permutation = { 3, 2, 1, 0 };
  }
  else {
    permutation = { 0, 3, 2, 1 }; // Default case
  }

  for (int i = 0; i < 4; ++i) {
    ie.nNode[i] = newElements[e].nNode[faceNodes[face][i]];
    ie.nNode[i + 4] = newElements[e2].nNode[otherFaceNodes[permutation[i]]];
  }

  interfaceList.push_back(ie);
  newElements[e].faces[face].directionID = -1;
  newElements[e2].faces[face2].directionID = 1;
}