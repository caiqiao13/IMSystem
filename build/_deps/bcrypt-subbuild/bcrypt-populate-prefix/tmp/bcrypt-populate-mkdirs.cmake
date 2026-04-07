# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/workspace/chat-system/build/_deps/bcrypt-src"
  "/workspace/chat-system/build/_deps/bcrypt-build"
  "/workspace/chat-system/build/_deps/bcrypt-subbuild/bcrypt-populate-prefix"
  "/workspace/chat-system/build/_deps/bcrypt-subbuild/bcrypt-populate-prefix/tmp"
  "/workspace/chat-system/build/_deps/bcrypt-subbuild/bcrypt-populate-prefix/src/bcrypt-populate-stamp"
  "/workspace/chat-system/build/_deps/bcrypt-subbuild/bcrypt-populate-prefix/src"
  "/workspace/chat-system/build/_deps/bcrypt-subbuild/bcrypt-populate-prefix/src/bcrypt-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/workspace/chat-system/build/_deps/bcrypt-subbuild/bcrypt-populate-prefix/src/bcrypt-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/workspace/chat-system/build/_deps/bcrypt-subbuild/bcrypt-populate-prefix/src/bcrypt-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
