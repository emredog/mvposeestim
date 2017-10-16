'''Train a deeper CNN on the HumanEva dataset.
  architecture is similar to VGG
'''

from __future__ import print_function
# from keras.preprocessing.image import ImageDataGenerator
from keras.models import Sequential
from keras.layers import Dense, Dropout, Activation, Flatten
from keras.layers import Convolution2D, MaxPooling2D, AveragePooling2D
from keras.layers.normalization import BatchNormalization
from keras.layers.advanced_activations import PReLU
from keras.utils import np_utils
from keras.callbacks import ModelCheckpoint, EarlyStopping
from keras.regularizers import l2
from keras.optimizers import SGD #RMSprop or Adam, or something else...
from keras import initializations

from loss_history import LossHistory

import sys, os
import he_load_data
import datetime as dt
import numpy as np
import cPickle as pickle

def weight_init(shape, name=None, dim_ordering='tf'):
    return initializations.he_normal(shape, name=None, dim_ordering='tf')

now = dt.datetime.now()
shortnote = '_arch14_lr1e-3_noDrop' # start with underscore
experiment_name = '%d%.2d%.2d-%.2d%.2d%s' % (now.year, now.month, now.day, now.hour, now.minute, shortnote)

# HYPERPARAMS============
REG_W_CNN = 0.01
REG_B_CNN = 0.01
REG_W_FC = 0.25
REG_B_FC = 0.25
DROPOUT_CNN = 0.0
DROPOUT_FC = 0.5
LEARNING_RATE = 0.001
MOMENTUM = 0.9
DECAY = 1e-6
BN_EPS = 1e-7
USE_PRELU = True # false means regular ReLU
DO_AVG_POOLING = False # false means MaxPooling
data_augmentation = True # use manually augmented data

batch_size = 128
nb_epoch = 200
nb_parts = he_load_data.NUM_PARTS


if data_augmentation:
  experiment_name += '_dAug'

# input image dimensions
img_rows, img_cols = he_load_data.IM_ROWS, he_load_data.IM_COLS 
# The HumanEva images are RGB.
img_channels = 3

print('Loading training data...')

# The data, shuffled and split between train and test sets:
if data_augmentation:
  X_train, Y_train, X_test, Y_test = he_load_data.load_aug_data()
else:
  X_train, Y_train, X_test, Y_test = he_load_data.load_data()
# X_train, Y_train, X_test, Y_test = he_load_data.load_tiny_data(1024, 256)


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

# Conv1
model.add(Convolution2D(32, 7, 7, init=weight_init, border_mode='same',
                      W_regularizer=l2(REG_W_CNN), b_regularizer=l2(REG_B_CNN),
                      input_shape=X_train.shape[1:]))
model.add(BatchNormalization(epsilon=BN_EPS, mode=0, axis=3,  # mode=1: samplewise normalization; 
                                                      # mode=0 axis=3 --> normalize per feature map (channels axis) 
                        momentum=0.99, beta_init='zero', gamma_init='one')) # we could also use regularizers for beta&gamma
if USE_PRELU:
  model.add(PReLU(init='zero', shared_axes=[1,2])) # same param for height & width for shape=(batch, height, width, channels)
else:
  model.add(Activation('relu'))


# Pooling & DropOut
if DO_AVG_POOLING:
  model.add(AveragePooling2D(pool_size=(2, 2)))
else:
  model.add(MaxPooling2D(pool_size=(2, 2)))
model.add(Dropout(DROPOUT_CNN))


# Conv2
model.add(Convolution2D(32, 5, 5, init=weight_init, border_mode='same',
                      W_regularizer=l2(REG_W_CNN), b_regularizer=l2(REG_B_CNN)))
model.add(BatchNormalization(epsilon=BN_EPS, mode=0, axis=3,  
                        momentum=0.99, beta_init='zero', gamma_init='one'))
if USE_PRELU:
  model.add(PReLU(init='zero', shared_axes=[1,2])) 
else:
  model.add(Activation('relu'))


# Pooling & DropOut
if DO_AVG_POOLING:
  model.add(AveragePooling2D(pool_size=(2, 2)))
else:
  model.add(MaxPooling2D(pool_size=(2, 2)))
model.add(Dropout(DROPOUT_CNN))


# Conv3
model.add(Convolution2D(32, 3, 3, init=weight_init, border_mode='same', 
                      W_regularizer=l2(REG_W_CNN), b_regularizer=l2(REG_B_CNN)))
model.add(BatchNormalization(epsilon=BN_EPS, mode=0, axis=3,
                        momentum=0.99, beta_init='zero', gamma_init='one'))
if USE_PRELU:
  model.add(PReLU(init='zero', shared_axes=[1,2])) # same param for height & width for shape=(batch, height, width, channels)
else:
  model.add(Activation('relu'))


# # Conv4
# model.add(Convolution2D(64, 3, 3, init=weight_init, border_mode='same', 
#                       W_regularizer=l2(REG_W_CNN), b_regularizer=l2(REG_B_CNN)))
# model.add(BatchNormalization(epsilon=BN_EPS, mode=0, axis=3,
#                         momentum=0.99, beta_init='zero', gamma_init='one'))
# if USE_PRELU:
#   model.add(PReLU(init='zero', shared_axes=[1,2]))
# else:
#   model.add(Activation('relu'))

# Pooling & DropOut
if DO_AVG_POOLING:
  model.add(AveragePooling2D(pool_size=(2, 2)))
else:
  model.add(MaxPooling2D(pool_size=(2, 2)))
model.add(Dropout(DROPOUT_CNN))


# Conv5
model.add(Convolution2D(128, 3, 3, init=weight_init, border_mode='same',
                      W_regularizer=l2(REG_W_CNN), b_regularizer=l2(REG_B_CNN)))
model.add(BatchNormalization(epsilon=BN_EPS, mode=0, axis=3,
                        momentum=0.99, beta_init='zero', gamma_init='one'))
if USE_PRELU:
  model.add(PReLU(init='zero', shared_axes=[1,2]))
else:
  model.add(Activation('relu'))

# Pooling & DropOut
if DO_AVG_POOLING:
  model.add(AveragePooling2D(pool_size=(2, 2)))
else:
  model.add(MaxPooling2D(pool_size=(2, 2)))
model.add(Dropout(DROPOUT_CNN))



model.add(Flatten()) # multi-dim to 1D

# # FC-512
# model.add(Dense(512, init=weight_init, 
#                 W_regularizer=l2(REG_W_FC), b_regularizer=l2(REG_B_FC))) 
# model.add(BatchNormalization(epsilon=BN_EPS, mode=0, # since the output is 1D, no need to specify axis here.
#                         momentum=0.99, beta_init='zero', gamma_init='one'))
# if USE_PRELU:
#   model.add(PReLU(init='zero', shared_axes=None)) 
# else:
#   model.add(Activation('relu'))
# model.add(Dropout(DROPOUT_FC))




# OUTPUT
model.add(Dense(nb_parts, init=weight_init, # output layer
                W_regularizer=l2(REG_W_FC), b_regularizer=l2(REG_B_FC))) 
if USE_PRELU:
  model.add(PReLU(init='zero', shared_axes=None)) 
else:
  model.add(Activation('relu'))



# INITIALIZE OPTIMIZER =================
#optim = RMSprop(lr=0.001, rho=0.9, epsilon=1e-08, decay=0.0)
#optim = Adam(lr=LEARNING_RATE, beta_1=0.9, beta_2=0.999, epsilon=1e-08, decay=0.0) # default params (from paper)

optim = SGD(lr=LEARNING_RATE, decay=DECAY, momentum=MOMENTUM, nesterov=True)

print(('Optimizer: SGD with Nesterov momentum=%.2f, decay=%.8f, lr=%f') % (MOMENTUM, DECAY, LEARNING_RATE))

# Let's train the model using 
model.compile(loss='mse',
              optimizer=optim, metrics=['mean_squared_error']) # or just skip the metrics param

model.summary() # prints model details

#sys.exit()

# create dir for checkpoints
chkpt_path = '/home/edogan/checkpoints/%s' % experiment_name
os.mkdir(chkpt_path)



# CALLBACKS:  ==================
# checkpoint
filepath = os.path.join(chkpt_path, 'weights-improvement-{epoch:02d}-{val_mean_squared_error:.2f}.hdf5')
checkpoint = ModelCheckpoint(filepath, monitor='val_mean_squared_error', verbose=0, save_best_only=True, mode='min')
# early stopping
# earlyStopping = EarlyStopping(monitor='val_mean_squared_error', min_delta=0.1, patience=50, verbose=0, mode='min')
# history of loss/mse
losshist = LossHistory()

# callbacks_list = [checkpoint, earlyStopping]
callbacks_list = [losshist, checkpoint]


# ACTUAL TRAINING: ==================
if not data_augmentation:
  print('Not using data augmentation.')
else:
  print('Using manually augmented data.')

model.fit(X_train, Y_train,
              batch_size=batch_size,
              nb_epoch=nb_epoch,
              validation_data=(X_test, Y_test),
              shuffle=True,
              callbacks=callbacks_list)
# else:
#     print('Using real-time data augmentation.')
#     # This will do preprocessing and realtime data augmentation:
#     datagen = ImageDataGenerator(
#         featurewise_center=True,  # set input mean to 0 over the dataset
#         samplewise_center=False,  # set each sample mean to 0
#         featurewise_std_normalization=True,  # divide inputs by std of the dataset
#         samplewise_std_normalization=False,  # divide each input by its std
#         zca_whitening=True,  # apply ZCA whitening
#         rotation_range=10,  # randomly rotate images in the range (degrees, 0 to 180)
#         width_shift_range=0.1,  # randomly shift images horizontally (fraction of total width)
#         height_shift_range=0.1,  # randomly shift images vertically (fraction of total height)
#         horizontal_flip=True,  # randomly flip images
#         vertical_flip=False)  # randomly flip images

#     # Compute quantities required for featurewise normalization
#     # (std, mean, and principal components if ZCA whitening is applied).
#     datagen.fit(X_train)

#     # Fit the model on the batches generated by datagen.flow().
#     model.fit_generator(datagen.flow(X_train, Y_train,
#                                      batch_size=batch_size),
#                         samples_per_epoch=X_train.shape[0],
#                         nb_epoch=nb_epoch,
#                         validation_data=(X_test, Y_test),
#                         callbacks=callbacks_list)


print('Completed %s' % dt.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
