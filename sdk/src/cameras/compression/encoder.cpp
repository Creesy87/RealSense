// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2016 Intel Corporation. All Rights Reserved.

#include "encoder.h"
#include "lz4_codec.h"
#include "rs/utils/log_utils.h"

namespace rs
{
    namespace core
    {
        namespace compression
        {

            encoder::encoder(std::vector<std::tuple<rs_stream, rs_format, bool, float>> configuration)
            {
                for(auto & config : configuration)
                {
                    rs_stream stream = rs_stream::RS_STREAM_COUNT;
                    bool enabled = true;
                    rs_format format = rs_format::RS_FORMAT_ANY;
                    float compression_level = 0;
                    std::tie(stream, format, enabled, compression_level) = config;
                    if(enabled)
                        add_codec(stream, format, compression_level);
                    else
                        m_codecs.emplace(stream, nullptr);
                }
            }

            encoder::~encoder()
            {

            }

            file_types::compression_type encoder::get_compression_type(rs_stream stream)
            {
                if(m_codecs.find(stream) != m_codecs.end())
                {
                    auto codec = m_codecs.at(stream);
                    return codec ? m_codecs[stream]->get_compression_type() : file_types::compression_type::none;
                }
                return file_types::compression_type::none;
            }

            file_types::compression_type encoder::compression_policy(rs_stream stream, rs_format format)
            {
                return file_types::compression_type::lz4;
            }

            void encoder::add_codec(rs_stream stream, rs_format format, float compression_level)
            {
                if(m_codecs.find(stream) != m_codecs.end()) return;
                auto & codec = m_codecs[stream];
                switch (compression_policy(stream, format))
                {
                    case file_types::compression_type::lz4: codec   = std::shared_ptr<codec_interface>(new lz4_codec(compression_level)); break;
                    default: codec                                  = nullptr; break;
                }
            }

            status encoder::encode_frame(file_types::frame_info &info, const uint8_t *input, uint8_t * output, uint32_t &output_size)
            {
                LOG_SCOPE();
                auto codec = m_codecs.at(info.stream);
                return codec ? m_codecs[info.stream]->encode(info, input, output, output_size) : status::status_feature_unsupported;
            }
        }
    }
}
