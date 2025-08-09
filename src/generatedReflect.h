#pragma once

#include "config.h"
#include "Reflect.h"
#if XC || NE
#include <xele/XeleSecuritiesUserApiStruct.h>
#include "generatedReflectXele.inl"
#endif
#if OST
#include <UTApiStruct.h>
#include "generatedReflectUT.inl"
#endif
