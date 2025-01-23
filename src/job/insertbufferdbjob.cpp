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

#include <arrow/api.h>
#include <arrow/io/api.h>
#include <arrow/filesystem/api.h>
#include <arrow/type.h>

#include <parquet/arrow/writer.h>
#include <parquet/arrow/schema.h>

#include "insertbufferdbjob.h"
#include "buffer.h"
#include "dbinterface.h"
#include "dbcontent/dbcontent.h"
//#include "dbcontent/variable/variable.h"
#include "stringconv.h"
#include "files.h"
#include "timeconv.h"
#include "parquettools.h"
#include "duckdbconnection.h"

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/filesystem.hpp"

using namespace Utils::String;

InsertBufferDBJob::InsertBufferDBJob(DBInterface& db_interface, DBContent& dbobject,
                                     std::shared_ptr<Buffer> buffer, bool emit_change)
    : Job("InsertBufferDBJob"),
      db_interface_(db_interface),
      dbobject_(dbobject),
      buffer_(buffer),
      emit_change_(emit_change)
{
    assert(buffer_);
}

InsertBufferDBJob::~InsertBufferDBJob() {}

// namespace parquet
// {
//     class ParquetWriter
//     {
//     public:
//         /**
//          */
//         ParquetWriter() {}

//         /**
//          */
//         virtual ~ParquetWriter()
//         {
//         }

//         /**
//          */
//         static std::shared_ptr<::arrow::DataType> arrowDataType(PropertyDataType dtype)
//         {
//             if (dtype == PropertyDataType::BOOL)
//                 return ::arrow::boolean();
//             else if (dtype == PropertyDataType::CHAR)
//                 return ::arrow::int8();
//             else if (dtype == PropertyDataType::UCHAR)
//                 return ::arrow::uint8();
//             else if (dtype == PropertyDataType::INT)
//                 return ::arrow::int32();
//             else if (dtype == PropertyDataType::UINT)
//                 return ::arrow::uint32();
//             else if (dtype == PropertyDataType::LONGINT)
//                 return ::arrow::int64();
//             else if (dtype == PropertyDataType::ULONGINT)
//                 return ::arrow::uint64();
//             else if (dtype == PropertyDataType::FLOAT)
//                 return ::arrow::float32();
//             else if (dtype == PropertyDataType::DOUBLE)
//                 return ::arrow::float64();
//             else if (dtype == PropertyDataType::STRING)
//                 return ::arrow::utf8();
//             else if (dtype == PropertyDataType::JSON)
//                 return ::arrow::utf8();
//             else if (dtype == PropertyDataType::TIMESTAMP)
//                 return ::arrow::timestamp(::arrow::TimeUnit::MILLI);
            
//             return ::arrow::utf8();
//         }

//         /**
//          */
//         static bool bufferPropertyToArrowArray(std::shared_ptr<::arrow::Array>& array,
//                                                ::Buffer& buffer,
//                                                size_t property_index)
//         {
//             const auto& property = buffer.properties().properties().at(property_index);
//             auto dtype = property.dataType();

//             size_t n = buffer.size();

//             try
//             {
//                 if (dtype == PropertyDataType::BOOL)
//                 {
//                     ::arrow::BooleanBuilder builder;

//                     const auto& values = buffer.get<bool>(property.name());
//                     for (size_t i = 0; i < n; ++i)
//                     {
//                         if (values.isNull(i))
//                             builder.AppendNull();
//                         else
//                             builder.Append(values.get(i));
//                     }

//                     builder.Finish(&array);
//                 }
//                 else if (dtype == PropertyDataType::CHAR)
//                 {
//                     ::arrow::Int8Builder builder;

//                     const auto& values = buffer.get<char>(property.name());
//                     for (size_t i = 0; i < n; ++i)
//                     {
//                         if (values.isNull(i))
//                             builder.AppendNull();
//                         else
//                             builder.Append(values.get(i));
//                     }

//                     builder.Finish(&array);
//                 }
//                 else if (dtype == PropertyDataType::UCHAR)
//                 {
//                     ::arrow::UInt8Builder builder;

//                     const auto& values = buffer.get<unsigned char>(property.name());
//                     for (size_t i = 0; i < n; ++i)
//                     {
//                         if (values.isNull(i))
//                             builder.AppendNull();
//                         else
//                             builder.Append(values.get(i));
//                     }

//                     builder.Finish(&array);
//                 }
//                 else if (dtype == PropertyDataType::INT)
//                 {
//                     ::arrow::Int32Builder builder;

//                     const auto& values = buffer.get<int>(property.name());
//                     for (size_t i = 0; i < n; ++i)
//                     {
//                         if (values.isNull(i))
//                             builder.AppendNull();
//                         else
//                             builder.Append(values.get(i));
//                     }

//                     builder.Finish(&array);
//                 }
//                 else if (dtype == PropertyDataType::UINT)
//                 {
//                     ::arrow::UInt32Builder builder;

//                     const auto& values = buffer.get<unsigned int>(property.name());
//                     for (size_t i = 0; i < n; ++i)
//                     {
//                         if (values.isNull(i))
//                             builder.AppendNull();
//                         else
//                             builder.Append(values.get(i));
//                     }

//                     builder.Finish(&array);
//                 }
//                 else if (dtype == PropertyDataType::LONGINT)
//                 {
//                     ::arrow::Int64Builder builder;

//                     const auto& values = buffer.get<long>(property.name());
//                     for (size_t i = 0; i < n; ++i)
//                     {
//                         if (values.isNull(i))
//                             builder.AppendNull();
//                         else
//                             builder.Append(values.get(i));
//                     }

//                     builder.Finish(&array);
//                 }
//                 else if (dtype == PropertyDataType::ULONGINT)
//                 {
//                     ::arrow::UInt64Builder builder;

//                     const auto& values = buffer.get<unsigned long>(property.name());
//                     for (size_t i = 0; i < n; ++i)
//                     {
//                         if (values.isNull(i))
//                             builder.AppendNull();
//                         else
//                             builder.Append(values.get(i));
//                     }

//                     builder.Finish(&array);
//                 }
//                 else if (dtype == PropertyDataType::FLOAT)
//                 {
//                     ::arrow::FloatBuilder builder;

//                     const auto& values = buffer.get<float>(property.name());
//                     for (size_t i = 0; i < n; ++i)
//                     {
//                         if (values.isNull(i))
//                             builder.AppendNull();
//                         else
//                             builder.Append(values.get(i));
//                     }

//                     builder.Finish(&array);
//                 }
//                 else if (dtype == PropertyDataType::DOUBLE)
//                 {
//                     ::arrow::DoubleBuilder builder;

//                     const auto& values = buffer.get<double>(property.name());
//                     for (size_t i = 0; i < n; ++i)
//                     {
//                         if (values.isNull(i))
//                             builder.AppendNull();
//                         else
//                             builder.Append(values.get(i));
//                     }

//                     builder.Finish(&array);
//                 }
//                 else if (dtype == PropertyDataType::STRING)
//                 {
//                     ::arrow::StringBuilder builder;

//                     const auto& values = buffer.get<std::string>(property.name());
//                     for (size_t i = 0; i < n; ++i)
//                     {
//                         if (values.isNull(i))
//                             builder.AppendNull();
//                         else
//                             builder.Append(values.get(i).data(), values.get(i).size());
//                     }

//                     builder.Finish(&array);
//                 }
//                 else if (dtype == PropertyDataType::JSON)
//                 {
//                     ::arrow::StringBuilder builder;

//                     const auto& values = buffer.get<nlohmann::json>(property.name());
//                     for (size_t i = 0; i < n; ++i)
//                     {
//                         if (values.isNull(i))
//                         {
//                             builder.AppendNull();
//                         }
//                         else
//                         {
//                             auto str = values.get(i).dump();
//                             builder.Append(str.data(), str.size());
//                         }
//                     }

//                     builder.Finish(&array);
//                 }
//                 else if (dtype == PropertyDataType::TIMESTAMP)
//                 {
//                     ::arrow::TimestampBuilder builder(::arrow::timestamp(::arrow::TimeUnit::MILLI), ::arrow::default_memory_pool());

//                     const auto& values = buffer.get<boost::posix_time::ptime>(property.name());
//                     for (size_t i = 0; i < n; ++i)
//                     {
//                         if (values.isNull(i))
//                             builder.AppendNull();
//                         else
//                             builder.Append(Utils::Time::toLong(values.get(i)));
//                     }

//                     builder.Finish(&array);
//                 }
//                 else
//                 {
//                     logerr << "bufferPropertyToArrowArray: writing buffer property failed: unknown property";
//                     return false;
//                 }
//             }
//             catch (const std::exception& ex)
//             {
//                 logerr << "bufferPropertyToArrowArray: writing buffer property failed: " << ex.what();
//                 return false;
//             }
//             catch(...)
//             {
//                 logerr << "bufferPropertyToArrowArray: writing buffer property failed: unknown error";
//                 return false;
//             }

//             return true;
//         }

//         std::map<std::string, std::vector<std::pair<std::string, PropertyDataType>>> m_props;

//         /**
//          */
//         bool writeBufferToParquet(DBInterface& interface,
//                                   const DBContent& dbcontent,
//                                   ::Buffer& buffer,
//                                   bool overwrite)
//         {
//             if (!interface.dbOpen())
//                 return false;

//             std::vector<std::shared_ptr<::arrow::Field>> fields;
//             std::vector<std::pair<std::string, PropertyDataType>> props;

//             for (auto& p : buffer.properties().properties())
//             {
//                 auto dtype = arrowDataType(p.dataType());
//                 fields.push_back(::arrow::field(p.name(), dtype));
//                 props.emplace_back(p.name(), p.dataType());
//             }

//             {
//                 auto it = m_props.find(dbcontent.name());
//                 if (it == m_props.end())
//                 {
//                     m_props[ dbcontent.name() ] = props;
//                 }
//                 else
//                 {
//                     if (it->second != props)
//                     {
//                         loginf << "Props do not match for dbcontent " << dbcontent.name();

//                         size_t n0 = it->second.size();
//                         size_t n1 = props.size();

//                         for (size_t i = 0; i < std::max(n0, n1); ++i)
//                         {
//                             std::string s0 = i < n0 ? it->second[ i ].first : "-";
//                             std::string s1 = i < n1 ? props[ i ].first : "-";
//                             std::string dtype0 = i < n0 ? Property::asString(it->second[ i ].second) : "-";
//                             std::string dtype1 = i < n1 ? Property::asString(props[ i ].second) : "-";

//                             std::cout << "   " << s0 << " (" << dtype0 << ")\t\t\t\t" << s1 << " (" << dtype1 << ")" << std::endl;
//                         }

//                         assert(false);
//                     }
//                 }
//             }

//             auto schema = std::make_shared<::arrow::Schema>(fields);

//             std::vector<std::shared_ptr<::arrow::Array>> arrays(fields.size());

//             for (size_t i = 0; i < fields.size(); i++)
//                 if (!bufferPropertyToArrowArray(arrays[ i ], buffer, i))
//                     return false;

//             auto table = ::arrow::Table::Make(schema, arrays);

//             std::string db_path      = Utils::Files::getDirectoryFromPath(interface.dbFilename());
//             std::string db_basename  = boost::filesystem::path(interface.dbFilename()).stem().string();
//             std::string parquet_path = db_path + "/" + db_basename;
            
//             if (!Utils::Files::directoryExists(parquet_path) &&
//                 !Utils::Files::createMissingDirectories(parquet_path))
//             {
//                 logerr << "writeBufferToParquet: could not create parquet path at " << parquet_path;
//                 return false;
//             }

//             std::string table_fn_base = parquet_path + "/" + "table_"  + dbcontent.name();
//             std::string table_fn;
//             size_t cnt = 0;
//             do
//             {
//                 table_fn = table_fn_base + "_part" + std::to_string(cnt++);
//             } while (Utils::Files::fileExists(table_fn));

//             auto outfile = createFile(table_fn);
//             if (!outfile)
//             {
//                 logerr << "writeBufferToParquet: could not open table file at " << table_fn;
//                 return false;
//             }

//             parquet::WriterProperties::Builder props_builder;
//             std::shared_ptr<parquet::WriterProperties> writer_props = props_builder.compression(::arrow::Compression::ZSTD)->build();

//             auto result = parquet::arrow::WriteTable(*table, ::arrow::default_memory_pool(), outfile, 64 * 1024 * 1024, writer_props);
//             if (!result.ok())
//             {
//                 logerr << "writeBufferToParquet: could not write table " << dbcontent.name() << ": " << result.message();
//                 return false;
//             }

//             return true;
//         }

//     private:
//         std::shared_ptr<::arrow::io::FileOutputStream> createFile(const std::string& table_fn)
//         {
//             auto file_result = ::arrow::io::FileOutputStream::Open(table_fn, false);
//             if (!file_result.ok())
//                 return std::shared_ptr<::arrow::io::FileOutputStream>();

//             return file_result.ValueUnsafe();
//         }
//     };
// }

void InsertBufferDBJob::run()
{
    logdbg << "InsertBufferDBJob: run: start";

    started_ = true;

    boost::posix_time::ptime loading_start_time;
    boost::posix_time::ptime loading_stop_time;

    loading_start_time = boost::posix_time::microsec_clock::local_time();

    logdbg << "InsertBufferDBJob: run: writing object " << dbobject_.name() << " size "
           << buffer_->size();
    assert(buffer_->size());

    db_interface_.insertBuffer(dbobject_, buffer_);
    loading_stop_time = boost::posix_time::microsec_clock::local_time();

    double load_time;
    boost::posix_time::time_duration diff = loading_stop_time - loading_start_time;
    load_time = diff.total_milliseconds() / 1000.0;

    loginf << "InsertBufferDBJob: run: buffer write done (" << doubleToStringPrecision(diff.total_milliseconds(), 2)
           << " ms).";

#if 0
    {
        parquettools::ParquetWriter writer(db_interface_, false);

        loading_start_time = boost::posix_time::microsec_clock::local_time();

        writer.writeBufferToParquet(dbobject_, *buffer_);

        loading_stop_time = boost::posix_time::microsec_clock::local_time();

        boost::posix_time::time_duration diff = loading_stop_time - loading_start_time;
        load_time = diff.total_milliseconds() / 1000.0;

        loginf << "InsertBufferDBJob: run: writing to parquet done (" << doubleToStringPrecision(diff.total_milliseconds(), 2) << " ms).";
    }
#endif

#if 1
    {
        auto& conn = db_interface_.duckDBConnection();

        loading_start_time = boost::posix_time::microsec_clock::local_time();

        bool ok = conn.insertBuffer(dbobject_, buffer_);
        assert(ok);

        loading_stop_time = boost::posix_time::microsec_clock::local_time();

        boost::posix_time::time_duration diff = loading_stop_time - loading_start_time;
        load_time = diff.total_milliseconds() / 1000.0;

        loginf << "InsertBufferDBJob: run: writing to duckdb done (" << doubleToStringPrecision(diff.total_milliseconds(), 2) << " ms).";
    }
#endif

    done_ = true;
}

bool InsertBufferDBJob::emitChange() const { return emit_change_; }
