# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
# 
#   http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.


set(CELIX_CMAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

if (ANDROID)
	add_definitions( -DANDROID )
endif ()

include(${CELIX_CMAKE_DIRECTORY}/cmake_celix/Dependencies.cmake)
include(${CELIX_CMAKE_DIRECTORY}/cmake_celix/BundlePackaging.cmake)
include(${CELIX_CMAKE_DIRECTORY}/cmake_celix/DeployPackaging.cmake)
include(${CELIX_CMAKE_DIRECTORY}/cmake_celix/DockerPackaging.cmake)
include(${CELIX_CMAKE_DIRECTORY}/cmake_celix/ApacheRat.cmake)
include(${CELIX_CMAKE_DIRECTORY}/cmake_celix/CodeCoverage.cmake)
include(${CELIX_CMAKE_DIRECTORY}/cmake_celix/BuildOptions.cmake)
