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

#include "Foo.h"
#include "FooActivator.h"

#include "celix/BundleActivator.h"

CELIX_GEN_CXX_BUNDLE_ACTIVATOR(FooActivator)

celix_status_t FooActivator::start(celix::BundleContext& ctx) {
    auto &mng = ctx.getDependencyManager();
    Component<Foo>& cmp = mng.createComponent<Foo>()
        .setCallbacks(nullptr, &Foo::start, &Foo::stop, nullptr);

    cmp.createServiceDependency<IAnotherExample>()
            .setRequired(true)
            .setVersionRange(IAnotherExample::CONSUMER_RANGE)
            .setCallbacks(&Foo::setAnotherExample);

    cmp.createCServiceDependency<example_t>(EXAMPLE_NAME)
            .setRequired(false)
            .setVersionRange(EXAMPLE_CONSUMER_RANGE)
            .setCallbacks(&Foo::setExample);
    return CELIX_SUCCESS;
}

celix_status_t FooActivator::stop(celix::BundleContext &) {return CELIX_SUCCESS;}