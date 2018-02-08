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

/**
 * Test program for testing the etcdlib.
 * Prerequisite is that etcdlib is started on localhost on default port (2379)
 * tested with etcd 2.3.7
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "etcd.h"

#include <pthread.h>

int simplewritetest() {
    int res = 0;
    char*value = NULL;
    etcd_set("simplekey", "testvalue", 5, false);
    etcd_get("simplekey", &value, NULL);
    if (value && strcmp(value, "testvalue")) {
        printf("etcdlib test error: expected testvalue got %s\n", value);
        res = -1;
    }
    free(value);
    return res;
}

void* waitForChange(void*arg) {
    int *idx = (int*)arg;
    char *action = NULL;
    char *prevValue;
    char *value;
    char *rkey;
    long long modifiedIndex;
    printf("Watching for index %d\n", *idx);
    etcd_watch("hier/ar", *idx, &action, &prevValue, &value, &rkey, &modifiedIndex);
    printf(" New value from watch : [%s]%s => %s\n", rkey, prevValue, value);
    free (action);
    free(prevValue);
    free(rkey);
    free(value);
    *idx = modifiedIndex+1;
    etcd_watch("hier/ar", *idx, &action, &prevValue, &value, &rkey, &modifiedIndex);
    printf(" New value from watch : [%s]%s => %s\n", rkey, prevValue, value);
    free (action);
    free(prevValue);
    free(rkey);
    return value;
}

int waitforchangetest() {
    int res = 0;
    char*value = NULL;

    etcd_set("hier/ar/chi/cal", "testvalue1", 5, false);

    int index;
    etcd_get("hier/ar/chi/cal", &value, &index);
    free(value);
    pthread_t waitThread;
    index++;
    pthread_create(&waitThread, NULL, waitForChange, &index);
    sleep(1);
    etcd_set("hier/ar/chi/cal", "testvalue2", 5, false);
    sleep(1);
    etcd_set("hier/ar/chi/cal", "testvalue3", 5, false);
    void *resVal;
    pthread_join(waitThread, &resVal);
    if(strcmp((char*)resVal,"testvalue3" )) {
        printf("etcdtest::waitforchange1 expected testvalue3, got %s\n", (char*)resVal);
        res = -1;
    }
    free(resVal);
    return res;
}

int main (void) {
    etcd_init("localhost", 2379, 0);

    int res = simplewritetest(); if(res) return res; else printf("simplewrite test success\n");
    res = waitforchangetest(); if(res) return res;else printf("waitforchange1 test success\n");

    return 0;
}

