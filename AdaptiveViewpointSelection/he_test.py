from keras.models import Sequential
from keras.optimizers import SGD
from keras.models import load_model
import numpy as np

# custom stuff
import he_load_data
from loss_history import LossHistory
import get_vgg_model
import get_he_topmodel

chk_path = '/home/edogan/checkpoints/'
#chk_path = '/media/emredog/research-data/HumanEva-I/cnn_checkpoints/'
chk_file = chk_path + 'finetune_he06_20170223-1739/weights-improvement-097-17.70.hdf5'
#chk_file = chk_path + 'finetune_he06_weights.hdf5'

prd_path = '/home/edogan/predictions/'
#prd_path = '/media/emredog/research-data/HumanEva-I/cnn_predictions/'
prd_file = prd_path + 'preds_finetune_he06.npy'
prd_file_train = prd_path + 'preds_finetune_he06_train.npy'
prd_file_val = prd_path + 'preds_finetune_he06_val.npy'

topModConf = [512]

batch_size = 128
nb_parts = he_load_data.NUM_PARTS

# input image dimensions
img_rows, img_cols = he_load_data.IM_ROWS, he_load_data.IM_COLS 
img_channels = 3

model = get_vgg_model.build_vgg_model(img_cols, img_rows)
print('Building the top model...')
top_model = get_he_topmodel.build_he_topmodel(model.output_shape[1:], fcLayers=topModConf)

model.add(top_model)

model.load_weights(chk_file)
print('Weights loaded: %s' % chk_file)

model.compile(loss='mse',
              optimizer=SGD, 
              metrics=['mean_squared_error'])

# print('Loading test data...')
# X_test, Y_test = he_load_data.load_test_data()
# print('Done.')

print('Loading train & validation data (non-augmented)...')
X_train, Y_train, X_val, Y_val = he_load_data.load_data()
print('Done.')

# print('Starting evaluation...')
# testRes = model.evaluate(X_test, Y_test, batch_size=batch_size, verbose=1)
# print(testRes)

# print('Starting prediction...')
# testPredictions = model.predict(X_test, batch_size=batch_size, verbose=1)

print('Starting evaluation on train set...')
trainRes = model.evaluate(X_train, Y_train, batch_size=batch_size, verbose=1)
print(trainRes)

print('Starting prediction on train set...')
trainPredictions = model.predict(X_train, batch_size=batch_size, verbose=1)

print('Starting evaluation on validation set...')
valRes = model.evaluate(X_val, Y_val, batch_size=batch_size, verbose=1)
print(valRes)

print('Starting prediction on validation set...')
valPredictions = model.predict(X_val, batch_size=batch_size, verbose=1)



# np.save(prd_file, testPredictions)
np.save(prd_file_train, trainPredictions)
np.save(prd_file_val, valPredictions)

