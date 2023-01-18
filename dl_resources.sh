#!/bin/bash

wget -nc https://www.cs.utexas.edu/~graphics/s23/cs378h/ray/overview/public_scenes_part1.tar.xz
wget -nc https://www.cs.utexas.edu/~graphics/s23/cs378h/ray/overview/public_scenes_part2.tar.xz
wget -nc https://www.cs.utexas.edu/~graphics/s23/cs378h/ray/overview/cubemaps.tar.xz

mkdir -p resources
tar -xf public_scenes_part1.tar.xz --directory resources/
mv resources/scenes resources/public1
tar -xf public_scenes_part2.tar.xz --directory resources/
mv resources/scenes resources/public2
tar -xf cubemaps.tar.xz --directory resources/

rm public_scenes_part1.tar.xz
rm public_scenes_part2.tar.xz
rm cubemaps.tar.xz