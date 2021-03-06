// Copyright(c) 2018-2021, Intel Corporation
//
// Redistribution  and  use  in source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of  source code  must retain the  above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name  of Intel Corporation  nor the names of its contributors
//   may be used to  endorse or promote  products derived  from this  software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
// IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
// LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
// CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
// SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
// INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
// CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

extern "C" {

#include <json-c/json.h>
#include <uuid/uuid.h>

#include "fpgad/api/logging.h"

}

#include <config.h>
#include <opae/fpga.h>

#include <array>
#include <cstdlib>
#include <cstring>
#include "gtest/gtest.h"
#include "mock/test_system.h"

using namespace opae::testing;

class fpgad_log_c_p : public ::testing::TestWithParam<std::string> {
 protected:
  fpgad_log_c_p() {}

  virtual void SetUp() override {
    std::string platform_key = GetParam();
    ASSERT_TRUE(test_platform::exists(platform_key));
    platform_ = test_platform::get(platform_key);
    system_ = test_system::instance();
    system_->initialize();
    system_->prepare_syfs(platform_);
    strcpy(tmpfpgad_log_, "tmpfpgad-XXXXXX.log");
    close(mkstemps(tmpfpgad_log_, 4));
    EXPECT_GT(log_open(tmpfpgad_log_), 0);
  }

  virtual void TearDown() override {
    log_close();
    system_->finalize();

    if (!::testing::Test::HasFatalFailure() &&
        !::testing::Test::HasNonfatalFailure()) {
      unlink(tmpfpgad_log_);
    }
  }

  char tmpfpgad_log_[20];
  test_platform platform_;
  test_system *system_;
};

/**
 * @test       log01
 * @brief      Test: log_open, log_printf, log_close
 * @details    log_printf sends the formatted output string to the log file<br>
 *             and returns the number of bytes written.<br>
 */
TEST_P(fpgad_log_c_p, log01) {
  EXPECT_EQ(3, log_printf("abc"));
}

/**
 * @test       log02
 * @brief      Test: log_set, log_printf, log_close
 * @details    log_set allows changing the output file to stdout.<br>
 */
TEST_P(fpgad_log_c_p, log02) {
  ::testing::internal::CaptureStdout();

  log_set(stdout);
  EXPECT_EQ(3, log_printf("abc"));

  std::string captured = ::testing::internal::GetCapturedStdout();

  EXPECT_STREQ(captured.c_str(), "abc");
}

INSTANTIATE_TEST_CASE_P(fpgad_log_c, fpgad_log_c_p,
                        ::testing::ValuesIn(test_platform::platforms({ "skx-p" })));
