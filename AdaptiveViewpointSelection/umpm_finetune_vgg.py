# python stuff
import sys, os
import h5py
import numpy as np
import datetime as dt
import cPickle as pickle

# keras stuff
from keras import optimizers
from keras.models import Sequential
from keras.layers import Convolution2D, MaxPooling2D, ZeroPadding2D
from keras.layers import Activation, Dropout, Flatten, Dense
from keras.layers.normalization import BatchNormalization
from keras.layers.advanced_activations import PReLU
from keras.utils import np_utils
from keras.callbacks import ModelCheckpoint, EarlyStopping
from keras.regularizers import l2
from keras.optimizers import SGD #RMSprop or Adam, or something else...
from keras import initializations

# custom stuff
import umpm_load_data
from loss_history import LossHistory
import get_vgg_model
import get_umpm_topmodel

expNo = '22'
topModNo = '14' 
topModConf = [128,128]
# topmod01: FC512, topmod02: FC1024, topmod04: FC512-FC512
# topmod05: FC256, topmod06: FC256 with fixed scaling bottleneck features
# Following topmodels are trained with full Video_training, and validated on Video_validation
# topmod07: FC256, topmod08: FC1024, topmod09: FC128, topmod10: FC512
# topmod11: FC1024-FC1024, topMod12:FC1025-Reg0.4

# path to the model weights files.
w_path = '/home/edogan/data/weights/'
weights_path = w_path + 'vgg16_weights.h5'
top_model_weights_path = '%sumpm_top_model%s_best.hdf5' % (w_path, topModNo)

now = dt.datetime.now()
experiment_name = 'finetune_umpm_%s_%d%.2d%.2d-%.2d%.2d' % (expNo, now.year, now.month, now.day, now.hour, now.minute)

# HYPERPARAMS============
REG_W_FC = 0.25
REG_B_FC = 0.25
DROPOUT_FC = 0.5
LEARNING_RATE = 0.0001
MOMENTUM = 0.9
BN_EPS = 1e-7
FREEZE_UPTO = 25 # number of freezed layers in VGG (25: finetune on last ConvBlock5, 18: finetune on ConvBlock4 and ConvBlock5)
data_augmentation = True # use manually augmented data

batch_size = 128
nb_epoch = 50
nb_parts = umpm_load_data.NUM_PARTS

# input image dimensions
img_rows, img_cols = umpm_load_data.IM_ROWS, umpm_load_data.IM_COLS 
img_channels = 3


print('Loading training data...')
if data_augmentation:
  X_train, Y_train = umpm_load_data.load_aug_data()
else:
  X_train, Y_train = umpm_load_data.load_data()
print('Loading validation data...')
X_val, Y_val = umpm_load_data.load_validation_data()

print(X_train.shape[0], 'train samples')
print(X_val.shape[0], 'validation samples')


# BUILD THE VGG MODEL ==================
print('Building vgg model...')
model = get_vgg_model.build_vgg_model(img_cols, img_rows, weights_path)



# BUILD THE TOPMODEL (upon the VGG16 model) ===========================
print('Building the top model...')
top_model = get_umpm_topmodel.build_umpm_topmodel(model.output_shape[1:], fcLayers=topModConf, reg_w=REG_W_FC, reg_b=REG_B_FC, 
                                                bn_eps=BN_EPS, dropout=DROPOUT_FC)

# note that it is necessary to start with a fully-trained
# classifier, including the top classifier,
# in order to successfully do fine-tuning
top_model.load_weights(top_model_weights_path) # weigths that are trained with umpm_bottleneck_topmodel.train_top_model

# add the model on top of the convolutional base
model.add(top_model)

# set the first FREEZE_UPTO layers (up to the last conv block)
# to non-trainable (weights will not be updated)
for layer in model.layers[:FREEZE_UPTO]:
  layer.trainable = False

# Let's train the model (only last conv layer + top_model)
model.compile(loss='mse',
              optimizer=SGD(lr=LEARNING_RATE, momentum=MOMENTUM), 
              metrics=['mean_squared_error']) # or just skip the metrics param

# Callbacks
losshist = LossHistory('finetune_umpm' + expNo) 
chkpt_path = '/home/edogan/checkpoints/%s' % experiment_name
os.mkdir(chkpt_path)
filepath = os.path.join(chkpt_path, 'weights-improvement-{epoch:03d}-{val_mean_squared_error:.2f}.hdf5')
checkpoint = ModelCheckpoint(filepath, monitor='val_mean_squared_error', verbose=0, save_best_only=True, mode='min')

print('Starting training...')

model.fit(X_train, Y_train,
              batch_size=batch_size,
              nb_epoch=nb_epoch,
              validation_data=(X_val, Y_val),
              verbose=1,              
              callbacks=[losshist, checkpoint])

print('Completed %s' % dt.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))

