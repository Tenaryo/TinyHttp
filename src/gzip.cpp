#include "gzip.hpp"

#include <expected>
#include <string>
#include <vector>
#include <zlib.h>

namespace tinyhttp {

auto compress_gzip(std::string_view input) -> std::expected<std::vector<std::byte>, std::string> {
    auto input_size = input.size();

    z_stream strm{};
    int rc = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);
    if (rc != Z_OK)
        return std::unexpected("deflateInit2 failed");

    strm.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(input.data()));
    strm.avail_in = static_cast<uInt>(input_size);

    auto bound = deflateBound(&strm, static_cast<uLong>(input_size));
    std::vector<std::byte> output(bound);

    strm.next_out = reinterpret_cast<Bytef*>(output.data());
    strm.avail_out = static_cast<uInt>(output.size());

    rc = deflate(&strm, Z_FINISH);
    deflateEnd(&strm);

    if (rc != Z_STREAM_END)
        return std::unexpected("deflate failed");

    output.resize(static_cast<size_t>(strm.next_out - reinterpret_cast<Bytef*>(output.data())));
    return output;
}

} // namespace tinyhttp
