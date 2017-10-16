# keras stuff
from keras.models import Sequential
from keras.layers import Activation, Dropout, Flatten, Dense
from keras.layers.advanced_activations import PReLU
from keras.layers.normalization import BatchNormalization
from keras.utils import np_utils
from keras.regularizers import l2
from keras import initializations

nb_parts = 26
defaultFCLayer = 1024

def weight_init(shape, name=None, dim_ordering='tf'):
	return initializations.he_normal(shape, name=None, dim_ordering='tf')


def build_he_topmodel(inputshape, fcLayers=None, reg_w=0.001, reg_b=0.001, bn_eps=1e-7, dropout=0.5):

	model = Sequential()

	# flatten
	model.add(Flatten(input_shape=inputshape))

	if fcLayers==None: # no parameter is given, just construct a default FC layer	
		model.add(Dense(defaultFCLayer, init=initializations.he_normal, W_regularizer=l2(reg_w), b_regularizer=l2(reg_b))) 
		model.add(BatchNormalization(epsilon=bn_eps, mode=0, momentum=0.99, beta_init='zero', gamma_init='one'))
		model.add(PReLU(init='zero', shared_axes=None)) 		
		model.add(Dropout(dropout))
	else: # construct a series of layers, according to argument
		for layerNodes in fcLayers:
			model.add(Dense(layerNodes, init=initializations.he_normal, W_regularizer=l2(reg_w), b_regularizer=l2(reg_b))) 
			model.add(BatchNormalization(epsilon=bn_eps, mode=0, momentum=0.99, beta_init='zero', gamma_init='one'))
			model.add(PReLU(init='zero', shared_axes=None)) 		
			model.add(Dropout(dropout))

	# Output layer:
	model.add(Dense(nb_parts, init=initializations.he_normal, W_regularizer=l2(reg_w), b_regularizer=l2(reg_b))) 
	model.add(PReLU(init='zero', shared_axes=None)) 
	
	print('==== TOP MODEL SUMMARY ====')
	model.summary()
	
	return model
