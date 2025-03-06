#include "SeperateNodes.h"
#include "mesh_entities.h"

#include <algorithm>
#include <cassert>

solidMesh MeshDuplicator::duplicateNodesHexa(
  const solidMesh &originalMesh,
  // solidMesh & newMesh,
  std::vector<nodeOrigin>& nodeOrigin
) {

  // get original element entities
  std::vector<solidElement> originalElements = originalMesh.elements;
  std::vector<point3D> originalPos = originalMesh.nodes;
  std::vector<interfaceElement> originalInterfaces = originalMesh.interfaces;

  // verify input
  assert(originalInterfaces.size() == 0);

  // Create output mesh
  solidMesh newMesh;
  newMesh.elements.clear();
  newMesh.nodes.clear();
  newMesh.interfaces.clear();
  newMesh.boundaries.clear();

  // Pre-allocate memory
  newMesh.elements.reserve(originalElements.size());
  newMesh.nodes.resize(8 * originalElements.size());

  // prepare nodeOrigin output 
  nodeOrigin.clear();
  nodeOrigin.reserve(8 * originalElements.size());

  // Step 1: Duplicate nodes
  int newnnode = 0;
  for (const auto& elem : originalElements) {
    solidElement newElem;
    for (int i = 0; i < 8; ++i) {
      nodeOrigin.push_back({ elem.nodes[i], static_cast<int>(newMesh.elements.size()), i });
      newElem.nodes[i] = newnnode++;
    }
    newMesh.elements.push_back(newElem);
  }

  // Step 2: Build face lookup
  std::unordered_map<std::array<int, 4>, std::pair<int, int>> faceLookup;
  buildFaceLookup(originalElements, faceLookup);

  // Step 3: Process faces
  std::vector<std::vector<bool>> paired(originalElements.size(), std::vector<bool>(6, false));
  for (int e = 0; e < originalElements.size(); ++e) {
    for (int face = 0; face < 6; ++face) {
      if (!paired[e][face]) {
        processFace(e, face, originalElements, faceLookup, paired, newMesh.elements, newMesh.interfaces, newMesh.boundaries);
      }
    }
  }

  // Step 4: Assign coordinates
  for (int i = 0; i < nodeOrigin.size(); ++i) {
    newMesh.nodes[i] = originalPos[nodeOrigin[i].originalNode];
  }

  return newMesh;
}

void MeshDuplicator::buildFaceLookup(
  const std::vector<solidElement>& elements,
  std::unordered_map<std::array<int, 4>, std::pair<int, int>>& lookup) {
  for (int e = 0; e < elements.size(); ++e) {
    for (int face = 0; face < 6; ++face) {
      std::array<int, 4> key;
      for (int i = 0; i < 4; ++i) {
        key[i] = elements[e].nodes[faceNodes[face][i]];
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
  const std::vector<solidElement>& originalElements,
  const std::unordered_map<std::array<int, 4>, std::pair<int, int>>& faceLookup,
  std::vector<std::vector<bool>>& paired,
  std::vector<solidElement>& newElements,
  std::vector<interfaceElement>& interfaceList,
  std::vector<elementFace>& boundaryList
) {
  const auto& elem = originalElements[e];
  std::array<int, 4> originalKey;
  for (int i = 0; i < 4; ++i) {
    originalKey[i] = elem.nodes[faceNodes[face][i]];
  }

  // Check permutations
  const std::array<std::array<int, 4>, 4> permutations = { {
      {3, 2, 1, 0},
      {2, 1, 0, 3},
      {1, 0, 3, 2},
      {0, 3, 2, 1}
  } };

  std::pair<int, int> foundPair(-1, -1);
  std::array<int, 4> foundPerm{ -1, -1, -1, -1 };
  for (const auto& perm : permutations) {
    std::array<int, 4> otherKey = { 
      originalKey[perm[0]], originalKey[perm[1]], originalKey[perm[2]], originalKey[perm[3]]};
    auto it = faceLookup.find(otherKey);
    if (it != faceLookup.end() && (it->second.first != e || it->second.second != face)) {
      foundPair = it->second;
      foundPerm = perm;
      break;
    }
  }

  if (foundPair.first == -1) {
    // Boundary face
    elementFace bf;
    bf.solidElemID = e;
    for (int i = 0; i < 4; ++i) {
      bf.nodes[i] = newElements[e].nodes[faceNodes[face][i]];
    }
    boundaryList.push_back(bf);
    newElements[e].faces[face].directionID = 0;

  }
  else {
    handleInternalFace(e, face, foundPair, foundPerm, originalElements, paired, newElements, interfaceList);
  }
}

void MeshDuplicator::handleInternalFace(
  int e,
  int face,
  const std::pair<int, int>& foundPair,
  const std::array<int, 4>& foundPerm,
  const std::vector<solidElement>& originalElements,
  std::vector<std::vector<bool>>& paired,
  std::vector<solidElement>& newElements,
  std::vector<interfaceElement>& interfaceList
) {
  int e2 = foundPair.first;
  int face2 = foundPair.second;
  if (paired[e2][face2]) return;

  paired[e][face] = true;
  paired[e2][face2] = true;

  interfaceElement ie;
  ie.solidID1 = e;
  ie.solidID2 = e2;
  ie.faceID1 = face;
  ie.faceID2 = face2;

  // Determine permutation
  const auto& otherFaceNodes = faceNodes[face2];
  const auto& thisFaecNodes = faceNodes[face];
  std::array<int, 4> otherOriginalNodes, thisOriginalNodes;
  for (int i = 0; i < 4; ++i) {
    thisOriginalNodes[i] = originalElements[e].nodes[thisFaecNodes[i]];
    otherOriginalNodes[i] = originalElements[e2].nodes[otherFaceNodes[foundPerm[i]]];
  }

  //std::array<int, 4> permutation;
  //const auto& originalKey = originalElements[e].nodes;
  //if (originalKey[faceNodes[face][0]] == otherOriginalNodes[3] &&
  //  originalKey[faceNodes[face][1]] == otherOriginalNodes[2]) {
  //  permutation = { 3, 2, 1, 0 };
  //}
  //else {
  //  permutation = { 0, 3, 2, 1 }; // Default case
  //}

  for (int i = 0; i < 4; ++i) {
    // add nodes data
    ie.nodes[i] = newElements[e].nodes[thisFaecNodes[i]];
    ie.nodes[i + 4] = newElements[e2].nodes[otherFaceNodes[foundPerm[i]]];

    // add face data
    ie.faces[0].nodes[i] = ie.nodes[i];
    ie.faces[1].nodes[i] = ie.nodes[i + 4];
  }

  // add remaining face data
  ie.faces[0].solidElemID = e;
  ie.faces[0].directionID = -1;

  ie.faces[1].solidElemID = e2;
  ie.faces[1].directionID = 1;

  // add interface element
  interfaceList.push_back(ie);
 
  // update solid element data
  newElements[e].faces[face].directionID = -1;
  newElements[e2].faces[face2].directionID = 1;

}