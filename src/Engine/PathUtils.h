#pragma once

#include <string>
#include <filesystem>

// windows路径是按照UTF16编码的
// imgui是按照UTF8解释的
// 因此需要这个转换
inline std::string PathToUtf8(const std::filesystem::path& path)
{
	const std::u8string text = path.generic_u8string();
	return std::string(text.begin(), text.end());
}