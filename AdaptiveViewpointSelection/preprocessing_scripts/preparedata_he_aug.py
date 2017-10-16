#!/usr/bin/env python

#import os
#import sys
#import time
import csv
import cPickle as pickle
import argparse

#import numpy as np

ISTRAIN = False
ISTEST = True
ISOLDCSV = False
#DATA_ROOT = '/home/edogan/data/UMPM/'
#CSV_PATH = '/home/edogan/data/wfv_data/'
DATA_ROOT = '/media/emredog/research-data/HumanEva-I/'
CSV_PATH = '/home/emredog/git/yrposeestim/yr_multiview_matlab/pose-release-FULL-ver1.3/code-basic/WeightsForViewpoints/wfv_data/output_gt/'


if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog='preparedata_he_aug')
    
    isAugment = False

    if ISTRAIN:
        print('Working on TRAIN set.')
        augImgDir = DATA_ROOT + 'train_aug_marg20/'
        outputFile = DATA_ROOT + 'HE_train_paths_marg20.pkl'
        parser.add_argument('-a', '--augment', help='Flag for augmenting the data, >0 means true', type=int, required=True)
        args = parser.parse_args()

        if args.augment > 0:
            isAugment = True        
            outputFile = DATA_ROOT + 'HE_train_aug_paths_marg20.pkl'
        
    elif ISTEST:
        print('Working on TEST set.')
        augImgDir = DATA_ROOT + 'test_bulk_marg20/'    
        outputFile = DATA_ROOT + 'HE_test_paths_marg20.pkl'
    else:
        print('ERROR: You should either select Train/Test')


    nbParts = 26

    actAbrevs = {'ThrowCatch':'thr', 'Walking':'wal', 'Box':'box', 'Gestures':'ges', 'Jog':'jog'}

    # csv reader:
    if ISTRAIN:
        csvfile=open(CSV_PATH + '20160425_he_settrain_solo_PbErrs.csv')
    else:
        csvfile=open(CSV_PATH + '20160307_he09_mv_test_PbErrs.csv')
    reader = csv.DictReader(csvfile)

    fullDict = {}

    counter = 0    
    if not ISOLDCSV: # new csv, 1 line per frame
        # for each entry:
        for row in reader:
            if counter % 1000 == 0 and counter > 0:
                print('Processed', counter)

            subj = row['Subject']
            if not subj.startswith('S'):
                subj = 'S' + subj
            act = row['Action']
            view = row['View']
            if not view.startswith('C'):
                view = 'C' + view
            frame = row['Frame']
            errList = (row['Err_Pt01'], row['Err_Pt02'], row['Err_Pt03'], row['Err_Pt04'], row['Err_Pt05'],
                       row['Err_Pt06'], row['Err_Pt07'], row['Err_Pt08'], row['Err_Pt09'], row['Err_Pt10'],
                       row['Err_Pt11'], row['Err_Pt12'], row['Err_Pt13'], row['Err_Pt14'], row['Err_Pt15'],
                       row['Err_Pt06'], row['Err_Pt17'], row['Err_Pt18'], row['Err_Pt19'], row['Err_Pt20'],
                       row['Err_Pt21'], row['Err_Pt22'], row['Err_Pt23'], row['Err_Pt24'], row['Err_Pt25'], row['Err_Pt26'])

            # orig: s1_box_c1_im0386.png
            imgPath = '%s%s_%s_%s_im%.4d.png' % (augImgDir, subj.replace('S', 's'), 
                                                    actAbrevs[act], view.replace('C', 'c'), int(frame)) 
            frameDict = {'Subject':subj, 'Action':act, 'View':view, 'Frame':frame, 'PbErrors':errList, 'Path':imgPath}
            fullDict[counter] = frameDict # append to main dictionary
            counter = counter+1

            if isAugment:
                # flip: s1_box_c1_flip_im0386.png
                imgPath = '%s%s_%s_%s_flip_im%.4d.png' % (augImgDir, subj.replace('S', 's'), 
                                                        actAbrevs[act], view.replace('C', 'c'), int(frame)) 
                
                frameDict = {'Subject':subj, 'Action':act, 'View':view, 'Frame':frame, 'PbErrors':errList, 'Path':imgPath}
                fullDict[counter] = frameDict # append to main dictionary
                counter = counter+1

                # blur: s1_box_c1_blur_im0386.png
                imgPath = '%s%s_%s_%s_blur_im%.4d.png' % (augImgDir, subj.replace('S', 's'), 
                                                        actAbrevs[act], view.replace('C', 'c'), int(frame)) 
                
                frameDict = {'Subject':subj, 'Action':act, 'View':view, 'Frame':frame, 'PbErrors':errList, 'Path':imgPath}
                fullDict[counter] = frameDict # append to main dictionary
                counter = counter+1

                # noise: s1_box_c1_blur_im0386.png
                imgPath = '%s%s_%s_%s_noise_im%.4d.png' % (augImgDir, subj.replace('S', 's'), 
                                                        actAbrevs[act], view.replace('C', 'c'), int(frame)) 
                
                frameDict = {'Subject':subj, 'Action':act, 'View':view, 'Frame':frame, 'PbErrors':errList, 'Path':imgPath}
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

                subj = row['Subject']
                act = row['Action']
                view = row['View']
                frame = row['Frame']

                # orig: s1_box_c1_im0386.png
                imgPath = '%s%s_%s_%s_im%.4d.png' % (augImgDir, subj.replace('S', 's'), 
                                                        actAbrevs[act], view.replace('C', 'c'), int(frame)) 
                
                frameDict = {'Subject':subj, 'Action':act, 'View':view, 'Frame':frame, 'PbErrors':errList, 'Path':imgPath}
                fullDict[counter] = frameDict # append to main dictionary
                counter = counter+1

                if isAugment:
                    # flip: s1_box_c1_flip_im0386.png
                    imgPath = '%s%s_%s_%s_flip_im%.4d.png' % (augImgDir, subj.replace('S', 's'), 
                                                            actAbrevs[act], view.replace('C', 'c'), int(frame)) 
                    
                    frameDict = {'Subject':subj, 'Action':act, 'View':view, 'Frame':frame, 'PbErrors':errList, 'Path':imgPath}
                    fullDict[counter] = frameDict # append to main dictionary
                    counter = counter+1

                    # blur: s1_box_c1_blur_im0386.png
                    imgPath = '%s%s_%s_%s_blur_im%.4d.png' % (augImgDir, subj.replace('S', 's'), 
                                                            actAbrevs[act], view.replace('C', 'c'), int(frame)) 
                    
                    frameDict = {'Subject':subj, 'Action':act, 'View':view, 'Frame':frame, 'PbErrors':errList, 'Path':imgPath}
                    fullDict[counter] = frameDict # append to main dictionary
                    counter = counter+1

                    # noise: s1_box_c1_blur_im0386.png
                    imgPath = '%s%s_%s_%s_noise_im%.4d.png' % (augImgDir, subj.replace('S', 's'), 
                                                            actAbrevs[act], view.replace('C', 'c'), int(frame)) 
                    
                    frameDict = {'Subject':subj, 'Action':act, 'View':view, 'Frame':frame, 'PbErrors':errList, 'Path':imgPath}
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
