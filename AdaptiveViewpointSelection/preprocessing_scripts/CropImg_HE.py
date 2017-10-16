#!/usr/bin/env python

import csv
import argparse
import os.path
# Import Pillow:
from PIL import Image


if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog='CropImg_HE')

    parser.add_argument('-m', '--margin', help='amount of extra margin around the detections.', type=int, required=True)
    parser.add_argument('-b', '--bboxes', help='csv file with bounding box (use a data without Margin) info e.g. ~/data/HumanEva/yr_bboxes/he_train_boxes_noMarg.csv', type=str, required=True)
    parser.add_argument('-i', '--inDir',  help='input dir e.g. ~/data/HumanEva/set_TRAIN/', type=str, required=True)
    parser.add_argument('-o', '--outDir', help='input dir e.g. ~/data/HumanEva/cropped/', type=str, required=True)

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

        subj = row['Subject']
        act = row['Action']
        view = row['View']
        if not view.startswith('C'):
            view = 'C' + view
        frame = row['Frame']
        Xmin = float(row['Xmin'])
        Ymin = float(row['Ymin'])
        Xmax = float(row['Xmax'])
        Ymax = float(row['Ymax'])

        targetImPath = '%sS%s_%s_1_(%s)/im%.4d.png' % (outDir, subj, act, view, int(frame))
        if os.path.exists(targetImPath):
            continue

        # check for path, makedir if necessary
        if not os.path.exists('%sS%s_%s_1_(%s)/' % (outDir, subj, act, view)):
            os.makedirs('%sS%s_%s_1_(%s)/' % (outDir, subj, act, view))

        try:
            im = Image.open('%sS%s_%s_1_(%s)/im%.4d.bmp' % (inDir, subj, act, view, int(frame)))
            imCropped = im.crop((Xmin-margin, Ymin-margin, Xmax+margin, Ymax+margin))
            imCropped.save(targetImPath)
        except Exception as e:
            print(targetImPath)
            print(e)