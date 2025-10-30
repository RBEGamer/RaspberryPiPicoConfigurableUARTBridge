# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/workspaces/pico-sexa-uart-bridge/_deps/picotool-src"
  "/workspaces/pico-sexa-uart-bridge/_deps/picotool-build"
  "/workspaces/pico-sexa-uart-bridge/_deps"
  "/workspaces/pico-sexa-uart-bridge/picotool/tmp"
  "/workspaces/pico-sexa-uart-bridge/picotool/src/picotoolBuild-stamp"
  "/workspaces/pico-sexa-uart-bridge/picotool/src"
  "/workspaces/pico-sexa-uart-bridge/picotool/src/picotoolBuild-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/workspaces/pico-sexa-uart-bridge/picotool/src/picotoolBuild-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/workspaces/pico-sexa-uart-bridge/picotool/src/picotoolBuild-stamp${cfgdir}") # cfgdir has leading slash
endif()
