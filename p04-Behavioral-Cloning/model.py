import os
import csv
import cv2
import numpy as np
from keras.models import  Model
from keras.layers import Dense, Lambda, Dense, Dropout, Flatten, ELU, Input
from keras.layers.convolutional import Convolution2D
import tensorflow as tf
from sklearn.model_selection import train_test_split
import matplotlib.pyplot as plt
from sklearn.model_selection import train_test_split
from sklearn.utils import shuffle

samples = []
with open('.\data\driving_log.csv') as csvfile:
    reader = csv.reader(csvfile)
    for line in reader:
        samples.append(line)

image_datapath = "data/"

def generator(samples, batch_size = 32):
    num_samples = len(samples)
    while 1: # Loop forever so the generator never terminates
        shuffle(samples)
        for offset in range(0, num_samples, batch_size):
            batch_samples = samples[offset:offset+batch_size]
            images = []
            angles = []
            for batch_sample in batch_samples:
                center_image = cv2.imread((image_datapath+batch_sample[0]).replace(' ',''))
                left_image = cv2.imread((image_datapath+batch_sample[1]).replace(' ',''))
                right_image = cv2.imread((image_datapath+batch_sample[2]).replace(' ',''))
                correction = 0.2
                center_angle = float(batch_sample[3])
                left_angle = center_angle + correction # correction for treating left images as center
                right_angle = center_angle - correction # correction for treating right images as center
                images.extend([center_image[70:135, :],left_image[70:135, :],right_image[70:135, :]]) # cropping images
                angles.extend([center_angle,left_angle,right_angle])
            augmented_images, augmented_angles = [], []
            for image, angle in zip(images, angles):
                augmented_images.append(image)
                augmented_angles.append(angle)
                augmented_images.append(cv2.flip(image, 1)) # flipping image for data augmentation
                augmented_angles.append(angle * -1.0)
            X_train = np.array(augmented_images)
            y_train = np.array(augmented_angles)

            yield (X_train, y_train)

train_samples, validation_samples = train_test_split(samples, test_size=0.2)

train_generator = generator(train_samples, batch_size=32)
validation_generator = generator(validation_samples, batch_size=32)

H, W, ch = 65, 320, 3

input_frame = Input(shape=(H, W, ch))
x = Lambda(lambda z: z / 255.0 - .5)(input_frame)
x = Convolution2D(24, 5, strides=(2, 2), padding = "valid", activation="relu")(x)
x = Convolution2D(36, 5, strides=(2, 2), padding = "valid",activation="relu")(x)
x = Convolution2D(48, 3, strides=(1, 1), padding = "valid",activation="relu")(x)
x = Convolution2D(64, 3, strides=(1, 1), padding = "valid",activation="relu")(x)
x = Flatten()(x)
x = Dense(100)(x)
x = ELU()(x)
x = Dropout(0.5)(x)
x = Dense(50)(x)
x = ELU()(x)
x = Dropout(0.5)(x)
x = Dense(10)(x)
x = ELU()(x)
out = Dense(1)(x)

model = Model(inputs=input_frame, outputs=out)
model.summary()

model.compile(loss = "mse", optimizer = "adam")
model.fit_generator(train_generator, steps_per_epoch = len(train_samples*2) , epochs=3, validation_data = validation_generator, validation_steps = len(validation_samples))

model.save("nvidia3.h5")