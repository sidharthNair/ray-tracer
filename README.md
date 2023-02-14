# ray-tracer

Project 1 for CS 384G -- Sidharth Nair (sn25377)

## Milestone 1

### References
- Formulas: https://www.cs.utexas.edu/~fussell/courses/cs384g/projects/raytracing/equations.pdf
- Barycentric Coordinates: https://ceng2.ktu.edu.tr/~cakir/files/grafikler/Texture_Mapping.pdf

### Features

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

## Milestone 2

Jeffrey Liu (jl72726), Sidharth Nair (sn25377)

Partner contribution breakdown: we both worked on different parts simultaneously; Jeffrey contributed the most to BVH acceleration data structure with SAH while Sidharth worked mostly on cube/texture mapping and super resolution.

### Features

#### Required

- [✓] (60 pts) Acceleration Structure (BVH with SAH)
- [✓] (10 pts) Texture mapping
- [✓] (10 pts) Cube mapping

#### Extra(s)

Super Resolution using SRGAN:

We implemented SRGAN as described in [<em>Photo-Realistic Single Image Super-Resolution Using a Generative Adversarial
Network</em>](https://arxiv.org/pdf/1609.04802.pdf) to upsample scenes rendered using our raytracer during post-processing. The model upsamples images to 4x their original resolution.

Training Details:

Trained for 4000 Epochs (2000 MSE Only, 2000 w/ Perceptual Loss) on the [DIV2K](https://data.vision.ee.ethz.ch/cvl/DIV2K/) HR training dataset

To Run:

Install the python dependencies in the requirements.txt file in the `super-resolution/` directory: `pip install -r requirements.txt`. If you are running on a Mac computer, please change `super-resolution/config.py` with the following definition: `DEVICE = torch.device('mps')`. Note that without a GPU it does take a considerable amount of time to run the upscaler (1-2 minutes).

Run the ray tracer after compiling with the `-u` flag:

`./build/bin/ray -u file.ray output.png`

The upsampled scene will be stored at `output_upscaled.png`

Results:

Please see `sr_samples/` for rendered scenes and their upscaled versions using our model. The model works especially well on cubemaps (the sharpness difference is very noticeable), as it aims to restore photo-realistic details while increasing resolution.

### Design Details

For our acceleration data structure we implemented BVH with SAH. The surface area heuristic is implemented by computing a cost function for each split point at each BVH node level (we're using explicit trees). It naively computes the best cut based on minimizing the expected number of intersections.

Cube mapping and texture mapping were relatively straight forward, and implemented as discussed in class.

