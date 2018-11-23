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

#include <memory.h>
#include "pubsub_nanomsg_common.h"

int psa_nanoMsg_localMsgTypeIdForMsgType(void *handle __attribute__((unused)), const char *msgType,
                                         unsigned int *msgTypeId) {
    *msgTypeId = utils_stringHash(msgType);
    return 0;
}

bool psa_nanomsg_checkVersion(version_pt msgVersion, const pubsub_nanmosg_msg_header_t *hdr) {
    bool check=false;
    int major=0,minor=0;

    if (msgVersion!=NULL) {
        version_getMajor(msgVersion,&major);
        version_getMinor(msgVersion,&minor);
        if(hdr->major==((unsigned char)major)){ /* Different major means incompatible */
            check = (hdr->minor>=((unsigned char)minor)); /* Compatible only if the provider has a minor equals or greater (means compatible update) */
        }
    }

    return check;
}

void psa_nanomsg_setScopeAndTopicFilter(const std::string &scope, const std::string &topic, char *filter) {
    for (int i = 0; i < 5; ++i) { // 5 ??
        filter[i] = '\0';
    }
    if (scope.size() >= 2)  { //3 ??
        filter[0] = scope[0];
        filter[1] = scope[1];
    }
    if (topic.size() >= 2)  { //3 ??
        filter[2] = topic[0];
        filter[3] = topic[1];
    }
}