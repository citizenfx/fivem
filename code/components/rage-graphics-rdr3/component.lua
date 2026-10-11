includedirs { 
    '../../../vendor/vulkan-headers/include/',
    '../rage-graphics-five/include'
}

return function()
    filter {}

    files {
        'components/rage-graphics-five/include/dxerr.h',
        'components/rage-graphics-five/src/dxerr.cpp',
        'components/rage-graphics-five/src/ReShadeFixups.cpp'
    }
end