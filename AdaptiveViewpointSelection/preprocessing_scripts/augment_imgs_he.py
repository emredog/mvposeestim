#!/usr/bin/env python

import os
from scipy.misc import imread, imsave, imresize
from imgaug import augmenters as iaa # for image augmentation, from https://github.com/aleju/imgaug

IM_ROWS = 153
IM_COLS = 50
ISAUG = False
ISTRAIN = False
ISTEST = True
DATA_ROOT = '/home/edogan/data/UMPM/'

if __name__ == '__main__':	
	
	if ISTRAIN:
		print('Working on TRAIN set...')
		croppedImgDir = DATA_ROOT + 'train_cropped_marg20/'
		targetDir = DATA_ROOT + 'train_aug_marg20/'
	elif ISTEST:
		print('Working on TEST set...')
		croppedImgDir = DATA_ROOT + 'test_cropped_marg20/'
		targetDir = DATA_ROOT + 'test_bulk_marg20/'	
	else:
		raise NameError('ERROR: should either select train/test')

	if not os.path.exists(targetDir):
		os.makedirs(targetDir)

	print 'FROM', croppedImgDir
	print 'TO', targetDir

	# prepare augmenters
	flipper = iaa.Fliplr(1.0)
	blurer = iaa.GaussianBlur(sigma=0.75)
	gausNoise = iaa.AdditiveGaussianNoise(scale=0.25)

	# traverse root directory, and list directories as dirs and files as files
	for root, dirs, files in os.walk(croppedImgDir):
		path = root.split(os.sep)			
		print('Processing %s...' % root)		
		seqName = path[-1] # last element
		seqName = seqName.replace('S', 's')
		seqName = seqName.replace('C', 'c')
		seqName = seqName.replace('(', '')
		seqName = seqName.replace(')', '')
		seqName = seqName.replace('Throwcatch_1', 'thr')
		seqName = seqName.replace('Walking_1', 'wal')
		seqName = seqName.replace('Box_1', 'box')
		seqName = seqName.replace('Gestures_1', 'ges')
		seqName = seqName.replace('Jog_1', 'jog')

		for f in files:
			filename_orig = '%s_%s' % (seqName, f)
			filename_flip = '%s_flip_%s' % (seqName, f)
			filename_blur = '%s_blur_%s' % (seqName, f)
			filename_noise = '%s_noise_%s' % (seqName, f)
			sourcePath = os.path.join(root, f)
			targetPath_orig = os.path.join(targetDir, filename_orig)
			targetPath_flip = os.path.join(targetDir, filename_flip)
			targetPath_blur = os.path.join(targetDir, filename_blur)
			targetPath_noise = os.path.join(targetDir, filename_noise)
			
			# read & resize img
			img = imresize(imread(sourcePath, mode='RGB'), (IM_ROWS, IM_COLS))						
			# save all
			imsave(targetPath_orig, img)
			if ISAUG:
				# augment image
				img_flip = flipper.augment_image(img)
				img_blur = blurer.augment_image(img)
				img_noise = gausNoise.augment_image(img)
				imsave(targetPath_flip, img_flip)
				imsave(targetPath_blur, img_blur)
				imsave(targetPath_noise, img_noise)



