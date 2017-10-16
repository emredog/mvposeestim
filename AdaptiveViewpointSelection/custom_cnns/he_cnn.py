'''Train a simple deep CNN on the HumanEva dataset.
'''

from __future__ import print_function
from keras.preprocessing.image import ImageDataGenerator
from keras.models import Sequential
from keras.layers import Dense, Dropout, Activation, Flatten
from keras.layers import Convolution2D, MaxPooling2D
from keras.layers.normalization import BatchNormalization
from keras.utils import np_utils
from keras.callbacks import ModelCheckpoint, EarlyStopping
from keras.regularizers import l2
from keras.optimizers import Adam # or RMSprop, or something else...
from keras import initializations

import sys, os
import he_load_data
import datetime as dt
import numpy as np

def weight_init(shape, name=None, dim_ordering='tf'):
    return initializations.he_normal(shape, name=None, dim_ordering='tf')

now = dt.datetime.now()
shortnote = '_adam' # start with underscore
experiment_name = '%d%.2d%.2d-%d%d%s' % (now.year, now.month, now.day, now.hour, now.minute, shortnote)

EVAL_PERCENTAGE = 0.2

batch_size = 32
nb_epoch = 100
nb_parts = he_load_data.NUM_PARTS
data_augmentation = False

if data_augmentation:
  experiment_name += '_dAug'

# input image dimensions
img_rows, img_cols = he_load_data.IM_ROWS, he_load_data.IM_COLS 
# The HumanEva images are RGB.
img_channels = 3

# The data, shuffled and split between train and test sets:
X_train, Y_train, X_test, Y_test = he_load_data.load_data(EVAL_PERCENTAGE, False)


# scale the data in [0,1]
X_train = X_train.astype('float32')
X_test = X_test.astype('float32')
X_train /= 255
X_test /= 255

# subtract the mean from images
X_train -= np.mean(X_train)

print('X_train shape:', X_train.shape)
print(X_train.shape[0], 'train samples')
print(X_test.shape[0], 'test samples')


# PREPARE THE MODEL  ==================
model = Sequential()

# conv1
model.add(Convolution2D(32, 3, 3, border_mode='same', init=weight_init,
                      W_regularizer=l2(0.01), b_regularizer=l2(0.01),
                      input_shape=X_train.shape[1:]))
model.add(BatchNormalization(epsilon=0.001, mode=0, axis=3, # axis=3 --> normalize per feature map (channels axis) 
                        momentum=0.99, beta_init=weight_init, gamma_init=weight_init)) # we could also use regularizers for beta & gamma
model.add(Activation('relu'))

# conv2
model.add(Convolution2D(32, 3, 3, init=weight_init,
                      W_regularizer=l2(0.01), b_regularizer=l2(0.01)))
model.add(BatchNormalization(epsilon=0.001, mode=0, axis=3, 
                        momentum=0.99, beta_init=weight_init, gamma_init=weight_init))
model.add(Activation('relu'))
model.add(MaxPooling2D(pool_size=(2, 2)))
model.add(Dropout(0.25))

# conv3
model.add(Convolution2D(64, 3, 3, border_mode='same', init=weight_init,
                      W_regularizer=l2(0.01), b_regularizer=l2(0.01)))
model.add(BatchNormalization(epsilon=0.001, mode=0, axis=3,
                        momentum=0.99, beta_init=weight_init, gamma_init=weight_init))
model.add(Activation('relu'))

# conv4
model.add(Convolution2D(64, 3, 3, init=weight_init,
                      W_regularizer=l2(0.01), b_regularizer=l2(0.01)))
model.add(Activation('relu'))
model.add(BatchNormalization(epsilon=0.001, mode=0, axis=3, 
                        momentum=0.99, beta_init=weight_init, gamma_init=weight_init))
model.add(MaxPooling2D(pool_size=(2, 2)))
model.add(Dropout(0.25))

model.add(Flatten()) # multi-dim to 1D
model.add(Dense(512, init=weight_init)) # fully connected
model.add(BatchNormalization(epsilon=0.001, mode=0, # since the output is 1D, no need to specify axis here.
                        momentum=0.99, beta_init=weight_init, gamma_init=weight_init))
model.add(Activation('relu'))
model.add(Dropout(0.5))
model.add(Dense(nb_parts, init=weight_init, # output layer
                W_regularizer=l2(0.01), b_regularizer=l2(0.01))) 
model.add(Activation('relu'))

# initialize optimizer:
#optim = RMSprop(lr=0.001, rho=0.9, epsilon=1e-08, decay=0.0)
optim = Adam(lr=0.001, beta_1=0.9, beta_2=0.999, epsilon=1e-08, decay=0.0) # default params (from paper)

print('Optimizer: ADAM')

# Let's train the model using 
model.compile(loss='mse',
              optimizer=optim, # experiment with adagrad, momentum etc.
              metrics=['mean_squared_error']) # or just skip the metrics param

model.summary() # prints model details

#sys.exit()

# create dir for checkpoints
chkpt_path = '/home/edogan/checkpoints/%s' % experiment_name
os.mkdir(chkpt_path)

# CALLBACKS:  ==================
# checkpoint
filepath = os.path.join(chkpt_path, 'weights-improvement-{epoch:02d}-{val_loss:.2f}.hdf5')
checkpoint = ModelCheckpoint(filepath, monitor='val_loss', verbose=0, save_best_only=True, mode='min')
# early stopping
earlyStopping = EarlyStopping(monitor='val_loss', min_delta=0.1, patience=10, verbose=0, mode='min')

callbacks_list = [checkpoint, earlyStopping]


# ACTUAL TRAINING: ==================
if not data_augmentation:
    print('Not using data augmentation.')
    model.fit(X_train, Y_train,
              batch_size=batch_size,
              nb_epoch=nb_epoch,
              validation_data=(X_test, Y_test),
              shuffle=True,
              callbacks=callbacks_list)
else:
    print('Using real-time data augmentation.')
    # This will do preprocessing and realtime data augmentation:
    datagen = ImageDataGenerator(
        featurewise_center=False,  # set input mean to 0 over the dataset
        samplewise_center=False,  # set each sample mean to 0
        featurewise_std_normalization=False,  # divide inputs by std of the dataset
        samplewise_std_normalization=False,  # divide each input by its std
        zca_whitening=False,  # apply ZCA whitening
        rotation_range=0,  # randomly rotate images in the range (degrees, 0 to 180)
        width_shift_range=0.1,  # randomly shift images horizontally (fraction of total width)
        height_shift_range=0.1,  # randomly shift images vertically (fraction of total height)
        horizontal_flip=True,  # randomly flip images
        vertical_flip=False)  # randomly flip images

    # Compute quantities required for featurewise normalization
    # (std, mean, and principal components if ZCA whitening is applied).
    datagen.fit(X_train)

    # Fit the model on the batches generated by datagen.flow().
    model.fit_generator(datagen.flow(X_train, Y_train,
                                     batch_size=batch_size),
                        samples_per_epoch=X_train.shape[0],
                        nb_epoch=nb_epoch,
                        validation_data=(X_test, Y_test),
                        callbacks=callbacks_list)

print('Completed %s' % dt.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))