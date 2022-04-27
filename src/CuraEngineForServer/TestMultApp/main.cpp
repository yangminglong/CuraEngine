//Copyright (c) 2018 Ultimaker B.V.
//CuraEngine is released under the terms of the AGPLv3 or higher.

#include <iostream> //To change the formatting of std::cerr.
#include <signal.h> //For floating point exceptions.
#if defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
#include <sys/resource.h> //For setpriority.
#endif

//JasonChen -->
#include <fstream> 
#include <sstream>
//JasonChen <--

#include "../../Application.h"
#include "../../communication/CommandLine.h"

#include "../../utils/logoutput.h"
#include "../CuraEngineForServer.h"
using namespace std;

namespace cura
{
    //Signal handler for a "floating point exception", which can also be integer division by zero errors.
    void signal_FPE(int n)
    {
        (void)n;
        logError("Arithmetic exception.\n");
        exit(1);
    }

}//namespace cura

// 创建测试用例：用户切片参数
void initSliceParamA(string& sliceParam) 
{
    sliceParam = R""({
            "layer_height" : "0.2",
            "material_print_temperature" : "180",
            "speed_print" : "60"
    })"";
}

// 创建测试用例：用户切片参数
void initSliceParamB(string& sliceParam)
{
    sliceParam = R""({
            "layer_height" : "0.1",
            "material_print_temperature" : "200",
            "speed_print" : "70"
    })"";
}

// 创建测试用例：用户切片参数
void initSliceParamC(string& sliceParam)
{
    sliceParam = R""({
            "layer_height" : "0.3",
            "material_print_temperature" : "205",
            "speed_print" : "80"
    })"";
}

// 创建测试用例：切片文件数据流
void initModel(shared_ptr<stringstream>& model, string path) 
{
    ifstream file;
    file.open(path, ios::in | ios::binary);
    if (!file.is_open()) {
        cout << "file open failed" << endl;
        return;
    }
    
    model = make_shared<stringstream>();
    (*model) << file.rdbuf();
    file.close();

    //cout << (*model).str() << endl;
}


// 验证切片结果：文件信息
void showDescA(shared_ptr<string> desc) {
}

// 验证切片结果：gcode数据流
void saveToGCodeFile(shared_ptr<stringstream> sliced)
{
    //cout << (*sliced).str() << endl;
}


void initSliceInterfaceA(SliceInterface* _interface)
{
    // 初始化参数-读取切片文件
    initSliceParamA(_interface->sliceParam);

    // 初始化参数-读取切片文件
    initModel(_interface->model, "D:/Users/Administrator/Documents/GitHub/CuraEngine/src/CuraEngineForServer/TestApp/0bunnyr.stl");

    // 初始化参数-模型矩阵
    _interface->matrix[0][0] = 1.0;
    _interface->matrix[1][0] = 0.0;
    _interface->matrix[2][0] = 0.0;
    _interface->matrix[3][0] = 0.0;
    _interface->matrix[0][1] = 0.0;
    _interface->matrix[1][1] = 1.0;
    _interface->matrix[2][1] = 0.0;
    _interface->matrix[3][1] = 0.0;
    _interface->matrix[0][2] = 0.0;
    _interface->matrix[1][2] = 0.0;
    _interface->matrix[2][2] = 1.0;
    _interface->matrix[3][2] = 0.0;

    // 初始化进度反馈
    _interface->progressHandler = [](int progress) {
        cout << "A progress: " << progress << endl;
    };

    // 初始化运行控制
    _interface->isKeepingHandler = []() {
        return true;
    };

    // 初始化错误反馈
    _interface->errorHandler = [](shared_ptr<string> err) {
        cout << "A error: " << err;
    };

    _interface->slicedHandler = [](shared_ptr<stringstream> sliced, shared_ptr<string> desc){
        // 显示切片结果文件信息
        cout << "\n--------------A desc------------------------" << endl;
        cout << *desc << endl;
        cout << "--------------A desc------------------------\n" << endl;

        // 保存gcode文件
        saveToGCodeFile(sliced);
    };
}



void initSliceInterfaceB(SliceInterface* _interface)
{
    // 初始化参数-读取切片文件
    initSliceParamB(_interface->sliceParam);

    // 初始化参数-读取切片文件
    initModel(_interface->model, "D:/Users/Administrator/Documents/GitHub/CuraEngine/src/CuraEngineForServer/TestApp/0bunnyr1.stl");

    // 初始化参数-模型矩阵
    _interface->matrix[0][0] = 1.0;
    _interface->matrix[1][0] = 0.0;
    _interface->matrix[2][0] = 0.0;
    _interface->matrix[3][0] = 0.0;
    _interface->matrix[0][1] = 0.0;
    _interface->matrix[1][1] = 1.0;
    _interface->matrix[2][1] = 0.0;
    _interface->matrix[3][1] = 0.0;
    _interface->matrix[0][2] = 0.0;
    _interface->matrix[1][2] = 0.0;
    _interface->matrix[2][2] = 1.0;
    _interface->matrix[3][2] = 0.0;

    // 初始化进度反馈
    _interface->progressHandler = [](int progress) {
        cout << "B progress: " << progress << endl;
    };

    // 初始化运行控制
    _interface->isKeepingHandler = []() {
        return true;
    };

    // 初始化错误反馈
    _interface->errorHandler = [](shared_ptr<string> err) {
        cout << "B error: " << err;
    };

    _interface->slicedHandler = [](shared_ptr<stringstream> sliced, shared_ptr<string> desc) {
        // 显示切片结果文件信息
        cout << "\n--------------B desc------------------------" << endl;
        cout << *desc << endl;
        cout << "--------------B desc------------------------\n" << endl;

        // 保存gcode文件
        saveToGCodeFile(sliced);
    };
}


void initSliceInterfaceC(SliceInterface* _interface)
{
    // 初始化参数-读取切片文件
    initSliceParamC(_interface->sliceParam);

    // 初始化参数-读取切片文件
    initModel(_interface->model, "D:/Users/Administrator/Documents/GitHub/CuraEngine/src/CuraEngineForServer/TestApp/0bunnyr2.stl");

    // 初始化参数-模型矩阵
    _interface->matrix[0][0] = 1.0;
    _interface->matrix[1][0] = 0.0;
    _interface->matrix[2][0] = 0.0;
    _interface->matrix[3][0] = 0.0;
    _interface->matrix[0][1] = 0.0;
    _interface->matrix[1][1] = 1.0;
    _interface->matrix[2][1] = 0.0;
    _interface->matrix[3][1] = 0.0;
    _interface->matrix[0][2] = 0.0;
    _interface->matrix[1][2] = 0.0;
    _interface->matrix[2][2] = 1.0;
    _interface->matrix[3][2] = 0.0;

    // 初始化进度反馈
    _interface->progressHandler = [](int progress) {
        cout << "C progress: " << progress << endl;
    };

    // 初始化运行控制
    _interface->isKeepingHandler = []() {
        return true;
    };

    // 初始化错误反馈
    _interface->errorHandler = [](shared_ptr<string> err) {
        cout << "C error: " << err;
    };

    _interface->slicedHandler = [](shared_ptr<stringstream> sliced, shared_ptr<string> desc) {
        // 显示切片结果文件信息
        cout << "\n--------------C desc------------------------" << endl;
        cout << *desc << endl;
        cout << "--------------C desc------------------------\n" << endl;

        // 保存gcode文件
        saveToGCodeFile(sliced);
    };
}

#include <thread>
#include <stdlib.h> //sleep

int main(int argc, char** argv)
{
#if defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
    //Lower the process priority on linux and mac. On windows this is done on process creation from the GUI.
    setpriority(PRIO_PROCESS, 0, 10);
#endif

#ifndef DEBUG
    //Register the exception handling for arithmetic exceptions, this prevents the "something went wrong" dialog on windows to pop up on a division by zero.
    signal(SIGFPE, cura::signal_FPE);
#endif
    cerr << boolalpha;

    // 初始化数据
    shared_ptr<SliceInterface> sliceInterfaceA = make_shared<SliceInterface>();
    initSliceInterfaceA(sliceInterfaceA.get());
    
    shared_ptr<SliceInterface> sliceInterfaceB = make_shared<SliceInterface>();
    initSliceInterfaceA(sliceInterfaceB.get());

    shared_ptr<SliceInterface> sliceInterfaceC = make_shared<SliceInterface>();
    initSliceInterfaceA(sliceInterfaceC.get());

    // 开始切片
    startSlicer(sliceInterfaceA.get());

    //thread th1(startSlicer, sliceInterfaceA.get()); //实例化一个线程对象th1，使用函数t1构造，然后该线程就开始执行了（t1()）
    //thread th2(startSlicer, sliceInterfaceB.get());
    //thread th3(startSlicer, sliceInterfaceC.get());

    //th1.join(); // 必须将线程join或者detach 等待子线程结束主进程才可以退出
    //th2.join();
    //th3.join();

    //or use detach
    //th1.detach();
    //th2.detach();

    cout << "here is main\n\n";
    return 0;
}
