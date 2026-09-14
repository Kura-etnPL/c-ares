/* MIT License
 *
 * Copyright (c) The c-ares project and its contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * SPDX-License-Identifier: MIT
 */

#include "ares-test.h"
#include "dns-proto.h"

namespace ares {
namespace test {

class SearchNoDataAsIsTest : public MockChannelOptsTest {
 public:
  SearchNoDataAsIsTest()
    : MockChannelOptsTest(1, AF_INET, false, false, nullptr, 0) {}
};

// Regression for #1268: once the as-is name exists but has no record of the
// requested type, search suffixes must not redirect the query to another name.
TEST_F(SearchNoDataAsIsTest, StopsBeforeSearchDomainsAfterAsIsNoData) {
  DNSPacket nodata;
  nodata.set_response().set_aa()
    .add_question(new DNSQuestion("www.example.com", T_A));

  EXPECT_CALL(server_, OnRequest("www.example.com", T_A))
    .WillOnce(SetReply(&server_, &nodata));
  EXPECT_CALL(server_, OnRequest("www.example.com.first.com", T_A)).Times(0);
  EXPECT_CALL(server_, OnRequest("www.example.com.second.org", T_A)).Times(0);
  EXPECT_CALL(server_, OnRequest("www.example.com.third.gov", T_A)).Times(0);

  SearchResult result;
  ares_search(channel_, "www.example.com", C_IN, T_A, SearchCallback, &result);
  Process();

  EXPECT_TRUE(result.done_);
  EXPECT_EQ(ARES_ENODATA, result.status_);
}

}  // namespace test
}  // namespace ares
