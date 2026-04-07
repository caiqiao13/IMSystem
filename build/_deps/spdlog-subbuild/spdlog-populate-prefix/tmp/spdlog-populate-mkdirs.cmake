# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/workspace/chat-system/build/_deps/spdlog-src"
  "/workspace/chat-system/build/_deps/spdlog-build"
  "/workspace/chat-system/build/_deps/spdlog-subbuild/spdlog-populate-prefix"
  "/workspace/chat-system/build/_deps/spdlog-subbuild/spdlog-populate-prefix/tmp"
  "/workspace/chat-system/build/_deps/spdlog-subbuild/spdlog-populate-prefix/src/spdlog-populate-stamp"
  "/workspace/chat-system/build/_deps/spdlog-subbuild/spdlog-populate-prefix/src"
  "/workspace/chat-system/build/_deps/spdlog-subbuild/spdlog-populate-prefix/src/spdlog-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/workspace/chat-system/build/_deps/spdlog-subbuild/spdlog-populate-prefix/src/spdlog-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/workspace/chat-system/build/_deps/spdlog-subbuild/spdlog-populate-prefix/src/spdlog-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
