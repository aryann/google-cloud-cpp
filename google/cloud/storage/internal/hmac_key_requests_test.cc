// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "google/cloud/storage/internal/hmac_key_requests.h"
#include <gmock/gmock.h>

namespace google {
namespace cloud {
namespace storage {
inline namespace STORAGE_CLIENT_NS {
namespace internal {
namespace {

using ::testing::HasSubstr;
using ::testing::Not;

TEST(HmacKeyRequestsTest, ParseFailure) {
  auto actual = internal::HmacKeyMetadataParser::FromString("{123");
  EXPECT_FALSE(actual.ok());
}

TEST(HmacKeyRequestsTest, ParseEmpty) {
  auto actual = internal::HmacKeyMetadataParser::FromString("{}");
  EXPECT_TRUE(actual.ok());
}

TEST(HmacKeyRequestsTest, Create) {
  CreateHmacKeyRequest request("test-project-id", "test-service-account");
  EXPECT_EQ("test-project-id", request.project_id());
  EXPECT_EQ("test-service-account", request.service_account());

  std::ostringstream os;
  os << request;
  auto str = os.str();
  EXPECT_THAT(str, HasSubstr("CreateHmacKeyRequest"));
  EXPECT_THAT(str, HasSubstr("test-project-id"));
  EXPECT_THAT(str, HasSubstr("test-service-account"));
}

TEST(HmacKeyRequestsTest, ParseCreateResponse) {
  std::string const resource_text = R"""({
      "accessId": "test-access-id",
      "etag": "XYZ=",
      "id": "test-id-123",
      "kind": "storage#hmacKey",
      "projectId": "test-project-id",
      "serviceAccountEmail": "test-service-account-email",
      "state": "ACTIVE",
      "timeCreated": "2019-03-01T12:13:14Z"
})""";
  nl::json const json_object{
      {"kind", "storage#hmacKeyCreate"},
      // To generate the secret use:
      //   echo -n "test-secret" | openssl base64
      {"secretKey", "dGVzdC1zZWNyZXQ="},
      {"resource", nl::json::parse(resource_text)},
  };

  std::string const text = json_object.dump();

  auto actual =
      CreateHmacKeyResponse::FromHttpResponse(HttpResponse{200, text, {}})
          .value();
  EXPECT_EQ("dGVzdC1zZWNyZXQ=", actual.secret);
  auto expected_resource =
      HmacKeyMetadataParser::FromString(resource_text).value();
  EXPECT_THAT(expected_resource, actual.resource);
}

TEST(HmacKeyRequestsTest, ParseCreateResponseFailure) {
  std::string text = R"""({123)""";

  auto actual =
      CreateHmacKeyResponse::FromHttpResponse(HttpResponse{200, text, {}});
  EXPECT_FALSE(actual.ok());
}

TEST(HmacKeyRequestsTest, ParseCreateResponseFailureInResource) {
  std::string text = R"""({"resource": "invalid-resource" })""";

  auto actual =
      CreateHmacKeyResponse::FromHttpResponse(HttpResponse{200, text, {}});
  EXPECT_FALSE(actual.ok());
}

TEST(HmacKeyRequestsTest, CreateResponseIOStream) {
  std::string const text = R"""({
      "secret": "dGVzdC1zZWNyZXQ=",
      "resource": {
        "accessId": "test-access-id"
      }
})""";

  auto parsed =
      CreateHmacKeyResponse::FromHttpResponse(HttpResponse{200, text, {}})
          .value();

  std::ostringstream os;
  os << parsed;
  std::string actual = std::move(os).str();
  EXPECT_THAT(actual, HasSubstr("test-access-id"));
  // We do not want the secrets accidentally leaked to the log.
  EXPECT_THAT(actual, Not(HasSubstr("dGVzdC1zZWNyZXQ=")));
}

TEST(HmacKeysRequestsTest, List) {
  ListHmacKeysRequest request("test-project-id");
  request.set_multiple_options(ServiceAccountFilter("test-service-account"),
                               Deleted(true));
  EXPECT_EQ("test-project-id", request.project_id());

  std::ostringstream os;
  os << request;
  std::string actual = os.str();
  EXPECT_THAT(actual, HasSubstr("test-project-id"));
  EXPECT_THAT(actual, HasSubstr("serviceAccount=test-service-account"));
  EXPECT_THAT(actual, HasSubstr("deleted=true"));
}

TEST(HmacKeysRequestsTest, ParseListResponse) {
  std::string key1_text = R"""({
      "accessId": "test-access-id-1",
      "etag": "XYZ=",
      "id": "test-id-1",
      "kind": "storage#hmacKey",
      "projectId": "test-project-id",
      "serviceAccountEmail": "test-service-account-email",
      "state": "ACTIVE",
      "timeCreated": "2019-03-01T12:13:14Z"
})""";
  std::string key2_text = R"""({
      "accessId": "test-access-id-2",
      "etag": "XYZ=",
      "id": "test-id-2",
      "kind": "storage#hmacKey",
      "projectId": "test-project-id",
      "serviceAccountEmail": "test-service-account-email",
      "state": "ACTIVE",
      "timeCreated": "2019-03-02T12:13:14Z"
})""";
  std::string text = R"""({
      "kind": "storage#hmacKeys",
      "nextPageToken": "some-token-42",
      "items":
)""";
  text += "[" + key1_text + "," + key2_text + "]}";

  auto key1 = internal::HmacKeyMetadataParser::FromString(key1_text).value();
  auto key2 = internal::HmacKeyMetadataParser::FromString(key2_text).value();

  auto actual =
      ListHmacKeysResponse::FromHttpResponse(HttpResponse{200, text, {}})
          .value();
  EXPECT_EQ("some-token-42", actual.next_page_token);
  EXPECT_THAT(actual.items, ::testing::ElementsAre(key1, key2));
}

TEST(HmacKeysRequestsTest, ParseListResponseFailure) {
  std::string text = R"""({123)""";

  auto actual =
      ListHmacKeysResponse::FromHttpResponse(HttpResponse{200, text, {}});
  EXPECT_FALSE(actual.ok());
}

TEST(HmacKeysRequestsTest, ParseListResponseFailureInItems) {
  std::string text = R"""({"items": [ "invalid-item" ]})""";

  auto actual =
      ListHmacKeysResponse::FromHttpResponse(HttpResponse{200, text, {}});
  EXPECT_FALSE(actual.ok());
}

}  // namespace
}  // namespace internal
}  // namespace STORAGE_CLIENT_NS
}  // namespace storage
}  // namespace cloud
}  // namespace google