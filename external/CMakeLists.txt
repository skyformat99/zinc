#
# MIT License
#
# Copyright (c) 2017 Rokas Kupstys
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

if (ZINC_WITH_PYTHON)
    add_subdirectory(pybind11)
endif ()

add_library(flat_hash_map INTERFACE)
target_include_directories(flat_hash_map INTERFACE flat_hash_map)

add_library(json INTERFACE)
target_include_directories(json INTERFACE json)

add_library(CLI11 INTERFACE)
target_include_directories(CLI11 INTERFACE CLI11)

add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE Catch)

add_library(Threadpool INTERFACE)
target_include_directories(Threadpool INTERFACE Threadpool)
