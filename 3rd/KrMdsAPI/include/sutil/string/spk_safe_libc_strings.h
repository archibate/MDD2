/*
 * Copyright 2009-2016 the original author or authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file    spk_safe_libc_strings.h
 *
 * 字符串处理的头文件 (对libc库函数包裹的安全方法)
 *
 * @version 0.17.6.1    2022/01/13
 * @since   0.17.6.1    2022/01/13
 */


#ifndef _SPK_SAFE_STRINGS_H
#define _SPK_SAFE_STRINGS_H


#include    <sutil/types.h>
#include    <sutil/compiler.h>
#include    <sutil/platform/spk_platforms.h>


#ifdef __cplusplus
extern "C" {
#endif


/* ===================================================================
 * 宏定义
 * =================================================================== */

/* 未定义NDEBUG, 替换原生库函数为带assert的版本 */
#if ! defined(NDEBUG)
# undef  snprintf
# undef  strncpy
# undef  strncat

# define snprintf                   SStr_SnprintfAssertable
# define strncpy                    SStr_StrncpyAssertable
# define strncat                    SStr_StrncatAssertable

/* 启用跨平台的安全版本 */
#elif defined(SPK_USE_SAFE_SNPRINTF)
# undef  snprintf
# define snprintf                   SStr_SnprintfSafe

/* 兼容windows平台 */
#elif defined(__WINDOWS__)
# undef  snprintf
# define snprintf                   SStr_SnprintfAssertable

#endif
/* -------------------------           */


/* ===================================================================
 * 函数声明
 * =================================================================== */

/* snprintf的包裹函数 (支持assert的跨平台版本) */
int32   SStr_SnprintfAssertable(char *pStr, int32 size, const char *fmt, ...);

/* snprintf的包裹函数 (支持参数校验的跨平台版本) */
int32   SStr_SnprintfSafe(char *pStr, int32 size, const char *fmt, ...);

/* strncpy的包裹函数 (支持assert的版本) */
char*   SStr_StrncpyAssertable(char *pDest, const char *pSrc, int32 n);

/* strncat的包裹函数 (支持assert的版本) */
char*   SStr_StrncatAssertable(char *pDest, const char *pSrc, int32 n);
/* -------------------------           */


#ifdef __cplusplus
}
#endif

#endif  /* _SPK_SAFE_STRINGS_H */
