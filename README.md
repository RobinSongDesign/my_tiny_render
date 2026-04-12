# Lesson 1
-Integer Division: Using int for slope $k$ caused truncation (resulting in $k=0$). Fixed by casting to float.
-Point Ordering: Drawing from right to left ($x_0 > x_1$) failed. Fixed by swapping start/end points as a pair: if (x0 > x1) std::swap(p0, p1).
-Steep Line Discontinuity: Lines with $|dy| > |dx|$ appeared as sparse dots. Resolved by transposing coordinates ($x, y$ swap) to iterate along the longer axis.
-Viewport Mapping: .obj coordinates are normalized $[-1, 1]$. Implemented scaling logic to map them to $[0, \text{width/height}]$ screen space.Overdrawing: Shared edges between triangles are drawn twice; without a Z-buffer, the last color drawn always wins (overwriting previous pixels).

## 📸 Current Output
![Wireframe Render](/screenshots/africanhead_line.png)