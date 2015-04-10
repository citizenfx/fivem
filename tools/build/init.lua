-- target game selection
dofile('init/game_select.lua')

-- platform-specific file ignores
dofile('init/platform_files.lua')

-- specific premake patches for POSIX build
dofile('init/posix_setup.lua')

-- files based on a root directory
dofile('init/project_files.lua')

-- component handling
dofile('components.lua')