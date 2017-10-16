import datetime as dt
import cPickle as pickle
import sys,os
from keras.callbacks import Callback

class LossHistory(Callback):
    def __init__(self, suffix=''):
        self.suffix = suffix
    def on_train_begin(self, logs={}):
    	now = dt.datetime.now()
    	self.filename = '/home/edogan/checkpoints/%d%.2d%.2d-%.2d%.2d%.2d_%s_history.pkl' % (now.year, now.month, now.day, now.hour, now.minute, now.second, self.suffix)
        self.histDict = {'loss':list(), 'val_loss':list(),
        				 'mean_squared_error':list(), 'val_mean_squared_error':list()}

    def on_epoch_end(self, epoch, logs={}):
        self.histDict['loss'].append(logs.get('loss'))
        self.histDict['val_loss'].append(logs.get('val_loss'))
        self.histDict['mean_squared_error'].append(logs.get('mean_squared_error'))
        self.histDict['val_mean_squared_error'].append(logs.get('val_mean_squared_error'))
        
        try:
        	os.remove(self.filename)
    	except:
    		pass

    	with open(self.filename, 'wb') as f:
    		pickle.dump(self.histDict, f)
