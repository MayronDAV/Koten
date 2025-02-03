function generate_build_info()
    local function read_changes_md(filename)
        local file = io.open(filename, "r")
        if not file then
            error("Error: Unable to open " .. filename)
        end

        local content = file:read("*all")
        file:close()

        local major, minor, patch = content:match("## (%d+)%.(%d+)%.(%d+)")

        if not major or not minor or not patch then
            error("Error: Unable to extract version from " .. filename)
        end

        local flavor = ""

        return major, minor, patch, flavor
    end

    local function generate_file(template_file, output_file, major, minor, patch, flavor)
        local file = io.open(template_file, "r")
        if not file then
            error("Error: Unable to open " .. template_file)
        end

        local content = file:read("*all")
        file:close()

        content = content:gsub("@major@", major)
                         :gsub("@minor@", minor)
                         :gsub("@patch@", patch)
                         :gsub("@flavor@", flavor)

        file = io.open(output_file, "w")
        if not file then
            error("Error: Unable to create " .. output_file)
        end

        file:write(content)
        file:close()

        print("File " .. output_file .. " generated successfully!")
    end

    local changes_md    = "glslang/CHANGES.md"
    local template_file = "glslang/build_info.h.tmpl"
    local output_file   = "glslang/glslang/build_info.h"

    local major, minor, patch, flavor = read_changes_md(changes_md)
    generate_file(template_file, output_file, major, minor, patch, flavor)
end
