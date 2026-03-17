#pragma once
#include <string>
#include <filesystem>

namespace GE {

class AssetBrowserPanel {
public:
    explicit AssetBrowserPanel(const std::string& assetsRoot = "assets");
    void OnImGuiRender();

private:
    void DrawDirectory(const std::filesystem::path& path);

    std::filesystem::path m_Root;
    std::filesystem::path m_Current;
};

} // namespace GE
