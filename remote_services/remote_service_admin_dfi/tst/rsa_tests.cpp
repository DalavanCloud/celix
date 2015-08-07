/*
 * Licensed under Apache License v2. See LICENSE for more information.
 */
#include <CppUTest/TestHarness.h>
#include <remote_constants.h>
#include <constants.h>
#include "CppUTest/CommandLineTestRunner.h"
#include "../../examples/calculator_service/public/include/calculator_service.h"

extern "C" {

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "launcher.h"
#include "framework.h"
#include "remote_service_admin.h"
#include "calculator_service.h"


    framework_pt framework = NULL;
    bundle_context_pt context = NULL;

    service_reference_pt rsaRef = NULL;
    remote_service_admin_service_pt rsa = NULL;

    service_reference_pt calcRef = NULL;
    calculator_service_pt calc = NULL;

    static void setupFm(void) {
        int rc = 0;

        rc = celixLauncher_launch("config.properties", &framework);
        CHECK_EQUAL(CELIX_SUCCESS, rc);

        bundle_pt bundle = NULL;
        rc = framework_getFrameworkBundle(framework, &bundle);
        CHECK_EQUAL(CELIX_SUCCESS, rc);

        rc = bundle_getContext(bundle, &context);
        CHECK_EQUAL(CELIX_SUCCESS, rc);

        rc = bundleContext_getServiceReference(context, (char *)OSGI_RSA_REMOTE_SERVICE_ADMIN, &rsaRef);
        CHECK_EQUAL(CELIX_SUCCESS, rc);
        CHECK(rsaRef != NULL);

        rc = bundleContext_getService(context, rsaRef, (void **)&rsa);
        CHECK_EQUAL(CELIX_SUCCESS, rc);

        rc = bundleContext_getServiceReference(context, (char *)CALCULATOR_SERVICE, &calcRef);
        CHECK_EQUAL(CELIX_SUCCESS, rc);
        CHECK(calcRef != NULL);

        rc = bundleContext_getService(context, calcRef, (void **)&calc);
        CHECK_EQUAL(CELIX_SUCCESS, rc);
    }

    static void teardownFm(void) {
        int rc = 0;
        rc = bundleContext_ungetService(context, rsaRef, NULL);
        CHECK_EQUAL(CELIX_SUCCESS, rc);

        rc = bundleContext_ungetService(context, calcRef, NULL);
        CHECK_EQUAL(CELIX_SUCCESS, rc);

        celixLauncher_stop(framework);
        celixLauncher_waitForShutdown(framework);
        celixLauncher_destroy(framework);

        rsaRef = NULL;
        rsa = NULL;
        calcRef = NULL;
        calc = NULL;
        context = NULL;
        framework = NULL;
    }

    static void testServices(void) {
        int rc = 0;
        array_list_pt exported = NULL;
        array_list_pt imported = NULL;
        arrayList_create(&exported);
        arrayList_create(&imported);

        rc = rsa->getExportedServices(rsa->admin, &exported);
        CHECK_EQUAL(CELIX_SUCCESS, rc);
        CHECK_EQUAL(0, arrayList_size(exported));

        rc = rsa->getImportedEndpoints(rsa->admin, &imported);
        CHECK_EQUAL(CELIX_SUCCESS, rc);
        CHECK_EQUAL(0, arrayList_size(imported));

        double result = 0;
        rc = calc->add(calc->calculator, 2.0, 5.0, &result);
        CHECK_EQUAL(CELIX_SUCCESS, rc);
        CHECK_EQUAL(7.0, result);
    }

    static void testExportService(void) {
        int rc = 0;
        char *calcId = NULL;
        array_list_pt regs = NULL;

        rc = arrayList_create(&regs);
        CHECK_EQUAL(CELIX_SUCCESS, rc);

        rc = serviceReference_getProperty(calcRef, (char *)"service.id", &calcId);
        CHECK_EQUAL(CELIX_SUCCESS, rc);

        rc = rsa->exportService(rsa->admin, calcId, NULL, &regs);
        CHECK_EQUAL(CELIX_SUCCESS, rc);

        CHECK_EQUAL(1, arrayList_size(regs));
    }

    static void testImportService(void) {
        int rc = 0;
        import_registration_pt reg = NULL;
        endpoint_description_pt endpoint = NULL;

        properties_pt props = properties_create();
        properties_set(props, (char *)OSGI_RSA_ENDPOINT_SERVICE_ID, (char *)"42");
        properties_set(props, (char *)OSGI_RSA_ENDPOINT_FRAMEWORK_UUID, (char *)"eec5404d-51d0-47ef-8d86-c825a8beda42");
        properties_set(props, (char *)OSGI_RSA_ENDPOINT_ID, (char *)"eec5404d-51d0-47ef-8d86-c825a8beda42-42");
        properties_set(props, (char *)OSGI_FRAMEWORK_OBJECTCLASS,(char *)"org.apache.celix.Example");

        rc = endpointDescription_create(props, &endpoint);
        CHECK_EQUAL(CELIX_SUCCESS, rc);

        rc = rsa->importService(rsa->admin, endpoint, &reg);
        CHECK_EQUAL(CELIX_SUCCESS, rc);
        CHECK(reg != NULL);

        service_reference_pt ref = NULL;
        rc = bundleContext_getServiceReference(context, (char *)"org.apache.celix.Example", &ref);
        CHECK_EQUAL(CELIX_SUCCESS, rc);
        CHECK(ref != NULL);

        /* Cannot test. uses requesting bundles descriptor
        void *service = NULL;
        rc = bundleContext_getService(context, ref, &service);
        CHECK_EQUAL(CELIX_SUCCESS, rc);
        CHECK(service != NULL);
         */
    }

    static void testBundles(void) {
        array_list_pt bundles = NULL;

        int rc = bundleContext_getBundles(context, &bundles);
        CHECK_EQUAL(0, rc);
        CHECK_EQUAL(3, arrayList_size(bundles)); //framework, rsa_dfi & calc

        arrayList_destroy(bundles);
    }

}


TEST_GROUP(RsaDfiTests) {
    void setup() {
        setupFm();
    }

    void teardown() {
        teardownFm();
    }
};

TEST(RsaDfiTests, InfoTest) {
    testServices();
}

TEST(RsaDfiTests, ExportService) {
    testExportService();
}

TEST(RsaDfiTests, ImportService) {
    testImportService();
}

TEST(RsaDfiTests, TestBundles) {
    testBundles();
}