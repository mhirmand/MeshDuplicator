# Hexahedral Mesh Node Duplication and Interface Generation
This project implements an algorithm to duplicate nodes in a hexahedral mesh such that no two elements share nodes. It also generates interface elements between adjacent faces and identifies boundary faces. The algorithm is implemented in C++ and is designed to work with hexahedral meshes, ensuring that the resulting mesh is suitable for applications requiring non-conforming meshes or interface elements.

## How it works

- **Node Duplication:** Duplicates nodes for each element in the mesh, ensuring no shared nodes between elements.

- **Interface Element Generation:** Creates interface elements between adjacent faces of neighboring elements.

- **Boundary Face Identification:** Identifies and stores boundary faces that do not have adjacent neighbors.

- **VTK Export:** Exports the resulting mesh to a VTK file for visualization in tools like ParaView.