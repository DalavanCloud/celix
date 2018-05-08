/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 *  KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "Phase2Cmp.h"
#include "Phase2Activator.h"
#include "log_service.h"

#include "celix/BundleActivator.h"

celix::IBundleActivator* celix::createBundleActivator(celix::BundleContext &ctx) {
    return new Phase2Activator{ctx};
}

Phase2Activator::Phase2Activator(celix::BundleContext& ctx) {
    auto &mng = ctx.getDependencyManager();

    Properties props {};
    props["name"] = "phase2b";

    Component<Phase2Cmp>& cmp = mng.createComponent<Phase2Cmp>()
        .addInterface<IPhase2>(IPHASE2_VERSION, props);

    cmp.createServiceDependency<IPhase1>()
            .setRequired(true)
            .setCallbacks(&Phase2Cmp::setPhase1);

    cmp.createCServiceDependency<log_service_t>(OSGI_LOGSERVICE_NAME)
            .setRequired(false)
            .setCallbacks(&Phase2Cmp::setLogService);
}

Phase2Activator::~Phase2Activator() {
    //nothing
}