//
//  Face.cpp
//  ManyMasks
//
//  Created by Joseph Howse on 2016-03-05.
//  Copyright © 2016 Nummist Media Corporation Limited. All rights reserved.
//

#include <opencv2/imgproc.hpp>

#include "Face.h"

Face::Face(Species species, const cv::Mat &mat, const cv::Point2f &leftEyeCenter, const cv::Point2f &rightEyeCenter, const cv::Point2f &noseTip)
: species(species)
, leftEyeCenter(leftEyeCenter)
, rightEyeCenter(rightEyeCenter)
, noseTip(noseTip)
{
    mat.copyTo(this->mat);
}

Face::Face() {
}

Face::Face(const Face &other)
: species(other.species)
, leftEyeCenter(other.leftEyeCenter)
, rightEyeCenter(other.rightEyeCenter)
, noseTip(other.noseTip)
{
    other.mat.copyTo(mat);
}

Face::Face(const Face &face0, const Face &face1) {
    if (face0.mat.total() > face1.mat.total()) {
        initMergedFace(face0, face1);
    } else {
        initMergedFace(face1, face0);
    }
}

bool Face::isEmpty() const {
    return mat.empty();
}

Species Face::getSpecies() const {
    return species;
}

const cv::Mat &Face::getMat() const {
    return mat;
}

int Face::getWidth() const {
    return mat.cols;
}

int Face::getHeight() const {
    return mat.rows;
}

const cv::Point2f &Face::getLeftEyeCenter() const {
    return leftEyeCenter;
}

const cv::Point2f &Face::getRightEyeCenter() const {
    return rightEyeCenter;
}

const cv::Point2f &Face::getNoseTip() const {
    return noseTip;
}

void Face::initMergedFace(const Face &biggerFace, const Face &smallerFace) {
    
    // Determine the species of the merged face.
    if (biggerFace.species == Human && smallerFace.species == Human) {
        species = Human;
    } else if (biggerFace.species == Cat && smallerFace.species == Cat) {
        species = Cat;
    } else {
        species = Hybrid;
    }
    
    // Warp the smaller face to align the eyes and nose with the bigger face.
    cv::Point2f srcPoints[3] = {
        smallerFace.getLeftEyeCenter(),
        smallerFace.getRightEyeCenter(),
        smallerFace.getNoseTip()
    };
    cv::Point2f dstPoints[3] = {
        biggerFace.leftEyeCenter,
        biggerFace.rightEyeCenter,
        biggerFace.noseTip
    };
    cv::Mat affineTransform = cv::getAffineTransform(srcPoints, dstPoints);
    cv::Size dstSize(biggerFace.mat.cols, biggerFace.mat.rows);
    cv::warpAffine(smallerFace.mat, mat, affineTransform, dstSize);
    
    // Perform any necessary color conversion.
    // Then, blend the warped face and the original bigger face.
    switch (mat.channels() - biggerFace.mat.channels()) {
        case 3: {
            // The warped face is BGRA and the bigger face is grayscale.
            cv::Mat otherMat;
            cv::cvtColor(biggerFace.mat, otherMat, cv::COLOR_GRAY2BGRA);
            cv::multiply(mat, otherMat, mat, 1.0 / 255.0);
            break;
        }
        case 2: {
            // The warped face is BGR and the bigger face is grayscale.
            cv::Mat otherMat;
            cv::cvtColor(biggerFace.mat, otherMat, cv::COLOR_GRAY2BGR);
            cv::multiply(mat, otherMat, mat, 1.0 / 255.0);
            break;
        }
        case 1: {
            // The warped face is BGRA and the bigger face is BGR.
            cv::Mat otherMat;
            cv::cvtColor(biggerFace.mat, otherMat, cv::COLOR_BGR2BGRA);
            cv::multiply(mat, otherMat, mat, 1.0 / 255.0);
            break;
        }
        case -1:
            // The warped face is BGR and the bigger face is BGRA.
            cv::cvtColor(mat, mat, cv::COLOR_BGR2BGRA);
            cv::multiply(mat, biggerFace.mat, mat, 1.0 / 255.0);
            break;
        case -2:
            // The warped face is grayscale and the bigger face is BGR.
            cv::cvtColor(mat, mat, cv::COLOR_GRAY2BGR);
            cv::multiply(mat, biggerFace.mat, mat, 1.0 / 255.0);
            break;
        case -3:
            // The warped face is grayscale and the bigger face is BGRA.
            cv::cvtColor(mat, mat, cv::COLOR_GRAY2BGRA);
            cv::multiply(mat, biggerFace.mat, mat, 1.0 / 255.0);
            break;
        default:
            // The color formats are the same.
            cv::multiply(mat, biggerFace.mat, mat, 1.0 / 255.0);
            break;
    }
    
    // The points of interest match the original bigger face.
    leftEyeCenter = biggerFace.leftEyeCenter;
    rightEyeCenter = biggerFace.rightEyeCenter;
    noseTip = biggerFace.noseTip;
}
