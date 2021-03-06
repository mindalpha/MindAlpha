//
// Copyright 2021 Mobvista
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once

#include <vector>
#include <mindalpha/filesys.h>

namespace mindalpha
{

/*! \brief local file system */
class LocalFileSystem : public FileSystem {
public:
    /*! \brief destructor */
    virtual ~LocalFileSystem() {
    }
    /*!
     * \brief get information about a path
     * \param path the path to the file
     * \return the information about the file
     */
    virtual FileInfo GetPathInfo(const URI &path);
    /*!
     * \brief list files in a directory
     * \param path to the file
     * \param out_list the output information about the files
     */
    virtual void ListDirectory(const URI &path, std::vector<FileInfo> *out_list);
    /*!
     * \brief open a stream, will report error and exit if bad thing happens
     * NOTE: the IStream can continue to work even when filesystem was destructed
     * \param path path to file
     * \param uri the uri of the input
     * \param allow_null whether NULL can be returned, or directly report error
     * \return the created stream, can be NULL when allow_null == true and file do not exist
     */
    virtual SeekStream *Open(const URI &path, const char *const flag, bool allow_null);
    /*!
     * \brief open a seekable stream for read
     * \param path the path to the file
     * \param allow_null whether NULL can be returned, or directly report error
     * \return the created stream, can be NULL when allow_null == true and file do not exist
     */
    virtual SeekStream *OpenForRead(const URI &path, bool allow_null);
    /*!
     * \brief get a singleton of LocalFileSystem when needed
     * \return a singleton instance
     */
    inline static LocalFileSystem *GetInstance(void) {
        static LocalFileSystem instance;
        return &instance;
    }
    LocalFileSystem() {
    }

private:
};

}
