/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "json.hpp"

#include <string>

#include <QObject>

class ASTERIXImportTask;

/**
 * Error info for ASTERIX import from a file.
 */
struct ASTERIXImportFileError
{
    enum ErrorType
    {
        NoError = 0,    // no error
        Invalid,        // file is invalid (e.g. non-existent)
        CouldNotParse,  // file could not be parsed
        DecodingFailed  // file decode check failed
    };

    void reset()
    {
        errtype       = ErrorType::NoError;
        errinfo       = "";
        analysis_info = {};
    }

    bool hasError() const
    {
        return (errtype != ErrorType::NoError);
    }

    nlohmann::json toJSON() const;

    ErrorType      errtype = ErrorType::NoError;
    std::string    errinfo;
    nlohmann::json analysis_info;
};

/**
 * Subsection of a file to be imported.
 * Used for files in which multiple ASTERIX data is packed (e.g. PCAP).
 */
struct ASTERIXImportFileSection
{
    nlohmann::json toJSON() const;

    std::string            id;                   // id to identify this section in a file
    int                    idx = -1;             // index to identify this section in a file
    std::string            description;          // textual description for display purpose
    std::vector<char>      raw_data;             // raw data (possibly only a part of the total data)
    size_t                 total_size_bytes = 0; // total size of this section in bytes
    
    ASTERIXImportFileError error;                // error information (e.g. decoding)
    bool                   used = true;          // use this section for import
};

/**
 * Info for an ASTERIX file to be imported.
*/
struct ASTERIXImportFileInfo
{
    typedef std::vector<ASTERIXImportFileSection> Sections;

    void reset();

    bool hasError() const;
    bool canDecode() const;
    bool decodingTested() const { return decoding_tested; }

    bool   hasSections() const;
    size_t numUsedSections() const;

    size_t sizeInBytes() const;

    nlohmann::json toJSON() const;

    std::string            filename;
    //unsigned int         line_id = 0; // TODO rework

    Sections               sections;

    ASTERIXImportFileError error;
    bool                   used = true;

private:
    friend class ASTERIXDecoderFile;

    bool decoding_tested = false;
};

/**
 * Describes the source of ASTERIX data to be imported.
 * - Enum describing type of data - what to import
 * - List of files to import - where to get it
*/
class ASTERIXImportSource : public QObject
{
    Q_OBJECT
public:
    enum class SourceType
    {
        FileASTERIX = 0,   // ASTERIX files
        FileJSON,          // JSON files generated by jASTERIX
        FilePCAP,          // Network recording given as PCAP file
        NetASTERIX         // Network stream
    };

    typedef std::vector<ASTERIXImportFileInfo> FileInfos;

    ASTERIXImportSource();
    virtual ~ASTERIXImportSource();

    bool valid() const;

    void setSourceType(SourceType source);
    void setSourceType(SourceType source, const std::vector<std::string>& filenames);

    void addFile(const std::string& fn);
    void addFiles(const std::vector<std::string>& filenames);
    void clearFiles();
    const FileInfos& files() const;
    std::string filesAsString() const;

    SourceType sourceType() const;
    std::string sourceTypeAsString() const;
    bool isFileType() const;

    size_t totalFileSizeInBytes() const;

    nlohmann::json toJSON() const;

    static std::string sourceTypeAsString(SourceType source_type);
    static bool isFileType(SourceType source_type);
    static bool isNetworkType(SourceType source_type);

signals:
    void changed();
    void filesChanged();
    void sourceChanged();

private:
    friend class ASTERIXDecoderFile; //file based decoder can access the file infos directly

    void clear_internal();
    void addFile_internal(const std::string& fn);
    void addFiles_internal(const std::vector<std::string>& filenames);

    SourceType source_type_ = SourceType::FileASTERIX;
    FileInfos  file_infos_;
};
