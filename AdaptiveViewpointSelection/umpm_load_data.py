import random
import cPickle as pickle
from scipy.misc import imread, imresize, imsave
import numpy as np
from collections import OrderedDict

NUM_PARTS = 26
IM_ROWS = 162
IM_COLS = 44
IM_SQ_SIZE = 128
IM_DEPT = 3
DATA_ROOT = '/home/edogan/data/UMPM/'

# np.max(Y_train) = 213.5322, np.min(Y_train) = 0.0062626
Ytrain_MAX = 213.5322
Ytrain_MIN = 0.0062626

# Training data
data_file = DATA_ROOT + 'UMPM_train_paths_marg20.pkl'
aug_data_file = DATA_ROOT + 'UMPM_train_aug_paths_marg20.pkl'

bin_data_file = DATA_ROOT + 'UMPM_train_bin_marg20.pkl'
bin_aug_data_file = DATA_ROOT + 'UMPM_train_aug_bin_marg20.pkl'

# Test data
test_data_file = DATA_ROOT + 'UMPM_test_paths_marg20.pkl'
bin_test_data_file = DATA_ROOT + 'UMPM_test_bin_marg20.pkl'

# Validation data
validation_data_file = DATA_ROOT + 'UMPM_validation_paths_marg20.pkl'
bin_validation_data_file = DATA_ROOT + 'UMPM_validation_bin_marg20.pkl'

def scale_and_normalize(X):
	# scale the data in [0,1]
	X = X.astype('float32')	
	X /= 255.0
	# subtract the mean from images
	X -= np.mean(X)
	return X	

def scale_labels(Y):
	print('Scaling Y labels with training MIN and MAX values: %f and %f' % (Ytrain_MIN, Ytrain_MAX))
	return (Y - Ytrain_MIN) / (Ytrain_MAX - Ytrain_MIN)

def load_validation_data():	
	# OUTPUT:
	# 	X_validation:		test images (nbTrain x IM_COLS x IM_ROWS x IM_DEPT), uint8
	# 	y_validation:		test labels (nbTrain x NUM_PARTS floats)
	

	try:		
		d = pickle.load(open(bin_validation_data_file, 'rb'))
		X_validation, y_validation = d['X_validation'], d['y_validation']

		X_validation = scale_and_normalize(X_validation)
		# y_validation = scale_labels(y_validation)
		return X_validation, y_validation
	except: # file not found
		print('Raw test image data is not found. Reading images from HDD & resizing. This may take a few minutes...')
		
		with open(validation_data_file, 'rb') as f:
			fulldict = pickle.load(f)

		nbSamples = len(fulldict)
		print('# of total test samples:', nbSamples)

		# init data
		X_validation = np.zeros((nbSamples, IM_ROWS, IM_COLS, IM_DEPT)).astype('uint8')	
		y_validation = np.zeros((nbSamples, NUM_PARTS)).astype('float32')		

		idx = list(range(nbSamples))
				
		for i, iid in enumerate(idx):
			samp = fulldict[iid] 
			impath = samp['Path']
			pbErr = samp['PbErrors']

			try:
				# read image
				resizedImg = imresize(imread(impath, mode='RGB'), (IM_ROWS, IM_COLS))

				X_validation[i] = resizedImg
				y_validation[i] = np.asarray(pbErr)

				if i%1000==0:
					print('Processed', i, '/', nbSamples)
			except IOError as e:
				print(e)
				print('Image not found: %s' % impath)

		# save data for later use:
		print('Dumping data for later use...')
		dataset = OrderedDict([('X_validation', X_validation), ('y_validation', y_validation)])
		pickle.dump(dataset, open(bin_validation_data_file, 'wb'))

		X_validation = scale_and_normalize(X_validation)
		# y_validation = scale_labels(y_validation)

		return X_validation, y_validation

def load_test_data():	
	# OUTPUT:
	# 	X_test:		test images (nbTrain x IM_COLS x IM_ROWS x IM_DEPT), uint8
	# 	y_test:		test labels (nbTrain x NUM_PARTS floats)
	

	try:		
		d = pickle.load(open(bin_test_data_file, 'rb'))
		X_test, y_test = d['X_test'], d['y_test']

		X_test = scale_and_normalize(X_test)
		# y_test = scale_labels(y_test)
		return X_test, y_test
	except: # file not found
		print('Raw test image data is not found. Reading images from HDD & resizing. This may take a few minutes...')
		
		with open(test_data_file, 'rb') as f:
			fulldict = pickle.load(f)

		nbSamples = len(fulldict)
		print('# of total test samples:', nbSamples)

		# init data
		X_test = np.zeros((nbSamples, IM_ROWS, IM_COLS, IM_DEPT)).astype('uint8')	
		y_test = np.zeros((nbSamples, NUM_PARTS)).astype('float32')		

		idx = list(range(nbSamples))
				
		for i, iid in enumerate(idx):
			samp = fulldict[iid] 
			impath = samp['Path']
			pbErr = samp['PbErrors']

			try:
				# read image
				resizedImg = imresize(imread(impath, mode='RGB'), (IM_ROWS, IM_COLS))

				X_test[i] = resizedImg
				y_test[i] = np.asarray(pbErr)

				if i%1000==0:
					print('Processed', i, '/', nbSamples)
			except IOError as e:
				print(e)
				print('Image not found: %s' % impath)

		# save data for later use:
		print('Dumping data for later use...')
		dataset = OrderedDict([('X_test', X_test), ('y_test', y_test)])
		pickle.dump(dataset, open(bin_test_data_file, 'wb'))

		X_test = scale_and_normalize(X_test)
		# y_test = scale_labels(y_test)

		return X_test, y_test	

def load_aug_data():	
	# OUTPUT:
	# 	X_train:	train images (nbTrain x IM_COLS x IM_ROWS x IM_DEPT), uint8
	# 	y_train:	train labels (nbTrain x NUM_PARTS floats)

	try:				
		d = pickle.load(open(bin_aug_data_file, 'rb'))

		X_train, y_train = d['X_train'], d['y_train']

		X_train = scale_and_normalize(X_train)		
		# y_train = scale_labels(y_train)		

		return X_train, y_train
	except: # file not found
		print('Raw image data is not found. Reading images from HDD & resizing. This may take a few minutes...')
		
		with open(aug_data_file, 'rb') as f:
			fulldict = pickle.load(f)

		nbSamples = len(fulldict)
		print('# of total samples:', nbSamples)		

		# init data
		X_train = np.zeros((nbSamples, IM_ROWS, IM_COLS, IM_DEPT)).astype('uint8')	
		y_train = np.zeros((nbSamples, NUM_PARTS)).astype('float32')		

		idx = list(range(nbSamples))
		
		for i, iid in enumerate(idx):
			samp = fulldict[iid] 
			impath = samp['Path']
			pbErr = samp['PbErrors']

			# read image
			resizedImg = imresize(imread(impath, mode='RGB'), (IM_ROWS, IM_COLS))
			
			X_train[i] = resizedImg
			y_train[i] = np.asarray(pbErr)

			if i%1000==0:
				print('Processed', i, '/', nbSamples)

		# save data for later use:
		print('Dumping data for later use...')
		dataset = OrderedDict([('X_train', X_train), ('y_train', y_train)])
		pickle.dump(dataset, open(bin_aug_data_file, 'wb'))
		
		X_train = scale_and_normalize(X_train)		
		# y_train = scale_labels(y_train)
		# y_eval = scale_labels(y_eval)
		
		return X_train, y_train


def load_data():	
	# OUTPUT:
	# 	X_train:	train images (nbTrain x IM_COLS x IM_ROWS x IM_DEPT), uint8
	# 	y_train:	train labels (nbTrain x NUM_PARTS floats)	

	try:		
		d = pickle.load(open(bin_data_file, 'rb'))
		X_train, y_train = d['X_train'], d['y_train']

		X_train = scale_and_normalize(X_train)		
		# y_train = scale_labels(y_train)		
		
		return X_train, y_train
	except: # file not found
		print('Raw image data is not found. Reading images & resizing from HDD...')
		
		with open(data_file, 'rb') as f:
			fulldict = pickle.load(f)

		nbSamples = len(fulldict)
		print('# of total samples:', nbSamples)		

		# init data
		X_train = np.zeros((nbSamples, IM_ROWS, IM_COLS, IM_DEPT)).astype('uint8')	
		y_train = np.zeros((nbSamples, NUM_PARTS)).astype('float32')		

		idx = range(nbSamples)			

		for i, iid in enumerate(idx):
			samp = fulldict[iid]
			impath = samp['Path']
			pbErr = samp['PbErrors']

			# read image
			resizedImg = imresize(imread(impath, mode='RGB'), (IM_ROWS, IM_COLS))
			
			X_train[i] = resizedImg
			y_train[i] = np.asarray(pbErr)

			if i%1000==0:
				print('Processed', i, '/', nbSamples)

		# save data for later use:
		print('Dumping data for later use...')
		dataset = OrderedDict([('X_train', X_train), ('y_train', y_train)])
		pickle.dump(dataset, open(bin_data_file, 'wb'))

		X_train = scale_and_normalize(X_train)		
		# y_train = scale_labels(y_train)		

		return X_train, y_train
	


def load_tiny_data(nbTinyTrain, nbTinyEval):
	# INPUT:	
	#
	# OUTPUT:
	# 	X_train:	train images (nbTrain x IM_COLS x IM_ROWS x IM_DEPT), uint8
	# 	y_train:	train labels (nbTrain x NUM_PARTS floats)
	#	X_eval: 	evaluation images (nbEval x IM_COLS x IM_ROWS x IM_DEPT), uint8
	#	y_eval:		evaluation labels (nbEval x NUM_PARTS floats)  

	with open(data_file, 'rb') as f:
		fulldict = pickle.load(f)

	nbSamples = len(fulldict)
	# print '# of total samples:', nbSamples
	# nbTrain = int(round(nbSamples*(1.0-evalPercentage)))
	# print '# of training samples:', nbTrain
	# nbEval = nbSamples - nbTrain

	# init data
	X_train = np.zeros((nbTinyTrain, IM_ROWS, IM_COLS, IM_DEPT)).astype('uint8')	
	y_train = np.zeros((nbTinyTrain, NUM_PARTS)).astype('float32')
	X_eval  = np.zeros((nbTinyEval, IM_ROWS, IM_COLS, IM_DEPT)).astype('uint8')
	y_eval  = np.zeros((nbTinyEval, NUM_PARTS)).astype('float32')


	idx = range(nbSamples)	

	for i, iid in enumerate(idx):
		samp = fulldict[iid+1]
		impath = samp['Path']
		pbErr = samp['PbErrors']

		# read image
		resizedImg = imresize(imread(impath, mode='RGB'), (IM_ROWS, IM_COLS))

		if i < nbTinyTrain: # add to train set
			X_train[i] = resizedImg
			y_train[i] = np.asarray(pbErr)
		elif i < nbTinyTrain+nbTinyEval:
			X_eval[i-nbTinyTrain] = resizedImg
			y_eval[i-nbTinyTrain] = np.asarray(pbErr)
		else:
			return X_train, y_train, X_eval, y_eval			

		if i%1000==0:
			print('Processed', i, '/', nbSamples)

	X_train = scale_and_normalize(X_train)
	X_eval = scale_and_normalize(X_eval)
	# y_train = scale_labels(y_train)
	# y_eval = scale_labels(y_eval)

	return X_train, y_train, X_eval, y_eval
