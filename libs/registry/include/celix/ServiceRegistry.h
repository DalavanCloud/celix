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

#ifndef CXX_CELIX_SERVICEREGISTRY_H
#define CXX_CELIX_SERVICEREGISTRY_H

#include <utility>
#include <vector>

#include "celix/Constants.h"
#include "celix/Properties.h"
#include "celix/Utils.h"
#include "celix/IResourceBundle.h"
#include "celix/IServiceFactory.h"

namespace celix {

    //forward declaration
    class ServiceRegistry;

    // RAII service registration: out of scope -> deregister service
    // NOTE access not thread safe -> TODO make thread save?
    class ServiceRegistration {
    public:
        ServiceRegistration();
        ServiceRegistration(ServiceRegistration &&rhs) noexcept;
        ServiceRegistration(const ServiceRegistration &rhs) = delete;
        ~ServiceRegistration();
        ServiceRegistration& operator=(ServiceRegistration &&rhs) noexcept;
        ServiceRegistration& operator=(const ServiceRegistration &rhs) = delete;

        long serviceId() const;
        bool valid() const;
        const celix::Properties& properties() const;
        const std::string& serviceName() const;
        bool factory() const;
        bool registered() const;

        void unregister();
    private:
        class Impl; //opaque impl class
        std::unique_ptr<celix::ServiceRegistration::Impl> pimpl;
        explicit ServiceRegistration(celix::ServiceRegistration::Impl *impl);
        friend ServiceRegistry;
    };

    template<typename I>
    struct ServiceTrackerOptions {
        ServiceTrackerOptions() = default;
        ServiceTrackerOptions(ServiceTrackerOptions &&rhs) noexcept = default;
        ServiceTrackerOptions(const ServiceTrackerOptions &rhs) = default;
        ServiceTrackerOptions& operator=(ServiceTrackerOptions &&rhs) = default;
        ServiceTrackerOptions& operator=(const ServiceTrackerOptions &rhs) = default;

        std::string filter{};

        /*TODO maybe refactor all I* to std::shared_ptr and use a custom deleter to sync whether a bundle is done using
        all the functions -> i.e. safe delete and possible lock free? Not sure, because a std::shared_ptr instance
        access it not thread safe?. Investigate */

        std::function<void(I *svc)> set = {};
        std::function<void(I *svc, const celix::Properties &props)> setWithProperties = {};
        std::function<void(I *svc, const celix::Properties &props, const celix::IResourceBundle &owner)> setWithOwner = {};

        std::function<void(I *svc)> add = {};
        std::function<void(I *svc, const celix::Properties &props)> addWithProperties = {};
        std::function<void(I *svc, const celix::Properties &props, const celix::IResourceBundle &owner)> addWithOwner = {};

        std::function<void(I *svc)> remove = {};
        std::function<void(I *svc, const celix::Properties &props)> removeWithProperties = {};
        std::function<void(I *svc, const celix::Properties &props, const celix::IResourceBundle &owner)> removeWithOwner = {};

        std::function<void(std::vector<I*> rankedServices)> update = {};
        std::function<void(std::vector<std::tuple<I*, const celix::Properties*>> rankedServices)> updateWithProperties = {};
        std::function<void(std::vector<std::tuple<I*, const celix::Properties*, const celix::IResourceBundle *>> rankedServices)> updateWithOwner = {};
    };

    //RAII service tracker: out of scope -> stop tracker
    // NOTE access not thread safe -> TODO make thread save?
    class ServiceTracker {
    public:
        ServiceTracker();
        ServiceTracker(ServiceTracker &&rhs) noexcept;
        ServiceTracker(const ServiceTracker &rhs) = delete;
        ~ServiceTracker();
        ServiceTracker& operator=(ServiceTracker &&rhs) noexcept;
        ServiceTracker& operator=(const ServiceTracker &rhs) = delete;

        int trackCount() const;
        const std::string& serviceName() const;
        const std::string& filter() const;
        bool valid() const;

        //TODO useService(s) calls

        void stop();
    private:
        class Impl; //opaque impl class
        std::unique_ptr<celix::ServiceTracker::Impl> pimpl;
        explicit ServiceTracker(celix::ServiceTracker::Impl *impl);
        friend ServiceRegistry;
    };

    //NOTE access thread safe
    class ServiceRegistry {
    public:
        explicit ServiceRegistry(std::string name);
        ~ServiceRegistry();
        ServiceRegistry(celix::ServiceRegistry &&rhs) noexcept;
        ServiceRegistry& operator=(celix::ServiceRegistry&& rhs);
        ServiceRegistry& operator=(ServiceRegistry &rhs) = delete;
        ServiceRegistry(const ServiceRegistry &rhs) = delete;

        const std::string& name() const;

        template<typename I>
        celix::ServiceRegistration registerService(I &svc, celix::Properties props = {}, std::shared_ptr<const celix::IResourceBundle> owner = {}) {
            auto svcName = celix::serviceName<I>();
            auto voidSvc = std::shared_ptr<void>(static_cast<void*>(&svc), [](void*){/*nop*/}); //transform to std::shared_ptr to minimize the underlining impl needed.
            return registerService(svcName, std::move(voidSvc), std::move(props), std::move(owner));
        }

        template<typename I>
        celix::ServiceRegistration registerService(std::shared_ptr<I> svc, celix::Properties props = {}, std::shared_ptr<const celix::IResourceBundle> owner = {}) {
            //TOOD refactor to using a service factory to store the shared or unique_ptr
            auto svcName = celix::serviceName<I>();
            return registerService(svcName, static_cast<std::shared_ptr<void>>(svc), std::move(props), std::move(owner));
        }

        template<typename F>
        celix::ServiceRegistration registerFunctionService(const std::string &functionName, F&& func, celix::Properties props = {}, std::shared_ptr<const celix::IResourceBundle> owner = {});

        template<typename I>
        celix::ServiceRegistration registerServiceFactory(std::shared_ptr<celix::IServiceFactory<I>> factory, celix::Properties props = {}, std::shared_ptr<const celix::IResourceBundle> owner = {});

        template<typename I>
        //NOTE C++17 typename std::enable_if<!std::is_callable<I>::value, long>::type
        long findService(const std::string &filter = "") const {
            auto services = findServices<I>(filter);
            return services.size() > 0 ? services[0] : -1L;
        }

        template<typename I>
        //NOTE C++17 typename std::enable_if<std::is_callable<I>::value, long>::type
        long findFunctionService(const std::string &functionName, const std::string &filter = "") const {
            auto services = functionServiceName<I>(functionName, filter);
            return services.size() > 0 ? services[0] : -1L;
        }

        template<typename I>
        //NOTE C++17 typename std::enable_if<!std::is_callable<I>::value, std::vector<long>>::type
        std::vector<long> findServices(const std::string &filter = "") const {
            auto svcName = celix::serviceName<I>();
            return findServices(svcName, filter);
        }

        template<typename F>
        //NOTE C++17 typename std::enable_if<std::is_callable<I>::value, std::vector<long>>::type
        std::vector<long> findFunctionServices(const std::string &functionName, const std::string &filter = "") const {
            auto svcName = celix::functionServiceName<F>(functionName);
            return findServices(svcName, filter);
        }

        template<typename I>
        celix::ServiceTracker trackServices(celix::ServiceTrackerOptions<I> options = {}, std::shared_ptr<const celix::IResourceBundle> requester = {}) {
            auto svcName = celix::serviceName<I>();
            return trackServices<I>(svcName, std::move(options), std::move(requester));
        }

        template<typename F>
        celix::ServiceTracker trackFunctionServices(std::string functionName, celix::ServiceTrackerOptions<F> options = {}, std::shared_ptr<const celix::IResourceBundle> requester = {}) {
            auto svcName = celix::functionServiceName<F>(functionName);
            return trackServices<F>(svcName, std::move(options), std::move(requester));
        }

        //TODO trackTrackers

        template<typename I>
        int useServices(std::function<void(I& svc)> use, const std::string &filter = "", std::shared_ptr<const celix::IResourceBundle> requester = {}) const {
            auto svcName = celix::serviceName<I>();
            return useServices<I>(svcName, use, nullptr, nullptr, filter, std::move(requester));
        }

        template<typename I>
        int useServices(std::function<void(I& svc, const celix::Properties &props)> use, const std::string &filter = "", std::shared_ptr<const celix::IResourceBundle> requester = {}) const {
            auto svcName = celix::serviceName<I>();
            return useServices<I>(svcName, nullptr, use, nullptr, filter, std::move(requester));
        }

        template<typename I>
        int useServices(std::function<void(I& svc, const celix::Properties &props, const celix::IResourceBundle &bnd)> use, const std::string &filter = "", std::shared_ptr<const celix::IResourceBundle> requester = {}) const {
            auto svcName = celix::serviceName<I>();
            return useServices<I>(svcName, nullptr, nullptr, use, filter, std::move(requester));
        }

        template<typename F>
        int useFunctionServices(const std::string &functionName, std::function<void(F &function)> use, const std::string &filter = "", std::shared_ptr<const celix::IResourceBundle> requester = {}) const {
            auto svcName = celix::functionServiceName<F>(functionName);
            return useServices<F>(svcName, use, nullptr, nullptr, filter, std::move(requester));
        }

        template<typename F>
        int useFunctionServices(const std::string &functionName, std::function<void(F &function, const celix::Properties&)> use, const std::string &filter = "", std::shared_ptr<const celix::IResourceBundle> requester = {}) const {
            auto svcName = celix::functionServiceName<F>(functionName);
            return useServices<F>(svcName, nullptr, use, nullptr, filter, std::move(requester));
        }

        template<typename F>
        int useFunctionServices(const std::string &functionName, std::function<void(F &function, const celix::Properties&, const celix::IResourceBundle&)> use, const std::string &filter = "", std::shared_ptr<const celix::IResourceBundle> requester = {}) const {
            auto svcName = celix::functionServiceName<F>(functionName);
            return useServices<F>(svcName, nullptr, nullptr, use, filter, std::move(requester));
        }

        template<typename I>
        bool useService(std::function<void(I& svc)> use, const std::string &filter = "", std::shared_ptr<const celix::IResourceBundle> requester = {}) const {
            auto svcName = celix::serviceName<I>();
            return useService<I>(svcName, use, nullptr, nullptr, filter, std::move(requester));
        }

        template<typename I>
        bool useService(std::function<void(I& svc, const celix::Properties &props)> use, const std::string &filter = "", std::shared_ptr<const celix::IResourceBundle> requester = {}) const {
            auto svcName = celix::serviceName<I>();
            return useService<I>(svcName, nullptr, use, nullptr, filter, std::move(requester));
        }

        template<typename I>
        bool useService(std::function<void(I& svc, const celix::Properties &props, const celix::IResourceBundle &bnd)> use, const std::string &filter = "", std::shared_ptr<const celix::IResourceBundle> requester = {}) const {
            auto svcName = celix::serviceName<I>();
            return useService<I>(svcName, nullptr, nullptr, use, filter, std::move(requester));
        }

        template<typename F>
        int useFunctionService(const std::string &functionName, std::function<void(F &function)> use, const std::string &filter = "", std::shared_ptr<const celix::IResourceBundle> requester = {}) const {
            auto svcName = celix::functionServiceName<F>(functionName);
            return useService<F>(svcName, use, nullptr, nullptr, filter, std::move(requester));
        }

        template<typename F>
        int useFunctionService(const std::string &functionName, std::function<void(F &function, const celix::Properties&)> use, const std::string &filter = "", std::shared_ptr<const celix::IResourceBundle> requester = {}) const {
            auto svcName = celix::functionServiceName<F>(functionName);
            return useService<F>(svcName, nullptr, use, nullptr, filter, std::move(requester));
        }

        template<typename F>
        int useFunctionService(const std::string &functionName, std::function<void(F &function, const celix::Properties&, const celix::IResourceBundle&)> use, const std::string &filter = "", std::shared_ptr<const celix::IResourceBundle> requester = {}) const {
            auto svcName = celix::functionServiceName<F>(functionName);
            return useService<F>(svcName, nullptr, nullptr, use, filter, std::move(requester));
        }


        long nrOfRegisteredServices() const;
        long nrOfServiceTrackers() const;
    private:
        class Impl;
        std::unique_ptr<celix::ServiceRegistry::Impl> pimpl;

        //register services
        celix::ServiceRegistration registerService(std::string svcName, std::shared_ptr<void> svc, celix::Properties props, std::shared_ptr<const celix::IResourceBundle> owner);
        celix::ServiceRegistration registerServiceFactory(std::string svcName, std::shared_ptr<celix::IServiceFactory<void>> factory, celix::Properties props, std::shared_ptr<const celix::IResourceBundle> owner);

        //use Services
        template<typename I>
        int useServices(
                const std::string &svcName,
                std::function<void(I &svc)> use,
                std::function<void(I &svc, const celix::Properties &props)> useWithProps,
                std::function<void(I &svc, const celix::Properties &props, const celix::IResourceBundle &bnd)> useWithOwner,
                const std::string &filter,
                std::shared_ptr<const celix::IResourceBundle> requester) const;
        int useServices(const std::string &svcName, std::function<void(void *svc, const celix::Properties &props, const celix::IResourceBundle &bnd)> &use, const std::string &filter, std::shared_ptr<const celix::IResourceBundle> requester) const;

        template<typename I>
        bool useService(
                const std::string &svcName,
                std::function<void(I &svc)> use,
                std::function<void(I &svc, const celix::Properties &props)> useWithProps,
                std::function<void(I &svc, const celix::Properties &props, const celix::IResourceBundle &bnd)> useWithOwner,
                const std::string &filter,
                std::shared_ptr<const celix::IResourceBundle> requester) const;
        bool useService(const std::string &svcName, std::function<void(void *svc, const celix::Properties &props, const celix::IResourceBundle &bnd)> &use, const std::string &filter, std::shared_ptr<const celix::IResourceBundle> requester) const;

        //find Services
        std::vector<long> findServices(const std::string &name, const std::string &filter) const;


        //track services
        template<typename I>
        celix::ServiceTracker trackServices(std::string svcName, celix::ServiceTrackerOptions<I> options, std::shared_ptr<const celix::IResourceBundle> requester);
        celix::ServiceTracker trackServices(std::string svcName, ServiceTrackerOptions<void> options, std::shared_ptr<const celix::IResourceBundle> requester);
    };
}


template<typename F>
inline celix::ServiceRegistration celix::ServiceRegistry::registerFunctionService(const std::string &functionName, F&& func, celix::Properties props, std::shared_ptr<const celix::IResourceBundle> owner) {
    class FunctionServiceFactory : public celix::IServiceFactory<void> {
    public:
        FunctionServiceFactory(F&& _function) : function{std::forward<F>(_function)} {}

        void* getService(const celix::IResourceBundle &, const celix::Properties &) noexcept override {
            return static_cast<void*>(&function);
        }
        void ungetService(const celix::IResourceBundle &, const celix::Properties &) noexcept override {
            //nop;
        }
    private:
        F function;
    };
    auto factory = std::shared_ptr<celix::IServiceFactory<void>>{new FunctionServiceFactory{std::forward<F>(func)}};

    std::string svcName = celix::functionServiceName<typename std::remove_reference<F>::type>(functionName);

    return registerServiceFactory(std::move(svcName), factory, std::move(props), std::move(owner));
}

template<typename I>
inline celix::ServiceRegistration celix::ServiceRegistry::registerServiceFactory(std::shared_ptr<celix::IServiceFactory<I>> factory, celix::Properties props, std::shared_ptr<const celix::IResourceBundle> owner) {
    std::string svcName = celix::serviceName<I>();

    class VoidServiceFactory : public celix::IServiceFactory<void> {
    public:
        VoidServiceFactory(std::shared_ptr<celix::IServiceFactory<I>> _factory) : factory{std::move(_factory)} {}

        void* getService(const celix::IResourceBundle &bnd, const celix::Properties &props) noexcept override {
            I* service = factory->getService(bnd, props);
            return static_cast<void*>(service);
        }
        void ungetService(const celix::IResourceBundle &bnd, const celix::Properties &props) noexcept override {
            factory->ungetService(bnd, props);
        }
    private:
        std::shared_ptr<celix::IServiceFactory<I>> factory;
    };

    auto voidFactory = std::shared_ptr<celix::IServiceFactory<void>>{new VoidServiceFactory{std::move(factory)}};
    return registerServiceFactory(std::move(svcName), std::move(voidFactory), std::move(props), std::move(owner));
}

template<typename I>
inline int celix::ServiceRegistry::useServices(
        const std::string &svcName,
        std::function<void(I &svc)> use,
        std::function<void(I &svc, const celix::Properties &props)> useWithProps,
        std::function<void(I &svc, const celix::Properties &props, const celix::IResourceBundle &bnd)> useWithOwner,
        const std::string &filter,
        std::shared_ptr<const celix::IResourceBundle> requester) const {
    std::function<void(void*,const celix::Properties&, const celix::IResourceBundle&)> voidUse = [&](void *svc, const celix::Properties &props, const celix::IResourceBundle &bnd) {
        I* typedSvc = static_cast<I*>(svc);
        if (use) {
            use(*typedSvc);
        }
        if (useWithProps) {
            useWithProps(*typedSvc, props);
        }
        if (useWithOwner) {
            useWithOwner(*typedSvc, props, bnd);
        }
    };
    return useServices(svcName, voidUse, filter, requester);
}

template<typename I>
inline bool celix::ServiceRegistry::useService(
        const std::string &svcName,
        std::function<void(I &svc)> use,
        std::function<void(I &svc, const celix::Properties &props)> useWithProps,
        std::function<void(I &svc, const celix::Properties &props, const celix::IResourceBundle &bnd)> useWithOwner,
        const std::string &filter,
        std::shared_ptr<const celix::IResourceBundle> requester) const {
    std::function<void(void*,const celix::Properties&, const celix::IResourceBundle&)> voidUse = [&](void *svc, const celix::Properties &props, const celix::IResourceBundle &bnd) -> void {
        I* typedSvc = static_cast<I*>(svc);
        if (use) {
            use(*typedSvc);
        }
        if (useWithProps) {
            useWithProps(*typedSvc, props);
        }
        if (useWithOwner) {
            useWithOwner(*typedSvc, props, bnd);
        }
    };
    return useService(svcName, voidUse, filter, requester);
}

template<typename I>
inline celix::ServiceTracker celix::ServiceRegistry::trackServices(std::string svcName, const celix::ServiceTrackerOptions<I> options, std::shared_ptr<const celix::IResourceBundle> requester) {
    ServiceTrackerOptions<void> opts{};
    opts.filter = std::move(options.filter);

    if (options.set != nullptr) {
        auto set = std::move(options.set);
        opts.set = [set](void *svc){
            I *typedSvc = static_cast<I*>(svc);
            set(typedSvc);
        };
    }
    if (options.setWithProperties != nullptr) {
        auto set = std::move(options.setWithProperties);
        opts.setWithProperties = [set](void *svc, const celix::Properties &props){
            I *typedSvc = static_cast<I*>(svc);
            set(typedSvc, props);
        };
    }
    if (options.setWithOwner != nullptr) {
        auto set = std::move(options.setWithOwner);
        opts.setWithOwner = [set](void *svc, const celix::Properties &props, const celix::IResourceBundle &owner){
            I *typedSvc = static_cast<I*>(svc);
            set(typedSvc, props, owner);
        };
    }

    if (options.add != nullptr) {
        auto add = std::move(options.add);
        opts.add = [add](void *svc) {
            I *typedSvc = static_cast<I*>(svc); //note actual argument is I*
            add(typedSvc);
        };
    }
    if (options.addWithProperties != nullptr) {
        auto add = std::move(options.addWithProperties);
        opts.addWithProperties = [add](void *svc, const celix::Properties &props) {
            I *typedSvc = static_cast<I*>(svc); //note actual argument is I*
            add(typedSvc, props);
        };
    }
    if (options.addWithOwner != nullptr) {
        auto add = std::move(options.addWithOwner);
        opts.addWithOwner = [add](void *svc, const celix::Properties &props, const celix::IResourceBundle &bnd) {
            I *typedSvc = static_cast<I*>(svc); //note actual argument is I*
            add(typedSvc, props, bnd);
        };
    }

    if (options.remove != nullptr) {
        auto rem = std::move(options.remove);
        opts.remove = [rem](void *svc) {
            I *typedSvc = static_cast<I*>(svc); //note actual argument is I*
            rem(typedSvc);
        };
    }
    if (options.removeWithProperties != nullptr) {
        auto rem = std::move(options.removeWithProperties);
        opts.removeWithProperties = [rem](void *svc, const celix::Properties &props) {
            I *typedSvc = static_cast<I*>(svc); //note actual argument is I*
            rem(typedSvc, props);
        };
    }
    if (options.removeWithOwner != nullptr) {
        auto rem = std::move(options.removeWithOwner);
        opts.removeWithOwner = [rem](void *svc, const celix::Properties &props, const celix::IResourceBundle &bnd) {
            I *typedSvc = static_cast<I*>(svc); //note actual argument is I*
            rem(typedSvc, props, bnd);
        };
    }

    if (options.update != nullptr) {
        auto update = std::move(options.update);
        opts.update = [update](std::vector<void*> rankedServices) {
            std::vector<I*> typedServices{};
            typedServices.reserve(rankedServices.size());
            for (void *svc : rankedServices) {
                typedServices.push_back(static_cast<I*>(svc));
            }
            update(std::move(typedServices));
        };
    }
    if (options.updateWithProperties != nullptr) {
        auto update = std::move(options.updateWithProperties);
        opts.updateWithProperties = [update](std::vector<std::tuple<void*, const celix::Properties *>> rankedServices) {
            std::vector<std::tuple<I*, const celix::Properties*>> typedServices{};
            typedServices.reserve(rankedServices.size());
            for (auto &tuple : rankedServices) {
                typedServices.push_back(std::make_tuple(static_cast<I*>(std::get<0>(tuple)), std::get<1>(tuple)));
            }
            update(std::move(typedServices));
        };
    }
    if (options.updateWithOwner != nullptr) {
        auto update = std::move(options.updateWithOwner);
        opts.updateWithOwner = [update](std::vector<std::tuple<void*, const celix::Properties *, const celix::IResourceBundle*>> rankedServices) {
            std::vector<std::tuple<I*, const celix::Properties*, const celix::IResourceBundle*>> typedServices{};
            typedServices.reserve(rankedServices.size());
            for (auto &tuple : rankedServices) {
                typedServices.push_back(std::make_tuple(static_cast<I*>(std::get<0>(tuple)), std::get<1>(tuple), std::get<2>(tuple)));
            }
            update(std::move(typedServices));
        };
    }

    return trackServices(std::move(svcName), std::move(opts), requester);
}

#endif //CXX_CELIX_SERVICEREGISTRY_H
