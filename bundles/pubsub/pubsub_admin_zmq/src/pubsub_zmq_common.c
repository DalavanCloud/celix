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
#include "pubsub_zmq_common.h"

int psa_zmq_localMsgTypeIdForMsgType(void* handle __attribute__((unused)), const char* msgType, unsigned int* msgTypeId) {
    *msgTypeId = utils_stringHash(msgType);
    return 0;
}

bool psa_zmq_checkVersion(version_pt msgVersion, const pubsub_zmq_msg_header_t *hdr) {
    bool check=false;
    int major=0,minor=0;

    if (hdr->major == 0 && hdr->minor == 0) {
        //no check
        return true;
    }

    if (msgVersion!=NULL) {
        version_getMajor(msgVersion,&major);
        version_getMinor(msgVersion,&minor);
        if(hdr->major==((unsigned char)major)){ /* Different major means incompatible */
            check = (hdr->minor>=((unsigned char)minor)); /* Compatible only if the provider has a minor equals or greater (means compatible update) */
        }
    }

    return check;
}

void psa_zmq_setScopeAndTopicFilter(const char* scope, const char *topic, char *filter) {
    for (int i = 0; i < 5; ++i) {
        filter[i] = '\0';
    }
    if (scope != NULL && strnlen(scope, 3) >= 2)  {
        filter[0] = scope[0];
        filter[1] = scope[1];
    }
    if (topic != NULL && strnlen(topic, 3) >= 2)  {
        filter[2] = topic[0];
        filter[3] = topic[1];
    }
}


celix_status_t psa_zmq_decodeHeader(const unsigned char *data, size_t dataLen, pubsub_zmq_msg_header_t *header) {
    int status = CELIX_ILLEGAL_ARGUMENT;
    if (dataLen == 6) {
        header->type = ((data[0] << 24) | (data[1] << 16) | (data[2] << 8) | (data[3] << 0));
        header->major = (unsigned char) data[4];
        header->minor = (unsigned char) data[5];
        status = CELIX_SUCCESS;
    }
    return status;
}

celix_status_t psa_zmq_encodeHeader(const pubsub_zmq_msg_header_t *msgHeader, unsigned char *data, size_t dataLen) {
    int status = CELIX_ILLEGAL_ARGUMENT;
    if (dataLen == 6) {
        unsigned int tmp = msgHeader->type & 0xFFFFFFFF;
        data[0] = (unsigned char)((tmp >> 24) & 0xFF);
        data[1] = (unsigned char)((tmp >> 16) & 0xFF);
        data[2] = (unsigned char)((tmp >> 8) & 0xFF);
        data[3] = (unsigned char)((tmp >> 0) & 0xFF);
        data[4] = (char)msgHeader->major;
        data[5] = (char)msgHeader->minor;
        status = CELIX_SUCCESS;
    }
    return status;
}