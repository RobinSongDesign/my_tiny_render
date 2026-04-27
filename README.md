## 📐 Lesson 2: Rasterization & Perspective
- **Z-buffer Depth Testing**: Solved the "overlapping triangles" issue by maintaining a depth buffer. Used `double` precision to prevent Z-fighting and ensured the buffer was initialized to negative infinity.
- **Barycentric Coordinates**: Implemented a robust triangle filling algorithm. Used barycentric interpolation for $z$, UV coordinates, and light intensity, ensuring smooth transitions across triangle faces.
- **Matrix-Based Pipeline**: 
    - Transitioned from hard-coded scaling to a formal **$4 \times 4$ Matrix** transformation.
    - Implemented **Viewport Matrix** to handle screen space mapping.
    - Added **Perspective Projection**: Introduced homogeneous coordinates $(x, y, z, w)$ and the crucial **Perspective Division** ($1/w$) to achieve a realistic "near-big, far-small" effect.
- **Shader-like Interpolation**: Achieved **Gouraud Shading** by interpolating vertex normals, significantly reducing the "faceted" look of the low-poly model.

## 🛠 Bug Fixes & Optimizations
- **Precision Trap**: Fixed "fragmented" renders caused by integer truncation during bounding box iteration. Switched to `int` for pixel looping and `double` for barycentric math.
- **Index Mismatch**: Resolved Z-buffer corruption by unifying the indexing formula ($x + y \times \text{width}$) across all rendering stages.
- **Perspective Order**: Fixed distorted meshes by ensuring the Perspective Projection occurs *before* Viewport mapping and proper $w$-division is applied.

## 📸 Current Output
| Render Mode | Screenshot |
| :--- | :--- |
| **Perspective + Texture** | ![Perspective Render](/screenshots/africanhead_perspective.png) |

# Lesson 1
-Integer Division: Using int for slope $k$ caused truncation (resulting in $k=0$). Fixed by casting to float.
-Point Ordering: Drawing from right to left ($x_0 > x_1$) failed. Fixed by swapping start/end points as a pair: if (x0 > x1) std::swap(p0, p1).
-Steep Line Discontinuity: Lines with $|dy| > |dx|$ appeared as sparse dots. Resolved by transposing coordinates ($x, y$ swap) to iterate along the longer axis.
-Viewport Mapping: .obj coordinates are normalized $[-1, 1]$. Implemented scaling logic to map them to $[0, \text{width/height}]$ screen space.Overdrawing: Shared edges between triangles are drawn twice; without a Z-buffer, the last color drawn always wins (overwriting previous pixels).

## 📸 Current Output
![Wireframe Render](/screenshots/africanhead_line.png)