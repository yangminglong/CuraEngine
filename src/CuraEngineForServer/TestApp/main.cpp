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

// ���������������û���Ƭ����
void initSliceParam(string& sliceParam) 
{
    sliceParam = R""({
            "layer_height" : "0.2",
            "material_print_temperature" : "180",
            "speed_print" : "80"
    })"";
}

// ����������������Ƭ�ļ�������
void initModel(shared_ptr<stringstream>& model) 
{
    ifstream file;
    file.open("D:/Users/Administrator/Documents/GitHub/CuraEngine/src/CuraEngineForServer/TestApp/0bunnyr.stl", ios::in | ios::binary);
    if (!file.is_open()) {
        cout << "file open failed" << endl;
        return;
    }
    
    model = make_shared<stringstream>();
    (*model) << file.rdbuf();
    file.close();

    cout << (*model).str() << endl;
}


// ��֤��Ƭ������ļ���Ϣ
void showDesc(shared_ptr<string> desc) {
    cout << "\n--------------shared_ptr<string> desc------------------------" << endl;
    cout << *desc << endl;
    cout << "--------------shared_ptr<string> desc------------------------\n" << endl;
}

// ��֤��Ƭ�����gcode������
void saveToGCodeFile(shared_ptr<stringstream> sliced)
{
    cout << (*sliced).str() << endl;
}


void initSliceInterface(SliceInterface* _interface)
{
    // ��ʼ������-��ȡ��Ƭ�ļ�
    initSliceParam(_interface->sliceParam);

    // ��ʼ������-��ȡ��Ƭ�ļ�
    initModel(_interface->model);

    // ��ʼ������-ģ�;���
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

    // ��ʼ�����ȷ���
    _interface->progressHandler = [](int progress) {
        cout << "progress: " << progress;
    };

    // ��ʼ�����п���
    _interface->isKeepingHandler = []() {
        return true;
    };

    // ��ʼ��������
    _interface->errorHandler = [](shared_ptr<string> err) {
        cout << "error: " << err;
    };

    _interface->slicedHandler = [](shared_ptr<stringstream> sliced, shared_ptr<string> desc){
        // ��ʾ��Ƭ����ļ���Ϣ
        showDesc(desc);

        // ����gcode�ļ�
        saveToGCodeFile(sliced);
    };
}


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

    // ��ʼ������
    shared_ptr<SliceInterface> sliceInterface = make_shared<SliceInterface>();
    initSliceInterface(sliceInterface.get());
    
    // ��ʼ��Ƭ
    startSlicer(sliceInterface.get());

    return 0;
}
