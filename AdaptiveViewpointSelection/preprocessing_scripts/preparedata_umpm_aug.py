#!/usr/bin/env python

#import os
#import sys
#import time
import csv
import pickle
import argparse

#import numpy as np

ISTRAIN = False
ISVALIDATION = True
ISTEST = False
ISOLDCSV = False
DATA_ROOT = '/home/edogan/data/UMPM/'
CSV_PATH = '/home/edogan/data/wfv_data/'


if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog='preparedata_he_aug')
    
    isAugment = False

    if ISTRAIN:
        print('Working on TRAIN set.')
        augImgDir = DATA_ROOT + 'train_aug_marg20/'
        outputFile = DATA_ROOT + 'UMPM_train_paths_marg20.pkl'
        parser.add_argument('-a', '--augment', help='Flag for augmenting the data, >0 means true', type=int, required=True)
        args = parser.parse_args()

        if args.augment > 0:
            isAugment = True        
            outputFile = DATA_ROOT + 'UMPM_train_aug_paths_marg20.pkl'
        
    elif ISTEST:
        print('Working on TEST set.')
        augImgDir = DATA_ROOT + 'test_bulk_marg20/'    
        outputFile = DATA_ROOT + 'UMPM_test_paths_marg20.pkl'

    elif ISVALIDATION:
        print('Working on VALIDATION set.')
        augImgDir = DATA_ROOT + 'validation_bulk_marg20/'    
        outputFile = DATA_ROOT + 'UMPM_validation_paths_marg20.pkl'

    nbParts = 26    

    # csv reader:
    if ISTRAIN:
        csvfile=open(CSV_PATH + '20170217_umpm10_train_solo_PbErrs.csv')
    elif ISTEST:
        csvfile=open(CSV_PATH + '20161220_umpm10_mv_test_PbErrs.csv')
    elif ISVALIDATION:
        csvfile=open(CSV_PATH + '20170220_umpm10_validate_solo_PbErrs.csv')
    reader = csv.DictReader(csvfile)

    fullDict = {}

    counter = 0
    if not ISOLDCSV: # new CSV (one line per frame)
        # for each entry
        for row in reader:
            if counter % 1000 == 0 and counter > 0:
                print('Processed', counter)

            act = row['Action']
            if act.endswith('_'):
                act = act[:-1] # remove last char, if there's an unnecessary underscore
            view = row['View']
            frame = row['Frame']
            errList = (row['Err_Pt01'], row['Err_Pt02'], row['Err_Pt03'], row['Err_Pt04'], row['Err_Pt05'],
                       row['Err_Pt06'], row['Err_Pt07'], row['Err_Pt08'], row['Err_Pt09'], row['Err_Pt10'],
                       row['Err_Pt11'], row['Err_Pt12'], row['Err_Pt13'], row['Err_Pt14'], row['Err_Pt15'],
                       row['Err_Pt06'], row['Err_Pt17'], row['Err_Pt18'], row['Err_Pt19'], row['Err_Pt20'],
                       row['Err_Pt21'], row['Err_Pt22'], row['Err_Pt23'], row['Err_Pt24'], row['Err_Pt25'], row['Err_Pt26'])

            # orig: p1_grab_3_r_im0001.png
            imgPath = '%s%s_%s_im%.4d.png' % (augImgDir, act, view, int(frame)) 
            
            frameDict = {'Action':act, 'View':view, 'Frame':frame, 'PbErrors':errList, 'Path':imgPath}
            fullDict[counter] = frameDict # append to main dictionary
            counter = counter+1

            if isAugment:
                # flip: p1_grab_3_s_flip_im0001.png
                imgPath = '%s%s_%s_flip_im%.4d.png' % (augImgDir, act, view, int(frame)) 
                    
                frameDict = {'Action':act, 'View':view, 'Frame':frame, 'PbErrors':errList, 'Path':imgPath}
                fullDict[counter] = frameDict # append to main dictionary
                counter = counter+1

                # blur: p1_grab_3_s_blur_im0001.png
                imgPath = '%s%s_%s_blur_im%.4d.png' % (augImgDir, act, view, int(frame))
                    
                frameDict = {'Action':act, 'View':view, 'Frame':frame, 'PbErrors':errList, 'Path':imgPath}
                fullDict[counter] = frameDict # append to main dictionary
                counter = counter+1

                # noise: p1_grab_3_r_noise_im0001.png
                imgPath = '%s%s_%s_noise_im%.4d.png' % (augImgDir, act, view, int(frame))
                    
                frameDict = {'Action':act, 'View':view, 'Frame':frame, 'PbErrors':errList, 'Path':imgPath}
                fullDict[counter] = frameDict # append to main dictionary
                counter = counter+1

    else: # old CSV (one line for per part)
        partCounter = 0
        errList = list()
        # for each entry:
        for row in reader:        
            partCounter = partCounter+1

            # after 26 parts, write a new entry for a frame
            if partCounter == 26:            
                errList.append(float(row['Error'])) # add the last pb error too   

                if (int(row['PartId']) != 26 or len(errList)!=26 ): # check for errors
                    print('PartId', int(row['PartId']))
                    print('Err len', len(errList))
                    raise ValueError('PartId is not 26!!')

                partCounter = 0            
            
                act = row['Action']
                view = row['View']
                frame = row['Frame']

                # orig: p1_grab_3_r_im0001.png
                imgPath = '%s%s_%s_im%.4d.png' % (augImgDir, act, view, int(frame)) 
            
                frameDict = {'Action':act, 'View':view, 'Frame':frame, 'PbErrors':errList, 'Path':imgPath}
                fullDict[counter] = frameDict # append to main dictionary
                counter = counter+1

                if isAugment:
                    # flip: p1_grab_3_s_flip_im0001.png
                    imgPath = '%s%s_%s_flip_im%.4d.png' % (augImgDir, act, view, int(frame)) 
                    
                    frameDict = {'Action':act, 'View':view, 'Frame':frame, 'PbErrors':errList, 'Path':imgPath}
                    fullDict[counter] = frameDict # append to main dictionary
                    counter = counter+1

                    # blur: p1_grab_3_s_blur_im0001.png
                    imgPath = '%s%s_%s_blur_im%.4d.png' % (augImgDir, act, view, int(frame))
                    
                    frameDict = {'Action':act, 'View':view, 'Frame':frame, 'PbErrors':errList, 'Path':imgPath}
                    fullDict[counter] = frameDict # append to main dictionary
                    counter = counter+1

                    # noise: p1_grab_3_r_noise_im0001.png
                    imgPath = '%s%s_%s_noise_im%.4d.png' % (augImgDir, act, view, int(frame))
                    
                    frameDict = {'Action':act, 'View':view, 'Frame':frame, 'PbErrors':errList, 'Path':imgPath}
                    fullDict[counter] = frameDict # append to main dictionary
                    counter = counter+1

                errList = list() # clears the list 
                if counter % 1000 == 0 and counter > 0:
                    print('Processed', counter)
            else:            
                errList.append(float(row['Error']))

    print('CSV is now parsed. Saving the output...')

    f = open(outputFile, 'wb')
    pickle.dump(fullDict, f, pickle.HIGHEST_PROTOCOL)
    print('Done.')

