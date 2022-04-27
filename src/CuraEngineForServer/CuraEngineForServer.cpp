#include "CuraEngineForServer.h"

#include <cstring> //For strtok and strcopy.
#include <fstream> //To check if files exist.
#include <errno.h> // error number when trying to read file
#include <numeric> //For std::accumulate.
#ifdef _OPENMP
#include <omp.h> //To change the number of threads to slice with.
#endif //_OPENMP
#include <rapidjson/rapidjson.h>
#include <rapidjson/error/en.h> //Loading JSON documents to get settings from them.
#include <rapidjson/filereadstream.h>
#include <unordered_set>

#include "../Application.h" //To get the extruders for material estimates.
#include "../ExtruderTrain.h"
#include "../FffProcessor.h" //To start a slice and get time estimates.
#include "../Slice.h"
#include "../utils/getpath.h"
#include "../utils/FMatrix4x3.h" //For the mesh_rotation_matrix setting.
#include "../utils/logoutput.h"
#include "../FffGcodeWriter.h"
#include "../progress/Progress.h"

#include <rapidjson/document.h> //Loading JSON documents to get settings from them.
// JasonChen -->
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
// JasonChen <--

using namespace cura;

void loadJSONPrinter(Settings& settings);
void loadJSONExtruder(Settings& settings);

void loadJSONSettings(const rapidjson::Value& element, Settings& settings)
{
    for (rapidjson::Value::ConstMemberIterator setting = element.MemberBegin(); setting != element.MemberEnd(); setting++)
    {
        const std::string name = setting->name.GetString();

        const rapidjson::Value& setting_object = setting->value;
        if (!setting_object.IsObject())
        {
            logError("JSON setting %s is not an object!\n", name.c_str());
            continue;
        }

        if (setting_object.HasMember("children"))
        {
            loadJSONSettings(setting_object["children"], settings);
        }
        else //Only process leaf settings. We don't process categories or settings that have sub-settings.
        {
            if (!setting_object.HasMember("default_value"))
            {
                logWarning("JSON setting %s has no default_value!\n", name.c_str());
                continue;
            }
            const rapidjson::Value& default_value = setting_object["default_value"];
            std::string value_string;
            if (default_value.IsString())
            {
                value_string = default_value.GetString();
            }
            else if (default_value.IsTrue())
            {
                value_string = "true";
            }
            else if (default_value.IsFalse())
            {
                value_string = "false";
            }
            else if (default_value.IsNumber())
            {
                std::ostringstream ss;
                ss << default_value.GetDouble();
                value_string = ss.str();
            }
            else
            {
                logWarning("Unrecognized data type in JSON setting %s\n", name.c_str());
                continue;
            }
            settings.add(name, value_string);
        }
    }
}

int loadJSON(const rapidjson::Document& document, Settings& settings)
{
    //Extruders defined from here, if any.
    //Note that this always puts the extruder settings in the slice of the current extruder. It doesn't keep the nested structure of the JSON files, if extruders would have their own sub-extruders.
    Scene& scene = Application::getInstance().current_slice->scene;
    if (document.HasMember("metadata") && document["metadata"].IsObject())
    {
        const rapidjson::Value& metadata = document["metadata"];
        if (metadata.HasMember("machine_extruder_trains") && metadata["machine_extruder_trains"].IsObject())
        {
            const rapidjson::Value& extruder_trains = metadata["machine_extruder_trains"];
            for (rapidjson::Value::ConstMemberIterator extruder_train = extruder_trains.MemberBegin(); extruder_train != extruder_trains.MemberEnd(); extruder_train++)
            {
                const int extruder_nr = atoi(extruder_train->name.GetString());
                if (extruder_nr < 0)
                {
                    continue;
                }
                while (scene.extruders.size() <= static_cast<size_t>(extruder_nr))
                {
                    scene.extruders.emplace_back(scene.extruders.size(), &scene.settings);
                }
                const rapidjson::Value& extruder_id = extruder_train->value;
                if (!extruder_id.IsString())
                {
                    continue;
                }
                loadJSONExtruder(scene.extruders[extruder_nr].settings);
            }
        }
    }

    if (document.HasMember("settings") && document["settings"].IsObject())
    {
        loadJSONSettings(document["settings"], settings);
    }
    if (document.HasMember("overrides") && document["overrides"].IsObject())
    {
        loadJSONSettings(document["overrides"], settings);
    }
    return 0;
}

void loadJSONPrinter(Settings& settings)
{
#include "../src/communication/fdmprinter_def_json.h"
    rapidjson::Document json_document_printer;
    json_document_printer.Parse(fdmprinter_def_json.c_str());
    loadJSON(json_document_printer, settings);
}

void loadJSONExtruder(Settings& settings)
{
#include "../src/communication/fdmexturder_def_json.h"
    rapidjson::Document json_document_extruder;
    json_document_extruder.Parse(fdmextruder_def_json.c_str());
    loadJSON(json_document_extruder, settings);
}

void loadJson(const std::string& sliceParam, Settings& settings)
{
    rapidjson::Document document;
    document.Parse(sliceParam.c_str());
    if (document.HasParseError())
    {
        logError("sliceParam parse Error !!!");
        return;
    }
    for (rapidjson::Value::ConstMemberIterator itr = document.MemberBegin(); itr != document.MemberEnd(); itr++) {
        rapidjson::Value jKey;
        rapidjson::Value jValue;
        rapidjson::Document::AllocatorType allocator;
        jKey.CopyFrom(itr->name, allocator);
        jValue.CopyFrom(itr->value, allocator);
        if (jKey.IsString() && jValue.IsString()) {
            std::string key = jKey.GetString();
            std::string value = jValue.GetString();
            settings.add(key, value);
        }
        else {
            logError("sliceParam parse Error !!!");
            return;
        }
    }
}

// 转为Json字符串 JasonChen -->
std::string slicedResultToJsonString(const cura::FffGcodeWriter::SlicedMeshInfo& slicedResult)
{
    float size_x = INT2MM(slicedResult.boundInfo.max.x - slicedResult.boundInfo.min.x);
    float size_y = INT2MM(slicedResult.boundInfo.max.y - slicedResult.boundInfo.min.y);
    float size_z = INT2MM(slicedResult.boundInfo.max.z - slicedResult.boundInfo.min.z);

    rapidjson::Document d;
    d.SetObject();
    rapidjson::Document::AllocatorType& allocator = d.GetAllocator();          // 获取分配器

    d.AddMember("size_x ", size_x, allocator);                                 // 切片后模型尺寸x
    d.AddMember("size_y ", size_y, allocator);                                 // 切片后模型尺寸y
    d.AddMember("size_z ", size_z, allocator);                                 // 切片后模型尺寸z
    d.AddMember("supplies_usage", slicedResult.totalFilamentUsed, allocator);  // 所需耗材
    d.AddMember("layers", slicedResult.totalLayers, allocator);                // 切片后模型层数
    d.AddMember("estimate", slicedResult.totalTimes, allocator);               // 预估打印时间，单位秒，
    d.AddMember("machine_name", "Anycubic", allocator);                        // 机器类型名称

    rapidjson::StringBuffer strBuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
    d.Accept(writer);

    return strBuf.GetString();
}
// JasonChen <--


void startSlicer(SliceInterface* sliceInterface)
{
    // 无意义,但是需要
    cura::Application::getInstance().initCommunication();

    Progress::init();

	// 重启计时
	FffProcessor::getInstance()->time_keeper.restart();

	// 切一组模型
	Slice slice(1);

	Application::getInstance().current_slice = &slice;

	Settings* last_settings = &slice.scene.settings;

	slice.scene.extruders.reserve(1); //Allocate enough memory to prevent moves.
	slice.scene.extruders.emplace_back(0, &slice.scene.settings); //Always have one extruder.

    // 加载默认切片参数
    loadJSONPrinter(*last_settings);
    loadJSONExtruder(*last_settings);

    // 设置用户切片参数
    loadJson(sliceInterface->sliceParam, *last_settings);

    // 设置进度反馈
    enableProgressLogging();
    progressHandler = sliceInterface->progressHandler; 

    // 设置错误反馈
    errorHandler = sliceInterface->errorHandler; 

    // 设置用户取消函数
    isKeepingHandler = sliceInterface->isKeepingHandler;

    // 设置待切片文件 // bool loadMeshStreamInfoMeshGroup(MeshGroup* meshgroup, stringstream* fileStream, const FMatrix4x3& transformation, Settings& object_parent_settings)
    if (!loadMeshStreamInfoMeshGroup(&slice.scene.mesh_groups[0], sliceInterface->model.get(), FMatrix4x3(sliceInterface->matrix), *last_settings))
    {
        logError("Failed to load model: %s. (error number %d)\n", sliceInterface->modeName, errno);
        return;
    }

    // 设置输出流
    std::shared_ptr<std::stringstream> gcodeStream = std::make_shared<std::stringstream>();
    FffProcessor::getInstance()->setTargetStream(gcodeStream.get());

    slice.scene.mesh_groups[0].finalize();
    log("Loaded from disk in %5.3fs\n", FffProcessor::getInstance()->time_keeper.restart());

    // 开始切片.
    slice.compute();

    // Finalize the processor. This adds the end g-code and reports statistics.
    FffProcessor::getInstance()->finalize();

    // 返回附加信息
    cura::FffGcodeWriter::SlicedMeshInfo slicedResult;
    FffProcessor::getInstance()->gcode_writer.getSlicedResults(slicedResult);

    // 汇报结果
    sliceInterface->slicedHandler(gcodeStream, std::make_shared<std::string>(slicedResultToJsonString(slicedResult)));
}

