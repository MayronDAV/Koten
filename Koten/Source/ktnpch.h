#if defined(_MSC_VER)
#pragma once
#endif

#include "Koten/Core/Base.h"
#include "Koten/Core/Log.h"
#include "Koten/Core/Assert.h"

// lib
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

// std
#include <iostream>
#include <memory>
#include <algorithm>
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <cstdint>
#ifdef KTN_WINDOWS
	#include <Windows.h>
#endif