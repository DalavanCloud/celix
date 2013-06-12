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
 * service_registry.h
 *
 *  \date       Aug 6, 2010
 *  \author    	<a href="mailto:celix-dev@incubator.apache.org">Apache Celix Project Team</a>
 *  \copyright	Apache License, Version 2.0
 */

#ifndef SERVICE_REGISTRY_H_
#define SERVICE_REGISTRY_H_

#include <apr_general.h>

typedef struct serviceRegistry * service_registry_pt;

#include "properties.h"
#include "filter.h"
#include "service_factory.h"
#include "service_event.h"
#include "array_list.h"
#include "service_registration.h"

service_registry_pt serviceRegistry_create(framework_pt framework, void (*serviceChanged)(framework_pt, service_event_type_e, service_registration_pt, properties_pt));
celix_status_t serviceRegistry_destroy(service_registry_pt registry);
celix_status_t serviceRegistry_getRegisteredServices(service_registry_pt registry, apr_pool_t *pool, bundle_pt bundle, array_list_pt *services);
array_list_pt serviceRegistry_getServicesInUse(service_registry_pt registry, bundle_pt bundle);
service_registration_pt serviceRegistry_registerService(service_registry_pt registry, bundle_pt bundle, char * serviceName, void * serviceObject, properties_pt dictionary);
service_registration_pt serviceRegistry_registerServiceFactory(service_registry_pt registry, bundle_pt bundle, char * serviceName, service_factory_pt factory, properties_pt dictionary);
void serviceRegistry_unregisterService(service_registry_pt registry, bundle_pt bundle, service_registration_pt registration);
void serviceRegistry_unregisterServices(service_registry_pt registry, bundle_pt bundle);
celix_status_t serviceRegistry_getServiceReferences(service_registry_pt registry, apr_pool_t *pool, const char *serviceName, filter_pt filter, array_list_pt *references);
void * serviceRegistry_getService(service_registry_pt registry, bundle_pt bundle, service_reference_pt reference);
bool serviceRegistry_ungetService(service_registry_pt registry, bundle_pt bundle, service_reference_pt reference);
void serviceRegistry_ungetServices(service_registry_pt registry, bundle_pt bundle);
array_list_pt serviceRegistry_getUsingBundles(service_registry_pt registry, apr_pool_t *pool, service_reference_pt reference);
service_registration_pt serviceRegistry_findRegistration(service_registry_pt registry, service_reference_pt reference);
celix_status_t serviceRegistry_createServiceReference(service_registry_pt registry, apr_pool_t *pool, service_registration_pt registration, service_reference_pt *reference);

celix_status_t serviceRegistry_getListenerHooks(service_registry_pt registry, apr_pool_t *pool, array_list_pt *hooks);

celix_status_t serviceRegistry_servicePropertiesModified(service_registry_pt registry, service_registration_pt registration, properties_pt oldprops);

#endif /* SERVICE_REGISTRY_H_ */
