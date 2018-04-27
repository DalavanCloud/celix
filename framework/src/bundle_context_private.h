/**
 *Licensed to the Apache Software Foundation (ASF) under one
 *or more contributor license agreements.  See the NOTICE file
 *distributed with this work for additional information
 *regarding copyright ownership.  The ASF licenses this file
 *to you under the Apache License, Version 2.0 (the
 *"License"); you may not use this file except in compliance
 *with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *Unless required by applicable law or agreed to in writing,
 *software distributed under the License is distributed on an
 *"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 *specific language governing permissions and limitations
 *under the License.
 */
/*
 * bundle_context_private.h
 *
 *  \date       Feb 12, 2013
 *  \author     <a href="mailto:dev@celix.apache.org">Apache Celix Project Team</a>
 *  \copyright  Apache License, Version 2.0
 */


#ifndef BUNDLE_CONTEXT_PRIVATE_H_
#define BUNDLE_CONTEXT_PRIVATE_H_

#include "bundle_context.h"
#include "celix_log.h"
#include "bundle_listener.h"

typedef struct celix_bundle_context_bundle_tracker {
	bundle_context_t *ctx;
	long trackId;
	bundle_listener_t listener;
	celix_bundle_tracker_options_t opts;
} celix_bundle_context_bundle_tracker_t;

struct bundleContext {
	struct framework * framework;
	struct bundle * bundle;

	celix_thread_mutex_t mutex; //protects fields below
	array_list_t *svcRegistrations;
	dm_dependency_manager_t *mng;
	long nextTrackerId;
	hash_map_t *bundleTrackers; //key = trackId, value = celix_bundle_context_bundle_tracker_t

};


#endif /* BUNDLE_CONTEXT_PRIVATE_H_ */
