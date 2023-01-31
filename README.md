# ray-tracer

Project 1 for CS 384G

## Milestone 1

### References
- Formulas: https://www.cs.utexas.edu/~fussell/courses/cs384g/projects/raytracing/equations.pdf
- Barycentric Coordinates: https://ceng2.ktu.edu.tr/~cakir/files/grafikler/Texture_Mapping.pdf

### Features

All features completed:
- [✓] (30 pts) Phong Shading
- [✓] (10 pts) Light Attenuation and Shadow Attenuation
- [✓] (10 pts) Reflection Term
- [✓] (10 pts) Refraction Term
- [✓] (10 pts) Triangle-Ray Intersection
- [✓] (10 pts) Phong Interpolation of Normals
- [✓] (20 pts) Basic Anti-aliasing

### Design Details

Implementation of above features were for the most part consistent with what was discussed in class. There are some differences between the reference solution and my solution when using raycheck.py, but these are undescernible to the human eye (from my testing). The following are design decisions that I made:

- Implemented total internal reflection. This was done using the same math used to handle reflections.
- To avoid floating point rounding problems, intersection points were usually adjusted using the RAY_EPSILON constant in ray.h. This should remove the chance for rays hitting the same surface twice.
- For diffuse lighting, I used the maximum of the dot product term (light and normal) and 0 for opaque materials and the absolute value for transparent materials. This makes sense because if the light is hitting the back of the surface with respect to the intersection, then we should only contribute to the overall color if the material is not opaque.
- Trimesh internal results were calculated for each face during construction. Caching these values should lead to significant speedup on trimesh test cases.
- Anti-aliasing was done by uniformly sampling (shooting a ray) each pixel by the sampling size, and averaging the results.
- Multithreading was implemented for both the main trace routine and the anti-aliasing routine. This was done by having each thread start with a unique thread id, and have them increment it by the specified number of threads. Using division and modulo they can compute indices into the image and trace the pixels concurrently.




