import cPickle as pickle
import numpy as np
import csv

# # testpathfile = '/media/emredog/research-data/HumanEva-I/HE_test_paths_marg20.pkl'
# testpathfile = '/Users/emredog/git/yrposeestim/yr_multiview_matlab/pose-release-FULL-ver1.3/code-basic/WeightsForViewpoints/python/keras/HE_train_paths_marg20.pkl'
# #predictionsfile = '/Users/emredog/git/yrposeestim/yr_multiview_matlab/pose-release-FULL-ver1.3/code-basic/WeightsForViewpoints/python/keras/finetune_umpm_16.npy'
# predictionsfile = '/media/emredog/research-data/HumanEva-I/cnn_predictions/finetune_he06.npy'
# #outputfile = '/Users/emredog/git/yrposeestim/yr_multiview_matlab/pose-release-FULL-ver1.3/code-basic/WeightsForViewpoints/python/keras/finetune_umpm_16_predictions.csv'
# outputfile = '/home/emredog/git/yrposeestim/yr_multiview_matlab/pose-release-FULL-ver1.3/code-basic/WeightsForViewpoints/wfv_data/predictions/finetune_he_06_predictions.csv'

# # load data
# testpaths = pickle.load(open(testpathfile, 'rb'))
# pred = np.load(predictionsfile)

# # check for size
# if len(testpaths) != pred.shape[0]:
# 	raise ValueError

# # open file to write
# outFile = open(outputfile, 'wb')
# csvWriter = csv.writer(outFile)

# # for each data sample
# for i in range(len(testpaths)):
# 	row = list()
# 	row.append(testpaths[i]['Subject'])
# 	row.append(testpaths[i]['Action'])
# 	row.append(testpaths[i]['Frame'])
# 	row.append(testpaths[i]['View'])
# 	for p in range(len(pred[i])):
# 		row.append(pred[i][p])

# 	csvWriter.writerow(row)

# 	if i%1000 == 0:
# 		print('Processed %d / %d' % (i, len(testpaths)))

