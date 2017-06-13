/**
 * MIT License
 *
 * Copyright (c) 2017 Rokas Kupstys
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#pragma once

#include <map>
#include <vector>
#include <functional>
#include <stdint.h>
#if !_WIN32
#	include <unistd.h>
#endif

namespace zinc
{

typedef uint32_t WeakHash;

class StrongHash
{
public:
    StrongHash();
    StrongHash(const void* m, size_t mlen);
    StrongHash(const std::string& str);
    StrongHash(const StrongHash& other);
    StrongHash& operator=(const StrongHash& other);
    bool operator==(const StrongHash& other);
    std::string to_string() const;

protected:
#if ZINC_WITH_STRONG_HASH_FNV
    uint8_t _data[8];
#else
    uint8_t _data[20];
#endif
};

struct BlockHashes
{
    BlockHashes();
	BlockHashes(const WeakHash& weak, const StrongHash& strong);
	BlockHashes(const WeakHash& weak, const std::string& strong);
    BlockHashes(const BlockHashes& other) = default;
    BlockHashes& operator=(const BlockHashes& other) = default;

    WeakHash weak;
    StrongHash strong;
};

struct DeltaElement
{
    DeltaElement() { }
	DeltaElement(size_t block_index, size_t block_offset);

	int64_t block_index = -1;
    int64_t local_offset = -1;
    int64_t block_offset = -1;

    bool is_download() { return local_offset == -1; }
    bool is_copy()     { return local_offset > 0 && !is_done(); }
    bool is_done()     { return block_offset == local_offset; }
	bool is_valid()    { return block_index >= 0 && block_offset >= 0; }
    bool operator==(const DeltaElement& other)
    {
        return block_index == other.block_index &&
               block_offset == other.block_offset &&
               local_offset == other.local_offset;
    }
};

typedef std::vector<uint8_t>                                                                     ByteArray;
/// A list of offsets of currenty present data in not yet updated file. -1 value signifies a missing block.
typedef std::vector<DeltaElement>                                                                DeltaMap;
/// Strong and weak hashes for each block.
typedef std::vector<BlockHashes>                                                                 RemoteFileHashList;
/// A callback that should obtain block data at specified index and return it.
typedef std::function<ByteArray(int64_t block_index, size_t block_size)>                          FetchBlockCallback;
/// A callback for reporting progress. Return true if patching should continue, false if patching should terminate.
typedef std::function<bool(int64_t bytes_done_now, int64_t bytes_done_total, int64_t file_size)> ProgressCallback;


/*!
 * Calculates strong and weak checksums for every block in the passed memory.
 * \param file_data a pointer to a memory block.
 * \param file_size size of \a file_data memory block. It must be multiple of \a block_size.
 * \param block_size size of single block.
 * \param report_progress a callback which will be invoked to report progress.
 * \return array of \a BlockHashes which contains weak and strong checksums of every block in the file.
 * \throws std::invalid_argument
 * \throws std::system_error
 */
RemoteFileHashList get_block_checksums(const void* file_data, int64_t file_size, size_t block_size,
                                       const ProgressCallback& report_progress = ProgressCallback());
/*!
 * Calculates strong and weak checksums for every block in the passed file.
 * \param file_path a path to a file.
 * \param block_size size of single block.
 * \param report_progress a callback which will be invoked to report progress.
 * \return array of \a BlockHashes which contains weak and strong checksums of every block in the file.
 * \throws std::invalid_argument
 * \throws std::system_error
 */
RemoteFileHashList get_block_checksums(const char* file_path, size_t block_size,
                                       const ProgressCallback& report_progress = ProgressCallback());

/*!
 * Calculates a delta map defining which blocks of data are to be reused from local files and which are to be downloaded.
 * \param file_data a pointer to a memory block.
 * \param file_size size of \a file_data memory block. It must be multiple of \a block_size.
 * \param block_size size of single block.
 * \param hashes \a RemoteFileHashList returned by \a get_block_checksums.
 * \param report_progress a callback which will be invoked to report progress.
 * \return \a DeltaMap describing how data should be reused from local file and which blocks should be downloaded.
 * \throws std::invalid_argument
 * \throws std::system_error
 */
DeltaMap get_differences_delta(const void* file_data, int64_t file_size, size_t block_size,
                               const RemoteFileHashList& hashes,
                               const ProgressCallback& report_progress = ProgressCallback());
/*!
 * Calculates a delta map defining which blocks of data are to be reused from local files and which are to be downloaded.
 * \param file_path a path to a file.
 * \param block_size size of single block.
 * \param hashes \a RemoteFileHashList returned by \a get_block_checksums.
 * \param report_progress a callback which will be invoked to report progress.
 * \return \a DeltaMap describing how data should be reused from local file and which blocks should be downloaded.
 * \throws std::invalid_argument
 * \throws std::system_error
 */
DeltaMap get_differences_delta(const char* file_path, size_t block_size, const RemoteFileHashList& hashes,
							   const ProgressCallback& report_progress = ProgressCallback());

/// `file_data` must be at least as big as remote data block.
/*!
 * Sync a local file to remote one.
 * \param file_data a memory block with a local file. It must be big enough to contain latest version of file.
 * \param file_size size of \a file_data memory block. It must be multiple of \a block_size.
 * \param block_size size of single block.
 * \param delta \a DeltaMap returned by \a get_differences_delta.
 * \param get_data a callback which should return \a block_size size block of data from remote file at
 * block_index * \a block_size position.
 * \param report_progress a callback which will be invoked to report progress.
 * \return if file update was successful.
 * \throws std::invalid_argument
 * \throws std::system_error
 */
bool patch_file(void* file_data, int64_t file_size, size_t block_size, DeltaMap& delta,
				const FetchBlockCallback& get_data, const ProgressCallback& report_progress = ProgressCallback());
/*!
 * Sync a local file to remote one.
 * \param file_path a path to a file.
 * \param file_final_size size of a remote file. \a file_path will be truncated to this size at the end.
 * \param block_size size of single block.
 * \param delta \a DeltaMap returned by \a get_differences_delta.
 * \param get_data a callback which should return \a block_size size block of data from remote file at
 * block_index * \a block_size position.
 * \param report_progress a callback which will be invoked to report progress.
 * \return if file update was successful.
 * \throws std::invalid_argument
 * \throws std::system_error
 */
bool patch_file(const char* file_path, int64_t file_final_size, size_t block_size, DeltaMap& delta,
				const FetchBlockCallback& get_data, const ProgressCallback& report_progress = ProgressCallback());

};
