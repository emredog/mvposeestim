from keras.models import Sequential
from keras.optimizers import SGD
from keras.models import load_model
import numpy as np

# custom stuff
import umpm_load_data
from loss_history import LossHistory
import get_vgg_model
import get_umpm_topmodel

chk_path = '/home/edogan/checkpoints/'
chk_file = chk_path + 'finetune_umpm_16_20170221-1133/weights-improvement-022-129.35.hdf5'
prd_path = '/home/edogan/predictions/'
prd_file = prd_path + 'finetune_umpm_16.npy'
prd_file_val = prd_path + 'finetune_umpm_16_val.npy'

topModConf = [1024]

batch_size = 128
nb_parts = umpm_load_data.NUM_PARTS

# input image dimensions
img_rows, img_cols = umpm_load_data.IM_ROWS, umpm_load_data.IM_COLS 
img_channels = 3

model = get_vgg_model.build_vgg_model(img_cols, img_rows)
print('Building the top model...')
top_model = get_umpm_topmodel.build_umpm_topmodel(model.output_shape[1:], fcLayers=topModConf)

model.add(top_model)

model.summary()

# model.load_weights(chk_file)
# print('Weights loaded: %s' % chk_file)

# model.compile(loss='mse',
#               optimizer=SGD, 
#               metrics=['mean_squared_error'])

# print('Loading test data...')
# X_test, Y_test = umpm_load_data.load_test_data()
# print('Done.')

# print('Loading validation data...')
# X_val, Y_val = umpm_load_data.load_validation_data()
# print('Done.')

# # print('Starting evaluation...')
# # testRes = model.evaluate(X_test, Y_test, batch_size=batch_size, verbose=1)
# # print(testRes)

# # print('Starting prediction...')
# # testPredictions = model.predict(X_test, batch_size=batch_size, verbose=1)

# print('Starting evaluation on validation data...')
# valRes = model.evaluate(X_val, Y_val, batch_size=batch_size, verbose=1)
# print(valRes)

# print('Starting prediction on validation data...')
# valPredictions = model.predict(X_val, batch_size=batch_size, verbose=1)



# # np.save(prd_file, testPredictions)
# np.save(prd_file_val, valPredictions)

