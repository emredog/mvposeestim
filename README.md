# mvposeestim
Multi-view pose estimation with flexible mixtures of parts and adaptive viewpoint selection.

_**Please cite [1] if you use this implementation.**_

**Keywords** human pose estimation, flexible mixtures of parts, multi-view geometry

## General Info
The implementation of the multi-view human pose estimation method described in [1].
![Overview](https://github.com/emredog/mvposeestim/blob/master/resources/misc/overview.png)

The single-view version of the code is built on [2] and its [Matlab implementation](http://www.ics.uci.edu/~yyang8/research/pose/).

There are different modes of execution:
* **Single-view** - This is the simplest version, and it is basically the C++ version of the [2].
* **Single-view CUDA** - Same as above, but some functions are implemented so that they run on GPU
* **Multi-view (MV)** - Multi-view human pose estimation code, that uses only geometric constraints (see [1] for details).
* **Multi-view & Part Type Compatibility (MV_PTC)** - Multi-view human pose estimation code, that uses geometric constraints *and* appearance constraints (see [1] for details).
* **Multi-view & Part Type Compatibility + Adaptive Viewpoint Selection (MV_PTC + AVS)** Multi-view human pose estimation code, that uses geometric constraints, appearance constraints *and* adaptive viewpoint selection scheme (see [1] for details).

Multi-view versions has an iterative scheme for inference, which is demonstrated below.
![Overview](https://github.com/emredog/mvposeestim/blob/master/resources/misc/flow.png)

## Usage
Execution mode is controlled via compiler instructions. See examples below to get a sense.
```
make                          -> build target in release mode
make DEBUG=yes                -> build target in debug mode
make test                     -> to debug Makefile
make FPTYPE=float             -> use floats instead of doubles
make MV=yes                   -> perform mV pose estimation
make MV=yes AVS=yes           -> perform mV pose estimation with Adaptive Viewpoint Selection
make MV_PTC=yes               -> perform MultiView pose estimation with Part Type Compatibility
make MV_PTC=yes AVS=yes       -> perform MultiView pose estimation with Part Type Compatibility and Adaptive Viewpoint Selection
make MV=yes SAVE_TEXT=no      -> do not save results as text files
make MV=yes SAVE_IMAGES=yes   -> save results as text files AND images
make HUMANEVA=no UMPM=yes     -> code configured for UMPM dataset, instead of HUMANEVA

```
So, go to `build` directory, compile the code to fit your needs. Run it as follows:

```
./detect_fast_C ../resources/HE/S1_Walking_1_\(C1\)/im0080.bmp ../resources/HE/S1_Walking_1_\(C2\)/im0080.bmp ../resources/epi_lines/S1_C2wrtC1.csv ../resources/epi_lines/S1_C1wrtC2.csv ../resources/PTC/he_C2wrtC1.csv ../resources/PTC/he_C1wrtC2.csv 12 0.1 somefolder/S1_Walking_1_C1-wrt-C2 ../resources/model_he.txt 0.07 0.03
```
This example runs the code for the MV_PTC version (with or without AVS), for the 1st subject of the [HumanEva-I](http://humaneva.is.tue.mpg.de/) dataset, where the action is walking and the frame number is 80. Epipolar and PTC files are provided under the `resources` folder. See *Notes* for implementation details.

Tested on Ubuntu 16.04.

## Some results
![Quantitative Results](https://github.com/emredog/mvposeestim/blob/master/resources/misc/quantitative.png)


## Notes
* Both HumanEva and UMPM datasets provide calibration parameters. In this implementation, we used custom built comma separated files as look-up tables to query epipolar correspondence, which are provided under `resources/epi_lines` folder.
* Appearance constraints (PTC files) are learned as described in [1]. We provide the co-occurrennce matrices under `resources/PTC` folder.
* Training code for the single-view model is available on the original source. We provide 2 models with the code:
** **model_he.txt** - Model trained on HumanEva - S1, all actions. Note that following the literature, we divided the original training set into training+validation and used the original validation set as our test set.
** **model_umpm.txt** Model trained on UMPM - p1, all actions.
* Hyper-parameters (e.g. heatmap multiplier (contribution of the epipolar constraints) and PTC multiplier (contribution of the appearance constraints)) are learned on the validation sets. When in doubt, use 0.07 and 0.03 respectively. Or experiment around to find what works for you the best.
* **Adaptive viewpoint selection (1)** - In this implementation, we ran the error-estimation ConvNet offline, and recorded the outputs for every frame in the dataset. The files are under `resources` and with names `error_prediction_*.csv`. Therefore, we implemented the `EstimateYrErrorCNN` class as a simple fetching code. This class can be re-written, of course, to run the ConvNet directly.
* **Adaptive viewpoint selection (2)** - The ConvNet is implemented in [Keras](https://keras.io/), and code to preprocess data, train and test etc. can be found under `AdaptiveViewpointSelection` folder. It is basically a finetuned VGG net with some additional tweaks and FC layers at the end.
![ConvNet](https://github.com/emredog/mvposeestim/blob/master/resources/misc/VGG-Finetuned.jpg)

## Acknowledgements
Special thanks to [Eric Lombardi](https://liris.cnrs.fr/membres?idn=lombardi), who contributed significantly to this project, particularly on porting the code to C++ and also CUDA implementation of certain functions.

Thanks to [Christian Wolf](http://liris.cnrs.fr/christian.wolf/), [Atilla Baskurt](http://liris.cnrs.fr/atilla.baskurt/wiki/doku.php) and [Gonen Eren](https://www.linkedin.com/in/goneneren/) who guided me through various stages of this work.

## References 
[1] Multi-view pose estimation with flexible mixtures of parts and adaptive viewpoint selection [[arxiv]](https://arxiv.org/abs/1709.08527)

[2] Articulated pose estimation with flexible mixtures-of-parts [[ieee]](http://ieeexplore.ieee.org/abstract/document/5995741/)
