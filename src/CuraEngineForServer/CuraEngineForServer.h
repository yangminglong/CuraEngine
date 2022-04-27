#ifndef _CURAENGINEFORSERVER_H
#define _CURAENGINEFORSERVER_H

#include <functional>
#include <string>
#include <memory>
#include "../utils/aabb3d.h"

struct  SliceInterface
{
    std::string sliceParam;   // json，切片参数
    std::string modeName;     // 切片文件名称
    std::shared_ptr<std::stringstream> model;    // 二进制，切片文件

    double matrix[4][3]; // 矩阵
    //以下是回调

    // 切片引擎调用该接口，根据返回值判断是否继续切片流程，如果为false终止切片流程
    // 切片过程，约每分钟调用一次即可
    std::function<bool()> isKeepingHandler;

    // 切片引擎调用该接口，将切片进度回调给
    // 切片过程，约每分钟调用一次即可
    std::function<void(int progress)> progressHandler;

    // 切片后图片，通过二进制返回
    std::function<void(std::shared_ptr<std::stringstream> picture)> pictureHandler;

    // 切片后文件，通过二进制返回，第二个参数为json携带切片后的信息
    //#sliced 切片文件
    //#slicedHandler (json) 切片文件返回的信息 如：切片文件长，宽，高，层数等信息
    std::function<void(std::shared_ptr<std::stringstream> sliced, std::shared_ptr<std::string> desc)> slicedHandler;

    // 切片出错，回调错误码和错误描述比如层数过多等
    std::function<void(std::shared_ptr<std::string>)> errorHandler;
};



void startSlicer(SliceInterface* sliceInterface);



#endif