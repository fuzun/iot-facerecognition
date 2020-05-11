/*
*    iot-facerecognition-server
*
*    Copyright (C) 2020, fuzun
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "DLIBWorker.h"

#include <QVector>
#include <QPixmap>
#include <QSettings>
#include <QFile>
#include <utility>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "dlib/dnn.h"
#include "dlib/image_io.h"
#include "dlib/image_processing/frontal_face_detector.h"
#include "dlib/opencv.h"

#include "config.h"

using namespace dlib;
using namespace std;
using namespace cv;

using FaceMap = DLIBWorker::FaceMap;

DLIBWorker::DLIBWorker(QObject *parent, class QSettings* config)
    : QObject(parent)
{  
    config->beginGroup(CONFIG_DLIB);
    m_faceLandmarkModelFile = config->value(CONFIG_DLIB_FACELANDMARKFILE, CONFIG_DLIB_DEFAULT_FACELANDMARKFILE).toString();
    m_faceRecognitionModelFile = config->value(CONFIG_DLIB_FACERECOGNITIONFILE, CONFIG_DLIB_DEFAULT_FACERECOGNITIONFILE).toString();
    m_threshold = config->value(CONFIG_DLIB_THRESHOLD, CONFIG_DLIB_DEFAULT_THRESHOLD).toDouble();
    m_faceDetailSize = config->value(CONFIG_DLIB_FACE_DETAILSIZE, CONFIG_DLIB_DEFAULT_FACE_DETAILSIZE).toUInt();
    m_faceDetailPadding = config->value(CONFIG_DLIB_FACE_PADDING, CONFIG_DLIB_DEFAULT_FACE_PADDING).toDouble();
    QString _refFile = config->value(CONFIG_DLIB_REFERENCEFACEFILE, CONFIG_DLIB_DEFAULT_REFERENCEFACEFILE).toString();
    config->endGroup();

    QFile refFile(_refFile);
    if(!refFile.open(QIODevice::ReadOnly))
    {
        emit throwException("Reference photo list file can not be opened!");
    }
    else
    {
        while(!refFile.atEnd())
        {
            QString line = refFile.readLine();
            QStringList lineSplit = line.split(":");
            m_refPhotoFileList.push_back(qMakePair(lineSplit[0].trimmed(), lineSplit[1].trimmed()));
        }
    }
}

FaceMap DLIBWorker::generateFaceMap(const QByteArray& photoData, const Mat* _mat)
{
    Mat mat;
    if(_mat)
        mat = *_mat;
    else
        mat = constructMatFromBuffer(photoData);

    cv_image<bgr_pixel> cvIm(mat);
    matrix<rgb_pixel> matrixIm;
    assign_image(matrixIm, cvIm);

    std::vector<matrix<rgb_pixel>> extractedFaces;
    std::vector<dlib::rectangle> extractedFaces2;
    for (auto face : detector(matrixIm))
    {
        auto shape = sp(matrixIm, face);
        matrix<rgb_pixel> face_chip;
        extract_image_chip(matrixIm, get_face_chip_details(shape, m_faceDetailSize, m_faceDetailPadding), face_chip);
        extractedFaces.push_back(move(face_chip));
        extractedFaces2.push_back(face);
    }

    FaceMap facemap;
    if(extractedFaces.size() == 0)
    {
        return facemap;
    }

    std::vector<dlib::matrix<float, 0, 1>> face_descriptors = net(extractedFaces);
    facemap.resize(face_descriptors.size());

    for(size_t i = 0; i < face_descriptors.size(); ++i)
    {
        facemap[i] = std::make_pair(extractedFaces2.at(i), face_descriptors.at(i));
    }

    return facemap;
}

FaceMap DLIBWorker::generateFaceMap(const QString& fileName)
{
    std::string fName = samples::findFile(fileName.toStdString());
    Mat mat = cv::imread(fName, IMREAD_COLOR);
    return generateFaceMap(QByteArray(), &mat);
}

void DLIBWorker::generateReferenceFaceMap(const QVector<QPair<QString, QString>>& filename)
{
    QVector<QPair<QString, FaceMap>> rMap;

    for(const auto& it : filename)
    {
        auto genFMap = generateFaceMap(it.second);
        QPair<QString, FaceMap> pair;
        pair.first = it.first;
        for(const auto& it2 : genFMap)
        {
            pair.second.push_back(it2);
        }
        rMap.push_back(pair);
    }

    m_referenceFaceMap = rMap;
}

void DLIBWorker::run()
{
    if(m_referenceFaceMap.size() == 0)
    {
        if(m_refPhotoFileList.size() == 0)
        {
            throwException("Reference face list is empty!");
        }
        else
        {
            detector = get_frontal_face_detector();
            string landmarkModel = m_faceLandmarkModelFile.toStdString();
            string recogModel = m_faceRecognitionModelFile.toStdString();
            deserialize(landmarkModel) >> sp;
            deserialize(recogModel) >> net;

            generateReferenceFaceMap(m_refPhotoFileList);
        }
    }

    QVector<QPair<QRect, QString>> cResults;
    if(m_referenceFaceMap.size() > 0)
    {
        FaceMap faceMap = generateFaceMap(m_inputBuffer);

        for(const auto& it : m_referenceFaceMap)
        {
            const auto& get = compareFaceMap(faceMap, it);
            for(const auto& it2 : get)
            {
                cResults.push_back(it2);
            }
        }
    }

    emit done(cResults);
}

void DLIBWorker::setInputBuffer(const QByteArray &buffer)
{
    m_inputBuffer = buffer;
}

QVector<QPair<QRect, QString>> DLIBWorker::compareFaceMap(const FaceMap& a, const QPair<QString, FaceMap> &ref)
{
    QVector<QPair<QRect, QString>> ret;

    for (const auto& i : a)
    {
        QRect rect;
        rect.setY(i.first.top());
        rect.setX(i.first.left());
        rect.setHeight(i.first.height());
        rect.setWidth(i.first.width());
        QPair<QRect, QString> pair(rect, QString());
        ret.push_back(pair);

        for (const auto& j : ref.second)
        {
            if (length(i.second - j.second) < m_threshold)
            {
                // Face is known
                ret.pop_back();
                pair = qMakePair(rect, ref.first);
                ret.push_back(pair);
                break;
            }
        }
    }
    return ret;
}

Mat DLIBWorker::constructMatFromBuffer(const QByteArray& buffer)
{
    Mat mat;
    unsigned char* _buffer = (unsigned char*)buffer.constData();
    std::vector<unsigned char> vBuffer(_buffer, _buffer + buffer.size());
    mat = cv::imdecode(vBuffer, IMREAD_COLOR);
    return mat;
}
