'''This script goes along the blog post
"Building powerful image classification models using very little data"
from blog.keras.io.
'''

# python stuff
import sys, os
import os.path
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
from keras.optimizers import SGD, RMSprop # or Adam, or something else...
from keras import initializations

# custom stuff
import he_load_data
from loss_history import LossHistory
import get_vgg_model
import get_he_topmodel


expNo = '10' 
topModelFCconfig = [512,512]

# path to the model weights files.
w_path = '/home/edogan/data/weights/'
weights_path = w_path + 'vgg16_weights.h5'
top_model_weights_path = '%she_top_model%s.h5' % (w_path, expNo)
train_feat_name = 'bottleneck_features_train_he.npy'
val_feat_name = 'bottleneck_features_validation_he.npy'

now = dt.datetime.now()
experiment_name = '%d%.2d%.2d-%.2d%.2d_finetune_%s' % (now.year, now.month, now.day, now.hour, now.minute, expNo)


# HYPERPARAMS FOR TOPMODEL============
REG_W_FC = 0.25
REG_B_FC = 0.25
DROPOUT_FC = 0.5
BN_EPS = 1e-7
data_augmentation = True # use manually augmented data

batch_size = 128
nb_epoch = 100
nb_parts = he_load_data.NUM_PARTS

# path to the model weights files.
w_path = '/home/edogan/data/weights/'
weights_path = w_path + 'vgg16_weights.h5'
val_feat_name = 'bottleneck_features_validation_he.npy'
if data_augmentation:    
    train_feat_name = 'bottleneck_features_train_aug_he.npy'    
else:    
    train_feat_name = 'bottleneck_features_train_he.npy'


# input image dimensions
img_rows, img_cols = he_load_data.IM_ROWS, he_load_data.IM_COLS
# The HumanEva images are RGB.
img_channels = 3

# fetch data
print('Loading training & validation data...')
if data_augmentation:
    X_train, y_train, X_val, y_val = he_load_data.load_aug_data() # the exact data that we'll use later
else:
    X_train, y_train, X_val, y_val = he_load_data.load_data() # the exact data that we'll use later
print('Done')


def check_bottleneck_features(pathToTrainFeats, pathToTestFeats):
    return os.path.exists(pathToTrainFeats) and os.path.exists(pathToTestFeats)

def save_bottleneck_features(X_train, X_val):    
    # build original VGG16 model
    model = get_vgg_model.build_vgg_model(img_cols, img_rows, weights_path)

    print('Doing prediction on default VGG weights...') 
    
    bottleneck_features_train = model.predict(X_train, batch_size=128, verbose=0)
    print('Prediction for train samples completed. Saving to disk...')
    np.save(open(w_path + train_feat_name, 'wb'), bottleneck_features_train)
    
    bottleneck_features_validation = model.predict(X_val, batch_size=128, verbose=0)
    print('Prediction for validation samples completed. Saving to disk...')
    np.save(open(w_path + val_feat_name, 'wb'), bottleneck_features_validation)
    print('Bottleneck features are saved at %s' % dt.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))


def train_top_model(y_train, y_val, train_data=None, validation_data=None, exp=None, previousWeights=None):  
    if exp==None:
        exp = expNo

    print('Training top model...')   
    train_labels = y_train
    if train_data==None:   
        train_data = np.load(open(w_path + train_feat_name))
    
    validation_labels = y_val
    if validation_data==None:
        validation_data = np.load(open(w_path + val_feat_name))
    
    print('Bottleneck features are loaded.')

    model = get_he_topmodel.build_he_topmodel(train_data.shape[1:], fcLayers=topModelFCconfig, 
                        reg_w=REG_W_FC, reg_b=REG_B_FC, bn_eps=BN_EPS, dropout=DROPOUT_FC)

    if previousWeights != None:
        model.load_weights(previousWeights)

    model.compile(optimizer='rmsprop', loss='mse', metrics=['mean_squared_error'])

    # callback to save history
    chkpt_path = '/home/edogan/data/weights/he_topmodel%s_checkpoints/' % exp
    os.mkdir(chkpt_path)
    filepath = os.path.join(chkpt_path, 'weights-improvement-{epoch:03d}-{val_mean_squared_error:.4f}.hdf5')
    checkpoint = ModelCheckpoint(filepath, monitor='val_mean_squared_error', verbose=0, save_best_only=True, mode='min')
    losshist = LossHistory('topmodel')

    model.fit(train_data, train_labels,
              nb_epoch=nb_epoch, batch_size=batch_size,
              validation_data=(validation_data, validation_labels),
              callbacks=[losshist, checkpoint])
        
    print('Topmodel training is completed at %s' % dt.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))

print('Looking for bottleneck features...')
if check_bottleneck_features(w_path + train_feat_name, w_path + val_feat_name) != True:
    save_bottleneck_features(X_train, X_val)
train_top_model(y_train, y_val)