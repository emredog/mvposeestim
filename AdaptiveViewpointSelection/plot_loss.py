#!/usr/bin/env python

import matplotlib.pyplot as plt
import numpy as np
import cPickle as pickle
import argparse


if __name__ == '__main__':

	parser = argparse.ArgumentParser(prog='plot_loss')
	parser.add_argument('-i', '--filepath', help='Pickle file with keras.History.hist dictionary', type=str, required=True)
	parser.add_argument('-limY', '--limY', help='Limit for Y axis', type=int, required=False, default=-1)
	args = parser.parse_args()
	filepath = args.filepath

	histDict = {}
	# load history file
	with open(filepath, 'rb') as f:
		histDict = pickle.load(f)

	# list all data in history
	print(histDict.keys())

	# summarize history for loss
	plt.plot(histDict['mean_squared_error'])
	plt.plot(histDict['val_mean_squared_error'])
	if args.limY > 0:		
		plt.ylim([0,args.limY])
	plt.title('model loss (MSE)')
	plt.ylabel('loss')
	plt.xlabel('epoch')
	plt.legend(['train', 'test'], loc='upper right')
	plt.show()

