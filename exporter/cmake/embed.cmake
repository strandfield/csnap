
## https://stackoverflow.com/questions/11813271/embed-resources-eg-shader-code-images-into-executable-library-with-cmake

# Creates C resources file from files in given directory
function(create_resources dir name)
    set(output "${CMAKE_BINARY_DIR}/${name}.cpp")
    # Create empty output file
    file(WRITE ${output} "")
    # Start writing content
    file(APPEND ${output} "#include <filesystem>\n")
    file(APPEND ${output} "#include <string>\n")
    file(APPEND ${output} "namespace csnap { extern void copy_resource(const std::string& name, const void* data, size_t nbbytes, const std::filesystem::path& outdir); }\n")
    file(APPEND ${output} "extern \"C\"{\n")
    # Collect input files
    file(GLOB bins ${dir}/*)
    # Iterate through input files
    foreach(bin ${bins})
        # Get short filename
        string(REGEX MATCH "([^/]+)$" filename ${bin})
        # Replace filename spaces & extension separator for C compatibility
        string(REGEX REPLACE "\\.| |-" "_" fileidentifier ${filename})
        # Read hex data from file
        file(READ ${bin} filedata HEX)
        # Convert hex data for C compatibility
        string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," filedata ${filedata})
        string(REPEAT "0x[0-9|a-f]+," 20 replacepattern)
        string(REGEX REPLACE "(${replacepattern})" "\\1\n" filedata ${filedata})
        # Append data to output file
        file(APPEND ${output} "const unsigned char ${fileidentifier}[] = {\n${filedata}};\nconst size_t ${fileidentifier}_size = sizeof(${fileidentifier});\n")
    endforeach()
    file(APPEND ${output} "} // extern \"C\"\n")
    file(APPEND ${output} "void export_resources_${name}(const std::filesystem::path& outdir){\n")
    foreach(bin ${bins})
      string(REGEX MATCH "([^/]+)$" filename ${bin})
      string(REGEX REPLACE "\\.| |-" "_" fileidentifier ${filename})
      file(APPEND ${output} "csnap::copy_resource(\"${filename}\", ${fileidentifier}, ${fileidentifier}_size, outdir);\n")
    endforeach()
    file(APPEND ${output} "}\n")
    set(EMBED_RESOURCE_${name} "${CMAKE_BINARY_DIR}/${name}.cpp" PARENT_SCOPE)
endfunction()
