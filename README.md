
## 🎥 Lesson 4: Camera & Transformation Pipeline

- **LookAt Matrix Implementation**: 
    - Moved away from a static camera. Implemented the `lookat` function to define camera state using **Eye**, **Center**, and **Up** vectors.
    - Derived the Change of Basis matrix by calculating orthonormal basis vectors $(\vec{x'}, \vec{y'}, \vec{z'})$.
    - Fixed the common pitfall: Ensuring the translation is calculated as the dot product $-(\vec{axis} \cdot \vec{eye})$ to correctly combine Rotation and Translation ($M = R \cdot T$).
  
- **Unified MVP Pipeline**: 
    - Established a formal vertex transformation flow:  
      $$V_{screen} = Viewport \cdot Projection \cdot View \cdot Model \cdot V_{local}$$
    - This architecture allows for independent control over object placement (Model), camera positioning (View), and lens perspective (Projection).

- **Perspective Optimization**:
    - Tuned the projection coefficient $c$. Experimented with different values to balance between "Telephoto" (large $c$) and "Wide-angle" (small $c$) effects.
    - Verified that smaller $c$ values intensify the "near-big, far-small" distortion but require viewport/model scaling to maintain subject size.

## 🛠 Bug Fixes & Technical Insights
- **Cross Product Order**: Corrected the $x$-axis calculation to `cross(up, z_axis)` to maintain a standard Right-Handed Coordinate System.
- **Orthonormalization**: Re-calculated the `y_axis` via `cross(z_axis, x_axis)` instead of using the raw `up` vector to ensure the basis is strictly orthogonal, preventing mesh shearing.
- **W-Divide Logic**: Confirmed that $w' = 1 - z/c$ is the heart of perspective, where $c$ acts as the focal length of our virtual lens.

## 📸 Current Output
[Side View](/screenshots/africanhead_side.png)
**Next Step**: Lesson 5 - Moving logic into a Shader-based architecture (`IShader` interface).

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