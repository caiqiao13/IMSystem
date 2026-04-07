# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/workspace/chat-system/build/_deps/asio-src"
  "/workspace/chat-system/build/_deps/asio-build"
  "/workspace/chat-system/build/_deps/asio-subbuild/asio-populate-prefix"
  "/workspace/chat-system/build/_deps/asio-subbuild/asio-populate-prefix/tmp"
  "/workspace/chat-system/build/_deps/asio-subbuild/asio-populate-prefix/src/asio-populate-stamp"
  "/workspace/chat-system/build/_deps/asio-subbuild/asio-populate-prefix/src"
  "/workspace/chat-system/build/_deps/asio-subbuild/asio-populate-prefix/src/asio-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/workspace/chat-system/build/_deps/asio-subbuild/asio-populate-prefix/src/asio-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/workspace/chat-system/build/_deps/asio-subbuild/asio-populate-prefix/src/asio-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
