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

#include "dm_activator.h"
#include "foo1.h"

#include <stdlib.h>

struct activator {
	foo1_t *foo;
};

static celix_status_t activator_start(activator_t *act, celix_bundle_context_t *ctx) {
	celix_status_t status = CELIX_SUCCESS;
	act->foo = foo1_create();
	if (act->foo == NULL) {
		status = CELIX_ENOMEM;
	} else {
		dm_component_pt cmp = NULL;
		component_create(ctx, "FOO1", &cmp);
		component_setImplementation(cmp, act->foo);

		/*
        With the component_setCallbacksSafe we register callbacks when a component is started / stopped using a component
         with type foo1_t*
        */
		component_setCallbacksSafe(cmp, foo1_t*, NULL, foo1_start, foo1_stop, NULL);

		dm_service_dependency_pt dep = NULL;
		serviceDependency_create(&dep);
		serviceDependency_setRequired(dep, true);
		serviceDependency_setService(dep, EXAMPLE_NAME, EXAMPLE_CONSUMER_RANGE, NULL);
		serviceDependency_setStrategy(dep, DM_SERVICE_DEPENDENCY_STRATEGY_LOCKING);

		/*
        With the serviceDependency_setCallbacksSafe we register callbacks when a service
        is added and about to be removed for the component type foo1_t* and service type example_t*.

        We should protect the usage of the
         service because after removal of the service the memory location of that service
        could be freed
        */
		serviceDependency_setCallbacksSafe(dep, foo1_t*, const example_t*, foo1_setExample, NULL, NULL, NULL, NULL);
		component_addServiceDependency(cmp, dep);

		dependencyManager_add(celix_bundleContext_getDependencyManager(ctx), cmp);

	}
	return status;
}

static celix_status_t activator_stop(activator_t *act, celix_bundle_context_t *ctx) {
	dependencyManager_removeAllComponents(celix_bundleContext_getDependencyManager(ctx));
	foo1_destroy(act->foo);
	return CELIX_SUCCESS;
}


CELIX_GEN_BUNDLE_ACTIVATOR(activator_t, activator_start, activator_stop)