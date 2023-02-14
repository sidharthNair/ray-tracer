# Super Resolution

We implemented SRGAN as described in [<em>Photo-Realistic Single Image Super-Resolution Using a Generative Adversarial
Network</em>](https://arxiv.org/pdf/1609.04802.pdf)

![image](https://user-images.githubusercontent.com/84476225/198865448-44935565-bc64-4849-9c99-64f45be3d54b.png)

## Model

Trained for 4000 Epochs (2000 MSE Only, 2000 w/ Perceptual Loss) on the [DIV2K](https://data.vision.ee.ethz.ch/cvl/DIV2K/) HR training dataset

For testing, download gen.tar and place it in this repository's folder. Then, run `python test.py` to generate SR images from the LR images in `test/` (output is in `test_output/`).

## Data

Put data in `data/`. Expected structure is:
```
.
├── class1_directory
│   ├── img1_file
│   ├── img2_file
│   └── ...
├── class2_directory
└── ...
```
