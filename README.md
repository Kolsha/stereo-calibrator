# Stereo Calibrator

A visual tool for calibrating the stereoscopic system.
A chessboard is needed to perform the calibration.
The number of horizontal and vertical boxes, the size of the square in any unit of measurement (meters, cm, mm, etc.) are specified, this unit will be used later in any measurement. 

## Features

Provides some features required for camera calibration:

* Capture images from a stereo pair
* See, select, swap and resize images for calibration
* Different evaluation criteria of calibration: OpenCV reprojection error, epipolar error, error of triangulation(described in [paper](paper.pdf)), euclidean distance error(like a triangulation) , and combinations thereof
* Automatic calibration search with the lowest error criteria value (local minimum)

The screenshot shows the behavior of the calibration evaluation criteria: OpenCV at the top, triangulation at the bottom.

![Screenshots](/imgs/error.png "Screenshots")

A detailed description is given in the [paper](paper.pdf) (russian).

## Getting Started

### Dependencies

You will need the following before you can get started.

- Qt 4.8
- OpenCV 3.4

### Building

Pull the project and edit .pro file for correct openCV path and others.

If you have any questions, please contact me.
