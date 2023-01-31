#!/bin/bash

rm -f ray.tgz
rm -rf ray/
mkdir -p ray/
rsync * -r --exclude='ray' ray/

( cd ray
    rm -rf build/
    rm -rf assets/
    rm -rf resources/
    rm -rf raycheck.out/
    rm -rf binaries/
    rm -rf .git/
    rm -rf .vscode/
    rm -rf third-party/glm/doc
)

tar -cvzf ray.tgz ray/
rm -rf ray

python3 sancheck.py ray.tgz
