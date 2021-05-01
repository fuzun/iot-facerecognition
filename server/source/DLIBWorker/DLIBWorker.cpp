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
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <vector>

#include "Client/Client.h"

#include <dlib/clustering.h>
#include <dlib/image_io.h>

#ifdef TURBOJPEG_AVAILABLE
#include <turbojpeg.h>
#endif

#include "config.h"

using namespace dlib;
using namespace std;

using Face = DLIBWorker::Face;

DLIBWorker::DLIBWorker(class QSettings* config, const Settings *settings)
    : m_busy(false), m_settings(settings)
{  
    config->beginGroup(CONFIG_DLIB);

    m_faceLandmarkModelFile = config->value(CONFIG_DLIB_FACELANDMARKFILE, CONFIG_DLIB_DEFAULT_FACELANDMARKFILE).toString();
    m_faceRecognitionModelFile = config->value(CONFIG_DLIB_FACERECOGNITIONFILE, CONFIG_DLIB_DEFAULT_FACERECOGNITIONFILE).toString();
    m_threshold = config->value(CONFIG_DLIB_THRESHOLD, CONFIG_DLIB_DEFAULT_THRESHOLD).toDouble();
    m_faceDetailSize = config->value(CONFIG_DLIB_FACE_DETAILSIZE, CONFIG_DLIB_DEFAULT_FACE_DETAILSIZE).toUInt();
    m_faceDetailPadding = config->value(CONFIG_DLIB_FACE_PADDING, CONFIG_DLIB_DEFAULT_FACE_PADDING).toDouble();
    QString _refFile = config->value(CONFIG_DLIB_REFERENCEFACEFILE, CONFIG_DLIB_DEFAULT_REFERENCEFACEFILE).toString();
    m_imageNetClassifierFile = config->value(CONFIG_DLIB_IMAGENETCLASSIFIERFILE, CONFIG_DLIB_DEFAULT_IMAGENETCLASSIFIERFILE).toString();
    m_numCrops = config->value(CONFIG_DLIB_NUMCROPS, CONFIG_DLIB_DEFAULT_NUMCROPS).toULongLong();
    config->endGroup();

    QFile refFile(_refFile);
    if(refFile.open(QIODevice::ReadOnly))
    {
        QByteArray data = refFile.readAll();

        QJsonDocument doc(QJsonDocument::fromJson(data));

        for (auto&& i : doc.array())
        {
            if (!i.isObject())
                continue;

            QJsonObject obj = i.toObject();

            QString tag = obj.value("tag").toString();

            auto&& k = obj.value("photos");

            if (k.isArray())
            {
                for (auto&& j : k.toArray())
                {
                    m_refPhotoFileList.push_back(qMakePair(tag, j.toString()));
                }
            }
            else if (k.isString())
            {
                m_refPhotoFileList.push_back(qMakePair(tag, k.toString()));
            }
        }
    }

#ifdef TURBOJPEG_AVAILABLE
    QMetaObject::invokeMethod(this, [this]() {
        m_tjHandle = tjInitDecompress();
    }, Qt::QueuedConnection);
#endif

    QMetaObject::invokeMethod(this, [this]() {
        detector = get_frontal_face_detector();
        string landmarkModel = m_faceLandmarkModelFile.toStdString();
        string recogModel = m_faceRecognitionModelFile.toStdString();
        deserialize(landmarkModel) >> sp;
        deserialize(recogModel) >> net;
    }, Qt::QueuedConnection);
}

DLIBWorker::~DLIBWorker()
{
#ifdef TURBOJPEG_AVAILABLE
    if (m_tjHandle)
        tjDestroy(m_tjHandle);
#endif
}

dlib::array2d<rgb_pixel> DLIBWorker::decodeJPEG(const QByteArray &jpegBuffer, void *tjHandle)
{
    array2d<rgb_pixel> img;

#ifdef TURBOJPEG_AVAILABLE
    bool destroyHandle = false;

    if (tjHandle == nullptr)
    {
        tjHandle = tjInitDecompress();
        destroyHandle = true;
    }

    assert(tjHandle);

    int width, height;
    int jpegSubsamp, jpegColorspace;

    int ret;

    auto ptrBuffer = reinterpret_cast<const unsigned char*>(jpegBuffer.constData());

    ret = tjDecompressHeader3(tjHandle,
        ptrBuffer,
        jpegBuffer.size(),
        &width, &height,
        &jpegSubsamp, &jpegColorspace);

    if (ret || width <= 0 || height <= 0)
    {
        if (destroyHandle)
            tjDestroy(tjHandle);

        return img;
    }

    img.set_size(height, width);

    tjDecompress2(tjHandle,
                  ptrBuffer,
                  jpegBuffer.size(),
                  static_cast<unsigned char*>(image_data(img)),
                  width, 0, height,
                  TJPF_RGB,
                  TJFLAG_FASTDCT);

    if (destroyHandle)
        tjDestroy(tjHandle);
#else
    Q_UNUSED(tjHandle);

    try
    {
        load_jpeg(img, buffer.constData(), buffer.size());
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        emit log(QString("dlib exception: %1").arg(e.what()));
    }
#endif

    return img;
}

dlib::array2d<rgb_pixel> DLIBWorker::decodeJPEG(const QByteArray &jpegBuffer)
{
    auto decoded = decodeJPEG(jpegBuffer,
#ifdef TURBOJPEG_AVAILABLE
        m_tjHandle
#else
        nullptr
#endif
        );

    if (decoded.size() <= 0)
#ifdef TURBOJPEG_AVAILABLE
        emit log(QString("libjpeg-turbo error code: %1").arg(tjGetErrorStr2(m_tjHandle)));
#else
        emit log("dlib::load_jpeg failed!");
#endif

    return decoded;
}

std::vector<Face> DLIBWorker::findFaces(const array2d<rgb_pixel>& img)
{
    std::vector<matrix<rgb_pixel>> extractedFaces;
    std::vector<dlib::rectangle> extractedFacesRects;

    for (const auto& face : detector(img))
    {
        auto shape = sp(img, face);
        matrix<rgb_pixel> face_chip;
        extract_image_chip(img, get_face_chip_details(shape, m_faceDetailSize, m_faceDetailPadding), face_chip);
        extractedFaces.push_back(move(face_chip));
        extractedFacesRects.push_back(face);
    }

    std::vector<Face> faces;

    if(extractedFaces.empty())
    {
        return faces;
    }

    std::vector<dlib::matrix<float, 0, 1>> face_descriptors = net(extractedFaces);
    faces.resize(face_descriptors.size());

    for(size_t i = 0; i < faces.size(); ++i)
    {
        faces.at(i) = std::make_tuple(QString(), extractedFacesRects.at(i), face_descriptors.at(i));
    }

    return faces;
}

std::vector<Face> DLIBWorker::findFaces(const QString& fileName)
{
    array2d<rgb_pixel> img;
    load_jpeg(img, fileName.toStdString());
    return findFaces(img);
}

rectangle DLIBWorker::make_cropping_rect_resnet(const dlib::matrix<rgb_pixel> &img, dlib::rand* rnd)
{
    double mins = 0.466666666, maxs = 0.875;
    auto scale = mins + (rnd ? rnd->get_random_double() : 0.5) * (maxs - mins);
    auto size = scale * std::min(img.nr(), img.nc());
    rectangle rect(size, size);

    auto x = img.nc() - rect.width();
    auto y = img.nr() - rect.height();

    if (rnd)
    {
        x = rnd->get_random_32bit_number() % x;
        y = rnd->get_random_32bit_number() % y;
    }

    point offset(x, y);
    return move_rect(rect, offset);
}

void DLIBWorker::crop_images(const dlib::matrix<rgb_pixel> &img, dlib::array<dlib::matrix<rgb_pixel> > &crops, dlib::rand* rnd, long num_crops)
{
    std::vector<chip_details> dets;
    for (long i = 0; i < num_crops; ++i)
    {
        auto rect = make_cropping_rect_resnet(img, rnd);
        dets.push_back(chip_details(rect, chip_dims(227, 227)));
    }

    extract_image_chips(img, dets, crops);

    if (rnd)
    {
        for (auto&& img : crops)
        {
            if (rnd->get_random_double() > 0.5)
                img = fliplr(img);

            apply_random_color_offset(img, *rnd);
        }
    }
}

std::vector<sample_pair> DLIBWorker::createGraph(const std::vector<DLIBWorker::Face> &faces, double threshold)
{
    std::vector<sample_pair> edges;
    for(size_t i = 0; i < faces.size(); ++i)
    {
        for(size_t j = i; j < faces.size(); ++j)
        {
            if(length(get<2>(faces[i]) - get<2>(faces[j])) < threshold)
                edges.push_back(sample_pair(i, j));
        }
    }

    return edges;
}

std::vector<DLIBWorker::Cluster> DLIBWorker::cluster(const std::vector<sample_pair> &graph, const std::vector<DLIBWorker::Face> &faces)
{
    std::vector<DLIBWorker::Cluster> clusters;

    std::vector<unsigned long> labels;
    size_t clusterCount = chinese_whispers(graph, labels);

    clusters.reserve(clusterCount);

    std::vector<dlib::rectangle> _faces;
    QString identifier;
    for (size_t cluster_id = 0; cluster_id < clusterCount; ++cluster_id)
    {
        _faces.clear();
        identifier.clear();
        for (size_t j = 0; j < labels.size(); ++j)
        {
            if (cluster_id == labels[j])
            {
                if(!get<0>(faces[j]).isEmpty())
                {
                    if (identifier.isEmpty())
                    {
                        identifier = get<0>(faces[j]);
                    }
                }
                else
                {
                    _faces.push_back(get<1>(faces[j]));
                }
            }
        }
        clusters.push_back(make_pair(identifier, _faces));
    }

    return clusters;
}

void DLIBWorker::setupReference(const QVector<QPair<QString, QString>>& list)
{
    for(const auto& it : list)
    {
        const auto filename = it.second;

        const auto& faces = findFaces(filename);

        for (const auto& it2 : faces)
        {
            const auto tag = it.first;

            emit log(QString("Resolved reference face with tag '%1' from the file '%2'!").arg(tag).arg(filename));
            referenceFaces.push_back(make_tuple(tag, rectangle(), get<2>(it2)));
        }
    }
}

void DLIBWorker::setupImageNet()
{
    deserialize(m_imageNetClassifierFile.toStdString()) >> net2 >> labels;

    snet.subnet() = net2.subnet();
}

void DLIBWorker::process(const QByteArray& buffer)
{
    if (m_busy)
        return;

    m_busy = true;

    try
    {
        auto img = decodeJPEG(buffer);

        if (img.size() <= 0)
        {
            m_busy = false;
            return;
        }

        if (m_settings->objectDetectionEnabled)
        {
            if (labels.empty())
            {
                setupImageNet();
            }

            matrix<rgb_pixel> crop;
            dlib::array<matrix<rgb_pixel>> images;
            matrix<rgb_pixel> imgMat = mat(img);

            if (m_settings->deterministicObjectDetection)
                crop_images(imgMat, images, nullptr, m_numCrops);
            else
                crop_images(imgMat, images, &rnd, m_numCrops);

            matrix<float,1,1000> p = sum_rows(mat(snet(images.begin(), images.end()))) / m_numCrops;

            QVector<QPair<float, QString>> result;

            for (size_t k = 0; k < m_settings->labelCount; ++k)
            {
                unsigned long predicted_label = index_of_max(p);
                result.append( { p(predicted_label), QString::fromStdString(labels[predicted_label]) } );
                p(predicted_label) = 0;
            }

            emit doneObject(result);
        }

        if (m_settings->faceRecognitionEnabled)
        {
            if(referenceFaces.size() == 0)
            {
                if(m_refPhotoFileList.size() == 0)
                {
                    emit log("WARNING: Reference face list is empty!");
                }
                else
                {
                    setupReference(m_refPhotoFileList);
                }
            }

            auto faces = findFaces(img);

            if (faces.size() > 0)
            {
                rectangle rect;
                for (const auto& face : referenceFaces)
                {
                    faces.push_back(make_tuple(get<0>(face), rect, get<2>(face)));
                }

                auto graph = createGraph(faces, m_threshold);

                auto clusters = cluster(graph, faces);

                QVector<QPair<QRect, QString>> linearFaces;

                for (const auto& it : clusters)
                {
                    QString str = it.first;
                    for (const auto& it2 : it.second)
                    {
                        linearFaces.push_back(qMakePair(QRect(it2.left(), it2.top(), it2.width(), it2.height()), str));
                    }
                }

                emit doneFace(linearFaces);
            }
        }

        m_busy = false;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        emit log(QString("dlib exception: %1").arg(e.what()));
    }
}

