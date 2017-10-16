import random
import pickle
from scipy.misc import imread, imresize, imsave
import numpy as np
from collections import OrderedDict

NUM_PARTS = 26
IM_ROWS = 153
IM_COLS = 50
IM_SQ_SIZE = 150
IM_DEPT = 3
RAND_SEED = 1984
EVAL_PERCENTAGE = 0.2
DATA_ROOT = '/home/edogan/data/HumanEva/'
#DATA_ROOT = '/media/emredog/research-data/HumanEva-I/'

# Training data
data_file = DATA_ROOT + 'HE_train_paths_marg20.pkl'
aug_data_file = DATA_ROOT + 'HE_train_aug_paths_marg20.pkl'

bin_data_file = DATA_ROOT + 'HE_train_bin_marg20.pkl'
bin_aug_data_file = DATA_ROOT + 'HE_train_aug_bin_marg20.pkl'

# Test data
test_data_file = DATA_ROOT + 'HE_test_paths_marg20.pkl'
bin_test_data_file = DATA_ROOT + 'HE_test_bin_marg20.pkl'

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


def load_test_data():	
	# OUTPUT:
	# 	X_test:		test images (nbTrain x IM_COLS x IM_ROWS x IM_DEPT), uint8
	# 	y_test:		test labels (nbTrain x NUM_PARTS floats)	

	try:		
		d = pickle.load(open(bin_test_data_file, 'rb'))
		X_test, y_test = d['X_test'], d['y_test']

		X_test = scale_and_normalize(X_test)
		return X_test, y_test
	except: # file not found
		print('Raw image data is not found. Reading images from HDD & resizing. This may take a few minutes...')
		
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
			except IOError as er:
				print('Not found: %s' % impath)
				print(er)
				continue

			if i%1000==0:
				print('Processed', i, '/', nbSamples)

		# save data for later use:
		print('Dumping test data for later use...')
		dataset = OrderedDict([('X_test', X_test), ('y_test', y_test)])
		pickle.dump(dataset, open(bin_test_data_file, 'wb'))
		X_test = scale_and_normalize(X_test)
		return X_test, y_test


def load_aug_data():	
	# OUTPUT:
	# 	X_train:	train images (nbTrain x IM_COLS x IM_ROWS x IM_DEPT), uint8
	# 	y_train:	train labels (nbTrain x NUM_PARTS floats)
	#	X_val: 	evaluation images (nbEval x IM_COLS x IM_ROWS x IM_DEPT), uint8
	#	y_val:		evaluation labels (nbEval x NUM_PARTS floats)  

	try:		
		d = pickle.load(open(bin_aug_data_file, 'rb'))
		X_train, y_train, X_val, y_val = d['X_train'], d['y_train'], d['X_val'], d['y_val']

		X_train = scale_and_normalize(X_train)
		X_val = scale_and_normalize(X_val)
		return X_train, y_train, X_val, y_val
	except: # file not found
		print('Raw image data is not found. Reading images from HDD & resizing. This may take a few minutes...')
		
		with open(aug_data_file, 'rb') as f:
			fulldict = pickle.load(f)



		# this is prepared manually to pick non-augmented images from the end of all sequences of all Subjects
		manIdxForVal = range(912,992,4)+range(1904,1984,4)+range(2896,2976,4)+range(4400,4600,4)+range(6024,6224,4)+range(7648,7848,4)+range(8276,8316,4)+range(8744,8784,4)+range(9212,9252,4)+range(10040,10120,4)+range(10908,10988,4)+range(11776,11856,4)+range(13988,14308,4)+range(16440,16760,4)+range(18892,19212,4)+range(19872,20032,4)+range(20692,20852,4)+range(21512,21672,4)+range(22572,22732,4)+range(23632,23792,4)+range(24692,24852,4)+range(26240,26440,4)+range(27828,28028,4)+range(29416,29616,4)+range(31208,31408,4)+range(33000,33200,4)+range(34792,34992,4)+range(36544,36744,4)+range(38296,38496,4)+range(40048,40248,4)+range(41732,41932,4)+range(43416,43616,4)+range(45100,45300,4)+range(45440,45480,4)+range(45620,45660,4)+range(45800,45840,4)+range(47356,47556,4)+range(49072,49272,4)+range(50792,50992,4)+range(52700,52900,4)+range(54608,54808,4)+range(56520,56720,4)

		nbSamples = len(fulldict)
		print('# of total samples:', nbSamples)	
		nbTrain = nbSamples-len(manIdxForVal)
		print('# of training samples:', nbTrain)
		nbEval = nbSamples-nbTrain		

		# init data
		X_train = np.zeros((nbTrain, IM_ROWS, IM_COLS, IM_DEPT)).astype('uint8')	
		y_train = np.zeros((nbTrain, NUM_PARTS)).astype('float32')
		X_val  = np.zeros((nbEval, IM_ROWS, IM_COLS, IM_DEPT)).astype('uint8')
		y_val  = np.zeros((nbEval, NUM_PARTS)).astype('float32')


		idx = list(range(nbSamples))				
		valCounter = 0
		trainCounter = 0
		for i, iid in enumerate(idx):
			samp = fulldict[iid] 
			impath = samp['Path']
			pbErr = samp['PbErrors']

			try:
				# read image
				resizedImg = imresize(imread(impath, mode='RGB'), (IM_ROWS, IM_COLS))
			except IOError as er:
				print('Not found: %s' % impath)
				print(er)
				continue

			if i in manIdxForVal: # add to validation set
				X_val[valCounter] = resizedImg
				y_val[valCounter] = np.asarray(pbErr)
				valCounter += 1
			else:
				X_train[trainCounter] = resizedImg
				y_train[trainCounter] = np.asarray(pbErr)
				trainCounter += 1

			if i%1000==0:
				print('Processed', i, '/', nbSamples)

		# save data for later use:
		print('Dumping data for later use...')
		dataset = OrderedDict([('X_train', X_train), ('y_train', y_train), ('X_val',  X_val), ('y_val', y_val)])
		pickle.dump(dataset, open(bin_aug_data_file, 'wb'))
		X_train = scale_and_normalize(X_train)
		X_val = scale_and_normalize(X_val)
		return X_train, y_train, X_val, y_val

	

def load_data():	
	# OUTPUT:
	# 	X_train:	train images (nbTrain x IM_COLS x IM_ROWS x IM_DEPT), uint8
	# 	y_train:	train labels (nbTrain x NUM_PARTS floats)
	#	X_val: 	evaluation images (nbEval x IM_COLS x IM_ROWS x IM_DEPT), uint8
	#	y_val:		evaluation labels (nbEval x NUM_PARTS floats)  

	try:		
		d = pickle.load(open(bin_data_file, 'rb'))
		X_train, y_train, X_val, y_val = d['X_train'], d['y_train'], d['X_val'], d['y_val']

		X_train = scale_and_normalize(X_train)
		X_val = scale_and_normalize(X_val)
		return X_train, y_train, X_val, y_val
	except: # file not found
		print('Raw image data is not found. Reading images & rezising from HDD...')
		
		with open(data_file, 'rb') as f:
			fulldict = pickle.load(f)

		nbSamples = len(fulldict)
		print('# of total samples:', nbSamples)
		nbTrain = int(round(nbSamples*(1.0-EVAL_PERCENTAGE)))
		print('# of training samples:', nbTrain)
		nbEval = nbSamples - nbTrain

		# init data
		X_train = np.zeros((nbTrain, IM_ROWS, IM_COLS, IM_DEPT)).astype('uint8')	
		y_train = np.zeros((nbTrain, NUM_PARTS)).astype('float32')
		X_val  = np.zeros((nbEval, IM_ROWS, IM_COLS, IM_DEPT)).astype('uint8')
		y_val  = np.zeros((nbEval, NUM_PARTS)).astype('float32')


		idx = range(nbSamples)		

		for i, iid in enumerate(idx):
			samp = fulldict[iid]
			impath = samp['Path']
			pbErr = samp['PbErrors']

			# read image
			resizedImg = imresize(imread(impath, mode='RGB'), (IM_ROWS, IM_COLS))

			if i < nbTrain: # add to train set
				X_train[i] = resizedImg
				y_train[i] = np.asarray(pbErr)
			else:
				X_val[i-nbTrain] = resizedImg
				y_val[i-nbTrain] = np.asarray(pbErr)

			if i%1000==0:
				print('Processed', i, '/', nbSamples)

		# save data for later use:
		print('Dumping data for later use...')
		dataset = OrderedDict([('X_train', X_train), ('y_train', y_train), ('X_val',  X_val), ('y_val', y_val)])
		pickle.dump(dataset, open(bin_data_file, 'wb'))

		X_train = scale_and_normalize(X_train)
		X_val = scale_and_normalize(X_val)
		return X_train, y_train, X_val, y_val
	


def load_tiny_data(nbTinyTrain, nbTinyEval):
	# INPUT:	
	#
	# OUTPUT:
	# 	X_train:	train images (nbTrain x IM_COLS x IM_ROWS x IM_DEPT), uint8
	# 	y_train:	train labels (nbTrain x NUM_PARTS floats)
	#	X_val: 	evaluation images (nbEval x IM_COLS x IM_ROWS x IM_DEPT), uint8
	#	y_val:		evaluation labels (nbEval x NUM_PARTS floats)  

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
	X_val  = np.zeros((nbTinyEval, IM_ROWS, IM_COLS, IM_DEPT)).astype('uint8')
	y_val  = np.zeros((nbTinyEval, NUM_PARTS)).astype('float32')


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
			X_val[i-nbTinyTrain] = resizedImg
			y_val[i-nbTinyTrain] = np.asarray(pbErr)
		else:
			return X_train, y_train, X_val, y_val			

		if i%1000==0:
			print('Processed', i, '/', nbSamples)

	X_train = scale_and_normalize(X_train)
	X_val = scale_and_normalize(X_val)
	return X_train, y_train, X_val, y_val