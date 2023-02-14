import numpy as np
from PIL import Image
import os
import sys
import torch

import albumentations as A
import albumentations.pytorch as AT

from PIL import Image
from torchvision.utils import save_image
from config import *
from models import Generator

from datetime import datetime

transform = A.Compose([
    A.Normalize(mean=[0, 0, 0], std=[1, 1, 1]),
    AT.ToTensorV2(),
])

def run_image(generator, filepath):
    image = np.asarray(Image.open(os.path.join(filepath)))
    print(image.shape)
    image = transform(image=image)["image"].unsqueeze(0).to(DEVICE)
    with torch.no_grad():
        upscaleImage = generator(image)
        save_image(upscaleImage * 0.5 + 0.5, filepath[:-4] + "_upscaled.png")

def main():
    filepath = sys.argv[1]
    generator = Generator(in_channels=CHANNELS, scaling_factor=SCALE_FACTOR).to(DEVICE)
    checkpoint = torch.load(os.path.join(os.path.dirname(os.path.realpath(__file__)), GEN_FILE), map_location=DEVICE)
    generator.load_state_dict(checkpoint["state_dict"])
    generator.eval()
    start = datetime.now()
    print("Upsampling Image...")
    run_image(generator, filepath)
    end = datetime.now()
    print("Upsample Time: ", (end-start).total_seconds(), "s")

if __name__ == "__main__":
    main()
