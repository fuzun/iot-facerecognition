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

#ifndef DLIBWORKER_H
#define DLIBWORKER_H

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QVector>
#include <QRect>
#include <QPair>

#include <dlib/dnn.h>
#include <dlib/image_processing/frontal_face_detector.h>

class DLIBWorker : public QObject
{
    Q_OBJECT

    friend class Client;

public:
    using Face = std::tuple<QString, dlib::rectangle, dlib::matrix<float, 0, 1>>;
    using Object = Face;
    using Cluster = std::pair<QString, std::vector<dlib::rectangle>>;

private:
    // <ResNet-Network>

    // (dnn_face_recognition_ex.cpp):
    template <template <int, template<typename>class, int, typename> class block, int N, template<typename>class BN, typename SUBNET>
    using residual = dlib::add_prev1<block<N, BN, 1, dlib::tag1<SUBNET>>>;

    template <template <int, template<typename>class, int, typename> class block, int N, template<typename>class BN, typename SUBNET>
    using residual_down = dlib::add_prev2<dlib::avg_pool<2, 2, 2, 2, dlib::skip1<dlib::tag2<block<N, BN, 2, dlib::tag1<SUBNET>>>>>>;

    template <int N, template <typename> class BN, int stride, typename SUBNET>
    using block = BN<dlib::con<N, 3, 3, 1, 1, dlib::relu<BN<dlib::con<N, 3, 3, stride, stride, SUBNET>>>>>;

    template <int N, typename SUBNET> using ares = dlib::relu<residual<block, N, dlib::affine, SUBNET>>;
    template <int N, typename SUBNET> using ares_down = dlib::relu<residual_down<block, N, dlib::affine, SUBNET>>;

    template <typename SUBNET> using alevel0 = ares_down<256, SUBNET>;
    template <typename SUBNET> using alevel1 = ares<256, ares<256, ares_down<256, SUBNET>>>;
    template <typename SUBNET> using alevel2 = ares<128, ares<128, ares_down<128, SUBNET>>>;
    template <typename SUBNET> using alevel3 = ares<64, ares<64, ares<64, ares_down<64, SUBNET>>>>;
    template <typename SUBNET> using alevel4 = ares<32, ares<32, ares<32, SUBNET>>>;

    using anet_type = dlib::loss_metric<dlib::fc_no_bias<128, dlib::avg_pool_everything<
        alevel0<
        alevel1<
        alevel2<
        alevel3<
        alevel4<
        dlib::max_pool<3, 3, 2, 2, dlib::relu<dlib::affine<dlib::con<32, 7, 7, 2, 2,
        dlib::input_rgb_image_sized<150>
        >>>>>>>>>>>>;

    // ResNet-34 (dnn_imagenet_ex.cpp):

    template <typename SUBNET> using level1 = ares<512,ares<512,ares_down<512,SUBNET>>>;
    template <typename SUBNET> using level2 = ares<256,ares<256,ares<256,ares<256,ares<256,ares_down<256,SUBNET>>>>>>;
    template <typename SUBNET> using level3 = ares<128,ares<128,ares<128,ares_down<128,SUBNET>>>>;
    template <typename SUBNET> using level4 = ares<64,ares<64,ares<64,SUBNET>>>;

    using anet_type_2 = dlib::loss_multiclass_log<dlib::fc<1000,dlib::avg_pool_everything<
         level1<
         level2<
         level3<
         level4<
         dlib::max_pool<3,3,2,2,dlib::relu<dlib::affine<dlib::con<64,7,7,2,2,
         dlib::input_rgb_image_sized<227>
         >>>>>>>>>>>;

    // </ResNet-Network>

    dlib::frontal_face_detector detector;
    dlib::shape_predictor sp;
    anet_type net;

    std::vector<std::string> labels;
    anet_type_2 net2;
    dlib::rand rnd;
    dlib::softmax<anet_type_2::subnet_type> snet;

    QVector<QPair<QString, QString>> m_refPhotoFileList;
    QString                          m_faceLandmarkModelFile;
    QString                          m_faceRecognitionModelFile;
    size_t                           m_faceDetailSize;
    double                           m_faceDetailPadding;
    double                           m_threshold;
    bool                             m_busy;
    QString                          m_imageNetClassifierFile;
    size_t                           m_numCrops;

#ifdef TURBOJPEG_AVAILABLE
    void* m_tjHandle = nullptr;
#endif

    const struct Settings* m_settings;

    std::vector<Face> referenceFaces;

    void setupReference(const QVector<QPair<QString, QString>>& list);
    void setupImageNet();

    static std::vector<dlib::sample_pair> createGraph(const std::vector<Face>& faces, double threshold);
    static std::vector<Cluster> cluster(const std::vector<dlib::sample_pair>& graph, const std::vector<Face>& faces);

public slots:
    void process(const QByteArray& buffer);

public:
    explicit DLIBWorker(class QSettings* config, const struct Settings* settings);
    ~DLIBWorker();

    static dlib::array2d<dlib::rgb_pixel> decodeJPEG(const QByteArray& jpegBuffer, void* tjHandle);
    dlib::array2d<dlib::rgb_pixel> decodeJPEG(const QByteArray& jpegBuffer);

    inline bool isBusy() const { return m_busy; };

    std::vector<Face> findFaces(const dlib::array2d<dlib::rgb_pixel>& img);
    std::vector<Face> findFaces(const QString& fileName);

    static dlib::rectangle make_cropping_rect_resnet(const dlib::matrix<dlib::rgb_pixel>& img, dlib::rand* rnd);
    static void crop_images(const dlib::matrix<dlib::rgb_pixel>& img, dlib::array<dlib::matrix<dlib::rgb_pixel>>& crops, dlib::rand* rnd, long num_crops);

public:
signals:
    void doneFace(const QVector<QPair<QRect, QString>>& results);
    void doneObject(const QVector<QPair<float, QString>>& results);

    void log(const QString& str);
};

#endif // DLIBWORKER_H
