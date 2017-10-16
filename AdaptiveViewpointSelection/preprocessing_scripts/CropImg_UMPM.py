#!/usr/bin/env python

import csv
import argparse
import os.path
# Import Pillow:
from PIL import Image


if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog='CropImg_UMPM')

    parser.add_argument('-m', '--margin', help='amount of extra margin around the detections.', type=int, required=True)
    parser.add_argument('-b', '--bboxes', help='csv file with bounding box info e.g. ~/data/UMPM/yr_bboxes/umpm_train_boxes_noMarg.csv', type=str, required=True)
    parser.add_argument('-i', '--inDir', help='input dir e.g. ~/data/UMPM/Video_training/', type=str, required=True)
    parser.add_argument('-o', '--outDir', help='input dir e.g. ~/data/UMPM/cropped/', type=str, required=True)

    args = parser.parse_args()
    outDir = os.path.expanduser(args.outDir) # Resolve tilda ~
    inDir = os.path.expanduser(args.inDir) # Resolve tilda ~
    args.bboxes = os.path.expanduser(args.bboxes) # Resolve tilda ~
    margin = args.margin
    
    # csv reader:
    csvfile= open(args.bboxes)
    reader = csv.DictReader(csvfile)

    counter = 0
    # for each entry:
    for row in reader:
    	counter = counter+1

    	if counter % 100 == 0:
    		print(counter)
    	
    	act = row['Action']
	if act.endswith('_'):
		act = act[:-1] # remove last char, if there's an unnecessary underscore
    	view = row['View']
    	frame = row['Frame']
    	Xmin = float(row['Xmin'])
    	Ymin = float(row['Ymin'])
    	Xmax = float(row['Xmax'])
    	Ymax = float(row['Ymax'])
        
    	targetImPath = '%s%s_%s/im%.4d.png' % (outDir, act, view, int(frame))
    	if os.path.exists(targetImPath):
    		continue

    	# check for path, makedir if necessary
    	if not os.path.exists('%s%s_%s/' % (outDir, act, view)):
    		os.makedirs('%s%s_%s/' % (outDir, act, view))

	try:
    		im = Image.open('%s%s_%s/im%.4d.png' % (inDir, act, view, int(frame)))
		imCropped = im.crop((Xmin-margin, Ymin-margin, Xmax+margin, Ymax+margin))
    		imCropped.save(targetImPath)
	except Exception as e:
		print(targetImPath)
		print(e)






