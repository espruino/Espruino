/* Copyright (c) 2012, Nordic Semiconductor ASA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *   * Neither the name of Nordic Semiconductor ASA nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/** @brief Utilities for verifying program logic
 */

#ifndef APP_ASSERT_H_
#define APP_ASSERT_H_

#include <stdint.h>

/** @brief This function handles assertions.
 *
 *
 * @note
 * This function is called when an assertion has triggered.
 * 
 *
 * @param line_num The line number where the assertion is called
 * @param file_name Pointer to the file name
 */
void app_assert_callback(uint32_t line_num, const uint8_t *file_name);

/*lint -emacro(506, ASSERT) */ /* Suppress "Constant value Boolean */ 
/*lint -emacro(774, ASSERT) */ /* Suppress "Boolean within 'if' always evaluates to True" */ \
/** @brief Check intended for production code
 *
 * Check passes if "expr" evaluates to true. */
#define APP_ASSERT(expr) \
if (expr)                                                                     \
{                                                                             \
}                                                                             \
else                                                                          \
{                                                                             \
  app_assert_callback((uint32_t)__LINE__, (uint8_t *)__FILE__);        \
  /*lint -unreachable */                                                      \
}

#endif /* APP_ASSERT_H_ */
