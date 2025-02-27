#include <vector>
#include <array>
#include <map>
#include <stdexcept>
#include <algorithm>

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
  int numNodes = 8;
  int solidID1, solidID2;
  int faceID1, faceID2;
  std::array<int, 8> nNode;
};

struct BoundaryFace {
  std::array<int, 4> nNode;
};

class MeshDuplicator {
public:
  const std::array<std::array<int, 4>, 6> faceNodes = { {
      {{0, 3, 2, 1}}, // Bottom face (z = -1)
      {{4, 5, 6, 7}}, // Top face (z = +1)
      {{0, 1, 5, 4}}, // Front face (x = +1)
      {{3, 7, 6, 2}}, // Back face (x = -1)
      {{0, 4, 7, 3}}, // Left face (y = -1)
      {{1, 2, 6, 5}}  // Right face (y = +1)
  } };

  void duplicateNodesHexa(
    const std::vector<Element>& originalElements,
    const std::vector<Point3D>& originalPos,
    std::vector<Element>& newElements,
    std::vector<Point3D>& newPos,
    std::vector<InterfaceElem>& interfaceList,
    std::vector<BoundaryFace>& boundaryList,
    std::vector<NodeOrigin>& nodeOrigin) {

    // Prepare output arrays
    newElements.clear();
    nodeOrigin.clear();
    newPos.clear();
    interfaceList.clear();
    boundaryList.clear();

    // reserve sizes (interfaceList and boundaryList size are not known in advance).
    newElements.reserve(originalElements.size());
    nodeOrigin.reserve(8 * originalElements.size());
    newPos.reserve(8 * originalElements.size()); 

    // Step 1: Duplicate nodes and create new elements
    int newnnode = 0;
    for (const auto& elem : originalElements) {
      Element newElem;
      for (int i = 0; i < 8; ++i) {
        nodeOrigin.push_back({ elem.nNode[i], static_cast<int>(newElements.size()), i });
        newElem.nNode[i] = newnnode++;
      }
      newElements.push_back(newElem);
    }

    // Step 2: Build face lookup using original elements
    std::map<std::array<int, 4>, std::pair<int, int>> faceLookup;
    for (int e = 0; e < originalElements.size(); ++e) {
      const auto& elem = originalElements[e];
      for (int face = 0; face < 6; ++face) {
        const auto& fNodes = faceNodes[face];
        std::array<int, 4> key;
        for (int i = 0; i < 4; ++i) {
          key[i] = elem.nNode[fNodes[i]];
        }
        if (faceLookup.find(key) != faceLookup.end()) {
          throw std::runtime_error("Duplicate face key found in face lookup.");
        }
        faceLookup[key] = { e, face };
      }
    }

    // Step 3: Identify adjacent faces and create interface elements
    std::vector<std::vector<bool>> paired(originalElements.size(), std::vector<bool>(6, false));

    for (int e = 0; e < originalElements.size(); ++e) {
      for (int face = 0; face < 6; ++face) {

        // skip if the face is already processed.
        if (paired[e][face]) continue;

        const auto& elem = originalElements[e];
        const auto& fNodes = faceNodes[face];
        std::array<int, 4> originalKey;
        for (int i = 0; i < 4; ++i) {
          originalKey[i] = elem.nNode[fNodes[i]];
        }

        // Generate other key permutations
        std::array<int, 4> otherKeys[4] = {
            {originalKey[3], originalKey[2], originalKey[1], originalKey[0]},
            {originalKey[2], originalKey[1], originalKey[0], originalKey[3]},
            {originalKey[1], originalKey[0], originalKey[3], originalKey[2]},
            {originalKey[0], originalKey[3], originalKey[2], originalKey[1]}
        };

        std::pair<int, int> foundPair(-1, -1);
        for (const auto& otherKey : otherKeys) {
          auto it = faceLookup.find(otherKey);
          if (it != faceLookup.end() && (it->second.first != e || it->second.second != face)) {
            foundPair = it->second;
            break;
          }
        }

        if (foundPair.first == -1) { // no other face was found --> create boudary face
          // Boundary face
          BoundaryFace bf;
          for (int i = 0; i < 4; ++i) {
            bf.nNode[i] = newElements[e].nNode[fNodes[i]];
          }
          boundaryList.push_back(bf);
          newElements[e].faces[face].directionID = 0;
        }
        else { // Internal face --> create interface element
          
          int e2 = foundPair.first;
          int face2 = foundPair.second;

          // skip if the other face is already processed
          if (paired[e2][face2]) continue;

          // update processed faces
          paired[e][face] = true;
          paired[e2][face2] = true;

          // create interface elements
          InterfaceElem ie;
          ie.solidID1 = e;
          ie.solidID2 = e2;
          ie.faceID1 = face;
          ie.faceID2 = face2;

          // Determine node order for the adjacent face
          const auto& otherFaceNodes = faceNodes[face2];
          std::array<int, 4> otherOriginalNodes;
          for (int i = 0; i < 4; ++i) {
            otherOriginalNodes[i] = originalElements[e2].nNode[otherFaceNodes[i]];
          }

          std::array<int, 4> permutation;
          if (originalKey[0] == otherOriginalNodes[3] && originalKey[1] == otherOriginalNodes[2] &&
            originalKey[2] == otherOriginalNodes[1] && originalKey[3] == otherOriginalNodes[0]) {
            permutation = { 3, 2, 1, 0 };
          }
          else if (originalKey[0] == otherOriginalNodes[2] && originalKey[1] == otherOriginalNodes[1] &&
            originalKey[2] == otherOriginalNodes[0] && originalKey[3] == otherOriginalNodes[3]) {
            permutation = { 2, 1, 0, 3 };
          }
          else if (originalKey[0] == otherOriginalNodes[1] && originalKey[1] == otherOriginalNodes[0] &&
            originalKey[2] == otherOriginalNodes[3] && originalKey[3] == otherOriginalNodes[2]) {
            permutation = { 1, 0, 3, 2 };
          }
          else {
            permutation = { 0, 3, 2, 1 };
          }

          for (int i = 0; i < 4; ++i) {
            ie.nNode[i] = newElements[e].nNode[fNodes[i]];
            ie.nNode[i + 4] = newElements[e2].nNode[otherFaceNodes[permutation[i]]];
          }

          interfaceList.push_back(ie);

          // Update faces in newElements
          // assign direction of the faces(first or the second side of the interface)
          // nodes on the  FIRST edge has a CCW ordering when looked at from outside of the element
          // nodes on the SECOND edge has a CW ordering when looked at from outside of the element
          newElements[e].faces[face].directionID = -1;  // -1 edge is the negative side of the interface (i.e., the 1st side)
          newElements[e2].faces[face2].directionID = 1; //+1 edge is the positive side of the interface(i.e., the 2nd side)
        }
      }
    }

    // Step 4: Create newPos
    for (int i = 0; i < nodeOrigin.size(); ++i) {
      newPos[i] = originalPos[nodeOrigin[i].originalNodeID];
    }
  }
};

