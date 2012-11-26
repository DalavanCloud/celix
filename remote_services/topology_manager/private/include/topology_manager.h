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
 * topology_manager.h
 *
 *  \date       Sep 29, 2011
 *  \author    	<a href="mailto:celix-dev@incubator.apache.org">Apache Celix Project Team</a>
 *  \copyright	Apache License, Version 2.0
 */

#ifndef TOPOLOGY_MANAGER_H_
#define TOPOLOGY_MANAGER_H_

#include "endpoint_listener.h"
#include "service_reference.h"
#include "bundle_context.h"

typedef struct topology_manager *topology_manager_t;

celix_status_t topologyManager_create(bundle_context_t context, apr_pool_t *pool, topology_manager_t *manager);

celix_status_t topologyManager_rsaAdding(void *handle, SERVICE_REFERENCE reference, void **service);
celix_status_t topologyManager_rsaAdded(void *handle, SERVICE_REFERENCE reference, void *service);
celix_status_t topologyManager_rsaModified(void *handle, SERVICE_REFERENCE reference, void *service);
celix_status_t topologyManager_rsaRemoved(void *handle, SERVICE_REFERENCE reference, void *service);

celix_status_t topologyManager_serviceChanged(void *listener, SERVICE_EVENT event);

celix_status_t topologyManager_endpointAdded(void *handle, endpoint_description_t endpoint, char *machtedFilter);
celix_status_t topologyManager_endpointRemoved(void *handle, endpoint_description_t endpoint, char *machtedFilter);

celix_status_t topologyManager_importService(topology_manager_t manager, endpoint_description_t endpoint);
celix_status_t topologyManager_exportService(topology_manager_t manager, SERVICE_REFERENCE reference);
celix_status_t topologyManager_removeService(topology_manager_t manager, SERVICE_REFERENCE reference);

celix_status_t topologyManager_listenerAdded(void *handle, ARRAY_LIST listeners);
celix_status_t topologyManager_listenerRemoved(void *handle, ARRAY_LIST listeners);

#endif /* TOPOLOGY_MANAGER_H_ */
