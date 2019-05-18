#include "asterixdecodejob.h"
#include "asteriximportertask.h"
#include "logger.h"

#include <jasterix/jasterix.h>

#include <QThread>

using namespace nlohmann;

ASTERIXDecodeJob::ASTERIXDecodeJob(ASTERIXImporterTask& task, const std::string& filename, const std::string& framing,
                                   bool test)
    : Job ("ASTERIXDecodeJob"), task_(task), filename_(filename), framing_(framing), test_(test)
{

}

void ASTERIXDecodeJob::run ()
{
    logdbg << "ASTERIXDecodeJob: run";

    started_ = true;

    using namespace std::placeholders;
    std::function<void(nlohmann::json&, size_t, size_t)> callback = std::bind(&ASTERIXDecodeJob::jasterix_callback,
                                                                              this, _1, _2, _3);

    if (framing_ == "")
        task_.jASTERIX()->decodeFile (filename_, callback);
    else
        task_.jASTERIX()->decodeFile (filename_, framing_, callback);

    done_ = true;

    logdbg << "ASTERIXDecodeJob: run: done";
}

void ASTERIXDecodeJob::jasterix_callback(nlohmann::json& data, size_t num_frames, size_t num_records)
{
    while (pause_) // block decoder until unpaused
    {
        QThread::sleep(1);
    }

    num_frames_ = num_frames;
    num_records_ = num_records;

    std::shared_ptr<json> moved_data {new json()};

    *moved_data = std::move(data);

    //loginf << "ASTERIXDecodeJob: jasterix_callback: got " << moved_data.size() << " records";

    emit decodedASTERIXSignal(moved_data);
}


size_t ASTERIXDecodeJob::numFrames() const
{
    return num_frames_;
}

size_t ASTERIXDecodeJob::numRecords() const
{
    return num_records_;
}



