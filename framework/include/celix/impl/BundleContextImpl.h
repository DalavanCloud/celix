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

#ifndef CELIX_IMPL_BUNDLECONTEXTIMPL_H
#define CELIX_IMPL_BUNDLECONTEXTIMPL_H

#include <mutex>
#include <cstring>
#include <memory>

#include "bundle_context.h"
#include "service_tracker.h"

#include "celix/impl/BundleImpl.h"
#include "celix/dm/DependencyManager.h"

namespace celix {

    namespace impl {

        static celix::Properties createFromCProps(const celix_properties_t *c_props) {
            celix::Properties result{};
            const char *key = nullptr;
            CELIX_PROPERTIES_FOR_EACH(const_cast<celix_properties_t*>(c_props), key) {
                result[key] = celix_properties_get(c_props, key);
            }
            return result;
        }

        class BundleContextImpl : public celix::BundleContext {
        public:
            BundleContextImpl(bundle_context_t *ctx, celix::Framework& _fw) : c_ctx{ctx}, fw{_fw}, bnd{c_ctx}, dm{c_ctx} {}

            virtual ~BundleContextImpl() {
                //NOTE no need to destroy the c bundle context -> done by c framework

                //clearing tracker entries
                {
                    std::lock_guard<std::mutex> lock{this->mutex};
                    for (auto &pair : this->trackEntries) {
                        celix_bundleContext_stopTracker(this->c_ctx, pair.first);
                    }
                    this->trackEntries.clear();
                }

                this->c_ctx = nullptr;
            }

            BundleContextImpl(const BundleContextImpl&) = delete;
            BundleContextImpl& operator=(const BundleContextImpl&) = delete;
            BundleContextImpl(BundleContextImpl&&) = delete;
            BundleContextImpl& operator=(BundleContextImpl&&) = delete;

            void unregisterService(long serviceId) noexcept override {
                celix_bundleContext_unregisterService(this->c_ctx, serviceId);
            }

            std::vector<long> findServices(const std::string &/*serviceName*/, const std::string &/*versionRange*/, const std::string &/*filter*/, const std::string &/*lang = ""*/) noexcept override  {
                std::vector<long> result{};
//                auto use = [&result](void *, const celix::Properties &props, const celix::Bundle &) {
//                    long id = celix::getProperty(props, OSGI_FRAMEWORK_SERVICE_ID, -1);
//                    if (id >= 0) {
//                        result.push_back(id);
//                    }
//                };
                //TODO useServicesWithOptions this->useServicesInternal(serviceName, versionRange, filter, use);
                return result;
            }

            void stopTracker(long trackerId) noexcept override {
                std::lock_guard<std::mutex> lock{this->mutex};
                celix_bundleContext_stopTracker(this->c_ctx, trackerId);
                auto it = this->trackEntries.find(trackerId);
                if (it != this->trackEntries.end()) {
                    this->trackEntries.erase(it);
                }
            }

            std::string getProperty(const std::string &key, std::string defaultValue) noexcept  override  {
                const char *val = nullptr;
                bundleContext_getPropertyWithDefault(this->c_ctx, key.c_str(), defaultValue.c_str(), &val);
                return std::string{val};
            }

            bool isInvalid() const noexcept {
                return this->c_ctx == nullptr;
            }

            long registerEmbeddedBundle(
                    std::string /*id*/,
                    std::function<void(celix::BundleContext & ctx)> /*start*/,
                    std::function<void(celix::BundleContext & ctx)> /*stop*/,
                    celix::Properties /*manifest*/,
                    bool /*autoStart*/
            ) noexcept override {
                return -1; //TODO
            };

            void registerEmbeddedBundle(const celix::BundleRegistrationOptions &/*opts*/) noexcept override {
                //TODO
            }

            long installBundle(const std::string &bundleLocation, bool autoStart) noexcept override {
                long bndId = -1;
                if (this->c_ctx != nullptr) {
                    bundle_t *bnd = nullptr;
                    bundleContext_installBundle(this->c_ctx, bundleLocation.c_str(), &bnd);
                    if (bnd != nullptr) {
                        bundle_getBundleId(bnd, &bndId);
                        if (autoStart) {
                            bundle_start(bnd);
                        }
                    }
                }
                return bndId;
            }


            void useBundles(const std::function<void(const celix::Bundle &bnd)> &use) noexcept override {
                auto c_use = [](void *handle, const celix_bundle_t *c_bnd) {
                    auto *func =  static_cast<std::function<void(const celix::Bundle &bnd)>*>(handle);
                    auto m_bnd = const_cast<celix_bundle_t*>(c_bnd);
                    celix::impl::BundleImpl bnd{m_bnd};
                    (*func)(bnd);
                };
                celix_bundleContext_useBundles(this->c_ctx, (void*)(&use), c_use);
            }

            bool useBundle(long bundleId, const std::function<void(const celix::Bundle &bnd)> &use) noexcept override {
                auto c_use = [](void *handle, const celix_bundle_t *c_bnd) {
                    auto *func =  static_cast<std::function<void(const celix::Bundle &bnd)>*>(handle);
                    auto m_bnd = const_cast<celix_bundle_t*>(c_bnd);
                    celix::impl::BundleImpl bnd{m_bnd};
                    (*func)(bnd);
                };
                return celix_bundleContext_useBundle(this->c_ctx, bundleId, (void*)(&use), c_use);
            }

            celix::Framework& getFramework() noexcept override {
                return this->fw;
            }

            celix::Bundle& getBundle() noexcept override {
                return this->bnd;
            };

            celix::dm::DependencyManager& getDependencyManager() noexcept override {
                return this->dm;
            }

        protected:

            long registerServiceInternal(const celix_service_registration_options_t &opts) noexcept override {
                return celix_bundleContext_registerServiceWithOptions(this->c_ctx, &opts);
            }

            long trackServiceInternal(const std::string &serviceName, std::function<void(void *svc)> set) noexcept override  {
                celix_service_tracking_options_t opts; //TODO why not ok? = CELIX_EMPTY_SERVICE_TRACKING_OPTIONS;
                std::memset(&opts, 0, sizeof(opts));

                auto c_set = [](void *handle, void *svc) {
                    auto *entry = static_cast<TrackEntry*>(handle);
                    //celix::Properties props = createFromCProps(c_props);
                    //auto m_bnd = const_cast<celix_bundle_t *>(c_bnd);
                    //celix::impl::BundleImpl bnd{m_bnd};
                    (entry->set)(svc);
                };
                const char *cname = serviceName.empty() ? nullptr : serviceName.c_str();

                opts.filter.serviceName = cname;
                opts.filter.serviceLanguage = CELIX_FRAMEWORK_SERVICE_CXX_LANGUAGE;

                auto te = std::unique_ptr<TrackEntry>{new TrackEntry{}};
                te->set = std::move(set);

                opts.callbackHandle = te.get();
                opts.set = c_set;

                long id = celix_bundleContext_trackServicesWithOptions(this->c_ctx, &opts);
                if (id >= 0) {
                    std::lock_guard<std::mutex> lock{this->mutex};
                    this->trackEntries[id] = std::move(te);
                }
                return id;
            }

            long trackServicesInternal(
                    const std::string &serviceName,
                    std::function<void(void *svc)> add,
                    std::function<void(void *svc)> remove
            ) noexcept override {
                celix_service_tracking_options_t opts;
                std::memset(&opts, 0, sizeof(opts));

                auto c_add = [](void *handle, void *svc) {
                    auto *entry = static_cast<TrackEntry*>(handle);
                    //celix::Properties props = createFromCProps(c_props);
                    //auto m_bnd = const_cast<celix_bundle_t *>(c_bnd);
                    //celix::impl::BundleImpl bnd{m_bnd};
                    (entry->add)(svc);
                };
                auto c_remove = [](void *handle, void *svc) {
                    auto *entry = static_cast<TrackEntry*>(handle);
                    //celix::Properties props = createFromCProps(c_props);
                    //auto m_bnd = const_cast<celix_bundle_t *>(c_bnd);
                    //celix::impl::BundleImpl bnd{m_bnd};
                    (entry->remove)(svc);
                };

                opts.filter.serviceName = serviceName.empty() ? nullptr : serviceName.c_str();
                opts.filter.serviceLanguage = CELIX_FRAMEWORK_SERVICE_CXX_LANGUAGE;

                auto te = std::unique_ptr<TrackEntry>{new TrackEntry{}};
                te->add = std::move(add);
                te->remove = std::move(remove);

                opts.callbackHandle = te.get();
                opts.add = c_add;
                opts.remove = c_remove;

                long id = celix_bundleContext_trackServicesWithOptions(this->c_ctx, &opts);
                if (id >= 0) {
                    std::lock_guard<std::mutex> lock{this->mutex};
                    this->trackEntries[id] = std::move(te);
                }
                return id;
            }

            bool useServiceInternal(
                    const std::string &serviceName,
                    const std::function<void(void *svc, const celix::Properties &props, const celix::Bundle &svcOwner)> &use) noexcept override {
                auto c_use = [](void *handle, void *svc, const celix_properties_t *c_props, const celix_bundle_t *c_svcOwner) {
                    auto *fn = static_cast<const std::function<void(void *svc, const celix::Properties &props, const celix::Bundle &svcOwner)> *>(handle);
                    celix::Properties props = createFromCProps(c_props);
                    celix_bundle_t *m_bnd = const_cast<celix_bundle_t*>(c_svcOwner);
                    celix::impl::BundleImpl bnd{m_bnd};
                    (*fn)(svc, props, bnd);
                };

                celix_service_use_options_t opts;
                std::memset(&opts, 0, sizeof(opts));

                opts.filter.serviceName = serviceName.empty() ? nullptr : serviceName.c_str();;
                opts.filter.serviceLanguage = celix::Constants::SERVICE_CXX_LANG;
                opts.callbackHandle = (void*)&use;
                opts.useWithOwner = c_use;

                return celix_bundleContext_useServiceWithOptions(this->c_ctx, &opts);
            }

            void useServicesInternal(
                    const std::string &serviceName,
                    const std::function<void(void *svc, const celix::Properties &props, const celix::Bundle &svcOwner)> &use) noexcept override {
                auto c_use = [](void *handle, void *svc, const celix_properties_t *c_props, const celix_bundle_t *c_svcOwner) {
                    auto *fn = static_cast<const std::function<void(void *svc, const celix::Properties &props, const celix::Bundle &svcOwner)> *>(handle);
                    celix::Properties props = createFromCProps(c_props);
                    celix_bundle_t *m_bnd = const_cast<celix_bundle_t*>(c_svcOwner);
                    celix::impl::BundleImpl bnd{m_bnd};
                    (*fn)(svc, props, bnd);
                };

                celix_service_use_options_t opts;
                std::memset(&opts, 0, sizeof(opts));

                opts.filter.serviceName = serviceName.empty() ? nullptr : serviceName.c_str();;
                opts.filter.serviceLanguage = celix::Constants::SERVICE_CXX_LANG;
                opts.callbackHandle = (void*)&use;
                opts.useWithOwner = c_use;

                celix_bundleContext_useServicesWithOptions(this->c_ctx, &opts);
            }

        private:
            //initialized in ctor
            bundle_context_t *c_ctx;
            celix::Framework& fw;
            celix::impl::BundleImpl bnd;
            celix::dm::DependencyManager dm;

            struct TrackEntry {
                std::function<void(void *)> set{};
                std::function<void(void *, const celix::Properties &)> setWithProperties{};
                std::function<void(void *, const celix::Properties &, const celix::Bundle &)> setWithOwner{};

                std::function<void(void *)> add{};
                std::function<void(void *, const celix::Properties &)> addWithProperties{};
                std::function<void(void *, const celix::Properties &, const celix::Bundle &)> addWithOwner{};

                std::function<void(void *)> remove{};
                std::function<void(void *, const celix::Properties &)> removeWithProperties{};
                std::function<void(void *, const celix::Properties &, const celix::Bundle &)> removeWithOwner{};
            };

            std::mutex mutex{};
            std::map<long,std::unique_ptr<TrackEntry>> trackEntries{};
        };
    }
}


template<typename I>
long celix::BundleContext::registerService(I *svc, const std::string &serviceName, Properties props) noexcept {
    celix::ServiceRegistrationOptions<I> opts{*svc, serviceName};
    opts.properties = std::move(props);
    return this->registerServiceWithOptions(opts);
}

template<typename I>
long celix::BundleContext::registerCService(I *svc, const std::string &serviceName, Properties props) noexcept {
    static_assert(std::is_pod<I>::value, "Service I must be a 'Plain Old Data' object");
    celix::ServiceRegistrationOptions<I> opts{*svc, serviceName};
    opts.properties = std::move(props);
    opts.serviceLanguage = celix::Constants::SERVICE_C_LANG;
    return this->registerServiceWithOptions(opts);
}

template<typename I>
long celix::BundleContext::registerServiceWithOptions(const celix::ServiceRegistrationOptions<I>& opts) noexcept {
    celix_properties_t *c_props = celix_properties_create();
    for (auto &pair : opts.properties) {
        celix_properties_set(c_props, pair.first.c_str(), pair.second.c_str());
    }

    celix_service_registration_options_t cOpts; //TODO compile error gcc = CELIX_EMPTY_SERVICE_REGISTRATION_OPTIONS;
    std::memset(&cOpts, 0, sizeof(cOpts));

    cOpts.svc = static_cast<void*>(&opts.svc);
    cOpts.serviceName = opts.serviceName.c_str();
    cOpts.serviceVersion = opts.serviceVersion.c_str();
    cOpts.serviceLanguage = opts.serviceLanguage.c_str();
    cOpts.properties = c_props;
    return this->registerServiceInternal(cOpts);
}

template<typename I>
long celix::BundleContext::trackService(const std::string &serviceName, std::function<void(I *svc)> set) noexcept {
    return this->trackServiceInternal(serviceName, [set](void *voidSvc) {
        I* typedSvc = static_cast<I*>(voidSvc);
        set(typedSvc);
    });
}

template<typename I>
long celix::BundleContext::trackServices(const std::string &serviceName,
        std::function<void(I *svc)> add, std::function<void(I *svc)> remove) noexcept {
    auto voidAdd = [add](void *voidSvc) {
        I *typedSvc = static_cast<I *>(voidSvc);
        add(typedSvc);
    };
    auto voidRemove = [remove](void *voidSvc) {
        I *typedSvc = static_cast<I *>(voidSvc);
        remove(typedSvc);
    };
    return this->trackServicesInternal(serviceName, std::move(voidAdd), std::move(voidRemove));
}


#endif //CELIX_IMPL_BUNDLECONTEXTIMPL_H
